#include "serverheaders.h"
#include "connecting_to_server.h"
#include <iostream>

using namespace std;

int connect_to_server(const string& host, int port) {
    struct addrinfo hints{}, *res, *p;
    int server_fd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP

    string port_str = to_string(port);

    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_fd < 0)
            continue;

        if (connect(server_fd, p->ai_addr, p->ai_addrlen) == 0) {
            break; // success
        }

        close(server_fd);
        server_fd = -1;
    }

    freeaddrinfo(res);
    return server_fd; // -1 if failed
}



