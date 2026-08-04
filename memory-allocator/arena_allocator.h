#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <print>
#include <stdexcept>

struct Block {
public:
	std::size_t size_;
	Block* next = nullptr;
	bool is_free = 1;
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
	Block* best_fit_free_node(std::size_t size) {
		//sequential search to find best fit node
		if (free_ptr_ == nullptr) return nullptr;
		Block* curr = free_ptr_;
		Block* best = nullptr;
		
		std::size_t req_size = size;

		while (curr != nullptr) {
			if (curr->size_ >= req_size) {
				if (best != nullptr && curr->size_ <= best->size_)
					best = curr;
				else if (best == nullptr) best = curr;
			}
			curr = curr->next;
		}
		return best;
	}
	void clean_free_list() {
		Block** link = &free_ptr_;

		while (*link != nullptr) {
			if (!(*link)->is_free) {
				*link = (*link)->next;
			} else {
				link = &(*link)->next;
			}
		}
	}
	void* allocate_free_node(std::size_t size) {
		if (free_ptr_ == nullptr) throw std::out_of_range("Out of memory");
		
		Block* req = best_fit_free_node(size);
		if (req == nullptr) throw std::out_of_range("No available free blocks");

		//resize block if needed
		std::size_t diff = req->size_ - size; 
		if (diff > sizeof(Block) + 1) {
			Block* resized = reinterpret_cast<Block*>(reinterpret_cast<char*>(req) + size);
			resized->size_ = diff;
			resized->is_free = true;
			resized->next = req->next;
			req->next = resized;
		}

		req->size_ = size;
		req->is_free = false;

		void* user = reinterpret_cast<char*>(req) + sizeof(Block);

		clean_free_list();

		return user;
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
		if (alignment <= 0)
			throw std::runtime_error("alignment is not positive");
		if ((alignment & (alignment - 1)) != 0)
			throw std::runtime_error("alignment must be power of 2");

		//apparently I have misunderstood pointers = integers, which is wrong, pointers are their own inherent type, so they need to be converted to integers to do bit manipulation
		std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(current_);
		//bit math to align the pointer to the next greatest multiple of the alignment factor
		addr = (addr + alignment - 1) & ~(alignment - 1);
		//taking in unaligned space in block, as part of internal fragmentation
		//so size = header size + user data size + alignment padding
		size = (size + sizeof(Block) + alignment - 1) & ~(alignment - 1);

		char* aligned = reinterpret_cast<char*>(addr);

		if (aligned + size > end_) {
			//allocate from free nodes, if present
			return allocate_free_node(size);
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
		std::size_t extra_size = 0;

		if (free_ptr_ != nullptr) {
			Block* curr = free_ptr_;
			while (curr != nullptr) {
					if (curr->is_free) extra_size += curr->size_;
					curr = curr->next;
			}
		}

		if (end_ >= current_) {
			auto remaining = static_cast<std::size_t>(end_ - current_);
			std::println("Memory left: {} bytes", remaining + extra_size);
		}
	}
};
