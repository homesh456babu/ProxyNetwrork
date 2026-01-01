#include "serverheaders.h"
void set_timeouts(int fd, int seconds) {
    timeval tv{};
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}
bool recv_until(int fd, std::string& out) {
    char buf[4096];
    while (out.find("\r\n\r\n") == std::string::npos) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) return false;
        out.append(buf, n);
    }
    return true;
}

bool recv_exact(int fd, string &body,string &out,int num){
    int received = 0;
    char buf[4096];
    while(received < num){
        int to_read = min((int)sizeof(buf), num - received);
        ssize_t n = recv(fd,buf,to_read,0);
        if(n<=0) return false;
        out.append(buf,n);
        body.append(buf,n);
        received+= n;
    }
    return true;
}
bool send_all(int fd, const string& data) {
    size_t total_sent = 0;
    size_t length = data.size();
    cout<<"Sending total "<< length <<" bytes"<<endl;
    while (total_sent < length) {
        ssize_t n = send(fd,
                          data.data() + total_sent,
                          length - total_sent,
                          0);
        if (n <= 0)
            return false;

        total_sent += n;
    }
    return true;
}

void relay_response(int server_fd, int client_fd) {
    char buffer[4096];

    while (true) {
        ssize_t bytes_read = recv(server_fd, buffer, sizeof(buffer), 0);

        if (bytes_read == 0) {
            // server closed connection
            break;
        }
        if (bytes_read < 0) {
            perror("recv from server");
            break;
        }

        size_t total_sent = 0;
        while (total_sent < (size_t)bytes_read) {
            ssize_t sent = send(client_fd,
                                buffer + total_sent,
                                bytes_read - total_sent,
                                0);
            if (sent <= 0) {
                perror("send to client");
                return;
            }
            total_sent += sent;
        }
    }
}

