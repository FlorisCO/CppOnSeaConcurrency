# Concurrency

Presented by Rainer Grimm

Contact:
- Phone: +49 7472 917441
- E-mail: schulung@ModernesCpp.de
- Web: www.ModernesCpp.de

## Disclaimer

These are my personal notes based on the slides of the workshop "Concurrency in Modern C++". They are not complete, may contain errors, and are just meant as an extension or abbreviation of points I found personally important.

## The memory model

### The contract

- Atomic operations: You can only observe state A or B

- Ordering of operations: When order is A, B, then observing B means A happened

- Weaker contract means more optimization potential for the system

- Under the hood, locking synchronization is based on atomics, hence we are *all* using it implicitly

### Atomics

- Sequential consistency
    - Interleavings (slides 24-25) grow exponentially
    - When sequential consistency is broken (because threads run on different cores), then there is no guarantee of the state, so a synchronization is needed (which are atomics)

## Multithreading (slides 45+)

- consider *std::thread t(function)*, then *t* is the **thread** and *function* is the **thread of execution**.

- When calling detach, you detach the thread t from the thread of execution, i.e. the thread handle is no longer valid, even though the thread of execution is still running.

- (S49) std::this_thread::yield() relinguishes the CPU(, and comes back later? Perhaps you expect more work by then?)

- Not joining a thread (or detaching it), you get an exception when the program exits

- Compiler explorer to see the compilation output, cppinsights.io to see what the compiler sees right before compilation.

## Locking (slides 53+)

- Spinning = trying to the action continuously (i.e. CPU goes to 100%)

### Deadlocks (slide 54)

- The example is bad because:
    - if getVar() throws an exception, the mutex is never unlocked
    - if getVar() takes its own mutex, interleaving problems for deadlocks
    - you don't know the cost/duration of the getVar function
    - if getVar() tries to lock the same mutex, deadlock (undefined), need for recursive lock.

- Improvements:
    - Calculate outside the lock with local data.
    - Only set the shared (critical) data within the lock.
    - Addendum: getVar() should be a pure function (result only depends on its inputs)

- Prevent deadlock with lock_guard or unique_lock (slide 55), how would this work?
    - In C++11, there is std::lock that accepts multiple lockables, which will be locked atomically, preventing a deadlock (but requires you to want to lock both at the same time). It uses a while loop that keeps trying to lock all, and releases the ones it got if at least one failed, before trying again. When it failed a loop, there will be a sleep because otherwise, you may run into a "life-lock", e.g. when two threads both try to call std::lock for the same locks.

- RAII is the a principle we have built-in with C++ regarding constructors and destructors (in scope). But wouldn't be the case in some other languages (C# has dispose for this reason?)
    - e.g. containers, lock_guard

- std::defer_lock means the mutex owner is set, but not locked yet. Then std::lock will take both at the same time. (see example "uniqueLock.cpp")

- However, C++17 has a nicer solution with std::scoped_lock, which is a lock_guard wich can take multiple mutexes at once, in a safe manner.

- Sanitize concurrency software while compiling:
    * g++ threadArguments.cpp -O3 -pthread -fsanitize=thread -g
    * .a.out

- Note, thread sanitizer just builds a tree of calls, and won't find a write-access in a read-lock.

## TBD

- Best practice/scenario: your data should not be shared and constant, otherwise, see slide 63

Thread-safe initialization of data:
    - Use constexpr => slice 64
    - Use std::call_once and std::once_flag => slide 65, also safeInitializationCallOnce.cpp
        => Note, this is a very slow solution, especially on windows.
        void initSharedDataFunction() { ... }
        std::once_flag initSharedDataFlag;
        std::call_once(initSharedDataFlag, initSharedDataFunction);
    - static variable with block scope => slide 66

- MeyersSingleton is safer because it is initialized in one step, whereas a classical singleton using a global static is not thread-safe by default, or incurring a large cost. (blogPost)[https://www.modernescpp.com/index.php/thread-safe-initialization-of-a-singleton]

- A static is bound to the main thread (for destruction)

- A tread_local static is bound to the thread that created it. (Slide 68, threadLocal.cpp), however, when a thread uses the variable will create its own copy of the static. Think about creating a threaded scope, in conjunction to a function scope.
    * Used to make a single threaded program multi-threaded and just change the variables that have become shared to thread_local, so there is no concurrency problem. Should still be considered if desired for the outcome though.

### Condition variables (slide 71)

- Help synchronize

- Have no state, so may need a predicate to make sure that there is no early wake or deadlock (started waiting after receiving a signal = lost wake-up). There's also spurse wakeups, which are not desired and are resolved with a predicate.

- Note, a lock_guard can only be locked and unlocked once, so the condition variable needs a different mutex lock, like a unique_lock. (conditionVariable.cpp)

- The code as shown only works for one producer and one consumer, multiple consumers need a different solution. (Also read [Be Aware of the Traps of Condition Variables](https://www.modernescpp.com/index.php/c-core-guidelines-be-aware-of-the-traps-of-condition-variables))

## Tasks (vs threads) (Slide 76+)

- With a task, the thread creation is optional and up to the compiler/infrastructure.

- Example: dotProductAsync.cpp

- From C++17, an ExecutionPolicy is available in some overloads that let you request the function to be handled with multiple threads to speed up its processing.

### Promise-Future

- A future-promise pair is a one-way direction, the communication doesn't flow the other way. When the future is ready, it is destroyed, when the promise is read, the shared channel is destroyed ...

- With packaged_task, you can prepare a data packet for a future, and let it execute later.

- Vectorization is when the CPU supports vector operations that use larger registers to do bulk processing.

- Read [this blog post](https://www.modernescpp.com/index.php/performance-of-the-parallel-stl-algorithmn)

- Creating a promise and getting the future means you have two objects that are connected and can be managed by any thread as desired.

- Exercise: promiseFuture.cpp; make it so that the division by zero throws an exception and is caught by the thread waiting on the future result. Not throwing an exception would generate an uncatchable exception otherwise.
    * Solution: promiseFutureException.cpp

### Shared_future

- Essentially equivalent to a shared_ptr. [read me](https://www.modernescpp.com/index.php/component/content/article/43-blog/multithreading/168-promise-and-future?Itemid=239)
    std::shared_future<> f = p.get_future();
