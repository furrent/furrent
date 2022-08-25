import sys
import threading

from echo10 import faker_echo10
from handshake import faker_handshake
from slow import faker_slow

if len(sys.argv) != 2:
    print("Usage: python faker.py <all|handshake>")
    sys.exit(1)

name = sys.argv[1]

if name == "all":
    all_fakers = [faker_handshake, faker_echo10, faker_slow]

    threads = []
    for faker in all_fakers:
        t = threading.Thread(target=faker)
        threads.append(t)
        t.start()

    for t in threads:
        t.join()
elif name == "handshake":
    faker_handshake()
elif name == "echo10":
    faker_echo10()
elif name == "slow":
    faker_slow()
else:
    print("Unknown faker: " + name)
    sys.exit(1)
