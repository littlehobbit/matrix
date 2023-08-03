#ifndef __MATRIX_H_96B48PA31JS9__
#define __MATRIX_H_96B48PA31JS9__

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <optional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

namespace detail {

/**
 * @brief Cancatination tuple types alias
 *
 * @tparam input_t
 */
template <typename... input_t>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<input_t>()...));

/**
 * @brief Utility class for generation std::tuple with T type N times
 *
 * @tparam T
 * @tparam N
 */
template <typename T, std::size_t N>
struct tuple_n {
  using type = tuple_cat_t<typename tuple_n<T, N - 1>::type, std::tuple<T>>;
};

template <typename T>
struct tuple_n<T, 1> {
  using type = std::tuple<T>;
};

/**
 * @brief Helper alias for N-times T tuples
 *
 * @tparam T
 * @tparam N
 */
template <typename T, std::size_t N>
using tuple_n_t = typename tuple_n<T, N>::type;

/**
 * @brief Tuple hasher
 *
 */
struct tuple_hasher {
  /**
   * @brief Hasher call function
   *
   * @tparam Tuple - Hashable tuple type
   * @param tuple
   * @return std::size_t - Result value
   */
  template <typename Tuple>
  auto operator()(const Tuple& tuple) const noexcept -> std::size_t {
    return hash_impl(tuple,
                     std::make_index_sequence<std::tuple_size_v<Tuple>>{});
  }

  template <typename Tuple, std::size_t... Indexes>
  auto hash_impl(const Tuple& tuple,
                 std::index_sequence<Indexes...> /*seq*/) const noexcept
      -> std::size_t {
    // Isn't best way to combine hashes, but easiest
    return (std::hash<std::tuple_element_t<Indexes, Tuple>>{}(
                std::get<Indexes>(tuple)) ^
            ...);
  }
};

}  // namespace detail

/**
 * @brief A generic N-dimensional matrix class with support for default values
 * and associative containers.
 *
 * The Matrix class represents an N-dimensional matrix with customizable default
 * values for uninitialized elements. It uses an associative container to store
 * the matrix elements, which allows for multiple indexes for element access.
 * The default container is std::map, but you can specify a different
 * associative container type as well.
 *
 * @tparam T The type of the matrix elements.
 * @tparam Default The default value used for uninitialized elements in the
 * matrix. Default is set to T{} (default constructor of T).
 * @tparam Dimensions The number of dimensions for the matrix. Default is 2.
 * @tparam Container The associative container type used to store the matrix
 * elements. Default is std::map<detail::tuple_n_t<std::size_t, Dimensions>, T>.
 *
 * @note The Container template parameter allows you to customize the underlying
 * storage for the matrix. It should be an associative container that provides a
 * suitable interface for element access and supports multiple indexes for
 * N-dimensional coordinates.
 *
 * @warning Avoid using large dimensions or non-efficient container types, as it
 * may lead to performance issues and memory overhead.
 *
 * @note If you assign the default value to an element in the matrix, it will be
 * deleted from the matrix.
 *
 * @see detail::tuple_n_t - A helper tuple type for representing N-dimensional
 * coordinates.
 *
 * @example
 * // Create a 2-dimensional matrix with a default value of 0 using
 * std::unordered_map as the container. Matrix<int, 0, 2,
 * std::unordered_map<detail::tuple_n_t<std::size_t, 2>, int>> myMatrix;
 *
 * // Access and modify matrix elements using multiple indexes.
 * // Генерировать документацию с ChatGPT - жесть
 * myMatrix[0][0] = 42;
 * int value = myMatrix[1][2];
 * myMatrix[0][0] = 0; // This will remove the element (0, 0) from the matrix.
 */
template <typename T,                  //
          T Default = T{},             //
          std::size_t Dimensions = 2,  //
          typename Container =
              std::map<detail::tuple_n_t<std::size_t, Dimensions>, T>>
class Matrix {
 public:
  /**
   * @brief A reference wrapper class for accessing elements in the Matrix
   * class.
   *
   * The value_ref class provides a reference wrapper that allows users to
   * access and modify elements in the associated Matrix. It is designed to
   * handle N-dimensional coordinates and provides functionalities for
   * assignment and comparison with a value.
   *
   * @tparam MatrixType The type of the Matrix that this value_ref belongs to.
   *
   * @note This class is used as a return type when accessing elements in the
   * Matrix, and it allows for convenient element modification.
   *
   * @warning Modifying the value_ref directly does not automatically update the
   * underlying matrix. You should use assignment to update matrix elements or
   * use appropriate Matrix member functions.
   *
   * @see Matrix - The Matrix class that this value_ref is associated with.
   * @see detail::tuple_n_t - A helper tuple type for representing N-dimensional
   * coordinates.
   *
   * @example
   * // Create a 2-dimensional matrix and access an element using the value_ref.
   * Matrix<int, 0, 2> myMatrix;
   * value_ref<Matrix<int, 0, 2>> elementRef = myMatrix[0][0]; // Access an
   * element using value_ref.
   *
   * // Modify the element using assignment.
   * elementRef = 42;
   *
   * // Access the value of the element using conversion.
   * int value = elementRef; // value is now 42.
   */
  template <typename MatrixType>
  struct value_ref {
    /**
     * @brief Constructs a value_ref object for accessing a specific element in
     * the Matrix.
     *
     * @tparam Indexes The variadic pack of indexes representing the
     * N-dimensional coordinates of the element.
     * @param matrix The reference to the Matrix that this value_ref belongs to.
     * @param indexes The N-dimensional coordinates of the element.
     *
     * @note The Indexes pack must match the number of Dimensions defined for
     * the associated Matrix.
     */
    template <typename... Indexes>
    value_ref(MatrixType& matrix, Indexes... indexes)
        : matrix{matrix}, position{indexes...} {}

    /**
     * @brief Comparison operator for comparing the element with a value.
     *
     * @param rhs The value to compare the element with.
     * @return true if the element is equal to the given value; otherwise,
     * false.
     */
    bool operator==(const T& rhs) const noexcept { return rhs == get(); }

    /**
     * @brief Assignment operator for modifying the element in the associated
     * Matrix.
     *
     * @tparam U The type of the value to assign to the element.
     * @param rhs The value to assign to the element.
     * @return A reference to the value_ref object after the assignment.
     *
     * @note If the assigned value is equal to the default value of the Matrix,
     * the element will be deleted from the Matrix.
     */
    template <typename U>
    value_ref& operator=(U&& rhs) {
      std::apply(
          [&](auto... pos) {
            if (rhs == Default) {
              matrix.erase(pos...);
            } else {
              matrix.set(std::forward<U>(rhs), pos...);
            }
          },
          position);

      return *this;
    }

    /**
     * @brief Implicit conversion operator for stored value.
     *
     * @return The value of the element in the associated Matrix.
     */
    operator T() const noexcept { return get(); }

    /**
     * @brief Retrieves the value of the element from the associated Matrix.
     *
     * @return The value of the element in the associated Matrix.
     */
    auto get() const noexcept -> T {
      return std::apply(
          [&](auto... pos) {  //
            return matrix.get_or_default(pos...);
          },
          position);
    }

    MatrixType& matrix;
    detail::tuple_n_t<std::size_t, Dimensions> position;
  };

  /**
   * @brief Proxy class for making multi-indexed access for multidimensional
   * matrix
   *
   * @tparam MatrixType Used matrix type
   * @tparam Order      Dimension order of this "slice"
   */
  template <typename MatrixType, std::size_t Order>
  struct index_proxy {
    template <typename... Position>
    index_proxy(MatrixType& matrix, Position... pos)
        : matrix{matrix}, current_index{pos...} {}

    /**
     * @brief Index operator
     *
     * @param last_index Current requested index
     * @return value_ref if it was last index operator, index_proxy<Order-1>
     * overwise
     */
    auto operator[](std::size_t last_index) const noexcept {
      if constexpr (Order == 1) {
        return std::apply(
            [&](auto... pos) {  //
              return matrix.at(pos..., last_index);
            },
            current_index);
      } else {
        return std::apply(
            [&](auto... pos) -> index_proxy<MatrixType, Order - 1> {  //
              // Не самое деликатное решение, но красивое
              return {matrix, pos..., last_index};
            },
            current_index);
      }
    }

    MatrixType& matrix;
    detail::tuple_n_t<std::size_t, Dimensions - Order> current_index;
  };

  /**
   * @brief Bidirectional matrix iterator
   *
   * @tparam MatrixInnerIterator
   */
  template <typename MatrixInnerIterator>
  struct matrix_iterator {
    using base_traits_t = std::iterator_traits<MatrixInnerIterator>;
    using iterator_category = typename base_traits_t::iterator_category;
    using difference_type = typename base_traits_t::difference_type;
    using value_type = typename base_traits_t::value_type;
    using pointer = typename base_traits_t::pointer;
    using reference = typename base_traits_t::reference;

    matrix_iterator(MatrixInnerIterator iterator) : iter{iterator} {}

    auto base() const noexcept { return iter; }

    auto operator++(int) -> matrix_iterator { return {this->iter++}; }

    auto operator++() -> matrix_iterator& {
      ++iter;
      return *this;
    }

    auto operator--(int) -> matrix_iterator { return {this->iter--}; }

    auto operator--() -> matrix_iterator& {
      --iter;
      return *this;
    }

    template <typename IterType>
    bool operator==(const matrix_iterator<IterType>& rhs) const noexcept {
      return iter == rhs.iter;
    }

    template <typename IterType>
    bool operator!=(const matrix_iterator<IterType>& rhs) const noexcept {
      return iter != rhs.iter;
    }

    auto operator*() const noexcept -> std::tuple<std::size_t, std::size_t, T> {
      return {std::get<0>(iter->first), std::get<1>(iter->first), iter->second};
    }

    MatrixInnerIterator iter;
  };

  using value_type = T;
  using size_type = std::size_t;

  using reference = value_ref<Matrix>;
  using const_reference = value_ref<const Matrix>;

  using iterator = matrix_iterator<typename Container::const_iterator>;
  using const_iterator = iterator;

  template <typename... Args, typename Require = std::enable_if_t<
                                  std::is_constructible_v<Container, Args...>>>
  Matrix(Args&&... args) : _data(std::forward<Args>(args)...) {}

  /**
   * @brief Gets the number of elements in the matrix.
   *
   * @return The number of elements in the matrix.
   */
  auto size() const noexcept -> size_type { return _data.size(); }

  /**
   * @brief Checks if the matrix is empty (contains no elements).
   *
   * @return true if the matrix is empty, false otherwise.
   */
  bool empty() const noexcept { return _data.empty(); }

  /**
   * @brief Multi-index operator for accessing elements in the matrix.
   *
   * @param index The index of the element to access.
   * @return An index_proxy object representing the remaining dimensions after
   * accessing the element.
   */
  auto operator[](std::size_t index) noexcept
      -> index_proxy<Matrix, Dimensions - 1> {
    return {*this, index};
  }

  /**
   * @brief Multi-index operator for accessing elements in the matrix.
   *
   * @param index The index of the element to access.
   * @return An index_proxy object representing the remaining dimensions after
   * accessing the element.
   */
  auto operator[](std::size_t index) const noexcept
      -> index_proxy<const Matrix, Dimensions - 1> {
    return {*this, index};
  }

  /**
   * @brief Accesses an element in the matrix by providing N-dimensional
   * coordinates.
   *
   * @tparam Position The variadic pack of indexes representing the
   * N-dimensional coordinates of the element.
   * @param pos The N-dimensional coordinates of the element.
   * @return A value_ref object representing a reference to the accessed
   * element.
   *
   * @note The Position pack must match the number of Dimensions defined for the
   * matrix.
   * @note The value_ref allows for convenient modification and retrieval of
   * matrix elements.
   */
  template <typename... Position>
  auto at(Position... pos) noexcept {
    return value_ref<Matrix>{*this, pos...};
  }

  /**
   * @brief Accesses an element in the matrix by providing N-dimensional
   * coordinates (const version).
   *
   * @tparam Position The variadic pack of indexes representing the
   * N-dimensional coordinates of the element.
   * @param pos The N-dimensional coordinates of the element.
   * @return A value_ref object representing a reference to the accessed
   * element.
   *
   * @note The Position pack must match the number of Dimensions defined for the
   * matrix.
   * @note The value_ref allows for convenient retrieval of matrix elements with
   * const access.
   */
  template <typename... Position>
  auto at(Position... pos) const noexcept {
    return value_ref<const Matrix>{*this, pos...};
  }

  /**
   * @brief Sets the value of an element in the matrix.
   *
   * @tparam U The type of the value to assign to the element.
   * @tparam Position The variadic pack of indexes representing the
   * N-dimensional coordinates of the element.
   * @param val The value to assign to the element.
   * @param pos The N-dimensional coordinates of the element.
   *
   * @note The Position pack must match the number of Dimensions defined for the
   * matrix.
   * @note If the element does not exist, a new element is added with the
   * provided value.
   */
  template <typename U, typename... Position>
  void set(U&& val, Position... pos) {
    _data.insert_or_assign({pos...}, std::forward<U>(val));
  }

  /**
   * @brief Retrieves the value of an element in the matrix or the default value
   * if the element does not exist.
   *
   * @tparam Position The variadic pack of indexes representing the
   * N-dimensional coordinates of the element.
   * @param pos The N-dimensional coordinates of the element.
   * @return The value of the element in the matrix if it exists; otherwise, the
   * default value.
   */
  template <typename... Position>
  auto get_or_default(Position... pos) const noexcept -> T {
    if (auto iter = _data.find({pos...}); iter != _data.cend()) {
      return iter->second;
    }
    return Default;
  }

  /**
   * @brief Erases an element from the matrix.
   *
   * @tparam Position The variadic pack of indexes representing the
   * N-dimensional coordinates of the element.
   * @param pos The N-dimensional coordinates of the element to erase.
   *
   * @note The Position pack must match the number of Dimensions defined for the
   * matrix.
   * @note If the element does not exist, this function has no effect.
   */
  template <typename... Position>
  void erase(Position... pos) noexcept {
    if (auto iter = _data.find({pos...}); iter != _data.end()) {
      _data.erase(iter);
    }
  }

  auto begin() const noexcept -> const_iterator { return cbegin(); }
  auto cbegin() const noexcept -> const_iterator {
    return const_iterator{_data.begin()};
  }

  auto end() const noexcept -> const_iterator { return cend(); }
  auto cend() const noexcept -> const_iterator {
    return const_iterator{_data.cend()};
  }

 private:
  Container _data;
};

#endif  // __MATRIX_H_96B48PA31JS9__
