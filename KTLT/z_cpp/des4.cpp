#include <bits/stdc++.h>
#include <chrono>

#include "randint.hpp"
#include "jellyfish_gen.hpp"
#include "fattree_gen.hpp"
#include "yen_ksp.hpp" // Include the new Yen's algorithm header 

bool vertify(const std::vector<std::vector<int>>& g) {
  int n = int(g.size());
  std::vector<bool> vis(n);
  std::queue<int> q;
  q.push(0);
  vis[0] = true;
  while (!q.empty()) {
    int u = q.front(); q.pop();
    for (int v : g[u]) if (!vis[v]) {
      vis[v] = true;
      q.push(v);
    }
  }
  return std::ranges::all_of(vis, [](bool x) { return x; });
}

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
std::vector<int> link_delays; // Propagation delay for each link
std::vector<long long> link_bits_transmitted; // Track bits transmitted on each link for throughput

const int LINK_BANDWIDTH = 1;   // Bandwidth of each link (bits per time unit)
const int PACKAGE_SIZE = 1000;     // Size of each package (bits)
const int MIN_LINK_DELAY = 1;     // Minimum propagation delay on links
const int MAX_LINK_DELAY = 10;    // Maximum propagation delay on links
const int K_PATHS = 3;           // Number of alternative paths to compute

// Precomputed routing tables using Yen's algorithm
std::vector<std::vector<std::vector<Path>>> k_shortest_paths;

// Calculate transmission time based on package size and link bandwidth
int calculate_transmission_time(int package_size) {
  return package_size / LINK_BANDWIDTH;  // Time in simulation units
}

// Statistics
int total_packages_sent = 0;
int total_packages_delivered = 0;
int total_packages_dropped = 0;
long long total_bits_delivered = 0;
int total_delivery_time = 0;


struct package {
  int src;
  int des;
  int time_to_live;
  int size;
  int creation_time;
  std::set<int> visited_nodes;  // Track visited nodes to avoid loops
  
  package(int _src, int _des, int _time_to_live, int _time, int _size = PACKAGE_SIZE) 
    : src(_src), des(_des), time_to_live(_time_to_live), size(_size), creation_time(_time) {
    // Initial node is added to visited
    visited_nodes.insert(_src);
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

// Find a suitable path from k-shortest paths that doesn't visit already visited nodes
int choose_next_hop_from_paths(int from, int to, const std::set<int>& visited_nodes) {
  // Get available paths from source to destination
  const auto& paths = k_shortest_paths[from][to];
  
  // If no paths available, return -1
  if (paths.empty()) {
    return -1;
  }
  
  // Try each path, starting with the shortest one
  for (const auto& path : paths) {
    if (path.nodes.size() < 2) continue; // Path too short
    
    int next_hop = path.nodes[1]; // First hop in the path (nodes[0] is 'from')
    
    // Check if the next hop has not been visited
    if (visited_nodes.find(next_hop) == visited_nodes.end()) {
      return next_hop;
    }
  }
  
  // If we've tried all k paths and all have visited nodes, pick a random path's next hop
  // that hasn't been visited, if possible
  std::vector<int> possible_next_hops;
  
  for (const auto& [neighbor, _] : convert_to_adjacency_list(topo, links, link_delays)[from]) {
    if (visited_nodes.find(neighbor) == visited_nodes.end()) {
      possible_next_hops.push_back(neighbor);
    }
  }
  
  if (!possible_next_hops.empty()) {
    return possible_next_hops[randint(0, int(possible_next_hops.size()) - 1)];
  }
  
  // No suitable next hop found
  return -1;
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
    // Add current node to visited
    p.visited_nodes.insert(to);
  }

  void process() {
    // Decrease time to live
    p.time_to_live--;
    
    // Check if package reached destination or TTL expired
    if (to == p.des) {
      // Package reached destination
      total_packages_delivered++;
      total_bits_delivered += p.size;
      total_delivery_time += (time_stamp - p.creation_time);
      return;
    }
    
    if (p.time_to_live <= 0) {
      // Package expired, discard it
      total_packages_dropped++;
      return;
    }
    
    // Find next hop using k-shortest paths that avoids visited nodes
    int next_hop = choose_next_hop_from_paths(to, p.des, p.visited_nodes);
    
    if (next_hop == -1) {
      // No valid path to destination, discard the package
      total_packages_dropped++;
      return;
    }
    
    // Find the link to use
    int link_id = find_link_id(to, next_hop);
    
    if (link_id == -1) {
      // No valid link found, should not happen with correct topology
      assert(false);
      total_packages_dropped++;
      return;
    }
    
    // Calculate when the package can be sent based on bandwidth
    int transmission_time = calculate_transmission_time(p.size);
    int send_time = std::max(time_stamp, link_busy_till[link_id]);
    link_busy_till[link_id] = send_time + transmission_time;  // Mark link as busy for transmission time
    
    // Add link's propagation delay
    int arrival_time = send_time + transmission_time + link_delays[link_id];
    
    // Track bits transmitted for throughput calculation
    link_bits_transmitted[link_id] += p.size;
    
    // Create arrival event at next hop with updated visited nodes
    pq.push(CreateRef<PackageArrival>(p, next_hop, arrival_time));
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
    // Create a new package with size PACKAGE_SIZE and record creation time
    package p(from, to, time_to_live, time_stamp, PACKAGE_SIZE);
    total_packages_sent++;
    
    // Find the next hop from source using k-shortest paths
    int first_hop = choose_next_hop_from_paths(from, to, p.visited_nodes);
    
    if (first_hop == -1) {
      // No valid path to destination, discard the flow
      total_packages_dropped++;
      return;
    }
    
    // Find the link to use
    int link_id = find_link_id(from, first_hop);
    
    if (link_id == -1) {
      // No valid link found, should not happen with correct topology
      assert(false);
      total_packages_dropped++;
      return;
    }
    
    // Calculate when the package can be sent based on bandwidth
    int transmission_time = calculate_transmission_time(p.size);
    int send_time = std::max(time_stamp, link_busy_till[link_id]);
    link_busy_till[link_id] = send_time + transmission_time;  // Mark link as busy for transmission time
    
    // Add link's propagation delay
    int arrival_time = send_time + transmission_time + link_delays[link_id];
    
    // Track bits transmitted for throughput calculation
    link_bits_transmitted[link_id] += p.size;
    
    // Create arrival event at first hop
    pq.push(CreateRef<PackageArrival>(p, first_hop, arrival_time));
  }
};

int main() {
  // Seed the random number generator
  srand(time(NULL));
  
  std::cout << "Generating topology...\n";
  const int k = 10;
  const int n = 125;
  const int r = 7;
  const int m = 250;
  auto g = jellyfish_gen(n, k, r, m);

  // const int k = 26;
  // const int n = 5 * k * k / 4;
  // const int m = k * k * k / 4;
  // auto g = fattree_gen(k);

  assert(vertify(g) && "Generated graph is not connected, wtf?");
  
  server_count = n;
  switch_count = m;

  for (int u = 0; u < n + m; u++) {
    for (int v : g[u]) {
      if (v > u) links.emplace_back(u, v);
    }
  }
  
  // Initialize link-related data structures
  link_busy_till.resize(links.size(), 0);
  link_delays.resize(links.size(), 0);
  link_bits_transmitted.resize(links.size(), 0);
  
  // Assign random propagation delays to links
  for (int i = 0; i < links.size(); i++) {
    link_delays[i] = MIN_LINK_DELAY + rand() % (MAX_LINK_DELAY - MIN_LINK_DELAY + 1);
  }

  topo.resize(n + m);
  for (int id = 0; auto [u, v] : links) {
    topo[u].push_back(id);
    topo[v].push_back(id);
    id++;
  }
  
  // Measure preprocessing time
  auto start_time = std::chrono::high_resolution_clock::now();
  
  // Instead of simple routing table, use Yen's algorithm to precompute k shortest paths
  std::cout << "Computing K=" << K_PATHS << " shortest paths routing table...\n";
  k_shortest_paths = precompute_k_shortest_paths(topo, links, link_delays, n + m, K_PATHS);
  
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
  
  std::cout << "Preprocessing completed in " << duration << " ms" << std::endl;
  
  std::cout << "\nStart simulation...\n";
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
  
  start_time = std::chrono::high_resolution_clock::now();
  
  while (!pq.empty() && processed < max_events) {
    auto e = pq.top(); pq.pop();
    max_time = std::max(max_time, e->time_stamp);
    e->process();
    processed++;
  }
  
  end_time = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
  
  // Calculate throughput and other metrics
  double total_throughput = (double)total_bits_delivered / max_time;
  double avg_delivery_time = total_packages_delivered > 0 ? 
                            (double)total_delivery_time / total_packages_delivered : 0;
  
  std::cout << "Simulation completed in " << duration << " ms:" << std::endl;
  std::cout << "Processed " << processed << " events" << std::endl;
  std::cout << "Simulation ran for " << max_time << " time units" << std::endl;
  std::cout << "Packages sent: " << total_packages_sent << std::endl;
  std::cout << "Packages delivered: " << total_packages_delivered << std::endl;
  std::cout << "Packages dropped: " << total_packages_dropped << std::endl;
  std::cout << "Delivery rate: " << (double)total_packages_delivered / total_packages_sent * 100 << "%" << std::endl;
  std::cout << "Average delivery time: " << avg_delivery_time << " time units" << std::endl;
  std::cout << "Total throughput: " << total_throughput << " bits/time unit" << std::endl;
  
  // Calculate network utilization
  double total_capacity = links.size() * LINK_BANDWIDTH * max_time;
  double utilization = 0;
  for (int i = 0; i < links.size(); i++) {
    // Approximate utilization based on busy time
    utilization += link_busy_till[i];
  }
  
  std::cout << "Network utilization: " << (utilization / total_capacity) * 100 << "%" << std::endl;
  
  // Print top 5 busiest links
  std::vector<std::pair<long long, int>> link_traffic;
  for (int i = 0; i < links.size(); i++) {
    link_traffic.push_back({link_bits_transmitted[i], i});
  }
  std::sort(link_traffic.rbegin(), link_traffic.rend());
  
  std::cout << "\nTop 5 busiest links:" << std::endl;
  for (int i = 0; i < std::min(5, (int)link_traffic.size()); i++) {
    int link_id = link_traffic[i].second;
    auto [u, v] = links[link_id];
    std::cout << "Link " << link_id << " (" << u << " - " << v << "): " 
              << link_bits_transmitted[link_id] << " bits, "
              << (double)link_bits_transmitted[link_id] / max_time << " bits/time unit throughput" << std::endl;
  }
  
  std::cerr << "Done\n";
  return 0;
}