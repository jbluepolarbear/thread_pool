thread_pool
==========

A Modern C++ Thread Pool.

The use case here is it block the main thread as little as possible.

Basic usage:
```c++

// make thread pool with 10 threads
thread_pool threadPool(10);

// add a task to be run on a thread
threadPoolpool.enqueue([](int answer)
{
    // do some work
    string msg = "This was made on a background thread.";

    // function that will be called on the main thread
    return [msg]()
    {
        std::cout << "Message Returned from Background thread: " << msg << std::endl;
    };
});

// must update thread_pool to make sure result functions are called
threadPool.update();

```