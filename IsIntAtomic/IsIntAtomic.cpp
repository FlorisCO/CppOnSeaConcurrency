#include <iostream>
#include <thread>
#include<vector>

int sharedInt { 0 };
int sharedInt2{ 0 };

constexpr int ExpectedIterations = 100'000'000;
constexpr int ExpectedThreads = 10;

void IncreaseTheSharedInt()
{
    for (int i = 0; i < ExpectedIterations; i++)
    {
        sharedInt++;
        ++sharedInt2;
    }
}

int main()
{
    std::vector<std::thread> v;

    for (int i = 0; i < ExpectedThreads; i++)
    {
        v.emplace_back(IncreaseTheSharedInt);
    }

    for(std::thread& t : v)
    {
        t.join();
    }

    std::cout.imbue(std::locale(""));
    std::cout << "Expected value: " << std::endl;
    std::cout << (ExpectedThreads * ExpectedIterations) << std::endl;
    std::cout << std::endl;
    std::cout << sharedInt << std::endl;
    std::cout << sharedInt2 << std::endl;
}
