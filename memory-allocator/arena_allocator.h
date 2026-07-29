#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <print>
#include <stdexcept>

class Arena {
private:
	char* start_; //char is 1 byte so good ol' byte-addressable memory
	char* end_;
	char* current_;
	enum Alignment { Align16 = 16, Align32 = 32, Align64 = 64 };
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
	void* allocate(std::size_t size, int alignment) {
		switch (alignment) {
			case 16:
			case 32:
			case 64:
					break;
			default:
					throw std::runtime_error("wrong alignment option");
		}

		//apparently I have misunderstood pointers = integers, which is wrong, pointers are their own inherent type, so they need to be converted to integers to do bit manipulation
		std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(current_);
		//bit math to align the pointer to the next greatest multiple of the alignment factor
		addr = (addr + alignment - 1) & ~(alignment - 1);

		char* aligned = reinterpret_cast<char*>(addr);

		if (aligned + size > end_) {
			throw std::out_of_range("Out of memory");
		}

		void* ptr = aligned;
		current_ = aligned + size;
		return ptr;

	}
	void* allocate(std::size_t size) {
		if (current_ + size > end_) {
			throw std::out_of_range("Out of memory");
		}

		void* ptr = current_;
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
