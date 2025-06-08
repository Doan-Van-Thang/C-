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
                int lat = latency_dist(rng);
                g[i].push_back({j, lat});
                g[j].push_back({i, lat});
            }
        }
    }
    return g;
}

// Forward packet, chặn gửi trùng từ cùng router
void forward_packet(int t, Packet packet, int current, const Graph& G, EventQueue& Q,
                    std::map<int,std::set<int>>& router_sent) {
    // Nếu router này đã gửi packet.id thì dừng
    if (router_sent[current].count(packet.id)) return;
    router_sent[current].insert(packet.id);
    packet.visited_routers.insert(current);

    for (auto &e: G[current]) {
        if (packet.visited_routers.count(e.to)) continue;
        Event ev;
        ev.time    = t + e.latency;
        ev.from    = current;
        ev.to      = e.to;
        ev.packet  = packet;
        ev.type    = (e.to == packet.destination
                      ? Event::ARRIVE
                      : Event::SEND);
        Q.push(ev);
    }
}

int main(){
    int N = 10;
    Graph graph = generate_random_graph(N, 5, 0.4);

    std::vector<Packet> in = {
        {0,0,3,0,{}},
        {1,2,4,1,{}},
        {2,1,3,2,{}}
    };

    EventQueue Q;
    for(auto &p: in){
        Q.push({p.timestamp_generated, p.source, p.source, p, Event::GENERATE});
    }

    std::map<int,std::vector<std::string>> log;
    std::map<int,std::set<int>> router_sent;                // chặn SEND trùng
    std::set<std::pair<int,int>> packet_delivered;         // chặn ARRIVE trùng
    int maxt = 0;

    while(!Q.empty()){
        auto e = Q.top(); Q.pop();
        maxt = std::max(maxt, e.time);

        switch(e.type){
            case Event::GENERATE:
                log[e.time].push_back(
                  "[P"+std::to_string(e.packet.id)+" generated at R"+std::to_string(e.from)+"]"
                );
                forward_packet(e.time, e.packet, e.from, graph, Q, router_sent);
                break;

            case Event::SEND:
                log[e.time].push_back(
                  "[P"+std::to_string(e.packet.id)+" "+ 
                   std::to_string(e.from)+"→"+std::to_string(e.to)+" sending]"
                );
                forward_packet(e.time, e.packet, e.to, graph, Q, router_sent);
                break;

            case Event::ARRIVE:
                // chỉ log lần đầu đến đích
                if (!packet_delivered.count({e.packet.id,e.to})) {
                    packet_delivered.insert({e.packet.id,e.to});
                    log[e.time].push_back(
                      "[P"+std::to_string(e.packet.id)+" arrived at R"+std::to_string(e.to)+"]"
                    );
                }
                break;
        }
    }

    // In kết quả
    for(int t=0;t<=maxt;++t){
        std::cout<<"Time "<<t<<": ";
        if(log.count(t)){
            for(auto &s: log[t]) std::cout<<s<<" ";
        }
        else std::cout<<"No activity";
        std::cout<<"\n";
    }
    return 0;
}
