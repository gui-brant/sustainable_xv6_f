# sustainable_xv6_f — Detailed Documentation

## Project Overview

This project extends xv6-riscv with four sustainability-focused features that enable resource accounting, enforcement, and analysis at the process level. The goal is to make it straightforward to measure CPU-based resource usage, assign per-process budgets, and exercise the scheduler to compare behavior under different policies. The work includes kernel changes (process accounting fields, a new syscall for energy retrieval, budget enforcement in the scheduler/timer path), and user-space test programs to exercise and validate each feature.


## Feature: energytest 

Purpose: Provide a user-level harness to measure the before/after energy accounting for a workload so that the effect of scheduling and kernel changes on per-process energy consumption can be observed.

Implementation highlights:
- Kernel changes: added an `int energy` field to `struct proc` (in `proc.h`) and updated the value in the timer/interrupt path (in `trap.c` / `clockintr()`), incrementing only when `p->state == RUNNING`.
- Syscall: `getenergy(pid)` performs a safe lookup of the given PID's `proc` structure (helper in `proc.c`) and returns its `energy` value. The syscall was added through standard xv6 syscall registration (defs.h, syscall.c, sysproc.c, usys.S, user.h. The project also provides `setenergybudget(pid,budget)` for budget control.)
- User tool: `energytest` reads the starting energy for a pid, runs a configurable workload, then reads the ending energy and prints the delta. Typical output shows PID, start energy, end energy, and delta.

Design notes and tradeoffs:
- This design models CPU time as the primary contributor to energy; it is simple and deterministic for experimental comparisons but does not capture I/O or other system-level energy costs.
- The tick-driven model makes per-run comparisons straightforward; if higher fidelity is needed, the counting mechanism can be replaced or augmented with hardware counters or more detailed sampling.

## Feature: schedtest

Purpose: Stress and validate the scheduler across different workload mixes and modes; collect per-process runtime statistics (such as dticks and run counts) for fairness and responsiveness analysis.

Implementation highlights:
- User tool: `schedtest` forks several children with different workload parameters and mode flags, synchronizes them (via pipes), and collects reports on completion. Each child can run busy loops or sleep patterns to exercise scheduling behavior.
- Kernel hooks: Where necessary, the kernel exposes per-process statistics (runtime, number of context switches, dticks) via small syscall(s) or via an extended `plist` snapshot so `schedtest` can gather results after runs.

Design notes and tradeoffs:
- `schedtest` focuses on reproducible micro-benchmarks rather than real applications; this is intentional to clearly surface scheduler effects.
- The program aggregates outputs into human-readable summaries that can be redirected into logs for later analysis.

## Feature: budgettest 

Purpose: Assign explicit resource budgets to processes and enforce them, demonstrating kernel-level limits and behavior when budgets are exceeded.

Implementation highlights:
- Kernel changes: `struct proc` extended to include a `budget` field (or reuse `energy` to track budget consumption). The timer interrupt decrements/increments the relevant fields and performs budget checks.
- Enforcement: When a process's consumed units exceed the assigned budget, the kernel enforces a policy that terminates the process and records the reason; `budgettest` reports which processes were killed versus which completed successfully.
- User tool: `budgettest` spawns multiple processes with different budget values and workloads to trigger a mix of outcomes (killed vs completed), printing a summary at the end.

Design notes and tradeoffs:
- The enforcement policy is intentionally aggressive (kill on exceed) to create clear experiment outcomes; a softer policy (suspend, throttle) could be implemented as an extension.
- Budgets are coarse-grained (tick-based) to keep accounting simple and reproducible.

## Feature: plist 

Purpose: Provide a user-accessible snapshot of the process table that includes accounting fields (runtime, energy, budget) for validation and debugging.

Implementation highlights:
- Kernel: Implemented a safe routine to copy process snapshot data into user-provided buffers, exposing pid, state, runtime counters, energy, and budget fields.
- User tool: `plist` invokes the kernel interface and prints a table of active processes and their accounting fields, helpful for validating the kernel-side bookkeeping.

Design notes and tradeoffs:
- The snapshot is intentionally read-only and consistent at a single point in time to avoid races; this limits concurrency artifacts in reports.

## Build and run

Follow the `README.md` in the repository root for quick setup and run steps. In short: clone the repo, navigate to it, run `make clean` and `make qemu`, then run the user tests from the xv6 shell in the order: `energytest`, `schedtest`, `budgettest`, `plist`.

## Contributors
- Gui — `energytest`
- Steven — `schedtest`
- Ryan — `budgettest`
- Matthew — `plist`

## Project status

All planned features are implemented and integrated. No remaining kernel features are required for the course deliverable.
