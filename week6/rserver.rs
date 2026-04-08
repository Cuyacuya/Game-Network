use std::io::Read;
use std::net::{TcpListener, TcpStream};

mod rcommon;
use rcommon::{DrawingMessage, print_error, PORT};

fn handle_client(mut stream: TcpStream) {
let mut buffer = [0; 24];
match stream.read_exact(&mut buffer) {
Ok(_) => {
let msg = DrawingMessage {
x1: i32::from_be_bytes(buffer[0..4].try_into().unwrap()),
y1: i32::from_be_bytes(buffer[4..8].try_into().unwrap()),
x2: i32::from_be_bytes(buffer[8..12].try_into().unwrap()),
y2: i32::from_be_bytes(buffer[12..16].try_into().unwrap()),
width: i32::from_be_bytes(buffer[16..20].try_into().unwrap()),
color: i32::from_be_bytes(buffer[20..24].try_into().unwrap()),
};
println!("std 서버 수신: {:?}", msg);
}
Err(e) => print_error(&format!("데이터 수신 오류: {}", e)),
}
}

fn main() -> std::io::Result<()> {
let addr = format!("127.0.0.1:{}", PORT);
let listener = TcpListener::bind(&addr)?;
println!("Rust (std) 서버가 {}에서 대기 중...", addr);

for stream in listener.incoming() {
match stream {
Ok(stream) => handle_client(stream),
Err(e) => print_error(&format!("연결 수락 오류: {}", e)),
}
}
Ok(())
}