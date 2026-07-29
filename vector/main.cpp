#include "vector.h"
#include <print>

int main() {
	Vector<int> v = {5, 4, 3, 2, 1};
	std::println("Size: {}\nCapacity: {}\n", v.size(), v.capacity());

	std::println("at: {}", v.at(2));
	std::println("front: {}", v.front());
	std::println("back: {}", v.back());
	std::println("pop_back: {}", v.pop_back());
	std::println("pop_back: {}\n", v.pop_back());

	for (int ele : v) {
		std::println("{}", ele);
	}

	std::println("\nSize: {}\nCapacity: {}\n", v.size(), v.capacity());

	return 0;
}
