#include<bits/stdc++.h>
using namespace std;

const int Max = 30;
int n, K;
int c[Max][Max], cMin = 1e9;
int x[Max]; 
int appear[Max];    //kiem tra xem xe da di den noi nao
int load = 0;           // so hanh khach hien tai tren xe
int result = 1e9;
int dist = 0;

void input() {
    cin >> n >> K;
    for (int i = 0; i <= 2 * n; i++) {
        for (int j = 0; j <= 2 * n; j++) {
            cin >> c[i][j];
            if (i != j) {
                cMin = min(cMin, c[i][j]);
            }
        }
    }
}

bool check(int v, int k) {
    if (appear[v] == 1) return false;
    if (v > n) {
        if (appear[v - n] == 0) return false;
    } 
    else {
        if (load >= K) return false;
    }
    return true;
    
}

void Try(int k) {
    for (int v = 1; v <= 2*n; v++) {
        if (check(v, k)) {
            x[k] = v;
            appear[v] = 1;
            dist += c[x[k-1]][x[k]];
            if (v <= n) load++;
            else load--;
            if (k == 2*n) {
                result = min(result, dist + c[x[k]][x[0]]);
            }
            else {
                if (dist + cMin * (2*n - k) < result) {
                    Try(k+1);
                }
            }
            appear[v] = 0;
            dist -= c[x[k-1]][x[k]];
            if (v <= n) load--;
            else load++;
        }
    }
}

int main() {

    input();

    Try(1);
    cout << result;

    return 0;
}