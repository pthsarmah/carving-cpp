#include "arena_allocator.h"

int main() {
	Arena arena(96);

	void* ptrs[3];

	for (void*& p : ptrs) {
		p = arena.allocate(5);
		arena.debug();
	}

//	arena.allocate(5);
	arena.deallocate(ptrs[1]);

	arena.debug();
	arena.allocate(5);

	arena.debug();

	return 0;
}
