
#include "serverheaders.h"
#include "tunnel.h"
void tunnel(int client_fd, int server_fd) {
    char buf[4096];

    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(client_fd, &fds);
        FD_SET(server_fd, &fds);

        int maxfd = max(client_fd, server_fd) + 1;

        if (select(maxfd, &fds, nullptr, nullptr, nullptr) <= 0)
            break;

        // client → server
        if (FD_ISSET(client_fd, &fds)) {
            ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
            if (n <= 0) break;
            send(server_fd, buf, n, 0);
        }

        // server → client
        if (FD_ISSET(server_fd, &fds)) {
            ssize_t n = recv(server_fd, buf, sizeof(buf), 0);
            if (n <= 0) break;
            send(client_fd, buf, n, 0);
        }
    }
}
