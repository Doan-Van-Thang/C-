#include<bits/stdc++.h>
using namespace std;

const long nMax = 100;
long n, M, tong = 0, result = 0;
long a[nMax], X[nMax];

void Try(long k) {
    long i = 1;
    do {
        X[k] = i;
        tong += a[k] * X[k];
        if (k == n) {
            if (tong == M) {
                result++;
            }
        }
        else {
            Try(k + 1);
        }
        i++;
        tong -= a[k] * X[k];
    } while (i <= (M - n + k - tong)/a[k]);
}

int main() {

    cin >> n >> M;
    for (int i = 1; i <= n; i++) {
        cin >> a[i];
    }

    Try(1);
    cout << result;
    return 0;

}