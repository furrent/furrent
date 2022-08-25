import socket


# Reads 10 bytes from the socket and writes them back
def faker_echo10():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('127.0.0.1', 4002))
    sock.listen()

    while True:
        conn, _ = sock.accept()
        msg = conn.recv(10)
        conn.send(msg)
