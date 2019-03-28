#include <vector>
#include <iostream>
#include <future>
#include <thread>
#include <libmail/send_mail.h>
#include <libmail/randomize_results.h>

void send(const std::string &address, const std::string &message, std::promise<bool> prom)
{
	auto b = libmail::send_mail(address, message);
	prom.set_value(b);
}

int main()
{
	libmail::randomize_results();
	std::vector<std::string> responsible_devs = {
		"dev1@company.com",
		"dev2@company.com",
		"dev3@company.com",
		"dev4@company.com",
		"dev5@company.com",
	};
	std::string message = "build failed";

	std::vector<std::promise<bool>> results{responsible_devs.size()};
	std::vector<std::future<bool>> futures{};
	std::vector<std::thread> threads;

	for (auto && promise : results) {
		futures.emplace_back(promise.get_future());
	}

	for (std::size_t i = 0u; i < responsible_devs.size(); i++) {
		threads.emplace_back(send, responsible_devs[i], message, std::move(results.at(i)));
	}
	for (std::size_t i = 0u; i < futures.size(); i++){
		futures[i].wait();
		if (!futures[i].get()) {
			std::cout << "Not delivered to " << responsible_devs[i] << std::endl;
		}
	}
	for (auto && thread : threads) {
		thread.join();
	}
}