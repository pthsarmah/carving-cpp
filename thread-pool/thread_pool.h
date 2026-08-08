#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
private:
	std::queue<std::function<void()>> taskQueue;
	std::vector<std::thread> threads;
	bool stop_all_workers_ = false;
	std::condition_variable cv;
	std::mutex mut;
public:
	ThreadPool(int num_threads) {
		threads.reserve(num_threads);
		for (int i=0; i<num_threads; i++) {
			threads.emplace_back([this](){
				//infinite loop to check if any task in queue
				while(1) {
					std::unique_lock<std::mutex> lock{mut};
					cv.wait(lock, [this] { return stop_all_workers_ || !taskQueue.empty(); });
					if (stop_all_workers_ && taskQueue.empty())
						return;
					auto task = std::move(taskQueue.front());
					taskQueue.pop();
					lock.unlock();
					task();
				}
			});
		}
	}
	template<typename F>
	void enqueue(F&& task) {
		std::unique_lock<std::mutex> lock{mut};
		taskQueue.emplace(std::forward<F>(task));
		lock.unlock();
		cv.notify_one();
	}
	~ThreadPool() {
		std::unique_lock<std::mutex> lock{mut};
		stop_all_workers_ = true;
		lock.unlock();
		cv.notify_all();
		for (std::thread& t : threads) {
			t.join();
		}
	}
};
