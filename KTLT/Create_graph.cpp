#include <iostream>
#include <vector>
#include <random>
#include <set>
using namespace std;

struct Packet {
    int id;
    int source; //gốc
    int destination;    // đích
    int timestamp_generated;    //mốc tg
    std::set<int> visited_routers;
};

struct Event {
    int time;
    int from;
    int to;
    Packet packet;
    enum Type { GENERATE, SEND, ARRIVE } type;

    bool operator<(const Event& o) const {
        return time > o.time; // min‑heap
    }
};

struct Edge {
    int to;
    int latency;
};

using Graph = vector<vector<Edge>>;

int main() {
    int NUM_ROUTERS = 10;
    double p = 0.3;  // xác suất tạo cạnh giữa 2 đỉnh
    Graph graph(NUM_ROUTERS);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist_prob(0.0, 1.0);
    uniform_int_distribution<> dist_weight(1, 5);

    for (int u = 0; u < NUM_ROUTERS; ++u) {
        for (int v = u + 1; v < NUM_ROUTERS; ++v) {
            if (dist_prob(gen) < p) {
                int w = dist_weight(gen);
                graph[u].push_back({v, w});
                graph[v].push_back({u, w});  // đồ thị vô hướng
            }
        }
    }

    // In đồ thị
    for (int u = 0; u < NUM_ROUTERS; ++u) {
        cout << "Dinh " << u << " ke voi:\n";
        for (auto &[v, w] : graph[u]) {
            cout << "  -> Dinh " << v << " voi trong so " << w << '\n';
        }
    }
}
