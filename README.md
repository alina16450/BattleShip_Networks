Networked Battleship (Qt + Python)
A real-time multiplayer Battleship implementation built to explore low-level networking concepts including:
-TCP vs UDP tradeoffs
-Event-driven socket multiplexing
-Server-authoritative game design
-Custom application-layer protocol design
-Cross-language networking (C++ client and Python server)

The system consists of:
Qt (C++) Client – GUI + networking layer
Python Server – Authoritative game engine and communication hub

Architecture Overview:
Client 1 (Qt)  ←→  Python Server  ←→  Client 2 (Qt)
        TCP (game logic, turns, state)
        UDP (chat messaging)
TCP	Game actions (ATTACK, RESULT, TURN, WIN/LOSE)	Requires reliability, ordering, and guaranteed delivery
UDP	Chat messaging	Low-latency, non-critical communication where occasional packet loss is acceptable

Server Design:
The server uses Python’s selectors module to implement a single-threaded, event-driven architecture. All TCP sockets are registered with a selector.
The server reacts to read-ready events. No blocking calls are used. UDP traffic is also handled in a non-blocking loop.
This allows multiple clients to be handled concurrently without spawning threads, avoiding synchronization complexity.

Player Management:
First TCP connection → assigned P1
Second TCP connection → assigned P2
Additional connections → rejected (SERVER FULL)
Game begins only once two players are connected
The server enforces strict 1v1 gameplay.

Server-Authoritative Game Logic:
The server maintains two 10×10 matrices (one per player), random ship placement, turn tracking, hit counters, win detection.
Clients cannot directly modify game state and all attack requests are validated server-side before execution.

Input Validation & Security Considerations:
The server validates command format, turn ownership, coordinate bounds (0–9), and win condition logic.
This prevents malformed or malicious client messages from crashing or corrupting the game state.
In real-world systems, the server must never trust client input. This project follows that principle by enforcing gameplay rules and validating all actions server-side.

Protocol Design:
The application uses a custom newline-delimited text protocol over TCP.

Example TCP Messages
Client → Server:
ATTACK 3 7

Server → Attacker:
RESULT 3 7 HIT

Server → Defender:
INCOMING 3 7 HIT

Turn control:
TURN P1
NOT_YOUR_TURN
WIN
LOSE

Board initialization:
BOARDROW 0 0 1 0 0 0 1 ...
BOARDEND
TCP Message Framing

This implementation uses:
Newline-delimited messages (\n)
Server splits incoming data using splitlines()
Client reads messages using readLine()
This ensures proper application-layer framing.

UDP Chat Protocol:
Client sends:
CHAT Hello

Server broadcasts:
CHAT Hello

UDP clients automatically register when sending their first datagram.
Chat traffic is intentionally decoupled from gameplay traffic.

Qt Client Architecture:
The client uses QTcpSocket for reliable gameplay communication and QUdpSocket for chat messaging.
Qt signals and slots for event-driven networking.

Layer Separation:
networkmanager – handles socket communication
chatmanager – manages chat formatting and UI updates
battleship – GUI and board interaction logic
Main – application initialization

Networking is separated from presentation logic.

How to Run
Start the Server
python server.py

Default configuration:
TCP: 127.0.0.1:4040
UDP: 127.0.0.1:8080

Start the Client
Open Qt project
Adjust server IP if needed
Run two instances to simulate two players, or run the second instance on a different pc connected to the same network (update TCP and UDP binding ips as needed for this).
Limitations

Supports only 1v1 matches
No matchmaking/lobby system
No multiple concurrent matches
In-memory state only (no persistence)

Potential Improvements:
Multi-match support
Lobby system
JSON-based protocol
Persistent game state
AsyncIO-based server implementation
Docker containerization
Structured logging
Replay system

What This Project Demonstrates:
Practical TCP vs UDP usage
Event-driven network programming
Application-layer protocol design
Server-authoritative multiplayer architecture
Defensive input validation

Cross-language socket interoperability

Real-time state synchronization
