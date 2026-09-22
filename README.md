# Concurrent FTP Server

A multi-client file transfer server built in C using raw TCP sockets.

Each client connection is handled in an isolated child process via `fork()`, allowing multiple simultaneous downloads without blocking.

---

## How it works

The server listens on a TCP port. When a client connects, it forks a new process to handle the session. The client sends a filename, the server responds with a size header (`OK <bytes>`), then streams the raw file bytes. The client writes them to disk.

```
Client → connect → send filename\n
Server → "OK <filesize>\n" → raw bytes
Client → write to output file
```

---

## Build

```bash
gcc server.c -o server
gcc client.c -o client
```

## Run

**Start the server:**
```bash
./server 9000
```

**Connect a client:**
```bash
./client 127.0.0.1 9000 Demo.txt Output.txt
```

| Argument | Description |
|---|---|
| `127.0.0.1` | Server IP |
| `9000` | Port number |
| `Demo.txt` | File to request from server |
| `Output.txt` | Local filename to save as |

---

## Features

- Fork-per-connection — each client gets an isolated child process
- Custom line-based protocol — filename request → size header → byte stream
- Handles partial reads correctly using a receive loop
- Clean process separation — parent never blocks on a client

---

## Requirements

- Linux / Unix
- GCC

---

## Author

Aditya Chavan — [github.com/aditya8242](https://github.com/aditya8242)