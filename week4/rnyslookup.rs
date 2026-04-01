use std::net::ToSocketAddrs;

fn main() {
    let hostname = "www.naver.com";

    // 포트가 필요해서 임시로 80을 붙여서 주소 해석
    let addr_iter = match (hostname, 80).to_socket_addrs() {
        Ok(iter) => iter,
        Err(e) => {
            eprintln!("Error: Unable to resolve hostname {}: {}", hostname, e);
            return;
        }
    };

    println!("Hostname: {}", hostname);

    let mut found = false;

    for addr in addr_iter {
        println!("IP Address: {}", addr.ip());
        found = true;
    }

    if !found {
        println!("No IP addresses found.");
    }
}