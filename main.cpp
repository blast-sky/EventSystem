#include <iostream>
#include "Event.hpp"

void staticFunction(const int&)
{
    std::cout << "staticFunction\n";
}

class Test
{
public:
    void onEvent(const int& num)
    {
        std::cout << "onEvent\n";
    }
};

int main()
{
    Event<const int&> event;
    Test test;

    event.attach(&Test::onEvent, &test);
    event.attach(&staticFunction);

    event(10);

    event.detach(&Test::onEvent, &test);

    event(10);

    event.detach(&staticFunction);

    event(10);

    return 0;
}