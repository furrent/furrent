import socket


def a_16kb_block():
    # This is known to have SHA1 hash equal to 19e5dc34eda79ca3ee73860b61b66185149b6fb4
    return bytes([1 for _ in range(2 ** 14)])


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
        # Send a bitfield indicating we have the first and only piece for a dummy torrent
        conn.send(b"\x00\x00\x00\x02\x05" + bytes([0b10000000]))
        # Send an unchoke message
        conn.send(b"\x00\x00\x00\x01\x01")

        # Unchoke
        assert conn.recv(5) == b"\x00\x00\x00\x01\x01"
        # Interested
        assert conn.recv(5) == b"\x00\x00\x00\x01\x02"

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

        conn.send(b"\x00\x00\x40\x09\x07\x00\x00\x00\x00\x00\x00\x00\x00" + a_16kb_block())
