#include "serverheaders.h"
#include "http_parser.h"
using namespace std;
#include <sstream>
bool http_parse(const string &raw,HttpRequest &req){
    istringstream ss(raw);
    string method,target,version;
    ss >> method >> target >> version;

    if(method.empty() || target.empty()) return false;
    req.method = method;

    string host;
    string path;
    //lets say the url request is absolute form//
    if(target.find("http://") != string :: npos){
        string req_tar = target.substr(7);
        if(req_tar.empty()) return false;

        size_t pos = req_tar.find('/');
        if(pos != string :: npos){
            host = req_tar.substr(0,pos);
            path = req_tar.substr(pos);
        }
        else{
            host = req_tar;
            path = "/";
        }
    }
    else{
        path = target;
    }
    int con_len = 0;
    //now try it for absolute form//
    string line;
    while(getline(ss,line)){
        //if it is blank space break the loop
        if(line.empty() || line == "\r") break;

        //get line removes /n 
        if(line.back() == '\r'){
            line.pop_back();
        }

        //make it all lower case
        for(char &c: line){
            c = tolower((unsigned char) c);
        }

        //extract host
        if(line.find("host:") == 0){
            host = line.substr(5);
        }
        //if not host search content-lenght
        else if(line.find("content-length:") == 0){
            string val = line.substr(15);
            val.erase(0,val.find_first_not_of(" "));
            con_len = stoi(val);
        }

    }
    //remove leading spaces
    host.erase(0,host.find_first_not_of(" "));
    host.erase(host.find_last_not_of(" \t\r\n")+1);
    if(host.empty()){
        return false;
    }
    if(path.empty()) path = "/";
    int port = 80; //http ddefault port
    size_t pos = host.rfind(':');
    if(pos != string :: npos){
        port = stoi(host.substr(pos+1));
        host = host.substr(0,pos);
    }

    req.port = port;
    req.path = path;
    req.host = host;
    req.content_length = con_len;
    req.version = version;

    return true;
}
