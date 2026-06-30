#pragma once

#include "Iterator.h"

#include <iterator>
#include <type_traits>
#include <initializer_list>
#include <algorithm>
#include <cstddef>
#include <utility>
#include <cstring>
#include <span>
#include <concepts>
#include <compare>
#include <memory>
#include <functional>
#include <stdexcept>

namespace VectorPro {

	// Listener Callable Constraints
	template<typename F, typename VectorType>
	concept Listener = std::invocable<F, const VectorType&, typename VectorType::EventData>;

	template<typename T,
		typename Allocator = std::allocator<T>,
		std::size_t GrowthNum = 2,
		std::size_t GrowthDen = 1>
		requires std::destructible<T>
	class Vector {
	public:

		// Types
		using allocator_type = Allocator;
		using value_type = T;
		using pointer = typename std::allocator_traits<Allocator>::pointer;
		using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		// Growth Factor Validation
		static_assert(GrowthNum != 0, "VectorPro::Vector: GrowthNum must not be zero");
		static_assert(GrowthDen != 0, "VectorPro::Vector: GrowthDen must not be zero");
		static_assert(GrowthNum > GrowthDen, "VectorPro::Vector: GrowthNum must be greater than GrowthDen to ensure growth");

		// Concept
		enum class EventType {
			PUSHBACK,
			EMPLACEBACK,
			POPBACK,
			INSERT,
			REMOVE,
			ERASE,
			CLEAR,
			SHRINK,
			RESERVE
		};

		struct EventData {
			EventType    type;
			std::size_t  index;
			std::size_t  oldSize;
			std::size_t  newSize;
		};

		using ListenerFn = std::function<void(const Vector&, EventData)>;
		using ListenerHandle = std::size_t;

		// Iterator Aliases
		using iterator = VectorPro::Iterator<T>;
		using const_iterator = VectorPro::Iterator<const T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	private:

		// Data
		Allocator    alloc_;
		pointer      data_ = nullptr;
		std::size_t  vsize_ = 0;
		std::size_t  vcap_ = 0;

		ListenerFn* listeners_ = nullptr;
		std::size_t  lsize_ = 0;
		std::size_t  lcap_ = 0;

		static constexpr std::size_t INITIAL_CAP = 8;

	public:

		// Constructors & Destructor
		explicit Vector(std::size_t count, const T& value = {});
		Vector(std::initializer_list<T> init);

		template<std::input_iterator It>
		Vector(It first, It last);

		explicit Vector(const Allocator& alloc = Allocator{});

		~Vector() noexcept;

		Vector(const Vector& other);
		Vector& operator=(const Vector& other);

		Vector(Vector&& other) noexcept;
		Vector& operator=(Vector&& other) noexcept;

		// Modifiers
		void push_back(const T& value);
		void push_back(T&& value);

		template<typename... Args>
			requires std::constructible_from<T, Args...>
		void emplace_back(Args&&... args);

		[[nodiscard]] iterator insert(const_iterator pos, const T& value);
		[[nodiscard]] iterator insert(const_iterator pos, T&& value);

		template<std::input_iterator It>
		iterator insert(const_iterator pos, It first, It last);

		template<typename... Args>
			requires std::constructible_from<T, Args...>
		[[nodiscard]] iterator emplace(const_iterator pos, Args&&... args);

		template<typename Predicate>
			requires std::predicate<Predicate, const T&>
		[[nodiscard]] std::size_t remove_if(Predicate pred);

		void pop_back();

		[[nodiscard]] iterator erase(const_iterator pos);
		[[nodiscard]] iterator erase(const_iterator first, const_iterator last);

		void clear() noexcept;

		void reserve(std::size_t newCap);
		void shrink_to_fit();

		// Observer
		template<typename F>
			requires Listener<F, Vector<T, Allocator, GrowthNum, GrowthDen>>
		[[nodiscard]] ListenerHandle subscribe(F&& listener);
		void unsubscribe(ListenerHandle handle);

		// Comparison Operators
		[[nodiscard]] bool operator==(const Vector& other)  const noexcept;
		[[nodiscard]] auto operator<=>(const Vector& other) const noexcept;

		// Span Accessors
		[[nodiscard]] std::span<T>        as_span()       noexcept;
		[[nodiscard]] std::span<const T>  as_span() const noexcept;

		// Element Access
		[[nodiscard]] pointer          data_ptr()       noexcept;
		[[nodiscard]] const_pointer    data_ptr() const noexcept;

		[[nodiscard]] reference        at(std::size_t index);
		[[nodiscard]] const_reference  at(std::size_t index) const;

		[[nodiscard]] reference        front();
		[[nodiscard]] const_reference  front() const;

		[[nodiscard]] reference        back();
		[[nodiscard]] const_reference  back() const;

		[[nodiscard]] reference        operator[](std::size_t index)       noexcept;
		[[nodiscard]] const_reference  operator[](std::size_t index) const noexcept;

		// Search
		[[nodiscard]] bool            contains(const T& value) const noexcept;
		[[nodiscard]] iterator        find(const T& value)           noexcept;
		[[nodiscard]] const_iterator  find(const T& value)     const noexcept;

		// Swap
		void swap(Vector& other) noexcept;

		// Capacity
		[[nodiscard]] bool         empty()     const noexcept;
		[[nodiscard]] std::size_t  size()      const noexcept;
		[[nodiscard]] std::size_t  capacity()  const noexcept;

		// Iterators
		[[nodiscard]] iterator                begin()          noexcept;
		[[nodiscard]] iterator                end()            noexcept;
		[[nodiscard]] const_iterator          begin()    const noexcept;
		[[nodiscard]] const_iterator          end()      const noexcept;
		[[nodiscard]] const_iterator          cbegin()   const noexcept;
		[[nodiscard]] const_iterator          cend()     const noexcept;
		[[nodiscard]] reverse_iterator        rbegin()         noexcept;
		[[nodiscard]] reverse_iterator        rend()           noexcept;
		[[nodiscard]] const_reverse_iterator  rbegin()   const noexcept;
		[[nodiscard]] const_reverse_iterator  rend()     const noexcept;
		[[nodiscard]] const_reverse_iterator  crbegin()  const noexcept;
		[[nodiscard]] const_reverse_iterator  crend()    const noexcept;

	private:

		// Internal Helpers
		void release() noexcept;
		[[nodiscard]] std::size_t growCapacity() const noexcept;
		void reallocate(std::size_t newCap);

		template<typename U>
		void reallocateBuffer(U*& buf, std::size_t& cap, std::size_t newCap);
		void notify(EventData data);

	};

	// Swap
	template<typename T,
		typename Allocator,
		std::size_t GrowthNum,
		std::size_t GrowthDen>
	void swap(
		Vector<T, Allocator, GrowthNum, GrowthDen>& a,
		Vector<T, Allocator, GrowthNum, GrowthDen>& b) noexcept;

} // namespace VectorPro

#include "Vector.tpp"