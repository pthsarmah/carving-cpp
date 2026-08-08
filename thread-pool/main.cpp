#include "thread_pool.h"
#include <chrono>
#include <print>
#include <sstream>
#include <string>
#include <thread>

std::string get_thread_id() {
	auto id = std::this_thread::get_id();
	std::stringstream ss;
	ss << id;
	std::string stringId = ss.str();
	return stringId;
}

int main() {
	ThreadPool pool(4);
	
	for (int i=0; i<8; i++) {
		pool.enqueue([i] {
			std::println("Task {} is running on thread {}", i, get_thread_id());
			std::this_thread::sleep_for(std::chrono::seconds(1));
		});
	}
	return 0;
}
