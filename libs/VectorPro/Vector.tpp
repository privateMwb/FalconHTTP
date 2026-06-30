namespace VectorPro {

// Constructors & Destructor
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
Vector<T, Allocator, GrowthNum, GrowthDen>::Vector(std::size_t count, const T& value) {
	if (count == 0) return;

	data_ = std::allocator_traits<Allocator>::allocate(alloc_, count);
	vcap_ = count;

	for (std::size_t i = 0; i < count; ++i) {
		std::allocator_traits<Allocator>::construct(alloc_, data_ + i, value);
	}

	vsize_ = count;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
Vector<T, Allocator, GrowthNum, GrowthDen>::Vector(std::initializer_list<T> init) {
	if (init.size() == 0) return;

	data_ = std::allocator_traits<Allocator>::allocate(alloc_, init.size());
	vcap_ = init.size();

	for (const auto& val : init) {
		std::allocator_traits<Allocator>::construct(alloc_, data_ + vsize_++, val);
	}
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
template<std::input_iterator It>
Vector<T, Allocator, GrowthNum, GrowthDen>::Vector(It first, It last) {
	for (; first != last; ++first) {
		push_back(*first);
	}
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
Vector<T, Allocator, GrowthNum, GrowthDen>::Vector(const Allocator& alloc)
	: alloc_(alloc) {}


template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
Vector<T, Allocator, GrowthNum, GrowthDen>::~Vector() noexcept {
	release();

	if (listeners_) {
		delete[] listeners_;
		listeners_ = nullptr;
	}
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
Vector<T, Allocator, GrowthNum, GrowthDen>::Vector(const Vector& other)
	: alloc_(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc_)) {

	if (other.vcap_ == 0) return;

	data_ = std::allocator_traits<Allocator>::allocate(alloc_, other.vcap_);
	vcap_ = other.vcap_;

	for (std::size_t i = 0; i < other.vsize_; ++i) {
		std::allocator_traits<Allocator>::construct(alloc_, data_ + i, other.data_[i]);
	}

	vsize_ = other.vsize_;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
Vector<T, Allocator, GrowthNum, GrowthDen>& Vector<T, Allocator, GrowthNum, GrowthDen>::operator=(const Vector& other) {
	if (this != &other) {
		Vector tmp(other);
		swap(tmp);
	}
	return *this;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
Vector<T, Allocator, GrowthNum, GrowthDen>::Vector(Vector&& other) noexcept
	: alloc_(std::move(other.alloc_))
	, data_(other.data_)
	, vsize_(other.vsize_)
	, vcap_(other.vcap_)
	, listeners_(other.listeners_)
	, lsize_(other.lsize_)
	, lcap_(other.lcap_)
{
	other.data_       = nullptr;
	other.vsize_      = 0;
	other.vcap_       = 0;
	other.listeners_  = nullptr;
	other.lsize_      = 0;
	other.lcap_       = 0;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
Vector<T, Allocator, GrowthNum, GrowthDen>& Vector<T, Allocator, GrowthNum, GrowthDen>::operator=(Vector&& other) noexcept {
	if (this != &other) {
		release();

		if (listeners_) {
			delete[] listeners_;
			listeners_ = nullptr;
		}

		alloc_      = std::move(other.alloc_);
		data_       = other.data_;
		vsize_      = other.vsize_;
		vcap_       = other.vcap_;
		listeners_  = other.listeners_;
		lsize_      = other.lsize_;
		lcap_       = other.lcap_;

		other.data_       = nullptr;
		other.vsize_      = 0;
		other.vcap_       = 0;
		other.listeners_  = nullptr;
		other.lsize_      = 0;
		other.lcap_       = 0;
	}

	return *this;
}

// Modifiers
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::push_back(const T& value) {
	if (vsize_ == vcap_) reallocate(growCapacity());

	std::allocator_traits<Allocator>::construct(alloc_, data_ + vsize_, value);

	std::size_t old = vsize_;
	++vsize_;
	notify({ EventType::PUSHBACK, old, old, vsize_ });
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::push_back(T&& value) {
	if (vsize_ == vcap_) reallocate(growCapacity());

	std::allocator_traits<Allocator>::construct(alloc_, data_ + vsize_, std::move(value));

	std::size_t old = vsize_;
	++vsize_;
	notify({ EventType::PUSHBACK, old, old, vsize_ });
}

template<typename T, typename Allocator, std::size_t GrowthNum, std::size_t GrowthDen>
requires std::destructible<T>
template<typename... Args>
requires std::constructible_from<T, Args...>
void Vector<T, Allocator, GrowthNum, GrowthDen>::emplace_back(Args&&... args) {
	if (vsize_ == vcap_)
		reallocate(growCapacity());

	std::allocator_traits<Allocator>::construct(alloc_, data_ + vsize_, std::forward<Args>(args)...);

	std::size_t old = vsize_;
	++vsize_;
	notify({ EventType::EMPLACEBACK, old, old, vsize_ });
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::insert(const_iterator pos, const T& value) {
	std::size_t index = pos - cbegin();

	if (vsize_ == vcap_) reallocate(growCapacity());

	if (vsize_ > index)
		std::allocator_traits<Allocator>::construct(alloc_, data_ + vsize_, std::move(data_[vsize_ - 1]));

	for (std::size_t i = vsize_ - 1; i > index; --i)
		data_[i] = std::move(data_[i - 1]);

	std::allocator_traits<Allocator>::construct(alloc_, data_ + index, value);

	std::size_t old = vsize_;
	++vsize_;
	notify({EventType::INSERT, index, old, vsize_});
	return iterator(data_ + index);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::insert(const_iterator pos, T&& value) {
	std::size_t index = pos - cbegin();

	if (vsize_ == vcap_) reallocate(growCapacity());

	if (vsize_ > index)
		std::allocator_traits<Allocator>::construct(alloc_, data_ + vsize_, std::move(data_[vsize_ - 1]));

	for (std::size_t i = vsize_ - 1; i > index; --i)
		data_[i] = std::move(data_[i - 1]);

	std::allocator_traits<Allocator>::construct(alloc_, data_ + index, std::move(value));

	std::size_t old = vsize_;
	++vsize_;
	notify({EventType::INSERT, index, old, vsize_});
	return iterator(data_ + index);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
template<std::input_iterator It>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::insert(const_iterator pos, It first, It last) {
	std::size_t index = pos - cbegin();
	std::size_t i     = index;

	for (; first != last; ++first, ++i) {
		(void)insert(cbegin() + i, *first);
	}

	return iterator(data_ + index);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
template<typename... Args>
requires std::constructible_from<T, Args...>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::emplace(const_iterator pos, Args&&... args) {
	std::size_t index = pos - cbegin();

	if (vsize_ == vcap_) reallocate(growCapacity());

	if (vsize_ > index)
		std::allocator_traits<Allocator>::construct(alloc_, data_ + vsize_, std::move(data_[vsize_ - 1]));

	for (std::size_t i = vsize_ - 1; i > index; --i)
		data_[i] = std::move(data_[i - 1]);

	std::allocator_traits<Allocator>::construct(alloc_, data_ + index, std::forward<Args>(args)...);

	std::size_t old = vsize_;
	++vsize_;
	notify({ EventType::INSERT, index, old, vsize_ });
	return iterator(data_ + index);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
template<typename Predicate>
requires std::predicate<Predicate, const T&>
std::size_t Vector<T, Allocator, GrowthNum, GrowthDen>::remove_if(Predicate pred) {
	std::size_t old   = vsize_;
	std::size_t dest  = 0;

	for (std::size_t i = 0; i < vsize_; ++i) {
		if (!pred(data_[i])) {
			data_[dest++] = std::move(data_[i]);
		} else {
			std::allocator_traits<Allocator>::destroy(alloc_, data_ + i);
		}
	}

	vsize_ = dest;
	notify({EventType::REMOVE, dest, old, vsize_});
	return old - vsize_;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::pop_back() {
	if (vsize_ == 0) return;

	std::size_t old = vsize_;
	std::allocator_traits<Allocator>::destroy(alloc_, data_ + vsize_ - 1);
	--vsize_;

	notify({EventType::POPBACK, vsize_, old, vsize_});
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::erase(const_iterator pos) {
	std::size_t index = pos - cbegin();

	std::allocator_traits<Allocator>::destroy(alloc_, data_ + index);

	for (std::size_t i = index; i < vsize_ - 1; ++i) {
		data_[i] = std::move(data_[i + 1]);
	}

	std::size_t old = vsize_;
	--vsize_;

	notify({EventType::ERASE, index, old, vsize_});
	return iterator(data_ + index);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::erase(const_iterator first, const_iterator last) {
	std::size_t indexFirst  = first - cbegin();
	std::size_t indexLast   = last - cbegin();
	std::size_t count       = indexLast - indexFirst;

	for (std::size_t i = indexFirst; i < indexLast; ++i) {
		std::allocator_traits<Allocator>::destroy(alloc_, data_ + i);
	}

	for (std::size_t i = indexFirst; i < vsize_ - count; ++i) {
		data_[i] = std::move(data_[i + count]);
	}

	std::size_t old = vsize_;
	vsize_ -= count;

	notify({EventType::ERASE, indexFirst, old, vsize_});
	return iterator(data_ + indexFirst);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::clear() noexcept {
	if (vsize_ == 0) return;

	for (std::size_t i = 0; i < vsize_; ++i) {
		std::allocator_traits<Allocator>::destroy(alloc_, data_ + i);
	}

	std::size_t old = vsize_;
	vsize_ = 0;

	notify({EventType::CLEAR, 0, old, 0});
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::reserve(std::size_t newCap) {
	if (newCap <= vcap_) return;

	std::size_t old = vcap_;
	reallocate(newCap);

	notify({EventType::RESERVE, 0, old, vcap_});
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::shrink_to_fit() {
	if (vsize_ == vcap_) return;

	std::size_t old = vcap_;
	reallocate(vsize_);

	notify({EventType::SHRINK, 0, old, vcap_});
}

// Observer
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
template<typename F>
requires Listener<F, Vector<T, Allocator, GrowthNum, GrowthDen>>
        typename Vector<T, Allocator, GrowthNum, GrowthDen>::ListenerHandle
Vector<T, Allocator, GrowthNum, GrowthDen>::subscribe(F&& listeners) {
	if(lsize_ == lcap_) {
		std::size_t newCap  = lcap_ == 0 ? INITIAL_CAP : lcap_ * 2;
		ListenerFn* newBuf  = new ListenerFn[newCap];

		for (std::size_t i = 0; i < lsize_; ++i) {
			newBuf[i] = std::move(listeners_[i]);
		}

		delete[] listeners_;
		listeners_  = newBuf;
		lcap_       = newCap;
	}

	listeners_[lsize_] = std::forward<F>(listeners);
	return lsize_++;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::unsubscribe(ListenerHandle handle) {
	if (handle >= lsize_) return;

	for (std::size_t i = handle; i < lsize_ - 1; ++i) {
		listeners_[i] = std::move(listeners_[i + 1]);
	}

	listeners_[lsize_ - 1] = nullptr;
	--lsize_;
}

// Comparison Operators
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
bool Vector<T, Allocator, GrowthNum, GrowthDen>::operator==(const Vector& other) const noexcept {
	return vsize_ == other.vsize_ && std::equal(begin(), end(), other.begin());
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
auto Vector<T, Allocator, GrowthNum, GrowthDen>::operator<=>(const Vector& other) const noexcept {
	return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end());
}

// Span Accessors
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
std::span<T> Vector<T, Allocator, GrowthNum, GrowthDen>::as_span() noexcept {
	return std::span<T>(data_, vsize_);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
std::span<const T> Vector<T, Allocator, GrowthNum, GrowthDen>::as_span() const noexcept {
	return std::span<const T>(data_, vsize_);
}

// Element Access
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::pointer
Vector<T, Allocator, GrowthNum, GrowthDen>::data_ptr() noexcept {
	return data_;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_pointer
Vector<T, Allocator, GrowthNum, GrowthDen>::data_ptr() const noexcept {
	return data_;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::reference
Vector<T, Allocator, GrowthNum, GrowthDen>::at(std::size_t index) {
	if (index >= vsize_) throw std::out_of_range("Vector::at() index out of range");

	return data_[index];
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_reference
Vector<T, Allocator, GrowthNum, GrowthDen>::at(std::size_t index) const {
	if (index >= vsize_) throw std::out_of_range("Vector::at() index out of range");

	return data_[index];
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::reference
Vector<T, Allocator, GrowthNum, GrowthDen>::front() {
	return data_[0];
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_reference
Vector<T, Allocator, GrowthNum, GrowthDen>::front() const {
	return data_[0];
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::reference
Vector<T, Allocator, GrowthNum, GrowthDen>::back() {
	return data_[vsize_ - 1];
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_reference
Vector<T, Allocator, GrowthNum, GrowthDen>::back() const {
	return data_[vsize_ - 1];
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::reference
Vector<T, Allocator, GrowthNum, GrowthDen>::operator[](std::size_t index) noexcept {
	return data_[index];
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_reference
Vector<T, Allocator, GrowthNum, GrowthDen>::operator[](std::size_t index) const noexcept {
	return data_[index];
}

// Search
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
bool Vector<T, Allocator, GrowthNum, GrowthDen>::contains(const T& value) const noexcept {
	for (std::size_t i = 0; i < vsize_; ++i) {
		if (data_[i] == value) return true;
	}

	return false;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::find(const T& value) noexcept {
	for (std::size_t i = 0; i < vsize_; ++i) {
		if (data_[i] == value) return iterator(data_ + i);
	}
	return end();
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::find(const T& value) const noexcept {
	for (std::size_t i = 0; i < vsize_; ++i) {
		if (data_[i] == value) return const_iterator(data_ + i);
	}
	return end();
}

// Swap
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::swap(Vector& other) noexcept {
	using std::swap;
	swap(alloc_,      other.alloc_);
	swap(data_,       other.data_);
	swap(vsize_,      other.vsize_);
	swap(vcap_,       other.vcap_);
	swap(listeners_,  other.listeners_);
	swap(lsize_,      other.lsize_);
	swap(lcap_,       other.lcap_);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
void swap(
    Vector<T, Allocator, GrowthNum, GrowthDen>& a,
    Vector<T, Allocator, GrowthNum, GrowthDen>& b
) noexcept {
	a.swap(b);
}

// Capacity
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
bool Vector<T, Allocator, GrowthNum, GrowthDen>::empty() const noexcept {
	return vsize_ == 0;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
std::size_t Vector<T, Allocator, GrowthNum, GrowthDen>::size() const noexcept {
	return vsize_;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
std::size_t Vector<T, Allocator, GrowthNum, GrowthDen>::capacity() const noexcept {
	return vcap_;
}

// Iterators
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::begin() noexcept {
	return iterator(data_);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::end() noexcept {
	return iterator(data_ + vsize_);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::begin() const noexcept {
	return const_iterator(data_);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::end() const noexcept {
	return const_iterator(data_ + vsize_);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::cbegin() const noexcept {
	return const_iterator(data_);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::cend() const noexcept {
	return const_iterator(data_ + vsize_);
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::reverse_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::rbegin() noexcept {
	return reverse_iterator(end());
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::reverse_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::rend() noexcept {
	return reverse_iterator(begin());
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_reverse_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::rbegin() const noexcept {
	return const_reverse_iterator(end());
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>:: const_reverse_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::rend() const noexcept {
	return const_reverse_iterator(begin());
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_reverse_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::crbegin() const noexcept {
	return const_reverse_iterator(end());
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
typename Vector<T, Allocator, GrowthNum, GrowthDen>::const_reverse_iterator
Vector<T, Allocator, GrowthNum, GrowthDen>::crend() const noexcept {
	return const_reverse_iterator(begin());
}

// Private Helpers
template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::release() noexcept {
	for (std::size_t i = 0; i < vsize_; ++i) {
		std::allocator_traits<Allocator>::destroy(alloc_, data_ + i);
	}

	std::allocator_traits<Allocator>::deallocate(alloc_, data_, vcap_);

	data_   = nullptr;
	vsize_  = 0;
	vcap_   = 0;
}


template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
std::size_t Vector<T, Allocator, GrowthNum, GrowthDen>::growCapacity() const noexcept {
	if (vcap_ == 0) return INITIAL_CAP;
	return vcap_ * GrowthNum / GrowthDen;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::reallocate(std::size_t newCap) {
	pointer newData = std::allocator_traits<Allocator>::allocate(alloc_, newCap);

	for (std::size_t i = 0; i < vsize_; ++i) {
		std::allocator_traits<Allocator>::construct(alloc_, newData + i, std::move(data_[i]));
		std::allocator_traits<Allocator>::destroy(alloc_, data_ + i);
	}

	data_ = newData;
	vcap_ = newCap;
}

template<typename T,
         typename Allocator,
         std::size_t GrowthNum,
         std::size_t GrowthDen>
requires std::destructible<T>
void Vector<T, Allocator, GrowthNum, GrowthDen>::notify(EventData data) {
	for (std::size_t i = 0; i < lsize_; ++i) {
		if (listeners_[i]) listeners_[i](*this, data);
	}
}

} // namespace VectorPro