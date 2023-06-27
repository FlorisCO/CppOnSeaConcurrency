#include <iostream>

#include <iostream>
#include <thread>
#include <mutex>

// #include <condition_variable>
// #include <vector>

bool bPingPongBall {false};

constexpr int maxIterations = 50;

std::atomic_int iterator { 0 };

std::mutex mxPingPongLock {};

std::condition_variable cvPing {};

std::condition_variable cvPong {};

void SetPingPongBall(bool state)
{
    bPingPongBall = state;
    std::cout << "Game state: " << (bPingPongBall ? "Ping" : "Pong") << std::endl;
}

void PingPong(bool desiredState, std::condition_variable & cvWait, std::condition_variable & cvSignal)
{
    while (iterator < maxIterations)
    {
        std::unique_lock<std::mutex> ul(mxPingPongLock);

        // Note, the condition variable has the lock when it checks the conditional
        cvWait.wait(ul, [desiredState] {return bPingPongBall != desiredState; });

        SetPingPongBall(desiredState);
        cvSignal.notify_one();

        iterator++;
    }
}

void Ping()
{
    PingPong(true, cvPong, cvPing);
    /*for (int i = 0; i < iterations; i++)
    {
        SetPingPongBall(true);
        cvPing.notify_one();

        std::unique_lock<std::mutex> ul(mxPingPongLock);
        cvPong.wait(ul, []{return !bPingPongBall;});
    }*/
}

void Pong()
{
    PingPong(false, cvPing, cvPong);

    /*for (int i = 0; i < iterations; i++)
    {
        std::unique_lock<std::mutex> ul(mxPingPongLock);
        cvPing.wait(ul, [] {return static_cast<bool>(bPingPongBall); });

        SetPingPongBall(false);
        cvPong.notify_one();
    }*/
}

int main()
{
    std::thread ping(Ping);
    std::thread pong(Pong);

    ping.join();
    pong.join();

    std::cout << "Game ended\n";
}
