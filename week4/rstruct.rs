use std::net::{Ipv4Addr, SocketAddrV4};

fn main() {
    // 1. IPv4 주소 생성
    let ip = Ipv4Addr::new(127, 0, 0, 1);

    // 2. 포트 번호 설정
    let port: u16 = 8080;

    // 3. 소켓 주소 생성
    let server_addr = SocketAddrV4::new(ip, port);

    // 4. IP / Port 출력
    println!("server IP   : {}", server_addr.ip());
    println!("server Port : {}", server_addr.port());

    // 5. 메모리(네트워크 바이트 순서) 값 확인
    let network_port = port.to_be();

    println!("port value in memory : 0x{:04X}", network_port);
    println!("real port value      : 0x{:04X}", u16::from_be(network_port));
}