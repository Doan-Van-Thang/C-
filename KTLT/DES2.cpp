#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <random>
#include <set>

struct Packet {
    int id;
    int source;
    int destination;
    int timestamp_generated;
};

struct Event {
    int time;
    int from;
    int to;
    Packet packet;

    bool operator<(const Event& other) const {
        return time > other.time; // min-heap
    }
};

struct Edge {
    int to;
    int latency;
};

using Graph = std::vector<std::vector<Edge>>;
using EventQueue = std::priority_queue<Event>;

// Sinh đồ thị ngẫu nhiên
Graph generate_random_graph(int n, int max_latency = 5, float density = 0.3) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> latency_dist(1, max_latency);
    std::uniform_real_distribution<float> prob_dist(0.0, 1.0);

    Graph g(n);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (prob_dist(rng) < density) {
                int latency = latency_dist(rng);
                g[i].push_back({j, latency});
                g[j].push_back({i, latency});
            }
        }
    }
    return g;
}

// Gửi gói tin đến các hàng xóm (trừ router vừa gửi đến)
void forward_packet(int time, const Packet& packet, int current, const Graph& graph, EventQueue& events, int last_hop = -1) {
    for (const auto& edge : graph[current]) {
        if (edge.to != last_hop) {
            Event e;
            e.time = time + edge.latency;
            e.from = current;
            e.to = edge.to;
            e.packet = packet;
            events.push(e);
        }
    }
}

int main() {
    int num_routers = 5;
    Graph graph = generate_random_graph(num_routers);

    // Danh sách gói tin đầu vào
    std::vector<Packet> input_packets = {
        {0, 0, 3, 0},
        {1, 2, 4, 1},
        {2, 1, 3, 2}
    };

    EventQueue event_queue;

    // Đưa các gói tin vào hàng đợi sự kiện
    for (const auto& packet : input_packets) {
        forward_packet(packet.timestamp_generated, packet, packet.source, graph, event_queue);
    }

    // Bảng ghi nhận các sự kiện theo từng giây
    std::map<int, std::vector<Event>> transfers_per_second;

    int max_time = 0;

    while (!event_queue.empty()) {
        Event e = event_queue.top();
        event_queue.pop();

        // Ghi nhận theo thời gian
        transfers_per_second[e.time].push_back(e);
        if (e.time > max_time) max_time = e.time;

        // Nếu chưa đến đích thì tiếp tục gửi
        if (e.to != e.packet.destination) {
            forward_packet(e.time, e.packet, e.to, graph, event_queue, e.from);
        }
    }

    // In thống kê từng giây (kể cả khi không có sự kiện)
    std::cout << "\n===== KẾT QUẢ MÔ PHỎNG =====\n";
    for (int t = 0; t <= max_time; ++t) {
        std::cout << "Giây " << t << ": ";
        if (transfers_per_second.count(t)) {
            for (const auto& e : transfers_per_second[t]) {
                std::cout << "[P" << e.packet.id << " " << e.from << "→" << e.to << "] ";
            }
        } else {
            std::cout << "Không có sự kiện.";
        }
        std::cout << "\n";
    }

    return 0;
}
