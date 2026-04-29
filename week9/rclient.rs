use std::net::UdpSocket;
use std::io::{self, Write};

// Rust 기반 초간단 UDP 클라이언트
fn main() -> std::io::Result<()> {
    // 1. 클라이언트 소켓 생성 (운영체제가 남는 포트를 자동으로 할당하도록 "0.0.0.0:0" 사용)
    let socket = UdpSocket::bind("0.0.0.0:0")?;
    let server_addr = "127.0.0.1:5000";

    println!("메시지를 입력하세요 (종료: Ctrl+C)");

    let mut input = String::new();
    let mut buf = [0; 1024];

    loop {
        print!("입력: ");
        io::stdout().flush()?; // "입력: " 글자를 즉시 화면에 출력
        input.clear();
        io::stdin().read_line(&mut input)?; // 사용자 키보드 입력 받기

        let msg = input.trim();
        if msg.is_empty() { continue; }

        // 2. 서버로 메시지 전송
        socket.send_to(msg.as_bytes(), server_addr)?;

        // 3. 서버로부터 에코 응답 수신
        let (amt, _) = socket.recv_from(&mut buf)?;
        let response = std::str::from_utf8(&buf[..amt]).unwrap_or("알 수 없는 응답");
        println!("서버 응답: {}", response);
    }
}
