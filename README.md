# sustainable_xv6_f — Project README

Project: sustainability-focused extensions to xv6 (energy accounting, budgeting, scheduler tests, and process listing).

## Overview
This repository is a modified xv6-riscv used for a course project. It contains four added user-space tools and corresponding kernel support to measure and control per-process resource usage for sustainability-focused experiments.

## Setup and run
1. Clone the repository into your workspace:

	git clone https://github.com/gui-brant/sustainable_xv6_f.git

2. Open a terminal and navigate to the repository directory.

3. Build and run the system:

	make clean
	make qemu

4. From the xv6 shell, run the user-space tests one by one in this order:

	energytest
	schedtest
	budgettest
	plist

## Feature summary

energytest — Measures a process energy-like counter before and after a controlled workload and reports the delta. This program is useful for comparing how different scheduler configurations or kernel changes affect per-process resource consumption.

schedtest — Launches multiple child processes with different modes and workloads to stress and evaluate the scheduler. It synchronizes children, collects runtime statistics, and prints scheduling behavior summaries for fairness and responsiveness analysis.

budgettest — Demonstrates per-process budget assignment and enforcement. The kernel tracks and decrements a budget counter; processes that exceed their budget are killed. The program reports which processes were terminated and which completed within budget.

plist — A user-level utility that requests a snapshot of the kernel process table and prints PID, state, and relevant accounting fields (runtime, budget/energy) to aid debugging and validation of kernel accounting features.

## Contributors
- Gui — energytest
- Steven — schedtest
- Ryan — budgettest
- Matthew — plist

