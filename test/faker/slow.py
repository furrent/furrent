import socket
import time


# Like echo10 but takes a long time to write back
def faker_slow():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('127.0.0.1', 4003))
    sock.listen()

    while True:
        conn, _ = sock.accept()
        msg = conn.recv(10)
        time.sleep(1)
        conn.send(msg)
