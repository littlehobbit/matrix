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

template <typename... input_t>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<input_t>()...));

template <typename T, std::size_t N>
struct tuple_n {
  using type = tuple_cat_t<typename tuple_n<T, N - 1>::type, std::tuple<T>>;
};

template <typename T>
struct tuple_n<T, 1> {
  using type = std::tuple<T>;
};

template <typename T, std::size_t N>
using tuple_n_t = typename tuple_n<T, N>::type;

template <typename Tuple>
struct tuple_hasher {
  auto operator()(const Tuple& tuple) const noexcept -> std::size_t {
    return hash_impl(tuple,
                     std::make_index_sequence<std::tuple_size_v<Tuple>>{});
  }

  template <std::size_t... Indexes>
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

template <typename T,                  //
          T Default = T{},             //
          std::size_t Dimensions = 2,  //
          typename Container =
              std::map<detail::tuple_n_t<std::size_t, Dimensions>, T>>
class Matrix {
  template <typename MatrixType>
  struct value_ref {
    template <typename... Indexes>
    value_ref(MatrixType& matrix, Indexes... indexes)
        : matrix{matrix}, position{indexes...} {}

    bool operator==(const T& rhs) const noexcept { return rhs == get(); }

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

    operator T() const noexcept { return get(); }

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

  template <typename MatrixType, std::size_t Order>
  struct index_proxy {
    template <typename... Position>
    index_proxy(MatrixType& matrix, Position... pos)
        : matrix{matrix}, current_index{pos...} {}

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

 public:
  using value_type = T;
  using size_type = std::size_t;

  using reference = value_ref<Matrix>;
  using const_reference = value_ref<const Matrix>;

  using iterator = matrix_iterator<typename Container::const_iterator>;
  using const_iterator = iterator;

  template <typename... Args, typename Require = std::enable_if_t<
                                  std::is_constructible_v<Container, Args...>>>
  Matrix(Args&&... args) : _data(std::forward<Args>(args)...) {}

  auto size() const noexcept -> size_type { return _data.size(); }

  bool empty() const noexcept { return _data.empty(); }

  auto operator[](std::size_t index) noexcept
      -> index_proxy<Matrix, Dimensions - 1> {
    return {*this, index};
  }

  auto operator[](std::size_t index) const noexcept
      -> index_proxy<const Matrix, Dimensions - 1> {
    return {*this, index};
  }

  template <typename... Position>
  auto at(Position... pos) noexcept {
    return value_ref<Matrix>{*this, pos...};
  }

  template <typename... Position>
  auto at(Position... pos) const noexcept {
    return value_ref<const Matrix>{*this, pos...};
  }

  template <typename U, typename... Position>
  void set(U&& val, Position... pos) {
    _data.insert_or_assign({pos...}, std::forward<U>(val));
  }

  template <typename... Position>
  auto get_or_default(Position... pos) const noexcept -> T {
    if (auto iter = _data.find({pos...}); iter != _data.cend()) {
      return iter->second;
    }
    return Default;
  }

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
