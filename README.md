# interact_ackermann
Interaction nets vs GHC performance comparison

## Running the benchmark

The C implementation of interaction nets requires [GJAlloc](https://github.com/arneg/GJAlloc) to be installed.

Then just run `./bench.sh`, that will compile, run and time the programs.

## Current results

```
Command being timed: "./interact_ackermann_call_stack"
User time (seconds): 6.86
System time (seconds): 0.00
Percent of CPU this job got: 99%
Elapsed (wall clock) time (h:mm:ss or m:ss): 0:06.86
Average shared text size (kbytes): 0
Average unshared data size (kbytes): 0
Average stack size (kbytes): 0
Average total size (kbytes): 0
Maximum resident set size (kbytes): 3652
Average resident set size (kbytes): 0
Major (requiring I/O) page faults: 0
Minor (reclaiming a frame) page faults: 630
Voluntary context switches: 1
Involuntary context switches: 4
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
User time (seconds): 9.66
System time (seconds): 0.02
Percent of CPU this job got: 100%
Elapsed (wall clock) time (h:mm:ss or m:ss): 0:09.68
Average shared text size (kbytes): 0
Average unshared data size (kbytes): 0
Average stack size (kbytes): 0
Average total size (kbytes): 0
Maximum resident set size (kbytes): 7460
Average resident set size (kbytes): 0
Major (requiring I/O) page faults: 0
Minor (reclaiming a frame) page faults: 1170
Voluntary context switches: 1
Involuntary context switches: 11
Swaps: 0
File system inputs: 0
File system outputs: 0
Socket messages sent: 0
Socket messages received: 0
Signals delivered: 0
Page size (bytes): 4096
Exit status: 0
```
