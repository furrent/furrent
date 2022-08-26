import socket


def faker_friendly():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('127.0.0.1', 4001))
    sock.listen()

    while True:
        conn, _ = sock.accept()
        handshake = conn.recv(68)
        print(f"Handshake from {handshake[-20:].decode()}")
        # Accept any handshake and reply with some sort of peer id
        conn.send(handshake[:-20] + b"WhoLetTheDogsOut----")
        # Send a bitfield
        conn.send(b"\x00\x00\x00\x02\x05\x05")
        # Send an unchoke message
        conn.send(b"\x00\x00\x00\x01\x01")
