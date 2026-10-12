# egos-2000: MLFQ scheduler and user tools (my fork)

This is my fork of [egos-2000](https://github.com/yhzhang0128/egos-2000), Cornell's 2,000-line teaching OS for RISC-V. It runs on QEMU and on RISC-V FPGA boards. I implemented the course projects below for the Operating Systems course (CSE231) at IIIT-Delhi. Everything after the divider is the original egos-2000 README.

## What I implemented

**Preemptive Multi-Level Feedback Queue scheduler** (`grass/process.c`, `grass/kernel.c`, `grass/process.h`)
- 5 priority levels. Level *k* gets a runtime budget of (k+1) × 100 ms (`MLFQ_LEVEL_RUNTIME`).
- On every timer interrupt the kernel charges the running process for its runtime. When the budget is used up, the process drops one level.
- The scheduler always runs a process from the highest non-empty level.
- Every 10 s (`MLFQ_RESET_PERIOD`) all processes are boosted back to level 0, so CPU-bound processes cannot starve.
- When there is pending keyboard input, the shell is reset to level 0, so the system stays responsive while background loops run.
- Per-process lifecycle statistics are printed when a process exits: turnaround time, response time, CPU time and number of timer interrupts.

**User tools** (`apps/user/grep.c`, `apps/user/wcl.c`)
- `grep`: substring matching over file contents with line-buffer management and argument validation.
- `wcl`: line counting across files.

```bash
make && make qemu        # needs the RISC-V toolchain and QEMU, see USAGES.md
> loop &                 # start CPU-bound background processes
> loop &
> ls                     # the shell stays responsive thanks to the keyboard boost
```

---

## Vision

This project's vision is to help **every** student read **all** the code of a teaching operating system.

With only **2000** lines of code, egos-2000 implements every component of a teaching operating system that runs on both QEMU and RISC-V boards.
The [EGOS book](https://egos.fun/book/overview.html) contains 9 course projects based on egos-2000.

![Fail to load an image of egos-2000.](tools/images/egos-2000.jpg)

```shell
# The cloc utility is used to count the lines of code.
> cloc egos-2000 --exclude-ext=md,txt,toml,json  # excluding text files
...
github.com/AlDanial/cloc v 1.94  T=0.05 s (949.3 files/s, 62349.4 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               29            424            576           1589
C/C++ Header                     9             62             99            261
Assembly                         3             14             46             92
make                             1             16              7             58
-------------------------------------------------------------------------------
SUM:                            42            516            728           2000 (exactly!)
-------------------------------------------------------------------------------
```

## Earth and Grass Operating System

The **egos** part of egos-2000 is named after its three-layer architecture.

* The **earth layer** implements hardware-specific abstractions.
    * tty and disk device interface
    * timer and memory management interface
* The **grass layer** implements hardware-independent abstractions.
    * process control block and system call interface
* The **application layer** implements file system, shell and user commands.

The definitions of `struct earth` and `struct grass` in header file [egos.h](library/egos.h) specify the layer interface.
Please read [USAGES.md](USAGES.md) for running egos-2000 and
the [instruction set manuals](https://github.com/riscv/riscv-isa-manual/releases) for the RISC-V privileged ISA.

## Acknowledgements

Many thanks to Meta for a [Facebook fellowship](https://research.facebook.com/blog/2021/4/announcing-the-recipients-of-the-2021-facebook-fellowship-awards/).
Many thanks to [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/), [Lorenzo Alvisi](https://www.cs.cornell.edu/lorenzo/), [Shan Lu](https://people.cs.uchicago.edu/~shanlu/), [Hakim Weatherspoon](https://www.cs.cornell.edu/~hweather/) and [Christopher Batten](https://www.csl.cornell.edu/~cbatten/) for their support.
Many thanks to [Cheng Tan](https://naizhengtan.github.io/) and [Yu-Ju Huang](https://yuju-huang.github.io/) for providing valuable feedback to the EGOS book and using egos-2000 in [Northeastern CS4973/6640](https://naizhengtan.github.io/25spring/) and [Cornell CS4411/5411](https://www.cs.cornell.edu/courses/cs4411/2025sp/).
Many thanks to [Haobin Ni](https://haobin.cx/) and [Hongbo Zhang](https://www.cs.cornell.edu/~hongbo/) for [porting egos-2000 to mriscv](https://github.com/0x486F626F/mriscv/tree/egos), a simple processor written in SystemVerilog.
Many thanks to [Brandon Fusi](https://www.linkedin.com/in/brandon-cheo-fusi-b94b1a171/) for [porting egos-2000 to Allwinner D1](https://github.com/cheofusi/egos-2000-d1) and Sipeed's [Lichee RV64 Nezha compute module](https://wiki.sipeed.com/hardware/en/lichee/RV/RV.html).

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/).



