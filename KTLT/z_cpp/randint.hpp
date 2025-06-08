std::mt19937_64 rng((unsigned int) std::chrono::steady_clock::now().time_since_epoch().count());
int randint(int l, int r) {
    assert(l <= r);
    return std::uniform_int_distribution<>(l, r)(rng);
}