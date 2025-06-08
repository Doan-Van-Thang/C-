#include <bits/stdc++.h>
#include <chrono>

#include "randint.hpp"
#include "jellyfish_gen.hpp"
#include "fattree_gen.hpp"

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
// const double PATH_RANDOMIZATION_PROB = 0.3; // Probability to choose a random next hop instead of shortest path

// Precomputed routing tables for faster next hop selection
std::vector<std::vector<std::vector<int>>> routing_tables; // [source][destination] = vector of possible next hops

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
  package(int _src, int _des, int _time_to_live, int _time, int _size = PACKAGE_SIZE) 
    : src(_src), des(_des), time_to_live(_time_to_live), size(_size), creation_time(_time) {
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


// Precompute next hop tables for all source-destination pairs with O(n²) complexity
void precompute_routing_tables() {
  int total_nodes = server_count + switch_count;
  routing_tables.resize(total_nodes);
  
  for (int src = 0; src < total_nodes; src++) {
    routing_tables[src].resize(total_nodes);
  }
  // For each destination, compute next hops from all sources in one BFS
  for (int dest = 0; dest < total_nodes; dest++) {
    
    // Reverse BFS from destination to find next hops from all sources
    std::vector<int> distance(total_nodes, INT_MAX);
    std::vector<std::vector<int>> next_hops(total_nodes);
    std::queue<int> q;
    
    // Start BFS from destination
    q.push(dest);
    distance[dest] = 0;
    
    while (!q.empty()) {
      int current = q.front();
      q.pop();
      
      // Process all neighbors
      for (int link_id : topo[current]) {
        auto [u, v] = links[link_id];
        int neighbor = (u == current) ? v : u;
        
        // If this is the first visit or same distance as best known path
        if (distance[neighbor] == INT_MAX) {
          // First visit - this is a shortest path
          distance[neighbor] = distance[current] + 1;
          next_hops[neighbor].push_back(current);
          q.push(neighbor);
        } else if (distance[neighbor] == distance[current] + 1) {
          // Same distance as best known path - an alternative shortest path
          next_hops[neighbor].push_back(current);
        }
      }
    }

    // Convert next_hops to routing_tables
    for (int src = 0; src < total_nodes; src++) {
      if (src == dest) continue;
      
      // If there's a path from src to dest
      if (!next_hops[src].empty()) {
        // For each possible next hop on a shortest path
        for (int next : next_hops[src]) {
          routing_tables[src][dest].push_back(next);
        }
      }
    }
  }
}

// Choose next hop with randomization
int choose_next_hop(int from, int to) {
  auto& table = routing_tables[from][to];
  assert(!table.empty() && "Can't find next hop, topology is wrong!");

  return table[randint(0, int(table.size()) - 1)];

  // // Check if we have precomputed routes
  // if (!routing_tables[from][to].empty()) {
  //   // With some probability, choose a random valid next hop
  //   if (((double)rand() / RAND_MAX) < PATH_RANDOMIZATION_PROB) {
  //     // Choose a random next hop from all valid options
  //     return routing_tables[from][to][rand() % routing_tables[from][to].size()];
  //   }
    
  //   // Otherwise use the first next hop (they're all shortest paths)
  //   return routing_tables[from][to][0];
  // }
  
  // return -1; // No valid next hop
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
      total_bits_delivered += p.size;
      total_delivery_time += (time_stamp - p.creation_time);
      return;
    }
    
    if (p.time_to_live <= 0) {
      // Package expired, discard it
      total_packages_dropped++;
      return;
    }
    
    // Find next hop using randomized path selection
    int next_hop = choose_next_hop(to, p.des);
    
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
    
    // Create arrival event at next hop
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
    
    // Find the next hop from source using randomized path selection
    int first_hop = choose_next_hop(from, to);
    
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
  // assert(std::freopen("result_fattree", "a", stdout));
  std::cout << "\n\n\n\n\n\n\n";

  // Seed the random number generator
  srand(time(NULL));
  
  std::cout << "Generating topology...\n";
  const int k = 26;
  const int n = 720;
  const int r = 18;
  const int m = 4500;
  auto g = jellyfish_gen(n, k, r, m);

  // const int k = 4;
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
  
  // Precompute routing tables for faster next hop selection - now O(n²) complexity
  std::cout << "Computing simple routing table...\n";
  precompute_routing_tables();
  
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