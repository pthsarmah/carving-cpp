#pragma once

#include <any>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

template<typename Vector>
class VectorIterator {
public:
	using value_type = typename Vector::value_type;
	using pointer_type = std::conditional_t<std::is_const_v<Vector>, const value_type*, value_type*>;
	using reference_type = std::conditional_t<std::is_const_v<Vector>, const value_type&, value_type&>;
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::bidirectional_iterator_tag;
public:
	VectorIterator(pointer_type ptr): ptr_(ptr) {}
	//this magic code is used to convert a normal iterator to a const_iterator, typename OtherVector makes sure that when we can accept 'iterator' for a const_iterator variable, if we wrote it as Vector it would not work since const_iterator would fix Vector during compile time, which would be Vector == const Vector<T>, thus defeating the purpose of conversion. Also the requires clause contains three conditions which need to be simultaneously true, 1. Vector must be const (const_iterator), 2. OtherVector must be non-const (iterator), 3. if we remove const from both they should point to the same type underneath.
	template<typename OtherVector> requires (std::is_const_v<Vector> && !std::is_const_v<OtherVector> && std::is_same_v<std::remove_const_t<OtherVector>, std::remove_const_t<Vector>>)
	VectorIterator(const VectorIterator<OtherVector>& other): ptr_(other.ptr_) {}
	VectorIterator& operator++() {
		++ptr_;
		return *this;
	}
	VectorIterator operator++(int) {
		VectorIterator iterator = *this;
		++(*this);
		return iterator;
	}
	VectorIterator& operator--() {
		--ptr_;
		return *this;
	}
	VectorIterator operator--(int) {
		VectorIterator iterator = *this;
		--(*this);
		return iterator;
	}
	difference_type operator-(const VectorIterator& other) const {
		return ptr_ - other.ptr_;
	}
	VectorIterator operator-(difference_type n) const {
		return VectorIterator(ptr_ - n);
	}
	VectorIterator operator+(difference_type n) const {
		return VectorIterator(ptr_ + n);
	}
	VectorIterator operator-=(difference_type n) {
		ptr_ -= n;
		return *this;
	}
	VectorIterator operator+=(difference_type n) {
		ptr_ += n;
		return *this;
	}
	bool operator==(const VectorIterator& other) const {
		return ptr_ == other.ptr_;
	}
	bool operator!=(const VectorIterator& other) const {
		return !(*this == other);
	}
	reference_type operator[](std::size_t index) const {
		return *(ptr_ + index);
	}
	pointer_type operator->() const {
		return ptr_;
	}
	reference_type operator*() const {
		return *ptr_;
	}
private:
	pointer_type ptr_;
	// we need a friend class so that VectorIterator<Vector> can also change stuff in all transformations of VectorIterator (like VectorIterator<const Vector>). Basically, telling VectorIterator that all the different types of itself are also its friends and it can access private variables of them.
	template<typename>
	friend class VectorIterator;
};

template<typename T>
class Vector {
private:
	std::unique_ptr<T[]> data_ = nullptr;
	std::size_t size_ = 0;
	std::size_t capacity_ = 0;
private:
	void realloc(std::size_t capacity) {
		auto newData = std::make_unique<T[]>(capacity);
		if (capacity < size_) {
			size_ = capacity;
		}

		for (std::size_t i=0; i<size_; i++) {
			newData[i] = std::move(data_[i]);
		}

		data_ = std::move(newData);
		capacity_ = capacity;
	}
public:
	using value_type = T;
	using iterator = VectorIterator<Vector<T>>;
	using const_iterator = VectorIterator<const Vector<T>>;
public:
	//default constructor
	Vector() { realloc(2); }
	//copy constructor
	Vector(const Vector& other): size_(other.size_), capacity_(other.capacity_), data_(capacity_ ? std::make_unique<T[]>(capacity_) : nullptr) {
		for (std::size_t i=0; i < size_; i++) {
			data_[i] = other.data_[i];
		}
	}
	//move constructor
	Vector(Vector&& other) noexcept : size_(std::exchange(other.size_, 0)), capacity_(std::exchange(other.capacity_, 0)), data_(std::move(other.data_)) {}
	//copy assignment operator
	Vector& operator=(const Vector& other) {
		if (this == &other) return *this;
		auto new_data = other.capacity_ ? std::make_unique<T[]>(other.capacity_) : nullptr;
		for (std::size_t i = 0; i < other.size_; i++) {
			new_data[i] = other.data_[i];
		}

		data_ = std::move(new_data);
		size_ = other.size_;
		capacity_ = other.capacity_;

		return *this;
	}
	//move assignment operator
	Vector& operator=(Vector&& other) {
		data_ = std::move(other.data_);
		size_ = other.size_;
		capacity_ = other.capacity_;

		other.capacity_ = 0;
		other.size_ = 0;

		return *this;
	}
	//destructor
	~Vector () {}

	//iterators
	iterator begin() { return iterator(data_.get()); }
	iterator end() { return iterator(data_.get() + size_); }

	//const iterators
	const_iterator begin() const { return const_iterator(data_.get()); }
	const_iterator end() const { return const_iterator(data_.get() + size_); }

	std::size_t size() const {
		return size_;
	}
	std::size_t capacity() const {
		return capacity_;
	}
	bool empty() const {
		return size_ == 0;
	}
	T& operator[](std::size_t idx) {
		return data_[idx];
	}
	const T& operator[](std::size_t idx) const {
		return data_[idx];
	}
	const T& at(std::size_t idx) const {
		if (idx >= size_) throw std::out_of_range("Index out of range");
		return data_[idx];
	}
	const T& front() const {
		if (size_ <= 0) throw std::out_of_range("No elements");
		return data_[0];
	}
	const T& back() const {
		if (size_ <= 0) throw std::out_of_range("No elements");
		return data_[size_ - 1];
	}
	void push_back(const T& element) {
		if (size_ == capacity_) {
			realloc(capacity_ == 0 ? 2 : capacity_ + capacity_ / 2);
		}

		data_[size_] = element;
		size_++;
	}
	void push_back(T&& element) {
		if (size_ == capacity_) {
			realloc(capacity_ == 0 ? 2 : capacity_ + capacity_ / 2);
		}

		data_[size_] = std::move(element);
		size_++;
	}
	T pop_back() {
		if (size_ <= 0 || capacity_ <= 0)
			throw std::out_of_range("No elements");

		size_--;
		return data_[size_];
	}
	template<typename... Args>
	T& emplace_back(Args&&... args) {
		if (size_ == capacity_)
			realloc(capacity_ == 0 ? 2 : capacity_ + capacity_ / 2);

		data_[size_] = T(std::forward<Args>(args)...);
		return data_[size_++];
	}
};
