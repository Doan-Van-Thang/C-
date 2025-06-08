#include<bits/stdc++.h>
using namespace std;

const int maxN = 1000;
int n,m;
int a[maxN][maxN];

int maxH(int hang[]) {
    
    stack<int> result;

    int top, maxS =0, S=0, i=0; 
	while (i < m) {
		if (result.empty() || hang[result.top()] <= hang[i]) {
            result.push(i++);
        }
		else {			
			top = hang[result.top()];
			result.pop();
			S = top * i;
			if (!result.empty())
				S = top * (i - result.top() - 1);
			if (S> maxS) maxS = S;
		}
	}
	while (!result.empty()) {
		top = hang[result.top()];
		result.pop();
		S = top * i;
		if (!result.empty())
			S = top * (i - result.top() - 1);
		if (S> maxS) maxS = S;
	}
	return maxS;
}

int main() {
    cin >> n >> m;

    for (int i = 0; i < n; i++) {
        cin >> a[0][i];
    }

    for (int i = 1; i < n; i++) {
        for (int j = 0 ; j < m; j++) {
            cin >> a[i][j];
            if (a[i][j] != 0) {
                a[i][j] = a[i-1][j] + 1;
            }
        }
    }



    // // check chuan hoa mang
    // for (int i = 0; i < n; i++) {
    //     cout << endl;
    //     for (int j = 0; j < m; j++) {
    //         cout << a[i][j] << " ";
    //     }
    // }

    int max_current = maxH(a[0]);
    for (int i = 1; i < n; i++) {
        max_current = max(max_current, maxH(a[i]));
    }

    cout << max_current;
    
    return 0;
}