## Design Document
### High-Level Architecture

* The proxy server follows a **client–proxy–server** architecture.

* Clients connect to the proxy instead of directly contacting destination servers. The proxy receives requests, applies filtering and logging rules, forwards valid requests to remote servers, and relays responses back to the clients.

###

#### Listener Module    `new_server.cpp`
* Creates and manages the listening TCP socket and accepts incoming client connections.

#### Connection Handler 'severheaders.cpp`
* Handles each client connection in a separate thread and manages request parsing, forwarding, and response relaying.

#### HTTP Parser  `http_parser.cpp`
* Parses HTTP request lines and headers to extract method, target, host, and content length.

#### Forwarding Module 'tunnel.cpp`
* Handles CONNECT tunneling

#### Filtering Module `filtering.cpp`
* Checks requested hosts against a blocklist and denies access when necessary.

#### Logging and Metrics Module `logging.cpp`
* Records request activity and maintains runtime statistics.

## Concurrency Model and Rationale

- The proxy uses a thread-per-connection concurrency model.
  
- Each incoming client connection is handled by a separate thread.
  
- This approach is simple to implement and easy to understand.
  
- It is suitable for educational projects and moderate workloads.
  
- Thread synchronization is handled using mutexes where shared data structures are accessed.
  
- This model was chosen for clarity and correctness rather than maximum scalability.

## Data Flow Description
### Incoming Request Handling
 
- Client establishes a TCP connection to the proxy.
 
- The proxy accepts the connection and spawns a new thread.
  
- The request is read from the client socket.
  
- The HTTP request line and headers are parsed.
 
- The request is checked against filtering rules.
 
- If blocked, an HTTP error response is sent to the client.
  
- If allowed, the request proceeds to forwarding.
  
### Outbound Forwarding and Response Handling

 ***For HTTP requests:***

- The proxy connects to the destination server.
 
- The full request (headers and body) is forwarded.
  
- The server response is streamed back to the client.
  
- Responses may be cached if eligible.

***For HTTPS requests (CONNECT):***

- The proxy establishes a TCP connection to the destination host.
 
- Sends HTTP/1.1 200 Connection Established.
  
- Forwards encrypted data bidirectionally without inspection.

## Error Handling

- Network read and write operations handle partial sends and receives.
  
- Socket timeouts prevent blocking indefinitely on inactive connections.
  
- Errors during request parsing or forwarding result in connection closure.
 
- All sockets are closed properly on errors or shutdown.

## Limitations

- No support for chunked transfer encoding parsing.
 
- HTTPS traffic is tunneled but not inspected.
 
- Caching is limited to HTTP GET requests and stored only in memory.
  
- Thread-per-connection model does not scale efficiently for very high loads.

- IPv4 only.

## Security Considerations

- HTTPS traffic is forwarded without inspection, preserving confidentiality.
  
- The proxy does not perform TLS interception.
  
- Domain and IP filtering reduces access to unwanted destinations.
  
- No authentication or access control is implemented.
  
- The proxy should not be exposed directly to untrusted networks.
