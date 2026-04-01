use std::io::{Read,Write};
use std::net::TcpListener;

fn main(){
    let listener = TcpListener::bind("0.0.0.0:5000").unwrap();
    let (mut stream, _) = listener.accept().unwrap();
    let mut buf = [0; 1024];

    loop {
        let n = stream.read(&mut buf).unwrap();
        if n == 0 {break;}
        stream.write_all(&buf[..n]).unwrap();
    }
}