use libc;
use std::io;
fn main() {
// epoll_create1 호출 (0은 기본 플래그)
let epoll_fd = unsafe { libc::epoll_create1(0) };
if epoll_fd == -1 {
let err = io::Error::last_os_error();
eprintln!("epoll_create1 생성 실패: {}", err);
return;
}
println!("Rust에서 생성된 Epoll 파일 디스크립터: {}", epoll_fd);
// 사용이 끝난 디스크립터 닫기
unsafe { libc::close(epoll_fd) };
}