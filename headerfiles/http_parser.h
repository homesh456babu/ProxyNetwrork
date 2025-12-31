#pragma once
#include <string>
using namespace std;
struct HttpRequest {
    string method;
    string path;
    string version;
    string host;
    int content_length;
    int port;
    string body;
};
bool http_parse(const string &raw,HttpRequest &req);

