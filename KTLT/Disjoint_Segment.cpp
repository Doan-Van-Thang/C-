#include<bits/stdc++.h>
using namespace std;

typedef pair<int, int> ii;
vector<ii> ab;
int a, ans = 0;

void input() {
    ios_base:: sync_with_stdio(0);
    cin.tie(0); cout.tie(0);

    cin >> a;
    ab.resize(a);

    for (int i = 0; i < a; i++) {
        cin >> ab[i].first >> ab[i].second;
    }
}

int main() {

    input();

    int last = -1;
    sort(ab.begin(), ab.end(), [](ii a, ii b) {
        return a.second < b.second;
    });

    for (int i = 0; i < a; i++) {
        if (ab[i].first > last) {
            last = ab[i].second;
            ans++;
        }
    }

    cout << ans;

    return 0;
}