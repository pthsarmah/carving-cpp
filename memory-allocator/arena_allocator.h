#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <print>
#include <stdexcept>

struct Block {
public:
	std::size_t size_;
	bool is_free;
	Block* next;
};

class Arena {
private:
	char* start_; //char is 1 byte so good ol' byte-addressable memory
	char* end_;
	char* current_;
	Block* free_ptr_ = nullptr;
	enum Alignment { Align16 = 16, Align32 = 32, Align64 = 64 };
private:
	void add_free_node(Block* ptr) {
		if (!ptr->is_free) return;
		if (free_ptr_ == nullptr) free_ptr_ = ptr;
		else {
			Block* curr = free_ptr_;
			while (curr->next != nullptr) curr = curr->next;
			curr->next = ptr;
		}
	}
	void remove_free_node(std::size_t size) {

	}
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
	void* allocate(std::size_t size, int alignment = 16) {
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
		//taking in unaligned space in block, as part of internal fragmentation
		//so size = header size + user data size + alignment padding
		size = (size + sizeof(Block) + alignment - 1) & ~(alignment - 1);

		char* aligned = reinterpret_cast<char*>(addr);

		if (aligned + size > end_) {
			throw std::out_of_range("Out of memory.");
		}

		Block* header = reinterpret_cast<Block*>(aligned);
		
		header->size_ = size;

		void* user = reinterpret_cast<char*>(header) + sizeof(Block);
		current_ = reinterpret_cast<char*>(header) + size;
		return user;
	}
	void deallocate(void* ptr) {
		Block* header = reinterpret_cast<Block*>(reinterpret_cast<char*>(ptr) - sizeof(Block));
		header->is_free = true;
		add_free_node(header);
	}
	void reset() {
		current_ = start_;
	}
	void free_list() const {
		if (free_ptr_ == nullptr) return;
		Block* curr = free_ptr_;
		while (curr != nullptr) {
			std::println("{}, Size: {}", static_cast<void*>(curr), curr->size_);
			curr = curr->next;
		}
	}
	void debug() const {
		if (end_ >= current_)
			std::println("Memory left: {} bytes", end_ - current_);
	}
};
