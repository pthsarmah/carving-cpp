#include "arena_allocator.h"
#include <print>

int main() {
	Arena arena(1024);

	void* ptr = arena.allocate(500, 16);
	void* tptr = arena.allocate(12, 16);

	std::println("{:p}", ptr);

	arena.debug();

	return 0;
}
