#include<bits/stdc++.h>
using namespace std;

const int maxN = 100001;   

int T, N, C, d;
int a[maxN];

void check(int d) {
    int last = a[1];
    int countC = 1;
    for (int i = 2; i <= N; i++) {
        if ((a[i] - last) >= d) {
            countC++;
            last = a[i];
        }
    }
    if (countC++ >= C) {
        cout << d << endl;
    }
    else check(d-1);
}

int main() {

    cin >> T;
    for (int i = 0; i<T; i++) {
        cin >> N >> C;
        for (int j = 1; j <= N; j++) {
            cin >> a[j];
        }
        sort(a+1, a+N+1);
    
        d = (a[N] - a[1]) / (C-1);
        check(d);
    }

    return 0;

}