#### 🎥 Vedio recording link :   https://drive.google.com/file/d/1usc9K60TwVfmgJNc-RIxB2aJxVdVuU2B/view?usp=drive_link
## Overview
* This project develops a custom `HTTP` and `HTTPS` proxy server using C++ and low-level TCP socket programming. The proxy is capable of handling regular HTTP requests as well as secure HTTPS connections by implementing the CONNECT tunneling mechanism.
* The project is designed to demonstrate core concepts of **low-level network programming, multi-threaded concurrency, request parsing, logging.**
* The proxy acts as an intermediary between clients and remote servers:

    * For **HTTP traffic**, it parses incoming requests, forwards them to destination servers, optionally caches responses, and logs request details.

    * For **HTTPS traffic**, it establishes a bidirectional TCP tunnel using the CONNECT method and forwards encrypted data without inspecting or modifying it.

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
##  ✨ Features

## HTTP Request Handling

- Parses HTTP request line and headers (method (`GET,POST`), target, host).

- Supports both absolute and relative request formats.
  * Absolute request format `GET http://host/path HTTP/1.1`
  * Relative request format `GET /path HTTP/1.1`

- Forwards request bodies using the Content-Length header.

- Connects to destination server and forwards requests using send loops.
  
- Streams server responses back to the client without full buffering.
  
## HTTPS tunneling via CONNECT
- HTTPS `CONNECT` Handling
  
- Supports HTTPS traffic using the `CONNECT` method
  
- Establishes a TCP connection to the requested host and port
  
- Responds with `HTTP/1.1 200` Connection Established on success
 
- Forwards data bidirectionally between client and server
  
- Does not inspect or modify encrypted TLS traffic
  
## Domain and IP blocking
- Uses a simple text file `forbidden.txt` to define blocked domains and IP addresses
  
- Normalizes hostnames before matching (lowercase, trimmed)
  
- Supports basic suffix-based domain blocking
  
- Returns `HTTP/1.1 403` Forbidden for blocked requests
  
- Logs all blocked access attempts
  
## Thread-per-connection model
- Creates a listening TCP socket bound to a configurable IP address and port
 
- Uses a **thread-per-connection** concurrency model to handle multiple clients
  
- Spawns a new thread for each incoming client connection
  
## Logging and Metrics

- Logs timestamp, client address, destination host, request line, action, status code, and transferred size
  
- Keeps log files bounded using a simple rotation strategy
  
- Maintains runtime metrics such as request count and top requested hosts

## Build Instructions

## Requirements

- Linux operating system
 
- g++ compiler (C++17 or later)
  
- POSIX socket support
## Build Using Makefile
``` text
make
 ```
## Run
``` text
make run
```
## Stopping the Server

To terminate the proxy server and display runtime metrics, press `Ctrl + C` in the terminal.

## Testing
### HTTP Testing with curl
``` text
curl -x http://127.0.0.1:8080 http://example.com
```
### HTTPS Testing (CONNECT)
```text
curl -v -x http://127.0.0.1:8080 https://www.google.com
```
### Concurrent Request Testing
```text
ab -n 100 -c 10 -X 127.0.0.1:8080 http://example.com/
```
### Blocking Configuration
Edit `forbidden.txt` (project root):
```text
youtube.com
forbidden.com
```
```text
make run
curl -x http://127.0.0.1:8080 http://forbidden.com
```
## Logging
All proxy activity is recorded in a log file named `proxy.log`, which is created in the project root directory when the server is running.
To view the entire log file:
``` text
cat proxy.log
```
Each log entry includes request and response details such as timestamps, client address, destination host, request method, status code, and data transferred.


