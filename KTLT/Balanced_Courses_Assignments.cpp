#include<bits/stdc++.h>
using namespace std;

const int mMax = 11, nMax = 31; 
int m, n, favourite[mMax][nMax], conflict[nMax][nMax];
int sol[nMax];
int nbTeach[mMax];
int maxLoad = 10000000;

bool check(int i, int k) {
    if (favourite[i][k] == 0) return false;
    for (int j = 1; j < k; j++) {
        if (conflict[k][j] == 1 && sol[j] == i) return false;
    }
    return true;
}

void Try(int k) {
    for (int i = 1; i <= m; i++) {
        if (check(i, k)) {
            sol[k] = i;
            nbTeach[i]++;
            
            if (k == n) {
                int loadcurrent = 0;
                for (int j = 1; j <= m; j++) {
                    loadcurrent = max(loadcurrent, nbTeach[j]);
                }
                maxLoad = min(loadcurrent, maxLoad);
            } 
            else {
                if (nbTeach[i] < maxLoad) {
                    Try(k+1);
                }
            }

            nbTeach[i]--;
        }   
    }
}

int main() {

    //input
    cin >> m >> n;
    for (int i = 1; i <= m; i++) {
        int numberFavourite;
        cin >> numberFavourite;
        for (int j = 1; j <= numberFavourite; j++) {
            int classFavourite;
            cin >> classFavourite;
            favourite[i][classFavourite] = 1;
        }
    }
    int k;
    cin >> k;
    for (int i = 0; i < k; i++) {
        int class1, class2;
        cin >> class1 >> class2;
        conflict[class1][class2] = 1;
        conflict[class2][class1] = 1;
    }

    //output
    Try(1);
    cout << maxLoad;
    return 0;

}