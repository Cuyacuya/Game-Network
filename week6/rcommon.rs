
// DrawingMessage 구조체 (Rust 공통 모듈)
pub const PORT: u16 = 5000;

#[derive(Debug)]
pub struct DrawingMessage {
pub x1: i32,
pub y1: i32,
pub x2: i32,
pub y2: i32,
pub width: i32,
pub color: i32,
}

// 에러 메시지 출력 함수 (공통)
pub fn print_error(msg: &str) {
eprintln!("에러 발생: {}", msg);
}