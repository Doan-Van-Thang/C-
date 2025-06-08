#include<bits/stdc++.h>
using namespace std;

const int maxN = 1000;
typedef pair<int, int> ii;

int n,m,r,c,a[maxN][maxN];
int visited[maxN][maxN] = {0};
int dist[maxN][maxN] = {0};
queue<ii> qe;
int dx[] = {1, 0, -1, 0};
int dy[] = {0, 1, 0, -1};

int BFS() {
    
    while (!qe.empty()) {
        ii cell = qe.front();
        qe.pop();
        int x = cell.first;
        int y = cell.second;

        if (x == 1|| y == 1 || x == n || y == m) {
            return dist[x][y];
        }

        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (a[nx][ny] == 0 && visited[nx][ny] == 0) {
                visited[nx][ny] = 1;
                dist[nx][ny] = dist[x][y] + 1;
                qe.push({nx,ny});
            }
        }
    }
    return -1;

}

int main()
{
    //dau vao
    cin >> n >> m >> r >> c;
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            cin >> a[i][j];
        }
    }

    //xu ly
    qe.push({r,c});
    dist[r][c] = 1;
    cout << BFS();

    return 0;
}