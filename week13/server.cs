using System;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)] //전부다 영향을 줌 but class한테는 영향을 안 줌.
struct Packet {
    public int Type;        // 1:로그인 2:로그아웃 3:이동 4:공격 5:아이템
    public int PlayerId;    // 서버가 로그인 응답으로 부여한 ID
    public float X, Y;
    public int TargetId;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string Username;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string Password;
}

class Player {
    public float X, Y;
    public int Hp;
    public bool IsActive;
    public TcpClient? Tcp;
    public IPEndPoint? UdpEP;
    public string Username = "";
}

class Server {
    const int MaxPlayers = 1000;
    const int Port = 8000;

    static Player[] players = new Player[MaxPlayers];
    static object stateLock = new object();

    static async Task Main() {
        for (int i = 0; i < MaxPlayers; i++)
            players[i] = new Player();

        Console.WriteLine($"Server started. Port: {Port}");
        var tcpListener = new TcpListener(IPAddress.Any, Port);
        tcpListener.Start();
        var udpClient = new UdpClient(Port);

        _ = HandleUdpAsync(udpClient);
        while (true) {
            var client = await tcpListener.AcceptTcpClientAsync();
            _ = HandleTcpAsync(client, udpClient);
        }
    }

    static async Task HandleUdpAsync(UdpClient udp) {
        while (true) {
            var result = await udp.ReceiveAsync();
            OnUdp(udp, FromBytes(result.Buffer), result.RemoteEndPoint);
        }
    }

    static async Task HandleTcpAsync(TcpClient tcp, UdpClient udp) {
        int myId = -1;
        var stream = tcp.GetStream();
        byte[] buf = new byte[Marshal.SizeOf<Packet>()];
        try {
            while (true) {
                int read = 0;
                while (read < buf.Length) {
                    int n = await stream.ReadAsync(buf, read, buf.Length - read);
                    if (n == 0) return;
                    read += n;
                }
                myId = OnTcp(FromBytes(buf), stream, tcp, udp, myId);
                if (myId == -2) return;
            }
        } finally {
            if (myId >= 0) {
                FreePlayer(myId);
                Console.WriteLine($"Logout (disconnect): {myId}");
            }
        }
    }

    static byte[] ToBytes(Packet pkt) {
        int size = Marshal.SizeOf<Packet>();
        byte[] buf = new byte[size];
        IntPtr ptr = Marshal.AllocHGlobal(size);
        Marshal.StructureToPtr(pkt, ptr, false);
        Marshal.Copy(ptr, buf, 0, size);
        Marshal.FreeHGlobal(ptr);
        return buf;
    }

    static Packet FromBytes(byte[] buf) {
        IntPtr ptr = Marshal.AllocHGlobal(buf.Length);
        Marshal.Copy(buf, 0, ptr, buf.Length);
        var pkt = Marshal.PtrToStructure<Packet>(ptr);
        Marshal.FreeHGlobal(ptr);
        return pkt;
    }

    static int AllocPlayer(TcpClient tcp, string username) {
        lock (stateLock) {
            for (int i = 0; i < MaxPlayers; i++) {
                if (!players[i].IsActive) {
                    players[i].IsActive  = true;
                    players[i].Hp        = 100;
                    players[i].Tcp       = tcp;
                    players[i].Username  = username;
                    return i;
                }
            }
        }
        return -1;
    }

    static void FreePlayer(int id) {
        lock (stateLock) {
            players[id].IsActive = false;
            players[id].Tcp      = null;
        }
    }

    static void BroadcastTcp(Packet pkt) {
        byte[] buf = ToBytes(pkt);
        lock (stateLock) {
            foreach (var p in players)
                if (p.IsActive)
                    try { p.Tcp?.GetStream().Write(buf, 0, buf.Length); } catch { }
        }
    }

    static void BroadcastUdp(UdpClient udp, Packet pkt) {
        byte[] buf = ToBytes(pkt);
        lock (stateLock) {
            foreach (var p in players)
                if (p.IsActive && p.UdpEP != null)
                    udp.Send(buf, buf.Length, p.UdpEP);
        }
    }

    // returns new myId; -2 = logout
    static int OnTcp(Packet pkt, NetworkStream stream, TcpClient tcp, UdpClient udp, int myId) {
        int pid = pkt.PlayerId;
        switch (pkt.Type) {
            case 1: { // 로그인: 서버가 ID 부여 후 응답
                int newId = AllocPlayer(tcp, pkt.Username ?? "");
                if (newId < 0) return myId;
                pkt.PlayerId = newId;
                byte[] resp = ToBytes(pkt);
                stream.Write(resp, 0, resp.Length);
                Console.WriteLine($"Login: {newId} ({pkt.Username})");
                BroadcastTcp(pkt);
                return newId;
            }
            case 2: // 로그아웃
                if (pid >= 0 && pid < MaxPlayers) {
                    FreePlayer(pid);
                    Console.WriteLine($"Logout: {pid}");
                    BroadcastTcp(pkt);
                }
                return -2;
            case 4:
                Console.WriteLine($"Attack: {pid} -> {pkt.TargetId}");
                BroadcastTcp(pkt);
                break;
            case 5:
                Console.WriteLine($"Item: {pid} used item");
                BroadcastTcp(pkt);
                break;
        }
        return myId;
    }

    static void OnUdp(UdpClient udp, Packet pkt, IPEndPoint ep) {
        int pid = pkt.PlayerId;
        if (pid < 0 || pid >= MaxPlayers) return;
        lock (stateLock) {
            if (!players[pid].IsActive) return;
            players[pid].X    = pkt.X;
            players[pid].Y    = pkt.Y;
            players[pid].UdpEP = ep;
        }
        Console.WriteLine($"Move: {pid} to ({pkt.X:F1}, {pkt.Y:F1})");
        BroadcastUdp(udp, pkt);
    }
}
