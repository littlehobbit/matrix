#include <cstddef>
#include <iostream>
#include <list>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "matrix.h"

int main() {
  using base_type = std::unordered_map<detail::tuple_n_t<std::size_t, 2>, int,
                                       detail::tuple_hasher>;
  Matrix<int, 0, 2, base_type> matrix;

  for (std::size_t col = 0, row = 0; col <= 9; col++, row++) {  // NOLINT
    matrix[row][col] = col;
  }

  for (std::int64_t col = 9, row = 0; row <= 9; col--, row++) {  // NOLINT
    matrix[row][col] = col;
  }

  for (std::size_t row = 1; row <= 8; row++) {    // NOLINT
    for (std::size_t col = 1; col <= 8; col++) {  // NOLINT
      std::cout << matrix[row][col] << ' ';
    }
    std::cout << std::endl;
  }

  std::cout << matrix.size() << std::endl;

  for (const auto &[x, y, val] : matrix) {
    std::cout << x << ' ' << y << ' ' << val << std::endl;
  }
}