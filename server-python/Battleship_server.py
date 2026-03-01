import selectors
import socket
import random

sel = selectors.DefaultSelector()

player_sockets = {}
boards = {
    1: [[0] * 10 for _ in range(10)],
    2: [[0] * 10 for _ in range(10)],
}
hits = {1: 0, 2: 0}
SHIP_SIZES = [3, 2, 4]
ship_tiles = sum(SHIP_SIZES)
current_turn = 1
clients = {}

tcp_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_server.bind(("127.0.0.1", 4040))
tcp_server.listen()
tcp_server.setblocking(False)

udp_port = 8080
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_socket.bind(("127.0.0.1", udp_port))
udp_socket.setblocking(False)

udp_clients = []

def can_place(board, r, c, length, horiz):
    if horiz:
        if c + length > 10:
            return False
        return all(board[r][c + i] == 0 for i in range(length))
    else:
        if r + length > 10:
            return False
        return all(board[r + i][c] == 0 for i in range(length))

def place_ship(board, r, c, length, horiz):
    if horiz:
        for i in range(length):
            board[r][c + i] = 1
    else:
        for i in range(length):
            board[r + i][c] = 1

def place_all_ships(board):
    for length in SHIP_SIZES:
        placed = False
        while not placed:
            r = random.randint(0, 9)
            c = random.randint(0, 9)
            horiz = random.choice([True, False])
            if can_place(board, r, c, length, horiz):
                place_ship(board, r, c, length, horiz)
                placed = True


place_all_ships(boards[1])
place_all_ships(boards[2])

def udp_loop():
    try:
        data, addr = udp_socket.recvfrom(1024)
    except BlockingIOError:
        return

    msg = data.decode().strip()

    if addr not in udp_clients:
        udp_clients.append(addr)
        print(f"UDP client registered: {addr}")

    if msg.startswith("CHAT"):
        chat_text = msg[5:]
        print(f"UDP CHAT from {addr}: {chat_text}")

        broadcast_packet = f"CHAT {chat_text}".encode()
        for c in udp_clients:
            udp_socket.sendto(broadcast_packet, c)


def send_board(conn, player_id):
    board = boards[player_id]
    for r in range(10):
        rowvals = " ".join(str(v) for v in board[r])
        conn.sendall(f"BOARDROW {r} {rowvals}\n".encode())
    conn.sendall(b"BOARDEND\n")

def accept(sock):
    conn, addr = sock.accept()
    conn.setblocking(False)
    print("TCP Client connected:", addr)
    clients[conn] = addr

    if 1 not in player_sockets:
        player_sockets[1] = conn
        conn.sendall(b"ROLE P1\n")
        player_id = 1
        print("Assigned P1")

    elif 2 not in player_sockets:
        player_sockets[2] = conn
        conn.sendall(b"ROLE P2\n")
        player_id = 2
        print("Assigned P2")

        player_sockets[1].sendall(b"TURN P1\n")

    else:
        conn.sendall(b"SERVER FULL\n")
        conn.close()
        return

    send_board(conn, player_id)
    sel.register(conn, selectors.EVENT_READ, read)

def read(conn):
    try:
        data = conn.recv(1024)
    except ConnectionResetError:
        return disconnect(conn)

    if not data:
        return disconnect(conn)

    for line in data.decode().splitlines():
        msg = line.strip()
        if msg:
            handle_message(conn, msg)
    return None


def disconnect(conn):
    print("TCP Client disconnected:", clients.get(conn))
    try:
        sel.unregister(conn)
    except Exception:
        pass
    conn.close()

    for pid, sock in list(player_sockets.items()):
        if sock is conn:
            del player_sockets[pid]

    clients.pop(conn, None)


def handle_message(conn, msg):
    global current_turn

    attacker = None
    for pid, sock in player_sockets.items():
        if sock is conn:
            attacker = pid
            break

    if attacker is None:
        conn.sendall(b"ERROR Not a player\n")
        return

    defender = 1 if attacker == 2 else 2
    defender_sock = player_sockets.get(defender)

    if defender_sock is None:
        conn.sendall(b"ERROR Waiting for opponent\n")
        return

    if attacker != current_turn:
        conn.sendall(b"NOT_YOUR_TURN\n")
        return

    if msg.startswith("ATTACK"):
        parts = msg.split()
        if len(parts) != 3:
            conn.sendall(b"ERROR Invalid ATTACK\n")
            return

        x = int(parts[1])
        y = int(parts[2])

        board = boards[defender]

        if board[x][y] == 1:
            board[x][y] = -1
            hits[attacker] += 1
            result = "HIT"

        elif board[x][y] == -1 or board[x][y] == -2:
            result = "ALREADY"

        else:
            board[x][y] = -2
            result = "MISS"

        attacker_packet = f"RESULT {x} {y} {result}\n"
        conn.sendall(attacker_packet.encode())

        defender_packet = f"INCOMING {x} {y} {result}\n"
        defender_sock.sendall(defender_packet.encode())

        if hits[attacker] == ship_tiles:
            conn.sendall(b"WIN\n")
            defender_sock.sendall(b"LOSE\n")
            return

        current_turn = defender
        defender_sock.sendall(f"TURN P{defender}\n".encode())
        return

    else:
        conn.sendall(b"ERROR Unknown command\n")


sel.register(tcp_server, selectors.EVENT_READ, accept)

print("=== Battleship Server Running ===")
print("TCP: 127.0.0.1:4040  |  UDP: 127.0.0.1:8080")
print("Waiting for P1 and P2...")

while True:
    udp_loop()
    events = sel.select(timeout=0.01)
    for key, mask in events:
        callback = key.data
        callback(key.fileobj)
