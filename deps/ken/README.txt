
LEGAL MATTERS
=============

    Copyright (c) 2011-2012, Hewlett-Packard Development Co., L.P.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the
      distribution.

    * Neither the name of the Hewlett-Packard Company nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
    WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    - - - - - - - - - -

    The Institute of Electrical and Electronics Engineers and The Open Group,
    have given us permission to incorporate code from their copyrighted
    documentation: IEEE Std 1003.1, 2008 Edition, Standard for Information
    Technology -- Portable Operating System Interface (POSIX), The Open Group
    Base Specifications Issue 7, Copyright (C) 2001-2008 by the Institute of
    Electrical and Electronics Engineers, Inc and The Open Group.

IMPATIENT?
==========

    Run "./compile.csh app00.c" to obtain "kenapp" executable.
    Invoke as "./kenapp 127.0.0.1:6789" to obtain a greeting.
    (Assuming that 127.0.0.1 is IP addr from "host localhost"
    and 6789 is a free port on your machine.)

RELEASE NOTES
=============

    The first public release of Ken (13 June 2012) coincided with
    the publication of Yoo et al., "Composable Reliability for
    Asynchronous Systems," in the USENIX Annual Technical Conference.
    This release tries to make it easy to get simple Ken-based
    applications up and running quickly.  Please share your first
    impressions and early experiences with us.

    The present release (circa 10 August 2012) contains a few
    minor revisions and enhancements compared to the releases of
    June 2012.  See the CHANGES file.

KEN'S HOME ON THE WEB
=====================

    http://ai.eecs.umich.edu/~tpkelly/Ken/

INTRODUCTION
============

    "Ken" is a rollback-recovery protocol for reliable distributed
    applications.  See the 2012 USENIX Annual Technical Conference
    paper by Yoo et al. describing the formal Ken protocol, its
    implementation in C, and "MaceKen," the integration of Ken into
    the Mace distributed systems toolkit.  The Ken protocol is
    abstracted from its initial implementation in the "Waterken" Java
    platform by Tyler Close; see HP Labs tech report 2010-155 for the
    genealogy of the protocol.

    The C implementation of Ken transparently protects distributed
    applications from packet loss and crash failures (e.g., power
    failures, process crashes, and kernel panics).  It unifies
    message reliability with application state reliability, i.e., it
    protects the integrity of application data "at rest" (local
    process state) and data "in motion" (inter-process messages).
    Ken ensures pairwise-FIFO exactly-once handling of messages
    between Ken processes.  Ken furthermore ensures that the outputs
    emitted by a collection of Ken processes *could have been*
    emitted in a failure-free execution of the distributed system,
    even if tolerated failures (crashes and packet drops) occur.
    Finally, this global correctness guarantee holds even for
    independently developed Ken-based systems that interact in
    unforeseen ways.  In other words, Ken's global correctness
    guarantees *compose* without any overt effort by the developers
    of the distributed components.  Ken thus supports decentralized
    software development by teams that need not coordinate a global
    reliability strategy.

    Non-tolerated failures include corruption or destruction of
    persistent state due to application software error or storage
    system failure and permanent network failures.

    Ken provides *reliability* in the face of crash failures.
    Complementary *availability* guarantees may be obtained from
    orthogonal mechanisms.  Examples include HP Serviceguard, a
    commercial product that supports high availability via monitoring
    and failover, and MaceKen, an open source distributed systems
    toolkit that is bundled with highly available replication-based
    services.

PROGRAMMING
===========

    Ken programming can be described as an event-driven or
    "actor"-like paradigm involving communicating event loops, but
    with an important twist: Each iteration of each event loop is an
    ACID transaction.

    The entry point for Ken application software is the handler
    function described in kenapp.h.  Ken applications don't see the
    conventional C program main() function, which is in the Ken
    infrastructure.  Inputs and messages from other Ken processes are
    passed to a Ken process's application-level code via invocations
    of the handler function by the Ken infrastructure.

    An invocation of the Ken handler function---called a "turn"---is
    atomic, even in the face of tolerated failures.  Either all of a
    turn's consequences on local state are made permanent and all of
    the turn's outbound messages are reliably delivered, or it is as
    though the turn never began.  Ken turns are ACID transactions in
    which the "C"---consistency---is the responsibility of the
    programmer who writes the handler function.

    See file ken.h for brief descriptions of the functions available
    to application code.  Examples include facilities to allocate and
    release memory from Ken's persistent heap and facilities to send
    messages to other Ken processes.  See the example Ken applications
    (files "app*.c") for illustrations of how to use Ken's facilities.

    The natural and recommended style of Ken application programming
    follows these guidelines:

    Write code as though tolerated failures can't happen.  Don't
    attempt to infer the occurrence of tolerated failures from within
    a Ken program---the whole point of Ken is to prevent application
    software from noticing that failures have occurred.  Address
    tolerated crash failures by re-starting crashed Ken processes.
    Address non-tolerated failures via out-of-band administrative
    mechanisms (e.g., if the file system is full, expand its capacity
    or de-clutter it).

    Crash with confidence.  Crashes can't destroy or corrupt
    application state, nor prevent messages sent during successfully
    completed turns from being processed by the recipient.  For
    example, it's good to check invariants carefully before returning
    from the handler and assert(0) if you detect any weirdness, which
    prevents the persistent heap from becoming corrupted.  More
    generally assert(0) is often the best solution to a problem; see
    the USENIX ATC 2012 paper for an example involving resource
    scarcity.

    Use only Ken's interfaces for sending messages/outputs.  All bets
    are off if during a turn you do something that can be detected
    beyond the confines of the Ken process performing the turn.
    Don't exit permanently until all messages you've sent have been
    acknowledged by their recipients.

    Don't use static, external, or global variables.  There's an
    alternative interface for setting/getting an entry point into the
    persistent heap, atop which it's easy to build a convenient
    substitute for globals etc.; see file kenvar.h.

    Don't use the conventional heap, because it is not preserved
    across crash/restart.  Use the Ken persistent heap only, or use
    alloca() if you know what you're doing.  Be conscious of the
    long-term implications and compatibility issues surrounding your
    data types.  For example, if you store in Ken's persistent heap a
    struct containing a foo_t field, where foo_t is a data type
    defined by someone else (e.g., in a system header file), your
    code could break if the type or size of a foo_t changes, which in
    turn could happen if dynamically linked libraries are upgraded
    and linked into a crashed/restarted Ken process.  One relatively
    safe policy is to maintain full control over all data types in
    your persistent heap, and to define these data types with a view
    toward future needs.  Static linking with system libraries could
    be part of an an alternative strategy to avoid problems.

    Don't use the file system.  To save data indefinitely, stash it
    in the persistent heap, where it will be safe until you free it.
    (This rule is often liberating rather than restrictive because
    there's only one format for your data, the in-memory
    representation, and no separate serial/external representation.)
    To transfer data from one Ken process to another, send the data
    in a Ken message.

    Don't use threads.  Intermediate programmers: join/terminate all
    threads before returning from the handler.  Advanced programmers:
    note that Ken doesn't checkpoint any thread stacks and be sure
    you know what you're doing.

    Beware of legacy libraries that violate Ken's prohibitions
    against static/external/global variables and ill-timed externally
    visible effects.  It's fine to use strlen(), but strtok() is bad
    because it uses static storage.  In many cases standard
    interfaces have reentrant variants, e.g., strtok_r(), that are
    safe.  Legacy libraries that "launch missiles" under the hood
    violate Ken's turn discipline and void Ken's warranties.

    Don't use conventional argc/argv[]; Ken provides an alternative
    interface to command-line arguments.

    Ken has been used successfully with C++ and the Standard Template
    Library; the MaceKen MaceKen project described in the 2012 USENIX
    ATC paper is an example.  The basic trick is to override global
    new, new[], delete, and delete[] to use the Ken allocation and
    de-allocation functions.  As with C programs, C++ programs that
    employ Ken must not use static, external, and global variables.
    The distribution includes a sample application illustrating the
    use of C++ / STL as of August 2012.  Be aware that C++ tends to
    deprive the programmer of control over the size and layout of
    data types; some knowledgeable people feel that persistent heaps
    and C++ don't mix well.

COMPILATION
===========

    The Ken distribution contains a shell script that illustrates one
    way to compile a Ken program.  Compiling all source files of your
    application together with all source files of Ken enables more
    thorough compile-time error checking and is therefore recommended
    where feasible.  A good static checker, e.g., HP Code Advisor,
    Coverity Static Analyzer, or PC-Lint/FlexeLint, is also
    recommended.  (In June 2012 the Ken team ran Coverity Static
    Analyzer on the Ken code base; previously we ran HP Code Advisor.
    Both tools found a few opportunities for improvement.)  As of
    August 2012 the distribution includes a Makefile that compiles a
    separate libken shared library, but this is not the recommended
    way to compile.

    Don't disable assertions via -DNDEBUG.  Assertions help Ken to
    protect application data from corruption.

    Ken uses mmap() with MAP_ANONYMOUS, and it has been reported that
    Mac OSX doesn't support this flag.  Unfortunately there's no
    POSIX-standardized way to perform anonymous mappings.  You might
    try MAP_ANON (deprecated by Linux and therefore not used in Ken)
    or /dev/zero.

CONFIGURATION
=============

    Ensure that fsync() commits data to durable media.  You must
    ensure that neither your file system nor your storage system nor
    any other relevant layer cheats.  Utilities like hdparm and
    sdparm can help you to disable volatile write caches in storage
    devices.

    Understand the reliability properties of storage devices.  Disks
    are relatively well understood; solid-state (flash) drives are
    less well understood as of mid-2012 and may have significantly
    different reliability characteristics with respect to power
    failures.

    Ken's patcher process relies on the assumption (universally true,
    as far as we know) that the size of a memory page is not smaller
    than the size of a file system block, which in turn is not
    smaller the size of a block in the storage system.

    Basic Ken uses UDP for messages, and Linux sockets have very
    small default send and receive buffers---roughly big enough for
    two max-size UDP datagrams.  Incoming messages can fill the
    buffer of the Ken process receiving them, which can cause
    datagram loss and performance problems.  You can make the buffers
    bigger by first raising the system wide ceilings with
    /sbin/sysctl and then changing SO_RCVBUF and SO_SNDBUF via
    setsockopt().  You would add the setsockopt() call to ken.c.

    Address space randomization can in theory screw up recovery,
    though we have never observed this to be a problem in practice
    (as of August 2012).  If it is a problem for you, on Linux
    systems you'd tweak /proc/sys/kernel/randomize_va_space.

    Problems can arise if a Ken process reaches the kernel limit on
    the number of memory map regions.  This can happen if a large
    number of non-adjacent/non-contiguous pages in the Ken persistent
    heap are dirtied during a turn.  The kernel limit can be raised
    with "sysctl -w vm.max_map_count=...".

    Memory overcommitment can become an issue for a Ken process whose
    persistent heap approaches or exceeds the size of available RAM
    plus swap space.  See files overcommit_memory and overcommit_ratio
    in directory /proc/sys/vm/ and related documentation.  If memory
    overcommitment is properly configured, then crashing and
    re-starting a Ken process will migrate its state to the file
    system and the resurrected Ken process will require very little
    RAM + swap.  See http://linux-mm.org/OverCommitAccounting

    Ken creates and deletes (typically small) files frequently.
    Some file systems might offer better performance than others
    for Ken's usage pattern.  It has been suggested that XFS might
    be a good match for Ken.

RUNNING
=======

    A Ken ID is an IP:port pair, e.g., "12.34.56.78:9012".  Invoke a
    Ken process with its Ken ID as the first argument on the command
    line.  To resurrect a crashed Ken process, invoke it again with
    the same command line in the same directory.

    When resurrecting a crashed Ken process, make sure that its
    previous avatar is thoroughly dead.  A Ken process consists
    of three Unix processes and all must be killed before the
    Ken process is re-started.  The "killall" utility can help.

    If you *annihilate* a Ken process by killing it thoroughly *and
    deleting all of its persistent state* (the files Ken creates in
    the file system), keep in mind that other Ken processes have not
    forgotten the sequence numbers of messages previously sent by the
    Ken process that you have annihilated.  If you subsequently
    re-use the KenID of the annihilated Ken process, this will
    confuse Ken processes who receive messages from both the old
    annihilated Ken processes and the new impostor process using the
    same KenID.  The rule is simple: If you annihilate a Ken process,
    don't re-use its KenID if Ken processes that communicted with the
    annihilated process are not themselves annihilated.  In practice
    it's easy to follow this rule:  After annihilating a Ken process,
    re-start it with a KenID containing a new port.

    If more than one of the three Unix processes that constitute a
    Ken process abort, it's helpful if they can leave separate core
    files.  On some systems the default core file name is simply
    "core", which isn't very helpful.  Some systems allow you to
    define a core file naming pattern to include information such
    as the PID of the Unix process that left it, which is better.

    Ken must run in a directory capable of containing large files
    (nominal size over 1 TB) and also sparse files, i.e., files that
    occupy far fewer blocks of storage than their nominal size would
    naively suggest.  (Note that file encryption can be incompatible
    with sparse files, and that transparent encryption of one's home
    directory and all beneath it is available as an install-time
    option on some Linux distributions.)  Following crash recovery of
    a simple Ken program with a small amount of data in its
    persistent heap, you'll see something like this:

        % ls -lh ken_<KenID>_state
        -rw------- 1 joe joe 1.1T 2012-06-06 04:54 ken_<KenID>_state

        % du -h ken_<KenID>_state
        16K     ken_<KenID>_state

    If you run several Ken processes on the same machine, run each in
    a separate directory, which may improve performance.

    Redirect the standard error stream of a Ken process to a file by
    passing it through a pipe to "cat".  In C Shell you might do
    this:  "% ken 12.34.56.78:9012 |& cat >& file".  Each Ken process
    consists of three Unix processes that all write() to the same
    stderr file descriptor, which must go to a pipe to prevent bad
    interleaving of diagnostics from the three Unix processes.

    If Ken's stdout is piped into an ordinary Unix process, the
    latter must consume Ken's output promptly, otherwise Ken will
    eventually block, which may impair performance.

    Ken may report three kinds of runtime errors via assertion
    failures:  NTF is non-tolerated failure, e.g., file system full.
    KENASRT is an internal should-never-happen error in Ken.  APPERR
    is an error by the application programmer, e.g., an invalid
    parameter is passed to one of Ken's functions.  Ordinary
    assertions are for Ken application software.

    If your network refuses to deliver UDP packets, the consequences
    for Ken's default UDP-based messages are obvious.  Some network
    providers and universities block UDP by default, unfortunately.

    Network address translation (NAT) causes a mismatch between the
    KenID in a Ken message header and the IP address reported by the
    receiver's OS.

