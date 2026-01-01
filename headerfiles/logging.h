#include <mutex>
#include <fstream>
#include <ctime>
#include <string>
#include <sys/stat.h>
using namespace std;
void log_request(const string& client,
                 const string& dest,
                 const string& request_line,
                 const string& action,
                 int status,
                 size_t bytes) ;
void rotate_log_if_needed();
void update_metrics(const string& host,mutex &metrics_mutex,
        map<string, int>& host_counter,
        atomic<long> &current_minute,
        atomic<int> &requests_this_minute) ;
void print_metrics(atomic<int> &total_requests,
        atomic<int> &active_connections,
        atomic<int> &blocked_requests,mutex &metrics_mutex,
        map<string, int>& host_counter,
        atomic<long> &current_minute,
        atomic<int> &requests_this_minute) ;    

