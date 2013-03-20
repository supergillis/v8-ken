# v8-ken
The goal of this project is to add orthogonal persistence to the V8 engine. So we want a V8 where all application state is stored in a persistent heap.

## Making V8 Persistent
So how do we plan to make V8 persistent? Well, we have to make all application data persistent!

* Persist global and static variables;
* Replace malloc et al. with ken_malloc and free with ken_free;
* Overload new and delete operators.

## Ken
Ken is the C library we use that provides abstractions for a persistent heap. You can read more about the Ken library at the following link, [http://ai.eecs.umich.edu/~tpkelly/Ken/](http://ai.eecs.umich.edu/~tpkelly/Ken/).

In this repository all the Ken stuff lives in [https://github.com/supergillis/v8-ken/blob/v8-ken/src/v8-ken-main.cc](src/v8-ken-main.cc):
* You can see that we overload the new and delete operators, so all heap objects will be created on the persistent heap;
* Then we have all the code for our (very) simple V8 shell;
* And finally we have the ken_handler which we need to incorporate Ken. You can read more about this at the provided link.

So that's the code for creating objects on the persistent heap and for running the (very) simple V8 shell. What about the code for saving and restoring global and static variables? This code lives in [https://github.com/supergillis/v8-ken/blob/v8-ken/src/v8-ken-data.cc](src/v8-ken-data.cc):
* We use an array of persists. A persist contains a pointer to a variable in the program and a pointer to the associated persistent memory;
* To persist a static variable we need to add the variable to the array of persists;
* The persists are restored on starting the application and they are saved on each turn in the ken_handler.

## Work Done
* All malloc's et al. are replaced with Ken's persistent allocations;
* All (or almost all?) static and global variables are persisted;
* The default isolate is also persisted;
* The new and delete operators are overloaded;
* VirtualMemory uses Ken memory instead of mmap;
* We can start an initial V8 shell, define some variables, crash the application (CTRL+C), restart the application and use the previously defined variables.

## Example
Run `v8_ken_shell 127.0.0.1:6666` for the first time and define some stuff.

    $ ./v8_ken_shell 127.0.0.1:6666
    17336:../deps/ken/ken.c:862: handler process starting
    17336:../deps/ken/kencom.c:200: resetting errno == 2: "No such file or directory"
    17336:../deps/ken/kencom.c:200: resetting errno == 2: "No such file or directory"
    17336:../deps/ken/kencom.c:200: resetting errno == 2: "No such file or directory"
    17336:../deps/ken/ken.c:992: resetting errno == 2: "No such file or directory"
    17337:../deps/ken/kenext.c:404: externalizer starting
    17336:../deps/ken/ken.c:999: handler process main loop starting
    17338:../deps/ken/kenpat.c:60: patcher starting
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

Crash (CTRL+C) the shell and run it again. Everything still works!

    $ ./v8_ken_shell 127.0.0.1:6666
    17348:../deps/ken/ken.c:862: handler process starting
    17348:../deps/ken/ken.c:533: zero-byte EOT file "ken_127.0.0.1:6666_eot_0000000000000000010"
    17348:../deps/ken/ken.c:574: deleting corrupt EOT file "ken_127.0.0.1:6666_eot_0000000000000000010"
    17348:../deps/ken/ken.c:670: begin truncate (hang here might be due to poor support for sparse files)
    17348:../deps/ken/ken.c:672: end truncate
    17348:../deps/ken/ken.c:983: recovering from turn 9
    17349:../deps/ken/kenext.c:404: externalizer starting
    17348:../deps/ken/ken.c:999: handler process main loop starting
    17350:../deps/ken/kenpat.c:60: patcher starting
    create_counter
    Restored session
    > function () { var counter = 0; return function() { return counter++; }; }
    > a();
    4
    > a();
    5
    > b();
    2
    > ^C

To start over again, just clean up Ken's state files.

    $ rm ken_127.0.0.1\:6666_* -f

## Future Work
* Better shell with history, ...;
* Integrate the persistent V8 in node.js!

## Compiling and Running
The code can only run on 64-bit POSIX machines since the Ken library can only function on 64-bit POSIX machines.

    git clone https://github.com/supergillis/v8-ken.git
    cd v8-ken
    git checkout v8-ken # this is our working branch
    make dependencies # gyp dependencies
    make x64.release # Ken only works for 64-bit machines
    cd out/x64.release
    ./v8_ken_shell 127.0.0.1:6666 # specify an IP and a port, Ken needs this for its messaging system

That's how you compile and run the `v8_ken_shell`.
