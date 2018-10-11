#pragma once

#include <random>

template <typename T>
struct rng {
  std::mt19937_64 generator;
  std::uniform_int_distribution<T> distribution;
  int count;

  rng(unsigned int seed, T from, T to, int count) : distribution(from, to), count(count) {
    generator.seed(seed);
  }

  struct iterator {
    rng& owner;
    int count;

    bool operator != (iterator const& rhs) const {
      return rhs.count != count;
    }

    iterator& operator++() { --count; return *this; }

    T operator* () { return owner.distribution(owner.generator); }

  };

  iterator begin() { return {*this, count}; }
  iterator end() { return {*this, 0}; }
};
