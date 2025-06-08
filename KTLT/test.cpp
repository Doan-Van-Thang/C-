#include<bits/stdc++.h>
using namespace std;

const int maxN = 1e6 + 1;
int n, M, result, tong;
int a[maxN];

void input() {
    cin >> n >> M;
    for (int i = 1; i <= n; i++) {
        cin >> a[i];
    }
}

void Try(int k) {
    for (int i = 1; i <= (M-tong)/a[k]; i++) {
        tong += a[k] * i;
        if (k == n) {
            if (tong == M) result++;
        } else {
            Try(k+1);
        }
        tong -= a[k] * i;
    }
}

int main() {
    input();
    Try(1);
    cout << result;
    return 0;
}