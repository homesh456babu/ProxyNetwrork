#include <iostream>
#include "serverheaders.h"
#include "http_parser.h"
#include "connecting_to_server.h"
#include "filtering.h"
//Global variables
vector<string> blocked_exact;
vector<string> blocked_suffix;

void handle_client(int client_fd) {
    set_timeouts(client_fd,10);
    HttpRequest req;
    string raw;

    if(!recv_until(client_fd,raw)){
        close(client_fd);
        return;
    }
    //cout<<raw<<endl;
    //parsing the headers
    if(!http_parse(raw,req)){
        close(client_fd);
        return;
    }
    string body;
    //getting the body
    size_t header_end = raw.find("\r\n\r\n");
    int already_read = raw.size() - (header_end + 4);
    int remaining = req.content_length - already_read;

    if (remaining > 0) {
        if (!recv_exact(client_fd,body,raw, remaining)) {
            close(client_fd);
            return;
        }
    }
    req.body = body;
    cout << "==== PARSED REQUEST ====\n";
    cout << "Method  : " << req.method << "\n";
    cout << "Target  : " << req.path << "\n";
    cout << "Version : " << req.version << "\n";
    cout << "Host    : " << req.host << "\n";
    cout << "Length  : " << req.content_length << "\n";
    cout << "Body    : " << req.body << "\n";
    cout << "========================\n";
    //before connecting to server check if host is blocked
    if(is_blocked(req.host,blocked_suffix,blocked_exact)){
        string body =
        "The host server is forbidden.\n"
        "Sorry, you can't connect to it.\n";

        string resp =
            "HTTP/1.1 403 Forbidden\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            body;
        if(!send_all(client_fd,resp)){
            cerr <<"Failed to send forbidden response to client\n";
            close(client_fd);
            return;
        }
        close(client_fd);
        cout<<"Blocked Request to "<< req.host <<endl;
        return;
    }
    //open a new connection tp dest server
    int server_fd = connect_to_server(req.host, req.port);
    if (server_fd < 0) {
        cerr << "Failed to connect to destination server\n";
        close(client_fd);
        return;
    }
    if(!send_all(server_fd,raw)){
        cerr <<"Failed to send all the data to dest.server\n";
        close(server_fd);
        close(client_fd);
        return;
    }

    //recieving information from the server and forwarding it to client
    relay_response(server_fd, client_fd);
    close(server_fd); //closing the server
    

    cout<<"Done"<<endl;
    close(client_fd);
}

int main(int argc, char* argv[]) {
    const char* bind_ip = "0.0.0.0";  // configurable
    int port = 8000;                  // configurable

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, bind_ip, &addr.sin_addr);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "Listening on " << bind_ip << ":" << port << std::endl;
    // Loading the forbidden file into memory
    load_blocklist("forbidden.txt",blocked_suffix,blocked_exact);
    cout<<"Blocked Exact Hosts:"<< blocked_exact.size()<<endl;
    cout<<"Blocked Suffix Hosts:"<< blocked_suffix.size()<<endl;    
    // Accept loop
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,
                               (sockaddr*)&client_addr,
                               &client_len);

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        // Thread per connection
        std::thread(handle_client, client_fd).detach();
    }

    close(server_fd);
    return 0;
}

