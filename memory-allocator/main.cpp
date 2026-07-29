#include "arena_allocator.h"
#include <print>

int main() {
	Arena arena(1024);

	void* ptr = arena.allocate(512);

	std::println("{:p}", ptr);

	arena.debug();

	return 0;
}
