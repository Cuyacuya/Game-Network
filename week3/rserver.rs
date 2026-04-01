use std::io::{Read, Write};
use std::net::{TcpListener, TcpStream};

fn print_error(context: &str, err: &dyn std::error::Error) {
    eprintln!("[ERR] {}: {}", context, err);
}

fn handle_client(mut stream: TcpStream) {
    let mut buf = [0u8; 1024];

    loop {
        let n = match stream.read(&mut buf) {
            Ok(n) => n,
            Err(e) => {
                print_error("read", &e);
                return;
            }
        };

        if n == 0 {
            println!("client disconnected");
            return;
        }

        let msg = String::from_utf8_lossy(&buf[..n]);
        println!("from client: {}", msg.trim_end());

        if let Err(e) = stream.write_all(&buf[..n]) {
            print_error("write_all", &e);
            return;
        }
    }
}

fn main() {
    let listener = match TcpListener::bind("0.0.0.0:5000") {
        Ok(listener) => listener,
        Err(e) => {
            print_error("TcpListener::bind", &e);
            return;
        }
    };

    println!("server listening on 0.0.0.0:5000");

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                println!("client connected");
                handle_client(stream);
            }
            Err(e) => {
                print_error("incoming", &e);
            }
        }
    }
}