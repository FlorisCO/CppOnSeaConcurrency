#include <future>
#include <iostream>
#include <thread>
#include <utility>

void product(std::promise<int>&& intPromise, int a, int b) {
	intPromise.set_value(a * b);
}

struct Div {

	void operator() (std::promise<int>&& intPromise, int a, int b) const
	{
		try {
			if (b == 0)
			{
				throw std::runtime_error("Can't divide by zero.");
			}

			intPromise.set_value(a / b);
		}
		catch (...)
		{
			intPromise.set_exception(std::current_exception());
		}
	}
};

int main() {

	int a = 20;
	int b = 0;

	std::cout << std::endl;

	// define the promises
	std::promise<int> prodPromise;
	std::promise<int> divPromise;

	// get the futures
	std::future<int> prodResult = prodPromise.get_future();
	std::future<int> divResult = divPromise.get_future();

	// calculate the result in a separat thread
	std::thread prodThread(product, std::move(prodPromise), a, b);
	Div div;
	std::thread divThread(div, std::move(divPromise), a, b);

	// get the result
	std::cout << "20*1= " << prodResult.get() << std::endl;

	std::cout << "20/0= ";

	try
	{
		std::cout << divResult.get();
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what();
	}

	std::cout << std::endl;

	prodThread.join();
	divThread.join();

	std::cout << std::endl;
}
