## v8-ken
The goal of this project is to add orthogonal persistence to the V8 engine. So we want a V8 where all application state is stored in a persistent heap.

### Making V8 Persistent
So how do we plan to make V8 persistent? Well, we have to make all application data persistent!

* Persist global and static variables;
* Replace malloc et al. with ken_malloc and free with ken_free;
* Overload new and delete operators.

### Ken
Ken is the C library we use that provides abstractions for a persistent heap. You can read more about the Ken library at the following link, [http://ai.eecs.umich.edu/~tpkelly/Ken/](http://ai.eecs.umich.edu/~tpkelly/Ken/).

In this repository all the Ken stuff lives in [https://github.com/supergillis/v8-ken/blob/v8-ken/src/v8-ken-main.cc](src/v8-ken-main.cc):
* You can see that we overload the new and delete operators, so all heap objects will be created on the persistent heap;
* Then we have all the code for our (very) simple V8 shell;
* And finally we have the ken_handler which we need to incorporate Ken. You can read more about this at the provided link.

So that's the code for creating objects on the persistent heap and for running the (very) simple V8 shell. What about the code for saving and restoring global and static variables? This code lives in [https://github.com/supergillis/v8-ken/blob/v8-ken/src/v8-ken-data.cc](src/v8-ken-data.cc):
* We use an array of persists. A persist contains a pointer to a variable in the program and a pointer to the associated persistent memory;
* To persist a static variable we need to add the variable to the array of persists;
* The persists are restored on starting the application and they are saved on each turn in the ken_handler.

### Work Done
* All malloc's et al. are replaced with Ken's persistent allocations;
* All (or almost all?) static and global variables are persisted;
* The default isolate is also persisted;
* The new and delete operators are overloaded;
* VirtualMemory uses Ken memory instead of mmap;
* We can start an initial V8 shell, define some variables, crash the application (CTRL+C), restart the application and print out the previously defined variables. Note that this doesn't always work (see below).

### Compiling and Running
Before getting to the issues you probably need to know how to compile the code. The code can only run on 64-bit POSIX machines since the Ken library can only function on 64-bit POSIX machines.

    git clone https://github.com/supergillis/v8-ken.git
    cd v8-ken
    git checkout v8-ken # this is our working branch
    make dependencies # gyp dependencies
    make x64.release # Ken only works for 64-bit machines
    cd out/x64.release
    ./v8_ken_shell 127.0.0.1:6666 # specify an IP and a port, Ken needs this for its messaging system

That's how you compile and run the v8_ken_shell. You can crash and restart the application. To clean the persistent heap and to run a clean v8_ken_shell just run `rm ken_* -f`.

### Issues
There are still some issues that remain unsolved. They are best explained using an example:
* Run `./v8_ken_shell 127.0.0.1:6666`;
* Define some variables, e.g. `a = 1` and `b = 2`;
* Crash and restart the shell;
* Now try to print the previously defined variables. There are two possible outputs:
    1. It always prints <null> even when entering unknown variables or when entering declaration of variables;
    2. It prints the variable, but printing undefined variables crashes the shell.

An example of these issues:
    gillis@gillis-desktop:~/projects/v8-ken$ rm ken_* -f
    gillis@gillis-desktop:~/projects/v8-ken$ ./v8_ken_shell 127.0.0.1:6666
    7259:../deps/ken/ken.c:841: handler process starting
    7259:../deps/ken/kencom.c:200: resetting errno == 2: "No such file or directory"
    7259:../deps/ken/kencom.c:200: resetting errno == 2: "No such file or directory"
    7259:../deps/ken/kencom.c:200: resetting errno == 2: "No such file or directory"
    7259:../deps/ken/ken.c:971: resetting errno == 2: "No such file or directory"
    7260:../deps/ken/kenext.c:404: externalizer starting
    7259:../deps/ken/ken.c:978: handler process main loop starting
    7261:../deps/ken/kenpat.c:60: patcher starting
    Initializing...
    > a
    ReferenceError: a is not defined
    > a=1
    1
    > a
    1
    > ^C
    gillis@gillis-desktop:~/projects/v8-ken$ ./v8_ken_shell 127.0.0.1:6666
    7264:../deps/ken/ken.c:841: handler process starting
    7264:../deps/ken/ken.c:512: zero-byte EOT file "ken_127.0.0.1:6666_eot_0000000000000000004"
    7264:../deps/ken/ken.c:553: deleting corrupt EOT file "ken_127.0.0.1:6666_eot_0000000000000000004"
    7264:../deps/ken/ken.c:649: begin truncate (hang here might be due to poor support for sparse files)
    7264:../deps/ken/ken.c:651: end truncate
    7264:../deps/ken/ken.c:962: recovering from turn 3
    7265:../deps/ken/kenext.c:404: externalizer starting
    7266:../deps/ken/kenpat.c:60: patcher starting
    7264:../deps/ken/ken.c:978: handler process main loop starting
    a
    Restoring from 7259...
    > <null>
    > ^C
    gillis@gillis-desktop:~/projects/v8-ken$ ./v8_ken_shell 127.0.0.1:6666
    7267:../deps/ken/ken.c:841: handler process starting
    7267:../deps/ken/ken.c:512: zero-byte EOT file "ken_127.0.0.1:6666_eot_0000000000000000005"
    7267:../deps/ken/ken.c:553: deleting corrupt EOT file "ken_127.0.0.1:6666_eot_0000000000000000005"
    7267:../deps/ken/ken.c:962: recovering from turn 4
    7268:../deps/ken/kenext.c:404: externalizer starting
    7269:../deps/ken/kenpat.c:60: patcher starting
    7267:../deps/ken/ken.c:978: handler process main loop starting
    a
    Restoring from 7264...
    > 1
    > b
    v8_ken_shell: ../deps/ken/ken.c:409: i_ken_SIGSEGV_sigaction: Assertion \`((void)"APPERR", (info->si_code == SEGV_ACCERR))' failed.
    7268:../deps/ken/kenext.c:508: read pipe returns zero, exit from externalizer
    Aborted
    7269:../deps/ken/kenpat.c:70: read pipe returns zero, exit from patcher

### Debugging
We have tried to debug this error but it isn't as simple as it looks. This is what we have tried, it might come in handy if you want to debug it yourself!

[Here](https://gist.github.com/4352602) is a Gist that contains the debugging output of the last listed issue.

Note that in GDB we first disable SIGSEGV stopping and printing with `handle SIGSEGV nostop noprint`. This is because Ken makes its persistent heap read-only. So each time you try to write to the persistent heap the SIGSEGV handler gets called and Ken knows that the written memory is dirty!
