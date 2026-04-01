use std::io::{self,Read,Write};
use std::net::TcpStream;

fn main(){
    let mut stream = TcpStream::connect("127.0.0.1:5000").unwrap();
    let mut buf = [0; 1024];

    loop {
        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();
        if input.trim().to_lowercase() == "q"{break;}

        //서버로 보냄
        stream.write_all(input.as_bytes()).unwrap(); 
        //읽기
        let n = stream.read(&mut buf).unwrap();
        if n == 0 {break;}
        //출력
        print!("Server: {}", String::from_utf8_lossy(&buf[..n]));
    }
}