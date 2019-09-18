# interact_ackermann

Interaction nets vs GHC performance comparison

In this repository I have tried to implement interaction nets as described in [Yves Lafont (1990), Interaction Nets](https://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.112.2822).

I think the explicit duplication agent acts like a kind of automatic reference counting mechanism. Suprisingly, this seems to be faster than GHC's garbage collector.

I haven't written a full compiler. Nevertheless, I expect that the C files could be generated automatically from a higher level discription of the interaction net.

One of the things that I experienced was that ordering of some of the functions inside the rules can be important. The current ordering is the best one I have found by trial and error. I do not know if it is possible to automatically determine the optimal order.

Before I made this a git repository I just duplicated the file to get primitive versioning. The current development file is `interact_ackermann.c`. The best version right now is `interact_ackermann_call_stack.c`. The others do not work correctly yet. It is called `call_stack` because it (ab)uses the call stack to store the interactions that are pending.

## Running the benchmark

The C implementation of interaction nets requires [GJAlloc](https://github.com/arneg/GJAlloc) to be installed.

Then just run `./bench.sh`. That will compile, run and time the programs.

## Current results

```
Command being timed: "./interact_ackermann_call_stack"
User time (seconds): 5.31
System time (seconds): 0.00
Percent of CPU this job got: 99%
Elapsed (wall clock) time (h:mm:ss or m:ss): 0:05.31
Average shared text size (kbytes): 0
Average unshared data size (kbytes): 0
Average stack size (kbytes): 0
Average total size (kbytes): 0
Maximum resident set size (kbytes): 3800
Average resident set size (kbytes): 0
Major (requiring I/O) page faults: 0
Minor (reclaiming a frame) page faults: 632
Voluntary context switches: 1
Involuntary context switches: 7
Swaps: 0
File system inputs: 0
File system outputs: 0
Socket messages sent: 0
Socket messages received: 0
Signals delivered: 0
Page size (bytes): 4096
Exit status: 0
```

```
Command being timed: "./interact_ackermann-hs"
User time (seconds): 8.07
System time (seconds): 0.01
Percent of CPU this job got: 99%
Elapsed (wall clock) time (h:mm:ss or m:ss): 0:08.09
Average shared text size (kbytes): 0
Average unshared data size (kbytes): 0
Average stack size (kbytes): 0
Average total size (kbytes): 0
Maximum resident set size (kbytes): 7516
Average resident set size (kbytes): 0
Major (requiring I/O) page faults: 0
Minor (reclaiming a frame) page faults: 1174
Voluntary context switches: 1
Involuntary context switches: 111
Swaps: 0
File system inputs: 0
File system outputs: 0
Socket messages sent: 0
Socket messages received: 0
Signals delivered: 0
Page size (bytes): 4096
Exit status: 0
```
