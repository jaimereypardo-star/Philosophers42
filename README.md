# Philosophers42


#  Philosophers — *I never thought philosophy would be so deadly*

## Description

This project is a C implementation of the classic **Dining Philosophers Problem**, a well-known concurrency thought experiment originally formulated by Edsger Dijkstra. It serves as an introduction to multithreading and synchronization in C.

A number of philosophers sit around a circular table with a bowl of spaghetti in the center. Between each pair of adjacent philosophers lies a single fork. To eat, a philosopher must pick up both the fork to their left and the fork to their right. After eating, they sleep, then think — and the cycle repeats. The simulation ends either when a philosopher dies of starvation or when every philosopher has eaten a required number of times.

The core challenge is preventing **deadlocks**, **data races**, and **starvation** while keeping the simulation accurate in real time.

### Key concepts explored

- POSIX threads (`pthread`) — one thread per philosopher
- Mutexes — used to represent forks and to protect shared state (meal times, simulation status, print output)
- Precise millisecond timing with `gettimeofday`
- A dedicated monitor thread that watches for deaths and meal completion
- Deadlock prevention via asymmetric fork-pickup order (odd/even philosopher strategy)

---

## Project Structure

```c
philo/
├── main.c        # Entry point, argument validation, init, simulation lifecycle
├── routines.c    # Philosopher and monitor thread routines, time utilities
├── philo.h       # Structs, macros, and function prototypes
└── Makefile      # Build rules
```

### Architecture overview

| Component | Role |
|---|---|
| `t_table` (`s_simulation`) | Shared simulation state: timings, forks, flags |
| `t_philo` (`s_philo`) | Per-philosopher data: id, meal count, last meal time, thread |
| `dining_routine` | Each philosopher's thread: eat → sleep → think loop |
| `monitor_routine` | Separate thread polling for deaths and full satiation |
| `sim_lock` mutex | Protects the `simulation_over` boolean flag |
| `print_lock` mutex | Guarantees non-overlapping log output |
| `meal_lock` mutex | Protects `last_meal_time` and `meals_eaten` per philosopher |
| `forks[]` mutexes | One mutex per fork — only one philosopher can hold it at a time |

---

## Instructions

### Requirements

- Linux or macOS
- `cc` (GCC or Clang)
- POSIX threads support (`-lpthread`)

### Compilation

```bash
git clone <your-repo-url>
cd philo
make
```

This produces the `philo` executable. Additional Makefile rules:

```bash
make clean    # Remove object files
make fclean   # Remove object files and binary
make re       # Full recompile
```

### Execution

```bash
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

| Argument | Description |
|---|---|
| `number_of_philosophers` | Number of philosophers (and forks) at the table |
| `time_to_die` (ms) | Time since last meal before a philosopher dies |
| `time_to_eat` (ms) | Duration of each eating session (requires 2 forks) |
| `time_to_sleep` (ms) | Duration of each sleeping session |
| `number_of_times_each_philosopher_must_eat` | *(Optional)* Simulation ends when all philosophers reach this count |

### Usage examples

```bash
# 5 philosophers — no one should die
./philo 5 800 200 200

# 4 philosophers — no one should die
./philo 4 410 200 200

# 1 philosopher — will always die (only one fork available)
./philo 1 800 200 200

# Simulation stops after each philosopher eats 7 times
./philo 5 800 200 200 7
```

### Expected output format

Each state change is printed as:

 
```
timestamp_in_ms  philosopher_id  action
```


For example:

```
0       1  has taken a fork
0       1  has taken a fork
0       1  is eating
200     1  is sleeping
400     1  is thinking
...
800     1  died
```

---

## Technical Choices

**Deadlock prevention** — Philosophers with an odd ID pick up their right fork first; even-numbered philosophers pick up their left fork first. This asymmetry breaks the circular dependency that causes deadlocks.

**Smart sleep** — Instead of a single `usleep` call (which cannot be interrupted), `smart_sleep` polls in 100 µs increments and exits early if the simulation ends. This keeps response time sharp without burning CPU.

**Monitor thread** — A dedicated monitor polls every philosopher's `last_meal_time` under their individual `meal_lock`, comparing it against `time_to_die`. Death messages are printed directly from the monitor while holding the `print_lock`, guaranteeing they are never interleaved and always within 10 ms of actual death.

**No global variables** — All shared state is passed through pointers to `t_table`. Each philosopher holds a back-pointer (`philo->table`) to access shared rules and resources.

---


## Bonus

The bonus part reimplements the simulation using **processes and semaphores** instead of threads and mutexes.

### Key differences from the mandatory part

| | Mandatory | Bonus |
|---|---|---|
| Concurrency model | Threads (`pthread`) | Processes (`fork`) |
| Synchronization | Mutexes | Named semaphores |
| Fork representation | One mutex per fork | Single counting semaphore |
| Death detection | Shared monitor thread | Per-process watcher thread |
| Simulation stop | Shared `simulation_over` flag | `dead_sem` signals parent |

### How it works

Each philosopher is spawned as a **child process** via `fork()`. Inside each process, a dedicated `death_watcher` thread runs in parallel, checking every 500 µs whether the philosopher has exceeded `time_to_die` since their last meal. If so, it prints the death message and posts to `dead_sem` before exiting.

The **parent process** waits on `dead_sem`. As soon as it is posted — whether from a death or from a philosopher reaching the meal count — the parent calls `kill(SIGKILL)` on every child and then `waitpid` to reap them all cleanly.

Forks are represented by a **single named semaphore** initialized to `nb_philos`. Any philosopher can grab any two forks by calling `sem_wait` twice, and releases them with `sem_post` after eating. This removes the need for the asymmetric fork-pickup strategy used in the mandatory part.

**Staggering** — instead of the odd/even delay, each philosopher waits a small offset proportional to their ID before starting, spreading out fork requests and preventing early collisions.

### Bonus files

```
philo_bonus/
├── main_bonus.c        # Entry point, init, process spawning, cleanup
├── routines_bonus.c    # Philosopher process logic, death watcher, time utils
├── philo_bonus.h       # Structs, semaphore names, prototypes
└── Makefile            # Build rules (produces philo_bonus)
```

### Running the bonus

```bash
make bonus
./philo_bonus number_of_philosophers time_to_die time_to_eat time_to_sleep [must_eat]
```

## Resources

### Concurrency & Threading

- [POSIX Threads Programming — LLNL HPC Tutorials](https://hpc-tutorials.llnl.gov/posix/)
- [pthread man pages — Linux manual](https://man7.org/linux/man-pages/man7/pthreads.7.html)
- [The Dining Philosophers Problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Mutex vs Semaphore — GeeksforGeeks](https://www.geeksforgeeks.org/mutex-vs-semaphore/)

### Useful videos

- [CodeVault — Threads in C playlist](https://www.youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2)

### AI Usage

Claude (Anthropic) was used during this project for the following tasks:

- Generating this README file based on the assignment specification and source code
- Clarifying concepts around mutex usage and deadlock prevention strategies
- Reviewing explanations of the fork-pickup asymmetry approach
- Vibe coding — using Claude to help write and iterate on parts of the implementation,
  including reviewing code structure and suggesting improvements to the threading logic

All AI-generated content was reviewed, tested, and understood before being included in the project.
