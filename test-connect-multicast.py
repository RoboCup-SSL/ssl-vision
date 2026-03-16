import socket
import struct
import threading

MCAST_GRP = '224.5.23.2'
PORTS = [10005, 10006]
BUFFER_SIZE = 1024  # Adjust as needed

def listen_multicast(port):
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

    # Allow multiple sockets to use the same PORT number
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        # Bind to the port on all interfaces
        sock.bind(('', port))
    except Exception as e:
        print(f"[Port {port}] Bind failed: {e}")
        return

    # Join the multicast group on all interfaces
    mreq = struct.pack("4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    print(f"[Port {port}] Listening for multicast messages on {MCAST_GRP}:{port}")

    # Receive loop
    while True:
        try:
            data, addr = sock.recvfrom(BUFFER_SIZE)
            print(f"[Port {port}] Received from {addr}: {data.decode(errors='ignore')}")
        except Exception as e:
            print(f"[Port {port}] Error receiving data: {e}")
            break

def main():
    threads = []

    # Start a listener thread for each port
    for port in PORTS:
        t = threading.Thread(target=listen_multicast, args=(port,), daemon=True)
        t.start()
        threads.append(t)

    # Keep the main thread alive
    try:
        while True:
            pass
    except KeyboardInterrupt:
        print("\nExiting...")

if __name__ == "__main__":
    main()

