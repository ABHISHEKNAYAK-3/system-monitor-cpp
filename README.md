# C++ System Monitor (top-like)

A real-time, `top`-like system monitor for Linux, built in C++ as a B.Tech 4th-year project. This tool reads the `/proc` virtual filesystem to display live, sorted data on active processes, CPU usage, and memory usage.

---

## 🚀 Core Features

* **Real-Time Data:** Refreshes the process list every second.
* **Process Parsing:** Reads and parses `/proc/<PID>/stat`, `/proc/<PID>/status`, and `/proc/<PID>/comm` for all active processes.
* **CPU % Calculation:** Accurately calculates real-time CPU percentage for each process by comparing "jiffies" (CPU ticks) over a 1-second interval.
* **Memory Usage:** Reads `VmRSS` (Resident Set Size) to show the actual physical memory used by each process.
* **Process Sorting:** Sorts the process list by the highest CPU usage by default.
* **Process Killing:** Includes a utility to terminate processes using `kill(PID, SIGKILL)` (e.g., `./monitor kill 1234`).

---

## 🛠️ How to Build and Run

This project must be built on a Linux environment (like Ubuntu via WSL) with a C++17 compliant compiler.

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/ABHISHEKNAYAK-3/system-monitor-cpp.git](https://github.com/ABHISHEKNAYAK-3/system-monitor-cpp.git)
    cd system-monitor-cpp
    ```

2.  **Compile the C++ code:**
    (The `-std=c++17` flag is required for `<filesystem>`)
    ```bash
    g++ -std=c++17 -o monitor monitor.cpp
    ```

3.  **Run the monitor (Live Mode):**
    ```bash
    ./monitor
    ```
    The screen will clear and the real-time dashboard will appear.
    * **Press `Ctrl+C` to exit.**

4.  **Run (Kill Mode):**
    To kill a process, find its PID from the monitor and run:
    ```bash
    ./monitor kill 1234
    ```
