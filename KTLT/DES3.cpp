#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
#include <unordered_map>
#include <random>
using namespace std;

struct Packet {
    int id;
    int source;
    int destination;
    int current;
    vector<int> path;
    int pathIndex = 0;
    bool arrived = false;
};

vector<int> dijkstra(const vector<vector<int>>& graph, int src, int dst) {
    int n = graph.size();
    vector<int> dist(n, INT_MAX), prev(n, -1);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;

    dist[src] = 0;
    pq.emplace(0, src);

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (u == dst) break;
        for (int v : graph[u]) {
            if (dist[u] + 1 < dist[v]) {
                dist[v] = dist[u] + 1;
                prev[v] = u;
                pq.emplace(dist[v], v);
            }
        }
    }

    vector<int> path;
    for (int at = dst; at != -1; at = prev[at])
        path.push_back(at);
    reverse(path.begin(), path.end());
    return (path.front() == src) ? path : vector<int>{};
}

int main() {
    const int NUM_NODES = 10;
    vector<vector<int>> graph(NUM_NODES);

    // Example topology (undirected graph)
    graph[0] = {1, 2};
    graph[1] = {0, 3};
    graph[2] = {0, 3, 4};
    graph[3] = {1, 2, 5};
    graph[4] = {2, 5, 6};
    graph[5] = {3, 4, 7};
    graph[6] = {4, 7};
    graph[7] = {5, 6, 8, 9};
    graph[8] = {7};
    graph[9] = {7};

    vector<int> packetGenerationTimes = {0, 1, 2, 4, 5, 6};
    vector<Packet> packets;
    int time = 0;
    int maxTime = 30;
    int packetId = 0;

    while (time <= maxTime) {
        // Generate packets at specified times
        for (int genTime : packetGenerationTimes) {
            if (genTime == time) {
                int src = rand() % NUM_NODES;
                int dst = rand() % NUM_NODES;
                while (dst == src) dst = rand() % NUM_NODES;

                Packet p;
                p.id = packetId++;
                p.source = src;
                p.destination = dst;
                p.current = src;
                p.path = dijkstra(graph, src, dst);

                if (!p.path.empty()) {
                    packets.push_back(p);
                    cout << "Time " << time << ": [P" << p.id << " generated at R" << p.source << "]\n";
                }
            }
        }

        vector<string> events;
        for (auto& p : packets) {
            if (p.arrived || p.path.empty()) continue;

            if (p.pathIndex + 1 < p.path.size()) {
                int from = p.path[p.pathIndex];
                int to = p.path[p.pathIndex + 1];
                events.push_back("[P" + to_string(p.id) + " " + to_string(from) + "→" + to_string(to) + " sending]");
                p.current = to;
                p.pathIndex++;
                if (to == p.destination) {
                    events.push_back("[P" + to_string(p.id) + " arrived at R" + to_string(p.destination) + "]");
                    p.arrived = true;
                }
            }
        }

        if (!events.empty()) {
            cout << "Time " << time << ": ";
            for (const string& e : events) cout << e << " ";
            cout << "\n";
        }

        ++time;
    }

    return 0;
}