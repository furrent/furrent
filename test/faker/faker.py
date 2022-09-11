import sys
import threading

from alice_seeder import faker_alice
from connect import faker_connect
from connect_plus_piece import faker_connect_plus_piece
from echo10 import faker_echo10
from slow import faker_slow

if len(sys.argv) != 2:
    print("Usage: python faker.py <all|echo10|slow|connect|connect_plus_piece|alice>")
    sys.exit(1)

name = sys.argv[1]

if name == "all":
    all_fakers = [faker_alice, faker_connect, faker_connect_plus_piece, faker_echo10, faker_slow]

    threads = []
    for faker in all_fakers:
        t = threading.Thread(target=faker)
        threads.append(t)
        t.start()

    for t in threads:
        t.join()
elif name == "alice":
    faker_alice()
elif name == "connect":
    faker_connect()
elif name == "connect_plus_piece":
    faker_connect_plus_piece()
elif name == "echo10":
    faker_echo10()
elif name == "slow":
    faker_slow()
else:
    print("Unknown faker: " + name)
    sys.exit(1)
