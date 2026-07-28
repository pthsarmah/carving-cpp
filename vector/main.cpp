#include "vector.h"
#include <print>

int main() {
	Vector<int> v;
	std::println("Size: {}\nCapacity: {}\n", v.size(), v.capacity());

	v.push_back(5);
	v.push_back(4);
	v.push_back(3);
	v.push_back(2);
	v.push_back(1);
	v.push_back(0);

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
