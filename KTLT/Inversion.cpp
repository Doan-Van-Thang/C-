#include<bits/stdc++.h>
using namespace std;

const int N = 1000001, Q = 1e9 + 7; 
int n, a[N], temp[N], nb = 0;

void input() {
    ios_base::sync_with_stdio(0);
    cin.tie(0); cout.tie(0);
    
    cin >> n;
    for (int i = 1; i <= n; i++) {
        cin >> a[i];
    }
}

void mergeSort(int left, int right) {
    if (left >= right) return;

    int mid = (left + right) / 2;
    mergeSort(left, mid);
    mergeSort(mid + 1, right);

    int i = left, j = mid + 1, k = left;
    while(i <= mid && j <= right) {
        if (a[i] <= a[j]) {
            temp[k++] = a[i++];
        } else {
            temp[k++] = a[j++];
            nb += (mid -i + 1);
            nb %= Q;
        }
    }

    while (i <= mid) temp[k++] = a[i++];
    while (j <= right) temp[k++] = a[j++];
    for (int i = left; i <= right; i++) a[i] = temp[i];
}

int main() {

    input();
    // for (int i = 1; i <= n; i++) {
    //     for (int j = i+1; j <=n; j++) {
    //         if (a[i] > a[j]) nb++;
    //     }
    // }

    mergeSort(1, n);
    cout << nb;
    return 0;
    
}