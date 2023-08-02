#include <algorithm>
#include <deque>
#include <iterator>
#include <list>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <gtest/gtest.h>

#include "matrix.h"

class MatrixTest : public ::testing::Test {
 public:
  static constexpr auto DEFAUT_VALUE = 42;
  Matrix<int, DEFAUT_VALUE> matrix{};
};

TEST_F(MatrixTest, ByDefaultIsEmpty) {  // NOLINT
  ASSERT_EQ(matrix.size(), 0);
  ASSERT_TRUE(matrix.empty());
}

TEST_F(MatrixTest, IfNoValue_GetDefault_AndThenAssign) {  // NOLINT
  auto val = matrix[0][0];
  ASSERT_EQ(val, DEFAUT_VALUE);
  ASSERT_EQ(matrix.size(), 0);

  val = 1;
  ASSERT_EQ(val, 1);
  ASSERT_EQ(matrix.size(), 1);
}

TEST_F(MatrixTest, IfValueExist_DeleteOnAssignDefault) {  // NOLINT
  matrix[0][0] = 1;
  ASSERT_EQ(matrix.size(), 1);

  auto val = matrix[0][0];
  ASSERT_EQ(val, 1);

  val = DEFAUT_VALUE;
  ASSERT_EQ(val, DEFAUT_VALUE);
  ASSERT_EQ(matrix.size(), 0);
}

TEST_F(MatrixTest, OnChangeValue_ChangesAreVisiableInEveryHolder) {  // NOLINT
  matrix[0][0] = 2;

  auto first_ref = matrix[0][0];
  auto second_ref = matrix[0][0];

  first_ref = 4;
  ASSERT_EQ(second_ref, 4);
}

TEST_F(MatrixTest, HolderImplicitlyConvertible_ToT) {  // NOLINT
  int val = matrix[0][0];                              // NOLINT
  ASSERT_EQ(val, 42);
}

TEST_F(MatrixTest, CopyHolderToHolder) {  // NOLINT
  auto second = matrix[0][1];
  second = 0;

  {
    auto copy = second;
    ASSERT_EQ(copy, 0);
  }

  {
    auto copy = matrix[0][0];
    copy = second;
    ASSERT_EQ(copy, 0);
  }
}

TEST_F(MatrixTest, GetFromConstMatrix) {  // NOLINT
  const auto& matrix_cref = matrix;

  auto val = matrix_cref[0][0];
  ASSERT_EQ(val, DEFAUT_VALUE);

  int converted = val.get();  // NOLINT
  ASSERT_EQ(converted, DEFAUT_VALUE);

  converted = val;
  ASSERT_EQ(converted, DEFAUT_VALUE);

  // NOTE: Should not compile
  // val = 2;
}

TEST_F(MatrixTest, GetBeginInterator_WithStructuredBindings) {  // NOLINT
  matrix[0][0] = 1;
  ASSERT_FALSE(matrix.empty());

  auto begin = matrix.begin();
  ASSERT_EQ(begin.base()->second, 1);
  ASSERT_EQ(begin.base()->first, (std::tuple<std::size_t, std::size_t>{0, 0}));

  auto [x, y, data] = *begin;
  ASSERT_EQ(x, 0);
  ASSERT_EQ(y, 0);
  ASSERT_EQ(data, 1);

  ASSERT_NE(matrix.begin(), matrix.end());
  ASSERT_EQ(begin++, matrix.begin());
  ASSERT_EQ(begin, matrix.end());
  ASSERT_EQ(++matrix.begin(), matrix.end());

  ASSERT_EQ(matrix.end()--, matrix.end());
  ASSERT_EQ(--matrix.end(), matrix.begin());
}

TEST_F(MatrixTest, GetCBegin) {  // NOLINT
  matrix[2][2] = 2;
  ASSERT_FALSE(matrix.empty());

  auto const_cbegin = matrix.cbegin();
  auto begin = matrix.begin();

  const auto& matrix_cref = matrix;
  auto const_begin = matrix_cref.begin();

  ASSERT_EQ(const_cbegin, begin);
  ASSERT_EQ(begin, const_begin);
}

TEST_F(MatrixTest, GetCEnd) {  // NOLINT
  auto cend = matrix.cend();
  auto end = matrix.end();

  const auto& m_cref = matrix;
  auto const_end = m_cref.end();

  ASSERT_EQ(cend, end);
  ASSERT_EQ(end, const_end);
}

TEST_F(MatrixTest, WorksWithRangeBasedFor) {  // NOLINT
  matrix[0][0] = 1;
  matrix[0][1] = 2;

  std::set<int> from_matrix;
  for (auto&& [x, y, data] : matrix) {
    from_matrix.emplace(data);
  }
  ASSERT_EQ(from_matrix, (std::set<int>{1, 2}));
}

TEST_F(MatrixTest, WorksWithStandardAlgorithms) {  // NOLINT
  matrix[0][0] = 1;
  matrix[0][1] = 2;

  auto max = std::max_element(matrix.begin(), matrix.end());
  ASSERT_EQ(max.base()->second, 2);
}

TEST(ThreeDimensionalMatrix, Create) {  // NOLINT
  Matrix<int, int{}, 3> matrix3d{};

  auto hoolder3d = matrix3d.at(0, 1, 2);
  hoolder3d = 222;  // NOLINT
  ASSERT_EQ(matrix3d.at(0, 1, 2), 222);
  ASSERT_EQ(matrix3d[0][1][2], 222);

  ASSERT_EQ(matrix3d.size(), 1);
  hoolder3d = int{};
  ASSERT_EQ(matrix3d.size(), 0);
}

TEST(TupleN, CreateType) {  // NOLINT
  using tuple3d = detail::tuple_n_t<std::size_t, 3>;
  static_assert(
      std::is_same_v<tuple3d,
                     std::tuple<std::size_t, std::size_t, std::size_t>>);
}

TEST(MatrixWithUnorderedMap, Create) {  // NOLINT
  using key_type = detail::tuple_n_t<std::size_t, 2>;
  using unordered_matrix_t =
      Matrix<int, int{}, 2,
             std::unordered_map<key_type, int, detail::tuple_hasher<key_type>>>;

  constexpr auto default_size = 2048;
  unordered_matrix_t unordered_matrix{default_size};

  unordered_matrix[0][2] = 42;
  ASSERT_FALSE(unordered_matrix.empty());
  ASSERT_EQ(unordered_matrix.size(), 1);

  auto [x, y, val] = *unordered_matrix.begin();
  ASSERT_EQ(x, 0);
  ASSERT_EQ(y, 2);
  ASSERT_EQ(val, 42);
}
