#include<bits/stdc++.h>
using namespace std;

const int maxN = 1001;
int n, T, D, a[maxN], t[maxN], S[maxN][maxN], ans = 0;

int main() {

    cin >> n >> T >> D;
    for (int i = 1; i <= n; i++) {
        cin >> a[i];
    }
    for (int i = 1; i <= n; i++) {
        cin >> t[i];
    }

    ans = a[1]; S[1][t[1]] = a[1];

    for (int i = 1; i <= n; i++) {
        for (int k = 1; k <= T; k++) {
            if (k >= t[i]) {
                for (int j = 1; j <= D && j <= i; j++) {
                    S[i][k] = max(S[i][k], S[i-j][k-t[i]]+a[i]);
                }
                ans = max(ans, S[i][k]);
            } else S[i][k] = 0;
        }
    }

    cout << ans;
    return 0;
}