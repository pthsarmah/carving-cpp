#include "arena_allocator.h"

int main() {
	Arena arena(128);

	void* ptrs[2];

	for (void*& p : ptrs) {
		p = arena.allocate(30);
		arena.debug();
	}

//	arena.allocate(5);
	arena.deallocate(ptrs[1]);

	arena.debug();
	arena.allocate(6);

	arena.debug();

	return 0;
}
