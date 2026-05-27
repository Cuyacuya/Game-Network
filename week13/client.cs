using System;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
struct Packet {
    public int Type;
    public int PlayerId;
    public float X, Y;
    public int TargetId;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string Username;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string Password;
}

class Client {
    const string ServerIp = "127.0.0.1";
    const int Port = 8000;

    static async Task Main() {
        var tcp = new TcpClient();
        tcp.Connect(ServerIp, Port);
        var stream = tcp.GetStream();
        var udp = new UdpClient();

        var rng = new Random();
        string username = $"user{rng.Next(10000)}";
        string password = "1234";

        // 로그인 (TCP) → 서버 응답으로 player_id 획득
        stream.Write(ToBytes(new Packet { Type = 1, Username = username, Password = password }));
        Console.WriteLine($"Sent: Login ({username})");
        var loginResp = await RecvTcp(stream);
        int myId = loginResp.PlayerId;
        Console.WriteLine($"Assigned player_id: {myId} ({loginResp.Username})");

        _ = PrintLoop(stream);
        Thread.Sleep(500);

        // 이동 (UDP)
        udp.Send(ToBytes(new Packet { Type = 3, PlayerId = myId, X = 10.5f, Y = 20.3f }),
                 Marshal.SizeOf<Packet>(), ServerIp, Port);
        Console.WriteLine("Sent: Move (UDP)");
        Thread.Sleep(500);

        // 공격 (TCP)
        stream.Write(ToBytes(new Packet { Type = 4, PlayerId = myId, TargetId = 1 }));
        Console.WriteLine("Sent: Attack -> 1");
        Thread.Sleep(500);

        // 아이템 (TCP)
        stream.Write(ToBytes(new Packet { Type = 5, PlayerId = myId }));
        Console.WriteLine("Sent: Item use");
        Thread.Sleep(500);

        // 로그아웃 (TCP)
        stream.Write(ToBytes(new Packet { Type = 2, PlayerId = myId }));
        Console.WriteLine("Sent: Logout");
        Thread.Sleep(500);

        tcp.Close();
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

    static async Task<Packet> RecvTcp(NetworkStream stream) {
        byte[] buf = new byte[Marshal.SizeOf<Packet>()];
        int read = 0;
        while (read < buf.Length) {
            int n = await stream.ReadAsync(buf, read, buf.Length - read);
            if (n == 0) throw new Exception("disconnected");
            read += n;
        }
        var pkt = FromBytes(buf);
        Console.WriteLine($"Recv: type={pkt.Type} player={pkt.PlayerId} ({pkt.Username}) ({pkt.X:F1},{pkt.Y:F1})");
        return pkt;
    }

    static async Task PrintLoop(NetworkStream stream) {
        try {
            while (true)
                await RecvTcp(stream);
        } catch { }
    }
}
