import random
import socket


def random_16kb_block():
    return bytes([random.randint(0, 255) for _ in range(2 ** 14)])


# Accepts a connection from a peer then:
#  - Handshakes
#  - Sends bitfield
#  - Unchokes
def faker_connect_plus_piece():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('127.0.0.1', 4005))
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

        request = conn.recv(17)
        # The length of a request message
        assert request[3] == 13
        # ID of a request message
        assert request[4] == 6
        # The index of the piece
        assert request[8] == 0
        # The offset from the beginning of the piece
        assert request[12] == 0
        # The length of the requested block. Together, these two bytes (big-endian encoded) make 16KB == 16384
        assert request[15] == 0x40
        assert request[16] == 0x00

        conn.send(b"\x00\x00\x40\x09\x07\x00\x00\x00\x00\x00\x00\x00\x00" + random_16kb_block())
