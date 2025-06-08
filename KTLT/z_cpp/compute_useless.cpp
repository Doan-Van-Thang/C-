#include <bits/stdc++.h>

std::mt19937_64 rng((unsigned int) std::chrono::steady_clock::now().time_since_epoch().count());

inline int randint(int l, int r) {
  return std::uniform_int_distribution(l, r)(rng);
}

int floor_sqrt(int n) {
	int v = int(sqrt(n));
	while (v * v < n) v++;
	while (v * v > n) v--;
	return v;
}


int main() {
  const int N = 1000 * 1000;
	const int n = floor_sqrt(N);
  
  for (int _ : std::views::iota(0, 10)) {
    std::vector g(n, std::vector(n, N));
    std::vector<int> a(n, n / 5);
    std::vector<int> idx(n);
    std::ranges::iota(idx, 0);
  
    while (idx.size() > 1) {
      int sz = idx.size();
      int i = randint(0, sz - 1);
      int j = randint(0, sz - 1);
      if (a[idx[i]] == 1) {
        while (j == i) j = randint(0, sz - 1);
      }
      i = idx[i];
      j = idx[j];
      a[i]--;
      a[j]--;
      g[i][j] = g[j][i] = 1;
      auto [first, last] = std::ranges::remove_if(idx, [&](int ii) { return a[ii] == 0; });
      idx.erase(first, last);
    }
    for (int k = 0; k < n; k++) {
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          g[i][j] = std::min(g[i][j], g[i][k] + g[k][j]);
        }
      }
    }
    int one = 0;
    int two = 0;
    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        switch (g[i][j]) {
          case 1: one++; break;
          case 2: two++; break;
        }
      }
    }
    std::cout << double(one) / (n * (n - 1) / 2) << ' ';
    std::cout << double(two) / (n * (n - 1) / 2) << ' ';
    std::cout << double(one) / (n * (n - 1) / 2) + double(two) / (n * (n - 1) / 2) << '\n';
  }

  std::vector<double> dp(n + 1);
  dp[0] = 1;
  for (int _ : std::views::iota(0, n)) {
    std::vector<double> ndp(n + 1);
    for (int k = 1; k <= n; k++) {
      ndp[k] = dp[k] * k / n + dp[k - 1] * (n - k + 1) / n;
    }
    dp = std::move(ndp);
  }
  double expected = 0;
  for (int k = 1; k <= n; k++) {
    expected += k * dp[k];
  }
  std::cout << expected << '\n';
  return 0;
}