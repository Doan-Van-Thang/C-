// #include<bits/stdc++.h>
// using namespace std;

// const int maxN = 1000001;
// int n, a[maxN], L1, L2, S[maxN];
// int MaxAlpha;

// int main() {

//     ios_base::sync_with_stdio(0);
//     cin.tie(0); cout.tie(0);
//     cin >> n >> L1 >> L2;
//     for (int i = 1; i <= n; i++) {
//         cin >> a[i];
//         S[i] = a[i];
//     }

//     for (int i = L1+1; i<= n; i++) {
//         for (int j = i-L2; j <= i-L1; j++) {
//             if (j > 0) {
//                 S[i] = max(S[i], S[j] + a[i]);
//             }
//         }
//     }

//     MaxAlpha = S[1];
//     for (int i = 2; i<= n; i++) {
//         MaxAlpha = max(MaxAlpha, S[i]);
//     }
//     cout << MaxAlpha;

//     return 0;

// }

#include<bits/stdc++.h>
using namespace std;

const int maxN = 1000001;
int n, L1, L2, a[maxN], S[maxN];

int main() {

    ios_base::sync_with_stdio(0);
    cin.tie(0); cout.tie(0);
    cin >> n >> L1 >> L2;
    for (int i = 1; i <= n; i++) {
        cin >> a[i];
    }

    deque<int> d;
    int ans = 0;
    
    for (int i = 1; i <= n; i++) {
        while ((!d.empty()) && d.front() < i - L2) d.pop_front();
        int j = i - L1;
        if (j >= 1) {
            while ((!d.empty()) && (S[d.back()] < S[j])) d.pop_back();
            d.push_back(j);
        }
        S[i] = a[i] + (d.empty() ? 0 : S[d.front()]);
        ans = max(ans, S[i]);
    }

    cout << ans;
    return 0;

}