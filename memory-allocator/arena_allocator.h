#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <print>
#include <stdexcept>

struct Block {
public:
	std::size_t block_size_;
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
			if (curr->block_size_ >= req_size) {
				if (best != nullptr && curr->block_size_ <= best->block_size_)
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
	void join_free_nodes(Block* curr, Block* next, std::size_t alignment) {
		if (curr == nullptr || next == nullptr)
			return;


		if (curr->is_free && next->is_free) {
			char* curr_end = reinterpret_cast<char*>(curr) + curr->block_size_;
			char* expected_next = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>((curr_end + alignment - 1)) & ~(static_cast<std::uintptr_t>(alignment - 1)));

			if (expected_next == reinterpret_cast<char*>(next)) {
				curr->block_size_ =
						reinterpret_cast<char*>(next) + next->block_size_ -
						reinterpret_cast<char*>(curr);

				curr->next = next->next;

				next->next = nullptr;
				next->block_size_ = 0;
				next->is_free = false;

				join_free_nodes(curr, curr->next, alignment);
				return;
			}
		}

		join_free_nodes(next, next->next, alignment);
	}
	void* allocate_free_node(std::size_t size, std::size_t alignment) {
		if (free_ptr_ == nullptr) throw std::out_of_range("Out of memory");

		Block* req = best_fit_free_node(size + sizeof(Block));
		if (req == nullptr) throw std::out_of_range("No available free blocks");

		std::size_t total_req = size + sizeof(Block);

		if (req->block_size_ <= total_req + sizeof(Block)) {
			req->is_free = false;
			clean_free_list();
			
			if (free_ptr_ && free_ptr_->next)
				join_free_nodes(free_ptr_, free_ptr_->next, alignment);

			return reinterpret_cast<char*>(req) + sizeof(Block);
		}

		char* split_start = reinterpret_cast<char*>(req) + total_req;

		std::uintptr_t user_addr =
			(reinterpret_cast<std::uintptr_t>(split_start + sizeof(Block)) + alignment - 1) &
			~static_cast<std::uintptr_t>(alignment - 1);

		Block* resized = reinterpret_cast<Block*>(user_addr - sizeof(Block));

		std::size_t padding = reinterpret_cast<char*>(resized) - split_start;
		std::size_t resized_size = req->block_size_ - total_req - padding;

		if (resized_size >= sizeof(Block) + 1) {
			resized->block_size_ = resized_size;
			resized->is_free = true;
			resized->next = req->next;
			req->next = resized;

			req->block_size_ = total_req;
		} else {
			req->block_size_ = req->block_size_;
		}

		req->is_free = false;
		clean_free_list();
		join_free_nodes(free_ptr_, free_ptr_->next, alignment);

		return reinterpret_cast<char*>(req) + sizeof(Block);
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

		//I was aligning the header pointer first and appending the block afterwards, immediately starting the user pointer, which means user pointer is not guaranteed to be aligned. Allocators prefer aligning the user pointer and adding the header before it
		std::uintptr_t raw = reinterpret_cast<std::uintptr_t>(current_) + sizeof(Block);
		std::uintptr_t user_addr = (raw + alignment - 1) & ~(static_cast<std::uintptr_t>(alignment - 1));
		Block* header = reinterpret_cast<Block*>(user_addr - sizeof(Block));

		std::size_t total_size = (user_addr - reinterpret_cast<std::uintptr_t>(current_)) + size;

		if (reinterpret_cast<char*>(header) + total_size > end_) {
			return allocate_free_node(size, alignment);
		}

		header->block_size_ = size + sizeof(Block);
		header->is_free = false;
		header->next = nullptr;

		current_ = reinterpret_cast<char*>(current_) + total_size;
		return reinterpret_cast<void*>(user_addr);
	}
	void deallocate(void* ptr) {
		Block* header = reinterpret_cast<Block*>(reinterpret_cast<char*>(ptr) - sizeof(Block));
		if (header->is_free) {
			header->next = nullptr;
			return;
		}
		header->is_free = true;
		header->next = nullptr;
		
		add_free_node(header);
	}	
	void free_list() const {
		if (free_ptr_ == nullptr) return;
		Block* curr = free_ptr_;
		while (curr != nullptr) {
			curr = curr->next;
		}
	}
	void debug() const {
		std::size_t extra_size = 0;

		if (free_ptr_ != nullptr) {
			Block* curr = free_ptr_;
			while (curr != nullptr) {
					if (curr->is_free) extra_size += curr->block_size_;
					curr = curr->next;
			}
		}

		if (end_ >= current_) {
			auto remaining = static_cast<std::size_t>(end_ - current_);
			std::println("Memory left: {} bytes", remaining + extra_size);
		}
	}
};
