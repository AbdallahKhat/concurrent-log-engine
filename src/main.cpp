#include "thread_safe_queue.hpp"
#include <iostream>
#include <thread>

void producer_worker(ThreadSafeQueue<std::string>& queue)
{
    for (int i = 0; i <= 50; ++i)
    {
        std::string log = "Log message line number: " + std::to_string(i);
        queue.push(log);
        std::cout << "[Producer] Pushed: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void consumer_worker(ThreadSafeQueue<std::string>& queue)
{
    std::string data;
    while (true)
    {
        queue.pop(data);
        std::cout << "[Consumer] Processed: " << data << std::endl;
    }
}

int main()
{
    ThreadSafeQueue<std::string> queue;

    std::jthread producer(producer_worker, std::ref(queue));
    std::jthread consumer(consumer_worker, std::ref(queue));

    return 0;
}
