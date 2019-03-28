#include <vector>
#include <iostream>
#include <future>
#include <thread>
#include <tuple>
#include <string>
#include <libmail/send_mail.h>
#include <libmail/randomize_results.h>

void send(const std::string &address, const std::string &message, std::promise<std::pair<bool, std::string>> prom)
{
	try {
		libmail::send_mail(address, message);
		prom.set_value(std::make_pair(true, ""));
	} catch (const std::runtime_error & re) {
		prom.set_value(std::make_pair(false, re.what()));
	}
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

	std::vector<std::promise<std::pair<bool, std::string>>> results{responsible_devs.size()};
	std::vector<std::future<std::pair<bool, std::string>>> futures{};
	std::vector<std::thread> threads;

	for (auto &&promise : results)
	{
		futures.emplace_back(promise.get_future());
	}

	for (std::size_t i = 0u; i < responsible_devs.size(); i++)
	{
		threads.emplace_back(send, responsible_devs[i], message, std::move(results.at(i)));
	}
	for (std::size_t i = 0u; i < futures.size(); i++)
	{
		futures[i].wait();
		auto result = futures[i].get();
		if (!result.first)
		{
			std::cout << "Not delivered to " << responsible_devs[i]
					  << ", cause: " << result.second << std::endl;
		}
	}
	for (auto &&thread : threads)
	{
		thread.join();
	}
}