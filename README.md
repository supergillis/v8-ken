# v8-ken
This project adds orthogonal persistence to the V8 engine. This means that we have created a version of V8 where all objects, arrays, functions, ... are automatically persisted.

## Example
If the above wasn't very clear, here is a small example of what this actually means.

There is a simple shell for the orthogonal persistent V8, i.e. `v8_ken_shell`. We run `v8_ken_shell 127.0.0.1:6666` for the first time and define some stuff.

    $ ./v8_ken_shell 127.0.0.1:6666
    Initialized session
    > var create_counter = function() { var counter = 0; return function() { return counter++; }; };
    > var a = create_counter();
    > a();
    0
    > a();
    1
    > a();
    2
    > var b = create_counter();
    > b();
    0
    > b();
    1
    > a();
    3
    > ^C

The last input, `CTRL+C`, just 'crashes' the shell. A non-orthogonal persistent language would lose all of its data - data that is not stored in an external database - but the persistent V8 engine does! Here's what happens when we restart the shell:

    $ ./v8_ken_shell 127.0.0.1:6666
    Restored session
    > create_counter
    function () { var counter = 0; return function() { return counter++; }; }
    > a();
    4
    > a();
    5
    > b();
    2
    > ^C

The V8 engine remembers all the variables, even the `create_counter` function! We can just continue programming as if there was no crash!

## Context
This project is part of my master thesis. The thesis actually consists of two parts:

  * The automatically-persisting V8 engine;
  * A JavaScript framework that provides ActiveRecord-style schemas, transactions, ...

The main idea of the latter is that when you have a persistent language, you have all kinds of data floating around, but there is no easy way to manage it. A big advantage when using a database backend is that you can define schemas, you can run queries, you can execute several statements in a transaction, ... With an orthogonal persistent language, this is not possible. Thus the goal of the framework is to make it easier for the developer to handle the unmanaged data.

## Technical Details
To make the V8 engine persistent we make use of the Ken library.

### Ken
Ken is a C library that provides abstractions to create a heap that is automatically persisted. It also provides abstractions to send messages which are guaranteed to be delivered to the destination (which is another Ken process). Here's an overview:

  * `ken_malloc(size_t size)`: allocates a chunk of memory on the persistent heap;
  * `ken_free(void* pointer)`: frees a chunk of memory on the persistent heap;
  * `ken_send(kenid_t dest, const void* msg, int32_t len)`: sends a message to another Ken application.

The Ken library is event-driven. Each event triggers a call to a user-specified handler. An invocation of this handler is called a turn. When a turn completes successfully, the dirty memory is written to the last working snapshot of the persistent heap.

The user-specified handler in our persistent V8 implementation is just read-eval-print. After reading, evaluating and printing, Ken writes the dirty memory to the persistent heap.

My simple explanation does not honor Ken at all, so if you want to know the details of the Ken library, you can read more about it at the following link, [http://ai.eecs.umich.edu/~tpkelly/Ken/](http://ai.eecs.umich.edu/~tpkelly/Ken/).

### Persistent V8
The integration of Ken in the V8 engine was cumbersome. Here are the steps, from least to most cumbersome:

  1. The new and delete operators had to be overloaded to also use the persistent heap;
  2. We had to replace all `malloc` and `free` with `ken_malloc` and `ken_free`;
  3. Then we had to make the V8 heap also use Ken memory.
  4. And last, but certainly not least, we had to track down all global and static variables and also persist them on the persistent heap.
  5. Debug, debug, debug!

## Compiling and Running
The code can only run on 64-bit POSIX machines since the Ken library can only function on 64-bit POSIX machines.

    git clone https://github.com/supergillis/v8-ken.git
    cd v8-ken
    git checkout v8-ken # this is our working branch
    make dependencies # gyp dependencies
    make x64.release # Ken only works for 64-bit machines
    cd out/x64.release
    ./v8_ken_shell 127.0.0.1:6666 # specify an IP and a port, Ken needs this for its messaging system
