use std::net::UdpSocket;

// Rust 기반 초간단 UDP 서버
fn main() -> std::io::Result<()> {
    // 1. 5000번 포트에 소켓 바인딩 (주소 할당)
    let socket = UdpSocket::bind("0.0.0.0:5000")?;
    println!("Rust UDP 서버가 5000 포트에서 대기 중...");

    let mut buf = [0; 1024];
    loop {
        // 2. 메시지 수신 (데이터 크기와 송신자 주소를 받음)
        let (amt, src) = socket.recv_from(&mut buf)?;
        
        // 받은 데이터를 문자열로 변환하여 출력
        let msg = std::str::from_utf8(&buf[..amt]).unwrap_or("알 수 없는 데이터");
        println!("클라이언트({}): {}", src, msg);

        // 3. 수신한 데이터를 그대로 다시 송신자에게 전송 (Echo)
        socket.send_to(&buf[..amt], &src)?;
    }
}
