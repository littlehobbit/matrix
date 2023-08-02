#ifndef __MATRIX_H_96B48PA31JS9__
#define __MATRIX_H_96B48PA31JS9__

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <variant>

namespace detail {
// ...
}  // namespace detail

template <typename T, T Default = T{}>
class Matrix {
  template <typename MatrixType>
  struct value_ref {
    value_ref(MatrixType& matrix, std::size_t x, std::size_t y)
        : matrix{matrix}, x{x}, y{y} {}

    bool operator==(const T& rhs) const noexcept { return rhs == get(); }

    template <typename U>
    value_ref& operator=(U&& rhs) {
      if (rhs == Default) {
        matrix.erase(x, y);
      } else {
        matrix.set(x, y, std::forward<U>(rhs));
      }

      return *this;
    }

    operator T() const noexcept { return get(); }

    auto get() const noexcept -> T { return matrix.get_or_default(x, y); }

    MatrixType& matrix;
    std::size_t x;
    std::size_t y;
  };

  template <typename MatrixType>
  struct index_proxy {
    index_proxy(MatrixType& matrix, std::size_t index)
        : matrix{matrix}, x{index} {}

    auto operator[](std::size_t y) noexcept { return matrix.at(x, y); }
    auto operator[](std::size_t y) const noexcept { return matrix.at(x, y); }

    MatrixType& matrix;
    std::size_t x;
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

  using iterator =
      matrix_iterator<typename std::map<std::pair<std::size_t, std::size_t>,
                                        T>::const_iterator>;
  using const_iterator = iterator;

  auto size() const noexcept -> size_type { return _data.size(); }

  bool empty() const noexcept { return _data.empty(); }

  auto operator[](std::size_t index) noexcept -> index_proxy<Matrix> {
    return index_proxy{*this, index};
  }

  auto operator[](std::size_t index) const noexcept
      -> index_proxy<const Matrix> {
    return index_proxy{*this, index};
  }

  auto at(std::size_t x, std::size_t y) noexcept {
    return value_ref<Matrix>{*this, x, y};
  }

  auto at(std::size_t x, std::size_t y) const noexcept {
    return value_ref<const Matrix>{*this, x, y};
  }

  template <typename U>
  void set(std::size_t x, std::size_t y, U&& val) {
    _data.insert_or_assign(std::pair{x, y}, std::forward<U>(val));
  }

  auto get_or_default(std::size_t x, std::size_t y) const noexcept -> T {
    if (auto iter = _data.find({x, y}); iter != _data.cend()) {
      return iter->second;
    }
    return Default;
  }

  void erase(std::size_t x, std::size_t y) noexcept {
    if (auto iter = _data.find(std::pair{x, y}); iter != _data.end()) {
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
  std::map<std::pair<std::size_t, std::size_t>, T> _data;
};

#endif  // __MATRIX_H_96B48PA31JS9__
