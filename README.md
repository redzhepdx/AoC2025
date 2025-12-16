## AoC 2025 – C Solutions

This repository contains my solutions for Advent of Code 2025, written entirely in C.

I got tired of slow Python code and over-reliance on performance libraries like NumPy, PyTorch, etc. While they are powerful, I realized that leaning on them too much degraded my fundamental understanding of algorithms, memory, and performance.

In my honest opinion, C is one of the best languages to truly learn algorithms and low-level mechanics. You are forced to do everything yourself—from memory management to data structures to debugging output. Even something as simple as aligned printing teaches discipline.

Yes, I refuse to use a debugger unless I’m working on a massive, tightly coupled system (like a physics engine where everything depends on everything—think Balkan family levels of coupling).

Yes, I believe you shouldn’t use a debugger 95% of the time.


### Goals
The main goals of this project were:

- Resharpen my C programming skills
- Rebuild intuition for performance-oriented thinking
- Learn and apply low-level data structures from scratch
- Become more conscious of what the computer is actually doing


### Libraries Used
I intentionally avoided large frameworks and instead focused on lightweight, single-header libraries:

- NOB – a minimal build system
https://github.com/tsoding/nob.h
- STB (stb_ds) – data structures for C
https://github.com/nothings/stb/blob/master/stb_ds.h

These libraries are not small enough to comfortably copy-paste into each solution file, so they are kept as headers.


### Topics & Algorithms Practiced
During the challenge, I worked with and implemented the following concepts:

- Dynamic arrays from NOB (still used somewhat naively)
- Hash tables in C
- Interval trees
- Disjoint sets (Union–Find)
- Dynamic memory management
- Manual debugging and careful reasoning
- Knowing what you are doing

### Code Quality Disclaimer
I got carried away by the problems, so some implementations are not as clean as I’d like.
There are likely memory bugs and suboptimal designs.
I plan to clean up the code, fix memory issues, and improve structure and readability over time.

### Problem Source
All problems are from Advent of Code:
https://adventofcode.com

### Repository Structure

- Each solution corresponds to a single `.c` file
- Files are self-contained except for required header libraries (NOB / STB)
- To understand the data structures used, you can inspect the relevant headers

#### Building & Running a Solution

1. Create a build folder

```mkdir build```

2. Update `nob.c`

Rename all relevant fields in nob.c to match your target solution file:
```
#if !defined(_MSC_VER)
    // On POSIX
    nob_cmd_append(&cmd,
        "cc",
        "-Wall", "-Wextra",
        "-o", BUILD_FOLDER "SOLUTION_SCRIPT_NAME",
        SRC_FOLDER "SOLUTION_SCRIPT_NAME.c");
#else
    // On MSVC
    nob_cmd_append(&cmd,
        "cl",
        "-I.",
        "-o", BUILD_FOLDER "hello",
        SRC_FOLDER "hello.c");
#endif  // _MSC_VER

// Execute the command
if (!nob_cmd_run(&cmd)) return 1;

// nob_cmd_run() automatically resets the cmd array

// Alternative compiler abstraction macros
nob_cc(&cmd);
nob_cc_flags(&cmd);
nob_cc_output(&cmd, BUILD_FOLDER "SOLUTION_SCRIPT_NAME");
nob_cc_inputs(&cmd, SRC_FOLDER "SOLUTION_SCRIPT_NAME.c");
```

3. Build NOB

```
gcc nob.c -o nob
```
(You can also use make if you prefer.)

4. Build your solution by running nob executable

```
./nob
```

5. Run the solution program
```
./build/q1_secret_entrance
```

Don't forget to replace the executable name with the solution you built.