#pragma once

#include <cstddef>
#include <print>
#include <stdexcept>

class Arena {
private:
	char* start_; //char is 1 byte so good ol' byte-addressable memory
	char* end_;
	char* current_;
public:
	Arena(const Arena& arena) = delete;
	Arena(std::size_t size) {
		start_ = new char[size];
		end_ = start_ + size; // end_ is 1 byte BEYOND the address boundary
		current_ = start_;
	}
	~Arena() {
		delete[] start_;
	}
	void* allocate(std::size_t size) {
		if (current_ + size > end_) {
			throw std::out_of_range("Out of memory");
		}

		void* ptr = (void*)current_;
		current_ += size;
		return ptr;
	}
	void reset() {
		current_ = start_;
	}
	void debug() const {
		if (end_ >= current_)
			std::println("Memory left: {} bytes", end_ - current_);
	}
};
