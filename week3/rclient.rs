use std::io::{self, Read, Write};
use std::net::TcpStream;

fn print_error(context: &str, err: &dyn std::error::Error) {
    eprintln!("[ERR] {}: {}", context, err);
}

fn main() {
    let mut stream = match TcpStream::connect("127.0.0.1:5000") {
        Ok(stream) => stream,
        Err(e) => {
            print_error("TcpStream::connect", &e);
            return;
        }
    };

    println!("connected to server: 127.0.0.1:5000");

    let mut buf = [0u8; 1024];

    loop {
        let mut input = String::new();

        if let Err(e) = io::stdin().read_line(&mut input) {
            print_error("stdin.read_line", &e);
            break;
        }

        if input.trim().eq_ignore_ascii_case("q") {
            break;
        }

        if let Err(e) = stream.write_all(input.as_bytes()) {
            print_error("write_all", &e);
            break;
        }

        let n = match stream.read(&mut buf) {
            Ok(n) => n,
            Err(e) => {
                print_error("read", &e);
                break;
            }
        };

        if n == 0 {
            println!("server closed connection");
            break;
        }

        print!("from server: {}", String::from_utf8_lossy(&buf[..n]));
    }
}