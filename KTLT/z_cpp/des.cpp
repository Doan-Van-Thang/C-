#include <bits/stdc++.h>

#include "randint.hpp"
#include "jellyfish_gen.hpp"

template<typename T> using Ref = std::shared_ptr<T>;
template<typename T, typename ...M>
Ref<T> CreateRef(M&&... args) {
  return std::make_shared<T>(std::forward<M>(args)...);
}

struct package;
struct Event;


int server_count;
int switch_count;
std::vector<std::vector<int>> topo;
std::vector<std::pair<int, int>> links;
std::vector<int> link_busy_till; // link_busy_till[i]: nếu xếp một gói vào hàng đợi của links[i] thì đến khi nào nó mới được gửi?
const int LINK_BANDWIDTH = 1;   // Bandwidth of each link (bits per time unit)
const int PACKAGE_SIZE = 1000;     // Size of each package (bits)
// Calculate transmission time based on package size and link bandwidth
int calculate_transmission_time(int package_size) {
  return package_size / LINK_BANDWIDTH;  // Time in simulation units
}

// Statistics
int total_packages_sent = 0;
int total_packages_delivered = 0;
int total_packages_dropped = 0;


struct package {
  int src;
  int des;
  int time_to_live;
  int size;
  package(int _src, int _des, int _time_to_live, int _size = PACKAGE_SIZE) 
    : src(_src), des(_des), time_to_live(_time_to_live), size(_size) {
  }
};

struct Event {
  int time_stamp;
  Event (int _t = INT_MAX) : time_stamp(_t) {
  }
  virtual void process() = 0;
};

std::priority_queue<
  Ref<Event>,
  std::vector<Ref<Event>>,
  decltype([](Ref<Event> a, Ref<Event> b) { return a->time_stamp > b->time_stamp; })
> pq;


// Shortest path algorithm using BFS for routing
std::vector<int> find_shortest_path(int start, int end) {
  std::vector<int> parent(topo.size(), -1);
  std::vector<bool> visited(topo.size(), false);
  std::queue<int> q;
  
  q.push(start);
  visited[start] = true;
  
  while (!q.empty()) {
    int current = q.front();
    q.pop();
    
    if (current == end) {
      break;
    }
    
    for (int link_id : topo[current]) {
      auto [u, v] = links[link_id];
      int next = (u == current) ? v : u;
      
      if (!visited[next]) {
        visited[next] = true;
        parent[next] = current;
        q.push(next);
      }
    }
  }
  
  // Reconstruct path
  std::vector<int> path;
  if (parent[end] != -1) {  // Path exists
    for (int at = end; at != start; at = parent[at]) {
      path.push_back(at);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
  }
  
  return path;
}

// Find the link ID between two nodes
int find_link_id(int u, int v) {
  for (int link_id : topo[u]) {
    auto [a, b] = links[link_id];
    if ((a == u && b == v) || (a == v && b == u)) {
      return link_id;
    }
  }
  return -1;  // Link not found
}

class PackageArrival : public Event {
  public:
  package p;
  int to;

  PackageArrival(package _p, int _to, int _t) : Event(_t), p(_p), to(_to) {
  }

  void process() {
    // Decrease time to live
    p.time_to_live--;
    
    // Check if package reached destination or TTL expired
    if (to == p.des) {
      // Package reached destination
      total_packages_delivered++;
      return;
    }
    
    if (p.time_to_live <= 0) {
      // Package expired, discard it
      total_packages_dropped++;
      return;
    }
    
    // Find next hop using shortest path routing
    std::vector<int> path = find_shortest_path(to, p.des);
    
    if (path.size() < 2) {
      // No valid path to destination, discard the package
      total_packages_dropped++;
      return;
    }
    
    // Get next hop
    int next_hop = path[1];
    
    // Find the link to use
    int link_id = find_link_id(to, next_hop);
    
    if (link_id == -1) {
      // No valid link found, should not happen with correct topology
      total_packages_dropped++;
      return;
    }
    
    // Calculate when the package can be sent based on bandwidth
    int transmission_time = calculate_transmission_time(p.size);
    int send_time = std::max(time_stamp, link_busy_till[link_id]);
    link_busy_till[link_id] = send_time + transmission_time;  // Mark link as busy for transmission time
    
    // Create arrival event at next hop
    pq.push(CreateRef<PackageArrival>(p, next_hop, send_time + transmission_time));
  }
};

class FlowStartEvent : public Event {
  public:
  int from;
  int to;
  int time_to_live = 20;

  FlowStartEvent(int _from, int _to, int _t) : Event(_t), from(_from), to(_to) {
  }

  void process() {
    // Create a new package with size PACKAGE_SIZE
    package p(from, to, time_to_live, PACKAGE_SIZE);
    total_packages_sent++;
    
    // Find the next hop from source using shortest path routing
    std::vector<int> path = find_shortest_path(from, to);
    
    if (path.size() < 2) {
      // No valid path to destination, discard the flow
      total_packages_dropped++;
      return;
    }
    
    // Get first hop
    int first_hop = path[1];
    
    // Find the link to use
    int link_id = find_link_id(from, first_hop);
    
    if (link_id == -1) {
      // No valid link found, should not happen with correct topology
      total_packages_dropped++;
      return;
    }
    
    // Calculate when the package can be sent based on bandwidth
    int transmission_time = calculate_transmission_time(p.size);
    int send_time = std::max(time_stamp, link_busy_till[link_id]);
    link_busy_till[link_id] = send_time + transmission_time;  // Mark link as busy for transmission time
    
    // Create arrival event at first hop
    pq.push(CreateRef<PackageArrival>(p, first_hop, send_time + transmission_time));
  }
};

int main() {
  int n = 200;
  int k = 12;
  int r = 6;
  int m = 900;
  auto g = jellyfish_gen(n, k, r, m);
  
  server_count = n;
  switch_count = m;

  for (int u = 0; u < n; u++) {
    for (int v : g[u]) {
      if (v > u) links.emplace_back(u, v);
    }
  }
  link_busy_till.resize(links.size(), 0);

  topo.resize(n + m);
  for (int id = 0; auto [u, v] : links) {
    topo[u].push_back(id);
    topo[v].push_back(id);
    id++;
  }
    
  // generate flow
  for (int u = n; u < n + m; u++) {
    int v = randint(n, n + m - 1);
    while (v == u) {
      v = randint(n, n + m - 1);
    }
    pq.push(CreateRef<FlowStartEvent>(u, v, 0));
  }

  // process
  int max_events = 100000;  // Limit to prevent infinite loops
  int processed = 0;
  int max_time = 0;
  
  while (!pq.empty() && processed < max_events) {
    auto e = pq.top(); pq.pop();
    max_time = std::max(max_time, e->time_stamp);
    e->process();
    processed++;
  }
  
  std::cout << "Simulation completed:" << std::endl;
  std::cout << "Processed " << processed << " events" << std::endl;
  std::cout << "Simulation ran for " << max_time << " time units" << std::endl;
  std::cout << "Packages sent: " << total_packages_sent << std::endl;
  std::cout << "Packages delivered: " << total_packages_delivered << std::endl;
  std::cout << "Packages dropped: " << total_packages_dropped << std::endl;
  std::cout << "Delivery rate: " << (double)total_packages_delivered / total_packages_sent * 100 << "%" << std::endl;
  
  // Calculate network utilization
  double total_capacity = links.size() * LINK_BANDWIDTH * max_time;
  double utilization = 0;
  for (int i = 0; i < links.size(); i++) {
    // Approximate utilization based on busy time
    utilization += link_busy_till[i];
  }
  
  std::cout << "Network utilization: " << (utilization / total_capacity) * 100 << "%" << std::endl;
  
  std::cout << '\n';
  return 0;
}