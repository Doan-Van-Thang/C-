#include<iostream>
using namespace std;

long t;

int main() {

    cin >> t;

    for (long i = 0; i < t; i++) {
        long y, x;
        cin >> y >> x;
        x--; y--;
        if (x == 0 && y == 0) {
            cout << 1 << endl;
        }
        else if (y>=x) {
            if (y % 2 == 0) {
                cout << y*y + x + 1 << endl;
            } else {
                cout << (y+1)*(y+1) - x << endl;
            }
        } else {
            if (x % 2 == 0) {
                cout << (x+1)*(x+1) - y << endl;
            }
            else {
                cout << x*x + y + 1<< endl;
            }
        }
    }

    return 0;

}