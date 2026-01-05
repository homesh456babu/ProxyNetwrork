#include "serverheaders.h"
#include "logging.h"
#include <iostream>
#include <algorithm>
using namespace std;

mutex log_mutex;

void print_metrics(atomic<int> &total_requests,
        atomic<int> &active_connections,
        atomic<int> &blocked_requests,mutex &metrics_mutex,
        map<string, int>& host_counter,
        atomic<long> &current_minute,
        atomic<int> &requests_this_minute) {
    cout << "\n===== METRICS SUMMARY =====\n";
    cout << "Total requests      : " << total_requests << endl;
    cout << "Active connections  : " << active_connections << endl; 
    cout << "Blocked requests    : " << blocked_requests << endl;
    cout << "Requests per minute : " << requests_this_minute << endl;

    cout << "\nTop requested hosts:\n";
    {
        lock_guard<mutex> lock(metrics_mutex);
        vector<pair<string, int>> host_vec(host_counter.begin(), host_counter.end());
        sort(host_vec.begin(), host_vec.end(), [](const auto& a, const auto& b) {
            return b.second < a.second;
        });
        int i = 0;
        for (const auto& pair : host_vec) {
            if (i++ >= 3) break;
            cout << "  " << pair.first << " : " << pair.second << endl;
        }
    }
    cout << "===========================\n";
}

void update_metrics(const string& host,mutex &metrics_mutex,
        map<string, int>& host_counter,
        atomic<long> &current_minute,
        atomic<int> &requests_this_minute) {
    time_t now = time(nullptr);
    long minute = now / 60;

    if (current_minute != minute) {
        current_minute = minute;
        requests_this_minute = 0;
    }
    requests_this_minute++;

    // top requested hosts
    {
        lock_guard<mutex> lock(metrics_mutex);
        host_counter[host]++;
    }
}

void log_request(const string& client,
                 const string& dest,
                 const string& request_line,
                 const string& action,
                 int status,
                 size_t bytes) {

    lock_guard<mutex> lock(log_mutex);

    //if log file doesn't exist it will be created automatically
    //or else it will be opened in append mode
    ofstream log("proxy.log", ios::app);

    time_t now = time(nullptr);
    tm* t = localtime(&now);

    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    log << timebuf << " | "
        << client << " | "
        << dest << " | "
        << request_line << " | "
        << action << " | "
        << status << " | "
        << bytes << " bytes\n";
}
//if log file size exceeds 5 MB, rotate it
//new log file will be created and old one will be renamed to proxy.log.old
//if there is already a proxy.log.old, it will be overwritten
void rotate_log_if_needed() {
    struct stat st;
    if (stat("proxy.log", &st) == 0) {
        if (st.st_size > 5 * 1024 * 1024) { // 5 MB
            rename("proxy.log", "proxy.log.old");
        }
    }
}





