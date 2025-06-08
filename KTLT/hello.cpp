#include<iostream>
using namespace std;

int main() {
    long n;
    cin >> n;
    if (n == 1) {
        cout << 1;
    } 
    else if (n == 2 || n == 3) {
        cout << "NO SOLUTION";
    }
    else {
        long even, odd;
        if (n % 2 == 0) {
            even = n;
            odd = n-1;
        }
        else {
            odd = n;
            even = n-1;
        }

        for (long i = 2; i <= even; i += 2) {
            cout << i << " ";
        }
        for (long i = 1; i <= odd; i += 2) {
            cout << i << " ";
        }
    }
    return 0;
}