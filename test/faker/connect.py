import socket


# Accepts a connection from a peer then:
#  - Handshakes
#  - Sends bitfield
#  - Unchokes
def faker_connect():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('127.0.0.1', 4004))
    sock.listen()

    while True:
        conn, _ = sock.accept()
        handshake = conn.recv(68)
        print(f"Handshake from {handshake[-20:].decode()}")
        # Accept any handshake and reply with some sort of peer id
        conn.send(handshake[:-20] + b"WhoLetTheDogsOut----")
        # Send a bitfield indicating we have the first and only piece for a dummy torrent
        conn.send(b"\x00\x00\x00\x02\x05" + bytes([0b10000000]))
        # Send an unchoke message
        conn.send(b"\x00\x00\x00\x01\x01")

        # Unchoke
        assert conn.recv(5) == b"\x00\x00\x00\x01\x01"
        # Interested
        assert conn.recv(5) == b"\x00\x00\x00\x01\x02"
