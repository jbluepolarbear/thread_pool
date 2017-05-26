thread_pool
==========

A Modern C++ Thread Pool.

The use case here is to block the main thread as little as possible.

Basic usage:
```c++

// make thread pool with 10 threads
thread_pool threadPool(10);

size_t total = 0;
size_t numTasks = 1000;
// add a task to be run on a thread
for (size_t i = 0; i < numTasks; ++i)
{
    threadPool.enqueue([i, &total]()
    {
        // do some work
        std::stringstream ss;
        ss << "Task " << i << ": This was made on a background thread.";

        // result function that will be called on the main thread
        return std::bind([&total](size_t addToTotal, std::string msg)
        {
            //safe because only ever done on main thread
            total += addToTotal;
            std::cout << "Message Returned from Background thread: " << msg << std::endl;
        }, i + 1, ss.str());
    });
}

int finalTotal = (numTasks * (numTasks + 1)) / 2;
// must update thread_pool to make sure result functions are called
do
{
    threadPool.update();
} while (total != finalTotal);

```