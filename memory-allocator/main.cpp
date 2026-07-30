#include "arena_allocator.h"
#include <print>

int main() {
	Arena arena(1024);
	void* ptr = arena.allocate(512, 16);

	std::println("{:p}", ptr);

	arena.debug();
	arena.free_list();

	arena.deallocate(ptr);

	arena.free_list();

	return 0;
}
