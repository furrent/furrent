#!/usr/bin/env python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('127.0.0.1', 4242))
sock.listen()

while True:
    conn, _ = sock.accept()
    handshake = conn.recv(68)
    print(f"Handshake from {handshake[-20:].decode()}")
    # Accept any handshake and reply with some sort of peer id
    conn.send(handshake[:-20] + b"WhoLetTheDogsOut----")
