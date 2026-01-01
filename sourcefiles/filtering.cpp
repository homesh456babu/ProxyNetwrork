
#include "serverheaders.h"
#include "filtering.h"
#include <sstream>
#include <fstream>
#include <string>
using namespace std;
bool is_ipv4(const string& s) {
    sockaddr_in sa{};
    return inet_pton(AF_INET, s.c_str(), &(sa.sin_addr)) == 1;
}
string trim(string s) {
    s.erase(0, s.find_first_not_of(" \t\r\n"));
    s.erase(s.find_last_not_of(" \t\r\n") + 1);
    return s;
}
string canonicalize_host(string host) {
    host = trim(host);
    transform(host.begin(), host.end(), host.begin(),
              [](unsigned char c){ return tolower(c); });
    if (!host.empty() && host.back() == '.')
        host.pop_back();
    return host;
}
bool ends_with(const string& s, const string& suffix) {
    if (s.size() < suffix.size())
        return false;
    return s.compare(s.size() - suffix.size(),
                     suffix.size(),
                     suffix) == 0;
}

void load_blocklist(const string& file,vector<string> &blocked_suffix,vector<string> &blocked_exact) {
    ifstream f(file);
    if (!f.is_open()) {
    cerr << "ERROR: Could not open blocklist file: " << file << endl;
    return;
    }
    string line;

    while (getline(f, line)) {
        size_t c = line.find('#');
        if (c != string::npos) line = line.substr(0, c);

        line = canonicalize_host(line);
        if (line.empty()) continue;

        if (line.rfind("*.", 0) == 0)
            blocked_suffix.push_back(line.substr(2));
        else
            blocked_exact.push_back(line);
    }
}

bool is_blocked(string host,vector<string> &blocked_suffix,vector<string> &blocked_exact) {
    host = canonicalize_host(host);

    if (is_ipv4(host)) { 
        // checking whether it is valid ip address or not
        for (auto& ip : blocked_exact)
            if (host == ip) return true;
        return false;
    }

    for (auto& d : blocked_exact)
        if (host == d) return true;

    for (auto& s : blocked_suffix)
        if (host == s || ends_with(host, "." + s)) return true;

    return false;
}
