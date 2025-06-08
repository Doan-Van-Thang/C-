#include<bits/stdc++.h>
using namespace std;

int main() {
    long n, m;
    cin >> n;
    long a[n], M[n][21];
    for (long i = 0; i < n; i++) {
        cin >> a[i];
        M[i][0] = i;
        
    }

    for (long j = 1; (1<<j) <= n; j++) {
        for (long i = 0; i + (1<<j) - 1 < n; i++) {
            if (a[M[i][j-1]] < a[M[i+(1<<(j-1))][j-1]]) {
                M[i][j] = M[i][j-1];
            } else {
                M[i][j] = M[i+(1<<(j-1))][j-1];
            }
        }
    }

    cin >> m;
    long sum = 0;
    for (long q = 0; q < m; q++) {
        long i, j;
        cin >> i >> j;
        long k = log2(j - i + 1);
        sum += min(a[M[i][k]], a[M[j - (1<<k) + 1][k]]);
    }
    cout << sum;
    return 0;
}