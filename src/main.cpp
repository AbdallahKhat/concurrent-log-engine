#include <iostream>
#include <thread>

int main()
{

    std::cout << "Engine Initialized." << std::endl;

    std::jthread jthr([] { std::cout << "jthread created successfully!" << std::endl; });

    return 0;
}
