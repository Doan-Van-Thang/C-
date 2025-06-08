#include <bits/stdc++.h>

std::mt19937_64 rng((unsigned int) std::chrono::steady_clock::now().time_since_epoch().count());

inline int randint(int l, int r) {
  return std::uniform_int_distribution(l, r)(rng);
}

int floor_sqrt(int n) {
  int v = int(sqrt(n));
	while (v * v < n) v++;
	while (v * v > n) v--;
	return v;
}

auto rand_grid_graph(int n) {
  const int Max_Link = 2;
  std::cout << "Hallo create grid graph!\n";

  std::vector<std::vector<int>> adj(n * n);
  std::set<std::pair<int, int>> st;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int di = -1; di <= 1; di++) for (int dj = -1; dj <= 1; dj++) {
        if ((di + dj) & 1) {
          int ni = i + di;
          int nj = j + dj;
          if (0 <= ni && ni < n && 0 <= nj && nj < n) {
            int u = i * n + j;
            int v = ni * n + nj;
            adj[u].push_back(v);
            st.emplace(std::min(u, v), std::max(u, v));
          }
        }
      }
    }
  }
	std::vector<std::vector<int>> randomLink(n * n);
	std::vector<int> c(n * n);
  std::vector<int> remain(n * n);
  std::ranges::iota(remain, 0);
  for (int u = 0; u < n * n; u++) {
		if (c[u] == Max_Link) continue;

    while (c[u] < Max_Link) {
      int x;
      int pos = 0;
      while (pos < int(remain.size())) {
        x = randint(pos, int(remain.size()) - 1);
        if (remain[x] == u) continue;
        if (st.contains({std::min(u, remain[x]), std::max(u, remain[x])})) {
          std::swap(remain[pos++], remain[x]);
          continue;
        }
        if (c[remain[x]] == Max_Link) {
          std::swap(remain[x], remain.back());
          remain.pop_back();
          continue;
        }
        break;
      }
      st.emplace(std::min(u, remain[x]), std::max(u, remain[x]));
      randomLink[u].push_back(remain[x]);
      randomLink[remain[x]].push_back(u);
      c[remain[x]]++, c[u]++;
    }
  }
  assert(std::ranges::count(c, Max_Link) == c.size());
	return std::pair(adj, randomLink);
}



struct bridge;
struct Pakage;
class Router;
class RandomShortcutGridGraph;
enum class protocol_type {
  none = -1, 
  infoBroad10, 
  infoBroad11, 
  infoBroad2, 
  bridgeBroad, 
	missingBridgeBroad, 
  pakageSwitch
};


struct bridge {
  std::vector<int> ids;
  int Bid = -1;
  int length = 0;
};

struct Pakage {
  protocol_type type;
  int src;
  int srcBid;
  int dest;
  int timeToLive;
  int length = 0;
  std::vector<int> visited;
  bridge br;
  
  int size;
};

class Router {
  static int s_delta, s_Tmax;
  
public:
  static void setDelta(int delta) { s_delta = delta; }
  static int getDelta() { return s_delta; }
  static void setTmax(int Tmax) { s_Tmax = Tmax; }
  static int getTmax() { return s_Tmax; }
  
	private:
  int m_id;
	int m_x, m_y;
	int m_Bid;
  int m_N;
  bool m_is_central;
	std::vector<Router*> m_adj, m_randomLink;
	std::vector<std::tuple<Router*, int, int>> m_localTable;                       					// L[u] [w, id, length]: go to router 'id' = forward to w
	std::vector<std::pair<Router*, bridge>> m_bridgeTable;                        					// 
	std::vector<std::vector<std::pair<Router*, bridge>>> m_B2B;                             // B2B[id] = bridge that connecting (*this)'s block and block 'id'
	std::vector<bridge> m_AllBridges, m_AltSet;
  std::queue<Pakage> m_queue;
	
  bool isAdj(int id1, int id2) { return std::abs(id1 - id2) == 1 || std::abs(id1 - id2) == m_N; }
  
public:
  Router(int id, int x, int y, int Bid, int noBlock, bool is_central) : 
  m_id(id), 
  m_x(x), 
  m_y(y), 
  m_Bid(Bid),
  m_is_central(is_central)
  {
    m_B2B.resize(noBlock);
    m_N = noBlock;
  }
  
	int getBlockId() {
    return m_Bid;
	}
  
  int getId() {
    return m_id;
  }
  
  bool isCentral() const { 
    return m_is_central; 
  }
  
  void addAdj(Router* v) {
    m_adj.push_back(v);
  }
  
  void addRandom(Router* v) {
    m_randomLink.push_back(v);
  }
  
  const decltype(m_localTable)& getLocalTable() const { return m_localTable; }
  const decltype(m_bridgeTable)& getBridgeTable() const { return m_bridgeTable; }
  const decltype(m_adj)& getAdj() const { return m_adj; }
  const decltype(m_AllBridges)& getAllBridges() const { return m_AllBridges; }
  const decltype(m_randomLink)& getLink() const {  return m_randomLink; }
  const decltype(m_B2B)& getB2B() const { return m_B2B; }


public:
  void queue_process() {
    while (not m_queue.empty()) {
      auto p = std::move(m_queue.front());
      m_queue.pop();
      if (p.dest == -1) {
        switch (p.type) {
          case protocol_type::infoBroad10: {
            for (auto neighbor : m_adj) {
              if (std::ranges::find(p.visited, neighbor->getId()) == p.visited.end()) {
                neighbor->receive(this, p);
              }
            }
            break;
          }
          case protocol_type::infoBroad11: {
            auto send = [&](Router* ng) {
              if (std::ranges::find(p.visited, ng->getId()) == p.visited.end()) {
                ng->receive(this, p);
              }
            };
            for (auto rneighbor : m_randomLink) if (rneighbor->getBlockId() == m_Bid) send(rneighbor);
            for (auto rneighbor : m_adj) if (rneighbor->getBlockId() == m_Bid) send(rneighbor);
            break;
          }
          case protocol_type::infoBroad2: {
            for (auto neighbor : m_randomLink) {
              if (std::ranges::find(p.visited, neighbor->getId()) == p.visited.end()) {
                neighbor->receive(this, p);
              }
            }
            if (1 < p.visited.size() && p.visited.size() < 4) {
              for (auto neighbor : m_adj) {
                if (std::ranges::find(p.visited, neighbor->getId()) == p.visited.end()) {
                  neighbor->receive(this, p);
                }
              }
            }
            break;
          }
          default: assert(false && "p must be one of the Broad types!");
        }
      } else {
        switch (p.type) {
          case protocol_type::bridgeBroad: {
            for (auto& [ng, id, _] : m_localTable) {
              if (id == p.dest) {
                ng->receive(this, p);
                break;
              }
            }
            break;
          }
        }
      }
    }
  }
  
  void receive(Router* prev, Pakage p) {
    p.timeToLive--;
    p.length++;
    auto push = [&]() {
      if (p.timeToLive != 0) {
        p.visited.push_back(m_id);
        m_queue.push(std::move(p));
      }
    };
    switch (p.type) {
      case protocol_type::infoBroad10: {
        auto it = std::ranges::find_if(m_localTable, [&](auto& e) { return std::get<1>(e) == p.src; });
        if (it == m_localTable.end()) {
          m_localTable.emplace_back(prev, p.src, p.length);
          push();
        }
        break;
      }
      case protocol_type::infoBroad11: {
        bool can_queue = true;
        bool have = false;
        for (auto& [ng, id, length] : m_localTable) {
          if (id == p.src) {
            have = true;
            if (length <= p.length) {
              can_queue = false;
            } 
            else {
              ng = prev;
              id = p.src;
              length = p.length;
            }
            break;
          }
        }
        if (can_queue) push();
        if (not have) m_localTable.emplace_back(prev, p.src, p.length);
        break;
      }
      case protocol_type::infoBroad2: {
        auto it = std::ranges::find_if(m_AllBridges, [&](auto& e) { return e.ids.back() == p.src; });
        if (it == m_AllBridges.end()) {
          if (p.srcBid != m_id) {
            auto b = p.visited;
            b.push_back(m_id);
            std::ranges::reverse(b);
            m_AllBridges.emplace_back(b, p.srcBid, p.length);
          }
          push();
        }
        break;
      }
      case protocol_type::bridgeBroad: {
        if (m_id == p.dest) {
          auto& [_, Bid, length] = p.br;
          length += p.length;
          m_B2B[Bid].emplace_back(prev, p.br);
        } else {
          m_queue.push(std::move(p));
        }
        break;
      }
      case protocol_type::pakageSwitch: {
        break;
      }
    }
  }

  // for phase 1
  void broadCastP10() {
    Pakage p;
    p.src = m_id;
    p.srcBid = m_Bid;
    p.dest = -1;
    p.timeToLive = s_delta;
    p.length = 0;
    p.type = protocol_type::infoBroad10;
    p.visited.push_back(m_id);
    // p.bridges;
    p.size = -1;
    m_queue.push(std::move(p));
  }
  void broadCastP11() {
    Pakage p;
    p.src = m_id;
    p.srcBid = m_Bid;
    p.dest = -1;
    p.timeToLive = 2 * m_N - 2;
    p.length = 0;
    p.type = protocol_type::infoBroad11;
    p.visited.push_back(m_id);
    p.size = -1;
    m_queue.push(std::move(p));
  }
  
  // for phase 2, find all bridge
  void broadCastP2() {
    Pakage p;
    p.src = m_id;
    p.srcBid = m_Bid;
    p.dest = -1;
    p.timeToLive = 4;
    p.length = 0;
    p.type = protocol_type::infoBroad2;
    p.visited.push_back(m_id);
    p.size = -1;
    m_queue.push(std::move(p));
  }
  void assignToB2B() {
    m_AllBridges.erase(std::remove_if(m_AllBridges.begin(), m_AllBridges.end(), [&](bridge& a) {
      if (a.Bid == m_Bid) return true;
      
      std::vector<bool> m((int)a.ids.size() - 1);
      for (int i = 0; i < m.size(); i++) {
        m[i] = not isAdj(a.ids[i], a.ids[i + 1]);
      }
      return not (m == std::vector{true} || m == std::vector{true, true} || m == std::vector{true, false, true} || m == std::vector{true, false, false, true});
    }), m_AllBridges.end());

    for (auto& a : m_AllBridges) {
      m_B2B[a.Bid].emplace_back(nullptr, a);
    }
  }
  
  void broadCastAllBridges() {
    for (auto& br : m_AllBridges) {
      broadCastBridge(br);
    }
  }
  
  void broadCastBridge(bridge br) {
    Pakage p;
    p.src = m_id;
    p.srcBid = m_Bid;
    p.timeToLive = 2 * m_N - 2;
    p.length = 0;
    p.type = protocol_type::bridgeBroad;
    p.br = br;
    p.size = -1;
    for (auto& [_, id, __] : m_localTable) {
      p.dest = id;
      m_queue.push(p);
    }
  }

  void broadCastMissingBridge() {
		for (int Bid = 0; Bid < m_N; Bid++) {
			if (Bid == m_Bid) continue;
			// if (m_B2B[Bid].empty()) {
			// }
		}
  }
  
  void createBridgeTable() {
    int Tbr = s_Tmax - (int)m_localTable.size();
    assert(Tbr > 0);
    for (auto& bs : m_B2B) {
      std::ranges::sort(bs, [&](auto& a, auto& b) {
        return a.second.length < b.second.length;
      });
    }
    int i = 0;
    while (Tbr > 0) {
      bool added = false;
      for (auto& bs : m_B2B) {
        if (i < int(bs.size())) m_bridgeTable.push_back(bs[i]), Tbr--, added = true;
        if (Tbr == 0) break;
      }
      if (not added) break;
      i++;
    }
  }
};
int Router::s_delta;
int Router::s_Tmax;

class RandomShortcutGridGraph { // distributed mode
private:
  int n;																					
	int l, m;
	int delta, Tmax;
  std::vector<Router*> g;

public:
	RandomShortcutGridGraph(int _n, int _delta, int _Tmax) : n(_n), delta(_delta), Tmax(_Tmax) {
    l = m = floor_sqrt(n);
    assert(l * m == n);
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        g.push_back(new Router(i * n + j, i, j, i / l * l + j / l, n, (i % l == l / 2 && j % l == l / 2)));
      }
    }
    auto [gg, gr] = rand_grid_graph(n);
    for (int u = 0; u < n * n; u++) {
      for (int v : gg[u]) {
        g[u]->addAdj(g[v]);
      }
      for (int v: gr[u]) {
        g[u]->addRandom(g[v]);
      }
    }
    Router::setDelta(delta);
    Router::setTmax(Tmax);
  }

  void phase1() {
    auto t = clock();
    std::cout << "Phase 1 is running...\n";
    
    std::cout << "Working on Bu...\n";
    for (Router* r : g) r->broadCastP11();
    for (int _ : std::views::iota(0, 2 * l)) for (Router* r : g) r->queue_process();
    
    std::cout << "Working on N_delta_u...\n";
    for (Router* r : g) r->broadCastP10();
    for (int _ : std::views::iota(0, delta)) for (Router* r : g) r->queue_process();
    
    std::cout << "Done phase 1!\nTime: " << double(clock() - t) / CLOCKS_PER_SEC << '\n';
    std::cout << '\n';
  }
  
  void phase2() {
    auto t = clock();
    std::cout << "Phase 2 is running...\n";
    
    std::cout << "Find all bridge...\n";
    for (Router* r : g) r->broadCastP2();
    for (int _ : std::views::iota(0, 4)) for (Router* r : g) r->queue_process();
    for (Router* r : g) r->assignToB2B(), r->broadCastAllBridges();
    for (int _ : std::views::iota(0, n)) for (Router* r : g) r->queue_process();

    std::cout << "Handle bridge missing...\n";
    for (Router* r : g) if (r->isCentral()) r->broadCastMissingBridge();

    std::cout << "Construct bridge table...\n";
    for (Router* r : g) r->createBridgeTable();

    std::cout << "Done phase 2!\nTime: " << double(clock() - t) / CLOCKS_PER_SEC << '\n';
    std::cout << '\n';
  }

  void trace(int u, int v) {
    std::cout << "Trace " << u << ' ' << v << ": ";
    int Vid = g[v]->getBlockId();
    std::cerr << "\nBridge routing: ";
    if (g[u]->getBlockId() != Vid) {
      std::cout << u << ' ';
      while (g[u]->getBlockId() != Vid) {
        while (true) {
          bool found = false;
          for (const auto& [ng, br] : g[u]->getBridgeTable()) {
            if (br.Bid == Vid) {
              if (ng != nullptr) {
                u = ng->getId();
                std::cout << u << ' ';
                found = true;
              } else {
                for (int i : br.ids | std::views::drop(1)) {
                  std::cout << i << ' ';
                }
                u = br.ids.back();
              }
              break;
            }
          }
          if (not found) break;
        }
      }
    }
    std::cerr << "\nLocal rounting: ";
    while (u != v) {
      for (auto [ng, id, _] : g[u]->getLocalTable()) {
        if (id == v) {
          u = ng->getId();
          break;
        }
      }
      std::cout << u << ' ';
    }
  }
};


int main() {
	const int N = 64 * 64;
	const int n = floor_sqrt(N);
  assert(n * n == N);

	RandomShortcutGridGraph g(n, 3, 500);

  g.phase1();
  g.phase2();

  // for (int _ : std::views::iota(0, 1000)) {
  //   g.trace(randint(0, N / 2 - 1), randint(N / 2, N - 1));
  //   std::cerr << "\n\n";
  // }
  return 0;
}