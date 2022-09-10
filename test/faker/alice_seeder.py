import os
import random
import socket
import struct
import time
from os import path

FIXTURE_PATH = path.realpath(path.join(path.dirname(__file__), "../fixtures/alice.txt"))

# From the alice.torrent file
N_PIECES = 5
# From the alice.torrent file. This is 32KB so 2 block requests per piece.
PIECE_LENGTH = 32768


# Check if the bitfield has a certain bit set
def bitfield_has(bitfield, i):
    return (bitfield & (1 << (8 - i - 1))) != 0


# Check if a bitfield is completely set to 1s
def bitfield_complete(bitfield):
    for i in range(N_PIECES):
        if not bitfield_has(bitfield, i):
            return False
    return True


# Randomly generate a bitfield
def random_bitfield_fresh():
    # This is simplified and only works with a one-byte bitfield
    bitfield = 0x00
    for i in range(N_PIECES):
        # Randomly set a bit with a 30% percent chance
        if random.random() < 0.3:
            bitfield |= 1 << (8 - i - 1)

    # This is an edge case, but we cannot let the bitfield be empty just because
    # this faker adds sets a new (missing) bit in the bitfield with each Request message.
    # This won't ever happen unless we have at least one piece, making the test failing while
    # Furrent might still be correct.
    if bitfield == 0x00:
        bitfield = 0b00001000

    return bitfield


def random_bitfield_update(bitfield):
    # Keep picking an index until you find a bit that can be flipped
    # When this function is called then a new bit MUST change
    while True:
        i = random.randint(0, N_PIECES - 1)
        new_bitfield = bitfield | (1 << (8 - i - 1))
        if new_bitfield != bitfield:
            return new_bitfield, i


def read_fixture(piece_index, piece_offset, length):
    with open(FIXTURE_PATH, "rb") as f:
        f.seek(piece_index * PIECE_LENGTH + piece_offset, os.SEEK_SET)
        return f.read(length)


def handle(conn):
    # Sometimes drop the socket before handshaking
    if random.random() < 0.0:
        conn.close()
        return

    handshake = conn.recv(68)

    try:
        print(f"Handshake from {handshake[-20:].decode()}")
    except UnicodeDecodeError:
        print("Handshake from <non utf8 peer id>")

    # Sometimes send the wrong info hash
    if random.random() < 0.05:
        handshake = bytearray(handshake)
        # Byte at index 30 is well withing the info-hash boundaries
        handshake[30] = 0
        handshake = bytes(handshake)

    # Accept any handshake and reply with some sort of peer id
    conn.send(handshake[:-20] + b"WhoLetTheDogsOut----")

    # Somtimes drop the connection after handshaking
    if random.random() < 0.05:
        conn.close()
        return

    bitfield = random_bitfield_fresh()
    print(f"Bitfield is {bitfield:08b}")

    # Sometimes, don't send a bitfield
    if not random.random() < 0.15:
        print("Sending bitfield")
        # Send a bitfield (there are 5 pieces so that makes it just 1 byte)
        conn.send(b"\x00\x00\x00\x02\x05" + struct.pack("B", bitfield))

    # Sometimes don't unchoke
    if not random.random() < 0.15:
        # Sometimes send a bad unchoke (unexpected payload)
        if random.random() < 0.3:
            print("Sending bad unchoke")
            conn.send(b"\x00\x00\x00\x02\x01\x05")
        else:
            # Send an unchoke message
            print("Sending unchoke")
            conn.send(b"\x00\x00\x00\x01\x01")

    # We should expect the peer to have dropped the connection by now, in the case that we did not send a bitfield

    maybe_unchoke = conn.recv(5)
    if not maybe_unchoke:
        print("EOF")
        return
    assert maybe_unchoke == b"\x00\x00\x00\x01\x01"

    maybe_interested = conn.recv(5)
    if not maybe_interested:
        print("EOF")
        return
    assert maybe_interested == b"\x00\x00\x00\x01\x02"

    while True:
        # Somtimes drop the connection between a message and the next
        if random.random() < 0.02:
            conn.close()
            return

        request = conn.recv(17)
        if not request:
            # This is an EOF
            print("EOF")
            break
        # The length of a request message
        assert request[3] == 13
        # ID of a request message
        assert request[4] == 6

        # The index of the piece
        piece_index = struct.unpack(">i", request[5:9])[0]

        # Check that the peer did not dare ask for a piece we don't have
        assert bitfield_has(bitfield, piece_index)

        # What's the offset from the start of the piece
        piece_offset = struct.unpack(">i", request[9:13])[0]
        # How many bytes are requested
        length = struct.unpack(">i", request[13:])[0]

        payload = read_fixture(piece_index, piece_offset, length)

        # Sometimes send a corrupt block
        if random.random() < 0.1:
            payload = bytearray(payload)
            # Set a random byte to a random value
            payload[random.randint(0, length - 1)] = random.randint(0, 255)
            payload = bytes(payload)

        message = bytes()
        message += struct.pack(">i", 9 + len(payload))
        message += b"\x07"
        message += struct.pack(">i", piece_index)
        message += struct.pack(">i", piece_offset)
        message += payload

        conn.send(message)

        if not bitfield_complete(bitfield):
            # Let's say we have acquired a new piece
            bitfield, new_piece_i = random_bitfield_update(bitfield)
            print(f"Now also have piece {new_piece_i}")

            # Send a Have message, but sometimes make the payload the wrong length
            if random.random() < 0.05:
                print("Sending bad message")
                conn.send(b"\x00\x00\x00\x06\x04" + struct.pack(">i", new_piece_i) + b"\x00")
            else:
                conn.send(b"\x00\x00\x00\x05\x04" + struct.pack(">i", new_piece_i))

        # Randomly send a KeepAlive, just because we can
        if random.random() < 0.2:
            conn.send(b"\x00\x00\x00\x00")

        # Randomly send an Interested, just because we can
        if random.random() < 0.2:
            conn.send(b"\x00\x00\x00\x01\x02")

        # Randomly send a NotInterested, just because we can
        if random.random() < 0.2:
            conn.send(b"\x00\x00\x00\x01\x03")

        # Randomly choke the peer with a 3% chance
        if random.random() < 0.03:
            # Choke
            conn.send(b"\x00\x00\x00\x01\x00")

            # Hang forever with a 30% chance
            if random.random() < 0.3:
                conn.detach()
                return
            else:
                time.sleep(1)
                # Unchoke
                conn.send(b"\x00\x00\x00\x01\x01")


# A simplified BitTorrent client that seeds an "Alice in Wonderland" .txt ISO
def faker_alice():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('127.0.0.1', 4006))
    sock.listen()

    while True:
        conn, _ = sock.accept()
        try:
            handle(conn)
        except ConnectionResetError:
            print("Connection reset")
        except BrokenPipeError:
            print("Broken pipe")
