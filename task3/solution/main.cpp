#include <vector>
#include <future>
#include <iostream>
#include <algorithm>

#include <libmail/send_mail.h>
#include <libmail/randomize_results.h>

std::future<bool> send_mail_async(const std::string& address, const std::string& message)
{
	std::promise<bool> p;
	auto f = p.get_future();
	auto send_mail_wrapper = [address, message](std::promise<bool> p)
	{
		try
		{
			libmail::send_mail(address, message);
			p.set_value(true);
		}
		catch(...)
		{
			p.set_exception(std::current_exception());
		}
	};
	std::thread t(send_mail_wrapper, std::move(p));
	t.detach();
	return f;
}

struct notification_result
{
	std::string debug_info;
	bool result;
};

std::vector<notification_result> notify_all(const std::vector<std::string>& addresses, const std::string& message)
{
	std::vector<notification_result> results(addresses.size());
	std::vector<std::future<bool>> async_results(addresses.size());

	for(std::size_t i = 0; i < addresses.size(); ++i)
	{
		async_results[i] = send_mail_async(addresses[i], message);
	}

	for(std::size_t i = 0; i < addresses.size(); ++i)
	{
		try
		{
			results[i].result = async_results[i].get();
		}
		catch(const std::exception& e)
		{
			results[i].debug_info = e.what();
			results[i].result = false;
		}
	}

	return results;
}

void log_results(const std::vector<std::string>& addresses, const std::vector<notification_result>& results)
{
	for(std::size_t i = 0; i < addresses.size(); ++i)
	{
		std::cout << "Sending mail to: " << addresses[i] << " - "
		          << (results[i].result ? "OK" : results[i].debug_info ) << std::endl;
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

	auto results = notify_all(responsible_devs, message);
	auto failed = [](const notification_result& notif_result)
	{
		return !notif_result.result;
	};
	if(std::any_of(results.begin(), results.end(), failed))
	{
		log_results(responsible_devs, results);
	}
}