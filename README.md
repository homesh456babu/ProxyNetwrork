## Overview

ProxyNetwork is a multi-threaded HTTP/HTTPS forward proxy written in C++ using Linux socket programming.

It supports HTTP request forwarding, HTTPS tunneling using the CONNECT method, domain/IP filtering, logging, and basic runtime metrics.

## Project Structure

```text
ProxyNetwork/
├── sourcefiles/
│   ├── new_server.cpp
│   ├── connecting_to_server.cpp
│   ├── http_parser.cpp
│   ├── tunnel.cpp
│   ├── logging.cpp
│   ├── serverheaders.cpp
│   └── filtering.cpp
│
├── headerfiles/
│   ├── connecting_to_server.h
│   ├── http_parser.h
│   ├── tunnel.h
│   ├── logging.h
|   ├── serverheaders.h
│   ├── filtering.h
│
├── forbidden.txt
├── proxy.log
├── Makefile
└── README.md
```
## Features

HTTP proxy support (GET, POST)

HTTPS tunneling via CONNECT

Domain and IP blocking

Thread-per-connection model

Request logging

Runtime metrics (total requests, blocked requests, top hosts)




