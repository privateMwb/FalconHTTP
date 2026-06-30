#pragma once

#include <iterator>
#include <cstddef>
#include <compare>

namespace VectorPro {
template<typename T>
class Iterator {
public:

	// Types
	using iterator_concept   = std::contiguous_iterator_tag;
	using iterator_category  = std::random_access_iterator_tag;
	using value_type         = std::remove_cv_t<T>;
	using element_type       = T;
	using difference_type    = std::ptrdiff_t;
	using pointer            = T*;
	using reference          = T&;


private:

	// Data
	pointer ptr_ = nullptr;

public:

	// Constructors
	constexpr Iterator() noexcept = default;
	constexpr explicit Iterator(pointer ptr) noexcept : ptr_(ptr) {}

	template<typename U>
	requires std::is_same_v<U, std::remove_cv_t<T>>
	        constexpr Iterator(const Iterator<U>& other) noexcept
		        : ptr_(other.operator->()) {}

	// Dereference
	[[nodiscard]] constexpr reference operator*() const noexcept {
		return *ptr_;
	}
	[[nodiscard]] constexpr pointer operator->() const noexcept {
		return ptr_;
	}
	[[nodiscard]] constexpr reference operator[](difference_type n) const noexcept {
		return ptr_[n];
	}

	// Arithmetic
	constexpr Iterator& operator++() noexcept {
		++ptr_;
		return *this;
	}
	[[nodiscard]] constexpr Iterator operator++(int) noexcept {
		auto t = *this;
		++ptr_;
		return t;
	}
	constexpr Iterator& operator--() noexcept {
		--ptr_;
		return *this;
	}
	[[nodiscard]] constexpr Iterator operator--(int) noexcept {
		auto t = *this;
		--ptr_;
		return t;
	}

	constexpr Iterator& operator+=(difference_type n) noexcept {
		ptr_ += n;
		return *this;
	}
	constexpr Iterator& operator-=(difference_type n) noexcept {
		ptr_ -= n;
		return *this;
	}

	[[nodiscard]] friend constexpr Iterator operator+(Iterator i, difference_type n) noexcept {
		i += n;
		return i;
	}
	[[nodiscard]] friend constexpr Iterator operator+(difference_type n, Iterator i) noexcept {
		i += n;
		return i;
	}
	[[nodiscard]] friend constexpr Iterator operator-(Iterator i, difference_type n) noexcept {
		i -= n;
		return i;
	}
	[[nodiscard]] friend constexpr difference_type operator-(Iterator a, Iterator b) noexcept {
		return a.ptr_ - b.ptr_;
	}

	// Comparison
	[[nodiscard]] friend constexpr bool operator==(const Iterator& a, const Iterator& b) noexcept {
		return a.ptr_ == b.ptr_;
	}
	[[nodiscard]] friend constexpr auto operator<=>(const Iterator& a, const Iterator& b) noexcept {
		return a.ptr_ <=> b.ptr_;
	}
};

} // namespace VectorPro