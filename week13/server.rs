use std::io::{Read, Write};
use std::mem;
use std::net::{TcpListener, TcpStream, UdpSocket};
use std::sync::{Arc, Mutex};
use std::thread;

const MAX_PLAYERS: usize = 1000;
const ADDR: &str = "0.0.0.0:8000";

#[repr(C)]
#[derive(Clone, Copy)]
struct Packet {
    pkt_type: i32,      // 1:로그인 2:로그아웃 3:이동 4:공격 5:아이템
    player_id: i32,     // 서버가 로그인 응답으로 부여한 ID
    x: f32,
    y: f32,
    target_id: i32,
    username: [u8; 32],
    password: [u8; 32],
}

struct Player {
    x: f32,
    y: f32,
    hp: i32,
    is_active: bool,
    tcp_write: Option<Arc<Mutex<TcpStream>>>,
    udp_addr: Option<std::net::SocketAddr>,
    username: [u8; 32],
}

impl Default for Player {
    fn default() -> Self {
        Player {
            x: 0.0,
            y: 0.0,
            hp: 0,
            is_active: false,
            tcp_write: None,
            udp_addr: None,
            username: [0; 32],
        }
    }
}

type State = Arc<Mutex<Vec<Player>>>;

fn main() {
    let state: State = Arc::new(Mutex::new(
        (0..MAX_PLAYERS).map(|_| Player::default()).collect()
    ));
    let udp = Arc::new(UdpSocket::bind(ADDR).unwrap());
    let tcp = TcpListener::bind(ADDR).unwrap();

    println!("Server started. Port: 8000");

    {
        let state = state.clone();
        let udp = udp.clone();
        thread::spawn(move || {
            let mut buf = [0u8; mem::size_of::<Packet>()];
            loop {
                if let Ok((_, src)) = udp.recv_from(&mut buf) {
                    on_udp(&state, &udp, &from_bytes(&buf), src);
                }
            }
        });
    }

    for stream in tcp.incoming().flatten() {
        let state = state.clone();
        let udp = udp.clone();
        let write_half = Arc::new(Mutex::new(stream.try_clone().unwrap()));
        let mut read_half = stream;
        thread::spawn(move || {
            let mut my_id: Option<usize> = None;
            let mut buf = [0u8; mem::size_of::<Packet>()];
            loop {
                if read_half.read_exact(&mut buf).is_err() {
                    break;
                }
                let mut pkt = from_bytes(&buf);
                if !on_tcp(&state, &udp, &mut pkt, &write_half, &mut my_id) {
                    break;
                }
            }
            if let Some(pid) = my_id {
                free_player(&state, pid);
                println!("Logout (disconnect): {}", pid);
            }
        });
    }
}

fn as_bytes(pkt: &Packet) -> &[u8] {
    unsafe { std::slice::from_raw_parts(pkt as *const _ as *const u8, mem::size_of::<Packet>()) }
}

fn from_bytes(buf: &[u8]) -> Packet {
    unsafe { std::ptr::read(buf.as_ptr() as *const Packet) }
}

fn str_from(name: &[u8; 32]) -> &str {
    std::str::from_utf8(name).unwrap_or("?").trim_end_matches('\0')
}

fn alloc_player(state: &State, write: Arc<Mutex<TcpStream>>, username: [u8; 32]) -> Option<usize> {
    let mut g = state.lock().unwrap();
    for (i, p) in g.iter_mut().enumerate() {
        if !p.is_active {
            p.is_active = true;
            p.hp        = 100;
            p.tcp_write = Some(write);
            p.username  = username;
            return Some(i);
        }
    }
    None
}

fn free_player(state: &State, id: usize) {
    let mut g = state.lock().unwrap();
    g[id].is_active = false;
    g[id].tcp_write = None;
}

fn broadcast_tcp(state: &State, pkt: &Packet) {
    let g = state.lock().unwrap();
    for p in g.iter() {
        if p.is_active {
            if let Some(w) = &p.tcp_write {
                w.lock().unwrap().write_all(as_bytes(pkt)).ok();
            }
        }
    }
}

fn broadcast_udp(state: &State, udp: &UdpSocket, pkt: &Packet) {
    let g = state.lock().unwrap();
    for p in g.iter() {
        if p.is_active {
            if let Some(addr) = p.udp_addr {
                udp.send_to(as_bytes(pkt), addr).ok();
            }
        }
    }
}

fn on_tcp(state: &State, udp: &UdpSocket, pkt: &mut Packet, write: &Arc<Mutex<TcpStream>>, my_id: &mut Option<usize>) -> bool {
    let pid = pkt.player_id as usize;
    match pkt.pkt_type {
        1 => { // 로그인: 서버가 ID 부여 후 응답
            if let Some(id) = alloc_player(state, write.clone(), pkt.username) {
                *my_id = Some(id);
                pkt.player_id = id as i32;
                write.lock().unwrap().write_all(as_bytes(pkt)).ok();
                println!("Login: {} ({})", id, str_from(&pkt.username));
                broadcast_tcp(state, pkt);
            }
        }
        2 => { // 로그아웃
            if pid < MAX_PLAYERS {
                free_player(state, pid);
                println!("Logout: {}", pid);
                broadcast_tcp(state, pkt);
                my_id.take();
            }
            return false;
        }
        4 => {
            println!("Attack: {} -> {}", pid, pkt.target_id);
            broadcast_tcp(state, pkt);
        }
        5 => {
            println!("Item: {} used item", pid);
            broadcast_tcp(state, pkt);
        }
        _ => {}
    }
    true
}

fn on_udp(state: &State, udp: &UdpSocket, pkt: &Packet, src: std::net::SocketAddr) {
    let pid = pkt.player_id as usize;
    if pid >= MAX_PLAYERS {
        return;
    }
    {
        let mut g = state.lock().unwrap();
        if !g[pid].is_active {
            return;
        }
        g[pid].x        = pkt.x;
        g[pid].y        = pkt.y;
        g[pid].udp_addr = Some(src);
    }
    println!("Move: {} to ({:.1}, {:.1})", pid, pkt.x, pkt.y);
    broadcast_udp(state, udp, pkt);
}
