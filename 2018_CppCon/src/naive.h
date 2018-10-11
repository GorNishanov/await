#pragma once

template <typename Iterator>
bool naive_binary_search(Iterator first, Iterator last, int val) {
  auto len = last - first;
  while (len > 0) {
    const auto half = len / 2;
    const auto middle = first + half;
    const auto middle_key = *middle;
    if (middle_key < val) {
      first = middle + 1;
      len = len - half - 1;
    } else {
      len = half;
    }
    if (middle_key == val)
      return true;
  }
  return false;
}
