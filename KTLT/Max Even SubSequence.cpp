#include<bits/stdc++.h>
using namespace std;

long long n, a, S = 0, mineven = 0, minodd = 1e9, maxeven;

int main() {

    cin >> n;
    for (int i = 1; i <= n; i++) {
        cin >> a;
        S+=a;
        if (S % 2 == 0) {
            mineven = min(S, mineven);
            maxeven = max(maxeven, S-mineven);
        } else {
            minodd = min(S, minodd);
            maxeven = max(maxeven, S-minodd);
        }
    }

    cout << maxeven;
    return 0;
}