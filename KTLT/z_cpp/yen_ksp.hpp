// optimized_yen_ksp.hpp - Optimized Yen's K-Shortest Paths Algorithm

#ifndef OPTIMIZED_YEN_KSP_HPP
#define OPTIMIZED_YEN_KSP_HPP

#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <functional>

// Path representation with nodes and total cost
struct Path {
    std::vector<int> nodes;
    int cost;
    
    Path() : cost(0) {}
    
    Path(const std::vector<int>& n, int c) : nodes(n), cost(c) {}
    
    bool operator<(const Path& other) const {
        return cost < other.cost;
    }
    
    bool operator>(const Path& other) const {
        return cost > other.cost;
    }
};

// For comparing paths in priority queue
struct PathCompare {
    bool operator()(const Path& a, const Path& b) const {
        return a.cost > b.cost;
    }
};

// Efficient hash function for vector<int> (path nodes)
struct VectorHash {
    std::size_t operator()(const std::vector<int>& path) const {
        std::size_t seed = path.size();
        for(auto& i : path) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

// Get edge weight between two nodes
inline int get_edge_weight(const std::vector<std::vector<std::pair<int, int>>>& adj, int u, int v) {
    for (const auto& [next, weight] : adj[u]) {
        if (next == v) {
            return weight;
        }
    }
    return -1; // Edge doesn't exist
}

// Optimized Dijkstra's algorithm with removed edges and nodes
// Uses edge filtering at query time instead of building modified graph
Path optimized_dijkstra(
    const std::vector<std::vector<std::pair<int, int>>>& adj, 
    int start, int end, 
    const std::unordered_map<int, std::unordered_set<int>>& removed_edges,
    const std::unordered_set<int>& removed_nodes) {
    
    int n = adj.size();
    std::vector<int> dist(n, std::numeric_limits<int>::max());
    std::vector<int> prev(n, -1);
    
    // Use std::greater for min-heap behavior
    std::priority_queue<std::pair<int, int>, 
                      std::vector<std::pair<int, int>>, 
                      std::greater<>> pq;
    
    dist[start] = 0;
    pq.push({0, start});
    
    while (!pq.empty()) {
        int u = pq.top().second;
        int d = pq.top().first;
        pq.pop();
        
        if (u == end) break;
        if (d > dist[u]) continue; // Skip outdated entries
        
        for (const auto& [v, weight] : adj[u]) {
            // Skip removed nodes
            if (removed_nodes.count(v) > 0) continue;
            
            // Skip removed edges more efficiently using the map-of-sets structure
            auto it = removed_edges.find(u);
            if (it != removed_edges.end() && it->second.count(v) > 0) continue;
            
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
    
    // Reconstruct path
    Path result;
    if (dist[end] == std::numeric_limits<int>::max()) {
        // No path found
        return result;
    }
    
    result.cost = dist[end];
    
    // Build path by tracing back from end to start
    int current = end;
    while (current != -1) {
        result.nodes.push_back(current);
        current = prev[current];
    }
    
    std::reverse(result.nodes.begin(), result.nodes.end());
    return result;
}

// Convert the given topology to an adjacency list with weights
std::vector<std::vector<std::pair<int, int>>> convert_to_adjacency_list(
    const std::vector<std::vector<int>>& topo,
    const std::vector<std::pair<int, int>>& links,
    const std::vector<int>& link_delays) {
    
    int n = topo.size();
    std::vector<std::vector<std::pair<int, int>>> adj(n);
    
    // Pre-allocate space to reduce reallocations
    for (int u = 0; u < n; u++) {
        adj[u].reserve(topo[u].size());
    }
    
    for (int u = 0; u < n; u++) {
        for (int link_id : topo[u]) {
            auto [a, b] = links[link_id];
            int v = (a == u) ? b : a;
            int weight = link_delays[link_id];
            adj[u].push_back({v, weight});
        }
    }
    
    return adj;
}

// Calculate the cost of a path directly from adjacency list
int calculate_path_cost(
    const std::vector<std::vector<std::pair<int, int>>>& adj,
    const std::vector<int>& path) {
    
    int cost = 0;
    for (size_t i = 0; i < path.size() - 1; i++) {
        int u = path[i];
        int v = path[i + 1];
        
        for (const auto& [next, weight] : adj[u]) {
            if (next == v) {
                cost += weight;
                break;
            }
        }
    }
    
    return cost;
}

// Optimized Yen's algorithm for k shortest paths
std::vector<Path> optimized_yen_ksp(
    const std::vector<std::vector<int>>& topo,
    const std::vector<std::pair<int, int>>& links,
    const std::vector<int>& link_delays,
    int source, int target, int k) {
    
    // Create adjacency list once
    std::vector<std::vector<std::pair<int, int>>> adj = 
        convert_to_adjacency_list(topo, links, link_delays);
    
    std::vector<Path> result;  // List of shortest paths
    std::priority_queue<Path, std::vector<Path>, PathCompare> candidates;
    
    // Use unordered_set with custom hash for faster lookups
    std::unordered_set<std::vector<int>, VectorHash> seen_paths;
    
    // Find the shortest path
    Path shortest_path = optimized_dijkstra(adj, source, target, {}, {});
    
    // If no path exists, return empty result
    if (shortest_path.nodes.empty() || shortest_path.nodes[0] != source) {
        return result;
    }
    
    // Add the shortest path to the result
    result.push_back(shortest_path);
    seen_paths.insert(shortest_path.nodes);
    
    // Find k-1 more paths
    for (int i = 1; i < k; i++) {
        // The last found shortest path
        const Path& prev_path = result.back();
        
        // For each node in the previous path except the last one
        for (size_t j = 0; j < prev_path.nodes.size() - 1; j++) {
            int spur_node = prev_path.nodes[j];
            std::vector<int> root_path(prev_path.nodes.begin(), prev_path.nodes.begin() + j + 1);
            
            // More efficient removed edges structure (u -> set of v nodes)
            std::unordered_map<int, std::unordered_set<int>> removed_edges;
            std::unordered_set<int> removed_nodes;
            
            // Remove edges that are part of previous shortest paths with the same root
            for (const Path& p : result) {
                if (j < p.nodes.size() - 1 &&
                    std::equal(root_path.begin(), root_path.end(), p.nodes.begin())) {
                    
                    int next_node = p.nodes[j + 1];
                    removed_edges[spur_node].insert(next_node);
                }
            }
            
            // Remove root path nodes (except spur node) to avoid loops
            for (size_t l = 0; l < j; l++) {
                removed_nodes.insert(root_path[l]);
            }
            
            // Calculate the spur path
            Path spur_path = optimized_dijkstra(adj, spur_node, target, removed_edges, removed_nodes);
            
            if (!spur_path.nodes.empty() && spur_path.nodes[0] == spur_node) {
                // Complete path: root_path + spur_path (minus the first node of spur_path which is already in root_path)
                Path total_path;
                total_path.nodes = root_path;
                total_path.nodes.insert(total_path.nodes.end(), 
                                     spur_path.nodes.begin() + 1, 
                                     spur_path.nodes.end());
                
                // Calculate total cost directly
                total_path.cost = calculate_path_cost(adj, total_path.nodes);
                
                // Add the new path to candidates if we haven't seen it yet
                if (seen_paths.count(total_path.nodes) == 0) {
                    candidates.push(total_path);
                    seen_paths.insert(total_path.nodes);
                }
            }
        }
        
        // No more candidates available
        if (candidates.empty()) {
            break;
        }
        
        // Add the best candidate to the result
        Path next_best = candidates.top();
        candidates.pop();
        result.push_back(next_best);
    }
    
    return result;
}

// Batch processing of requests for better cache locality
std::vector<std::vector<std::vector<Path>>> precompute_k_shortest_paths(
    const std::vector<std::vector<int>>& topo,
    const std::vector<std::pair<int, int>>& links,
    const std::vector<int>& link_delays,
    int num_nodes, int k) {
    
    std::vector<std::vector<std::vector<Path>>> paths(num_nodes, 
        std::vector<std::vector<Path>>(num_nodes));
    
    std::cout << "Precomputing k=" << k << " shortest paths for all pairs..." << std::endl;
    
    // Create adjacency list once for all computations
    auto adj = convert_to_adjacency_list(topo, links, link_delays);
    
    // Process in batches for better cache locality
    const int BATCH_SIZE = 16;
    
    // For each batch of source nodes
    for (int src_batch = 0; src_batch < num_nodes; src_batch += BATCH_SIZE) {
        int src_end = std::min(src_batch + BATCH_SIZE, num_nodes);
        
        #pragma omp parallel for schedule(dynamic)
        for (int src = src_batch; src < src_end; src++) {
            for (int dest = 0; dest < num_nodes; dest++) {
                if (src != dest) {
                    paths[src][dest] = optimized_yen_ksp(topo, links, link_delays, src, dest, k);
                }
            }
        }
        
        // Progress report
        std::cout << "  Completed " << src_end << "/" << num_nodes << " source nodes" << std::endl;
    }
    
    return paths;
}

#endif // OPTIMIZED_YEN_KSP_HPP