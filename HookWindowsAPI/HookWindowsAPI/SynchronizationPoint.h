#pragma once

#include <future>

using namespace std;

class SynchronizationPoint
{
public:

    SynchronizationPoint() : promiseObject(), futureObject(promiseObject.get_future())
    {
    }

    // Signal that the thread has completed its task
    void Signal()
    {
        promiseObject.set_value();
    }

    // Wait for the signal that the task has been completed
    void Wait()
    {
        futureObject.wait();
    }

private:

    std::promise<void> promiseObject;
    std::future<void> futureObject;
};

