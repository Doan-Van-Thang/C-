#include<bits/stdc++.h>
using namespace std;

const int maxN = 5001;

int n, m, C[maxN], D[maxN];
vector<int> adj[maxN];
int dist[maxN][maxN];

void input() {
    cin >> n >> m;
    for (int i = 1; i <= n; i++) {
        cin >> C[i] >> D[i];
    }
    for (int i = 1; i <= m; i++) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u); 
    }
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            dist[i][j] = INT_MAX;
        }
    }
}

void buildGraph(int start) {
    queue<pair<int,int>> q;
    q.push(make_pair(start, 0));
    dist[start][start] = 0;
    while(!q.empty()) {
        auto u = q.front();
        q.pop();
        if (u.second < D[start]) {
            for (auto v : adj[u.first]) {
                if (dist[start][v] > C[start]) {
                    dist[start][v] = C[start];
                    q.push(make_pair(v, u.second + 1));
                }
            }
        }
    }
}

int main() {

    input();
    for (int i = 1; i <= n; i++) {
        buildGraph(i);
    }

    for (int i = 1; i <= n; i++){
        for (int j = 1; j <= n; j++)
            cout << dist[i][j] << " ";
        cout << '\n';
    }

    return 0;

}   