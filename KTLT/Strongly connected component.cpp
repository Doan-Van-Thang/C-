#include<bits/stdc++.h>
using namespace std;

const int maxN = 200001;
int n, m, result;
int d[maxN];
vector<int> G[maxN];
vector<int> GT[maxN];
vector<bool> visited;
vector<int> order;

void input() {
    ios_base::sync_with_stdio(0);
    cin.tie(0);
    cin >> n >> m;

    for (int i = 1; i <= m; i++) {
        int u, v;
        cin >> u >> v;
        G[u].push_back(v);
        GT[v].push_back(u);
    }
}

void dfs1(int u) {
    visited[u] = true;
    for (auto v : G[u])
        if (!visited[v])    
            dfs1(v);

    order.push_back(u);
}

void dfs2(int u) {
    visited[u] = true;
    for (auto v : GT[u])
        if (!visited[v])    
            dfs2(v);
}



int main() {

    input();
    visited.assign(n+1, false);
    for (int i = 1; i <= n; i++) {
        if (!visited[i]) 
            dfs1(i);
    }

    visited.assign(n+1, false);
    reverse(order.begin(), order.end());

    for (int i : order) {
        if (!visited[i]) {
            dfs2(i);
            result++;
        }
    }

    cout << result;
    return 0;

}