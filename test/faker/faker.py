import sys
import threading

from handshake import faker_handshake

if len(sys.argv) != 2:
    print("Usage: python faker.py <all|handshake>")
    sys.exit(1)

name = sys.argv[1]

if name == "all":
    all_fakers = [faker_handshake]

    threads = []
    for faker in all_fakers:
        t = threading.Thread(target=faker)
        threads.append(t)
        t.start()

    for t in threads:
        t.join()
elif name == "handshake":
    faker_handshake()
else:
    print("Unknown faker: " + name)
    sys.exit(1)
