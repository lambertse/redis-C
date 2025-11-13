# Redis-C - In-Memory Key-Value Store Inspired by Redis

Redis-C is a fast, in-memory key-value storage system developed entirely in C. Drawing inspiration from Redis, it utilizes the RESP (Redis Serialization Protocol), ensuring complete compatibility with redis-cli.

## Core Capabilities

- **RESP Protocol Support**: Achieves full compatibility with redis-cli and other Redis-compatible clients.

- **Efficient I/O Operations**: Employs a single-threaded event-loop model with I/O multiplexing (leveraging epoll on Linux and kqueue on macOS) to manage thousands of simultaneous connections.

- **Native Data Structure Implementations**: Features hand-crafted advanced data structures, including:
  - TBD 

- **Probabilistic Algorithms**: Incorporates specialized data structures such as:
  - TBD 


## Quick Start Guide
```bash

```

## 🛠️ Available Commands

The server provides comprehensive command support organized by data category:

| Category | Commands |
|----------|----------|
| General | PING |

## Planned Enhancements

- [ ] Cuckoo filter
