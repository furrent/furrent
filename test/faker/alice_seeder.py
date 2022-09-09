import os
import socket
import struct
from os import path

FIXTURE_PATH = path.realpath(path.join(path.dirname(__file__), "../fixtures/alice.txt"))

# From the alice.torrent file
N_PIECES = 5
# From the alice.torrent file. This is 32KB so 2 block requests per piece.
PIECE_LENGTH = 32768


def read_fixture(piece_index, piece_offset, length):
    with open(FIXTURE_PATH, "rb") as f:
        f.seek(piece_index * PIECE_LENGTH + piece_offset, os.SEEK_SET)
        return f.read(length)


# A simplified BitTorrent client that seeds an "Alice in Wonderland" .txt ISO
def faker_alice():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('127.0.0.1', 4006))
    sock.listen()

    while True:
        conn, _ = sock.accept()
        handshake = conn.recv(68)

        try:
            print(f"Handshake from {handshake[-20:].decode()}")
        except UnicodeDecodeError:
            print("Handshake from <non utf8 peer id>")

        # Accept any handshake and reply with some sort of peer id
        conn.send(handshake[:-20] + b"WhoLetTheDogsOut----")
        # Send a bitfield (there are 5 pieces so that makes it just 1 byte)
        conn.send(b"\x00\x00\x00\x02\x05" + bytes([0b11111000]))
        # Send an unchoke message
        conn.send(b"\x00\x00\x00\x01\x01")

        # Unchoke
        assert conn.recv(5) == b"\x00\x00\x00\x01\x01"
        # Interested
        assert conn.recv(5) == b"\x00\x00\x00\x01\x02"

        pieces_left = N_PIECES
        while pieces_left > 0:
            request = conn.recv(17)
            # The length of a request message
            assert request[3] == 13
            # ID of a request message
            assert request[4] == 6

            # The index of the piece
            piece_index = struct.unpack(">i", request[5:9])[0]
            # What's the offset from the start of the piece
            piece_offset = struct.unpack(">i", request[9:13])[0]
            # How many bytes are requested
            length = struct.unpack(">i", request[13:])[0]

            payload = read_fixture(piece_index, piece_offset, length)

            message = bytes()
            message += struct.pack(">i", 9 + len(payload))
            message += b"\x07"
            message += struct.pack(">i", piece_index)
            message += struct.pack(">i", piece_offset)
            message += payload

            conn.send(message)
