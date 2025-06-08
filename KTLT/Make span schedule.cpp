#include<bits/stdc++.h>
using namespace std;

const int maxN = 1e4 + 1;
int n, m, result;
int t[maxN], bac_vao[maxN], dist[maxN];
vector<int> adj[maxN];

void input() {
    cin >> n >> m;
    for (int i = 1; i <= n; i++) {
        cin >> t[i];
    }
    for (int i = 0; i < m; i++) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        bac_vao[v]++;
    }
}

int main() {

    input();

    queue<int> q;
    for (int i = 1; i <= n; i++) {
        if (bac_vao[i] == 0) {
            dist[i] = 0;
            q.push(i);
        }
    }

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : adj[u]) {
            dist[v] = max(dist[v], dist[u] + t[u]);
            bac_vao[v]--;
            if (bac_vao[v] == 0) {
                q.push(v);
            }
        }
    }

    for (int i = 1; i <= n; i++) {
        result = max(result, dist[i] + t[i]);
    }

    cout << result;

    return 0;

}