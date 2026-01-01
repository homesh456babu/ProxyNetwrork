#include <algorithm>
bool ends_with(const string& s, const string& suffix);
bool is_ipv4(const string& s);
string trim(string s);
string canonicalize_host(string host);
void load_blocklist(const string& file,
    vector<string> &blocked_suffix,
    vector<string> &blocked_exact);
bool is_blocked(string host,
    vector<string> &blocked_suffix,
    vector<string> &blocked_exact);