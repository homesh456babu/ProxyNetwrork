#include <iostream>
#include "serverheaders.h"
#include "http_parser.h"
#include "connecting_to_server.h"
#include "filtering.h"
#include "logging.h"
#include "tunnel.h"
using namespace std;
//Global variables
vector<string> blocked_exact;
vector<string> blocked_suffix;

atomic<int> total_requests{0};
atomic<int> active_connections{0};
atomic<int> blocked_requests{0};

mutex metrics_mutex;
map<string, int> host_counter;

// for requests per minute
atomic<long> current_minute{0};
atomic<int> requests_this_minute{0};

//print metrics on Ctrl+C
void signal_handler(int signum) {
    cout << "\n\nCtrl+C received. Printing metrics...\n";
    print_metrics(total_requests,
        active_connections,
        blocked_requests,metrics_mutex,
        host_counter,
        current_minute,
        requests_this_minute);
    exit(0);
}

void handle_client(int client_fd) {
    total_requests++;
    set_timeouts(client_fd,10);
    HttpRequest req;
    string raw;
    //getting client ip address for logging
    string client_ip = get_client_address(client_fd);
    cout<<"Client connected: "<< client_ip <<endl;

    //receiving the request headers and storing in raw
    if(!recv_until(client_fd,raw)){
        close(client_fd);
        return;
    }

    //CONNECT method handling
    if(raw.find("CONNECT") == 0){
        //special parsing for connect method
        //as it has different format than normal http methods
        size_t header_end = raw.find("\r\n\r\n");
        if(header_end == string::npos){
            cerr << "Invalid CONNECT request\n";
            close(client_fd);
            return;         
        }
        //extracting the first line
        string first_line = raw.substr(0, raw.find("\r\n"));
        if(first_line.empty()){
            cerr << "Invalid CONNECT request\n";
            close(client_fd);
            return;
        }
        //parsing the first line
        //Example: CONNECT www.example.com:443 HTTP/1.1
        //extract host and port
        size_t pos_space1 = first_line.find(' ');
        size_t pos_space2 = first_line.find(' ', pos_space1 + 1);
        if (pos_space1 == string::npos || pos_space2 == string::npos) {
            cerr << "Invalid CONNECT request\n";
            close(client_fd);
            return;     
        }
        //parsing the connect request
        size_t pos1 = raw.find(' ');
        size_t pos2 = raw.find(' ', pos1 + 1);
        if (pos1 == string::npos || pos2 == string::npos) {
            cerr << "Invalid CONNECT request\n";
            close(client_fd);
            return;
        }
        string host_port = raw.substr(pos1 + 1, pos2 - pos1 - 1);
        size_t colon_pos = host_port.find(':');
        if (colon_pos != string::npos) {
            req.host = host_port.substr(0, colon_pos);
            req.port = stoi(host_port.substr(colon_pos + 1));
        } else {
            req.host = host_port;
            req.port = 443; // default HTTPS port
        }
        req.method = "CONNECT";
        req.path = "";
        req.version = "HTTP/1.1";
        req.content_length = 0;
        update_metrics(req.host,metrics_mutex,
            host_counter,
            current_minute,
            requests_this_minute);
        // cout << "==== PARSED REQUEST ====\n";
        // cout << "Method  : " << req.method << "\n";
        // cout << "Host    : " << req.host << "\n";
        // cout << "Port    : " << req.port << "\n";
        // cout << "========================\n";
        //before connecting to server check if host is blocked
        if(is_blocked(req.host,blocked_suffix,blocked_exact)){
            blocked_requests++;
            //send 403 forbidden response to client
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
            //logging the blocked request    
            rotate_log_if_needed();
            log_request(client_ip,
                req.host + ":" + to_string(req.port),
                req.method + " " + req.path + " " + req.version,
                "BLOCKED",
                403,
                0);
            if(!send_all(client_fd,resp)){
                cerr <<"Failed to send forbidden response to client\n";
                close(client_fd);
                return;     
            }
            close(client_fd);
            cout<<"Blocked Request to "<< req.host <<endl;
            return; 
        }
        //proceed to connect to server
        int server_fd = connect_to_server(req.host, req.port);
        if (server_fd < 0) {
            send(client_fd,
                "HTTP/1.1 502 Bad Gateway\r\n\r\n",
                28, 0);
            close(client_fd);
            return;
        }

        send(client_fd,
            "HTTP/1.1 200 Connection Established\r\n\r\n",
            39, 0);
        rotate_log_if_needed();
        log_request(client_ip,
            req.host + ":" + to_string(req.port),
            req.method + " " + req.path + " " + req.version,
            "FORWARDED",
            200,
            raw.size());
        tunnel(client_fd, server_fd);
        close(server_fd);
        close(client_fd);
        return;
    }
    //cout<<raw<<endl;
    //parsing the headers
    if(!http_parse(raw,req)){
        cerr << "Failed to parse HTTP request\n";
        close(client_fd);
        return;
    }

    //updating metrics
    update_metrics(req.host,metrics_mutex,
        host_counter,
        current_minute,
        requests_this_minute);

    string body;
    //getting the body
    size_t header_end = raw.find("\r\n\r\n");
    int already_read = raw.size() - (header_end + 4);
    int remaining = req.content_length - already_read;

    if (remaining > 0) {
        if (!recv_exact(client_fd,body,raw, remaining)) {
            cerr << "Failed to receive request body\n";
            close(client_fd);
            return;
        }
    }
    req.body = body;
    // cout << "==== PARSED REQUEST ====\n";
    // cout << "Method  : " << req.method << "\n";
    // cout << "Target  : " << req.path << "\n";
    // cout << "Version : " << req.version << "\n";
    // cout << "Host    : " << req.host << "\n";
    // cout << "Length  : " << req.content_length << "\n";
    // cout << "Body    : " << req.body << "\n";
    // cout << "========================\n";
    //before connecting to server check if host is blocked
    if(is_blocked(req.host,blocked_suffix,blocked_exact)){
        blocked_requests++;
        //send 403 forbidden response to client
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

        //logging the blocked request    
        rotate_log_if_needed();
        log_request(client_ip,
            req.host + ":" + to_string(req.port),
            req.method + " " + req.path + " " + req.version,
            "BLOCKED",
            403,
            0);

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

    //sending the raw request to the server
    if(!send_all(server_fd,raw)){
        cerr <<"Failed to send all the data to dest.server\n";
        close(server_fd);
        close(client_fd);
        return;
    }
    //logging the forwarded request
    rotate_log_if_needed();
    log_request(client_ip,
        req.host + ":" + to_string(req.port),
        req.method + " " + req.path + " " + req.version,
        "FORWARDED",
        200,
        raw.size());
    //recieving information from the server and forwarding it to client
    relay_response(server_fd, client_fd);
    close(server_fd); //closing the server
    
    //cout<<"Done"<<endl;
    close(client_fd);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
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

