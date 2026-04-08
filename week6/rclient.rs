use std::io::Write;
use std::net::TcpStream;

mod rcommon;
use rcommon::{DrawingMessage, print_error, PORT};

fn main() -> std::io::Result<()> {
let addr = format!("127.0.0.1:{}", PORT);
match TcpStream::connect(&addr) {
Ok(mut stream) => {
println!("서버({})에 연결 성공!", addr);
let msg = DrawingMessage {
x1: 200, y1: 200, x2: 300, y2: 400, width: 10, color: 0x0000FF,
};

let mut buffer = Vec::new();
buffer.extend_from_slice(&msg.x1.to_be_bytes());
buffer.extend_from_slice(&msg.y1.to_be_bytes());
buffer.extend_from_slice(&msg.x2.to_be_bytes());
buffer.extend_from_slice(&msg.y2.to_be_bytes());
buffer.extend_from_slice(&msg.width.to_be_bytes());
buffer.extend_from_slice(&msg.color.to_be_bytes());

stream.write_all(&buffer)?;
println!("데이터 전송 완료.");
}
Err(e) => print_error(&format!("서버 연결 실패: {}", e)),
}
Ok(())
}