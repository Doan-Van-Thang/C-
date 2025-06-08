#include<bits/stdc++.h>
using namespace std;

const int maxN = 1000001;

int N, M, T = 0, bridges, numjoint;
vector<vector<int>> adj(maxN);
vector<int> Num(maxN);
vector<int> Low(maxN);
vector<bool> joint(maxN);

void DFS(int u, int pre) {

    int child = 0;
    T++; Num[u] = T; Low[u] = T;
    for (auto v : adj[u]) {
        if (v == pre) continue;
        if (Num[v] > 0) {
            Low[u] = min(Low[u], Num[v]);
        } else {
            DFS(v, u);
            Low[u] = min(Low[u], Low[v]);
            if (Low[v] > Num[u]) bridges++;
            child++;
            if (u == pre) {
                if (child > 1) joint[u] = true;
            } else {
                if (Low[v] >= Num[u]) joint[u] = true;
            }
        }
    }

}

int main() {

    ios_base::sync_with_stdio(0);
    cin.tie(0); cout.tie(0);

    cin >> N >> M;

    for (int i = 0; i < M; i++) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    for (int i = 1; i <= N; i++) {
        if (Num[i] == 0) {
            DFS(i, i);
        }
    }

    for (int i = 1; i <= N; i++) {
        if (joint[i]) numjoint++;
    }

    cout << numjoint << " " << bridges;

    return 0;
}