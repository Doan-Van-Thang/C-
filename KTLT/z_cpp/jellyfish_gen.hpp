// n: số switch, k: số cổng mỗi đỉnh, r: số cổng sử dụng
std::vector<std::vector<int>> regular_graph_gen(int n, int k, int r) {
    assert(k >= r);

    std::vector<std::vector<int>> g(n);
    std::vector<int> remain(n);
    std::ranges::iota(remain, 0);

    for (int u = 0; u < n; u++) {
        while (g[u].size() < r) {
            int sz = int(remain.size());
            int i = randint(0, sz - 1);

            while (remain[i] == u || std::ranges::find(g[u], remain[i]) != g[u].end() || g[remain[i]].size() == r) {
                std::swap(remain[i], remain[sz - 1]);
                sz--;

                if (g[remain[sz]].size() == r) {
                    std::swap(remain[sz], remain.back());
                    remain.pop_back();
                }

                if (sz == 0) {
                    break;
                }

                i = randint(0, sz - 1);
            }

            if (sz == 0) {
                break;
            }
            
            assert(g[remain[i]].size() + 1 <= r);
            g[u].push_back(remain[i]);
            g[remain[i]].push_back(u);
        }
    }

    std::ranges::all_of(g, [&](const auto& e) { return e.size() <= r; });
    return g;
}

// n: số switch, k: số cổng mỗi đỉnh, r: số cổng sử dụng switch - swtich, m: số server
std::vector<std::vector<int>> jellyfish_gen(int n, int k, int r, int m) {
    assert(m <= n * (k - r));

    auto g = regular_graph_gen(n, k, r);
    g.resize(n + m);
    std::vector<int> remain(n);
    std::ranges::iota(remain, 0);

    for (int u = n; u < n + m; u++) {
        int i = randint(0, int(remain.size()) - 1);

        while (g[remain[i]].size() == k) {
            std::swap(remain[i], remain.back());
            remain.pop_back();

            if (remain.empty()) {
                break;
            }

            i = randint(0, int(remain.size()) - 1);
        }
        
        if (remain.empty()) {
            assert(false);
            // break;
        }
        g[u].push_back(remain[i]);
        g[remain[i]].push_back(u);
    }
    return g;
}
