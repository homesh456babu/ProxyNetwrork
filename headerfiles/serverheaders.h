#pragma once
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <netinet/in.h>
#include <thread>
#include <cstring>
#include <vector>
#include <atomic>
#include <mutex>
#include <sys/time.h>
#include <csignal>
#include <map>
using namespace std;
string get_client_address(int client_fd);
void set_timeouts(int fd, int seconds = 10);
bool recv_until(int fd, std::string& out);
bool recv_exact(int fd,string &body, string &out,int num);
bool send_all(int fd, const string& data);
void relay_response(int server_fd, int client_fd);
