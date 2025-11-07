#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <thread>
#include <chrono>
#include <iomanip>
#include <map>
#include <algorithm>
#include <csignal>
#include <stdexcept>

namespace fs = std::filesystem;

// A struct to hold all info for one process
struct ProcessInfo {
    std::string pid;
    std::string command;
    double memory_MB;
    double cpu_percent;
};

// --- NEW: Helper function to clear the terminal screen ---
void clearScreen() {
    // ANSI escape codes to clear screen and move cursor to top-left
    std::cout << "\033[2J\033[1;1H";
}

// --- Helper Functions (Identical to Day 4) ---

long getTotalSystemJiffies() {
    std::ifstream statFile("/proc/stat");
    std::string line;
    std::getline(statFile, line);
    statFile.close();
    std::stringstream ss(line);
    std::string cpuStr;
    long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> cpuStr >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

long getProcessJiffies(const std::string& pid) {
    std::ifstream statFile("/proc/" + pid + "/stat");
    if (!statFile.is_open()) return 0;
    std::string line;
    std::getline(statFile, line);
    statFile.close();
    std::stringstream ss(line);
    std::string value;
    long utime, stime;
    for (int i = 1; i <= 13; ++i) {
        ss >> value;
    }
    ss >> utime >> stime;
    return utime + stime;
}

std::string getCommand(const std::string& pid) {
    std::ifstream commFile("/proc/" + pid + "/comm");
    if (!commFile.is_open()) return "";
    std::string comm;
    std::getline(commFile, comm);
    commFile.close();
    return comm;
}

long getMemory(const std::string& pid) {
    std::ifstream statusFile("/proc/" + pid + "/status");
    if (!statusFile.is_open()) return 0;
    std::string line;
    long mem = 0;
    while (std::getline(statusFile, line)) {
        if (line.find("VmRSS:") == 0) {
            std::stringstream ss(line);
            std::string key;
            std::string unit;
            ss >> key >> mem >> unit;
            break;
        }
    }
    statusFile.close();
    return mem; // Returns in kB
}

bool compareByCpu(const ProcessInfo& a, const ProcessInfo& b) {
    return a.cpu_percent > b.cpu_percent;
}

// --- Main function (Modified for real-time loop) ---
int main(int argc, char* argv[]) {
    // --- Kill functionality (from Day 4) ---
    if (argc == 3 && std::string(argv[1]) == "kill") {
        try {
            pid_t pidToKill = std::stoi(argv[2]);
            if (kill(pidToKill, SIGKILL) == 0) {
                std::cout << "Successfully sent SIGKILL to PID " << pidToKill << std::endl;
            } else {
                perror("kill");
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: Invalid PID." << std::endl;
        }
        return 0; // Exit after attempting to kill
    }
    // --- End Kill functionality ---

    // --- Real-time Monitor Loop ---
    std::map<std::string, long> prevProcJiffies;
    long prevSysJiffies = 0;

    // Get initial "Time 1" values before the loop
    prevSysJiffies = getTotalSystemJiffies();
    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string pid = entry.path().filename().string();
            if (std::all_of(pid.begin(), pid.end(), ::isdigit)) {
                prevProcJiffies[pid] = getProcessJiffies(pid);
            }
        }
    }

    // This is the main real-time loop
    while (true) {
        // Wait for 1 second (this is our refresh rate)
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Get "Time 2" values
        long currentSysJiffies = getTotalSystemJiffies();
        std::map<std::string, long> currentProcJiffies;
        for (const auto& entry : fs::directory_iterator("/proc")) {
            if (entry.is_directory()) {
                std::string pid = entry.path().filename().string();
                if (std::all_of(pid.begin(), pid.end(), ::isdigit)) {
                    currentProcJiffies[pid] = getProcessJiffies(pid);
                }
            }
        }
        
        // --- Calculate and store process info ---
        long systemDelta = currentSysJiffies - prevSysJiffies;
        std::vector<ProcessInfo> processes; 

        for (const auto& pair : currentProcJiffies) {
            std::string pid = pair.first;
            if (prevProcJiffies.find(pid) == prevProcJiffies.end()) continue; 

            long procDelta = pair.second - prevProcJiffies.at(pid);
            double cpuPercent = (systemDelta > 0) ? 100.0 * (double)procDelta / (double)systemDelta : 0.0;
            std::string command = getCommand(pid);
            if (command.empty()) continue; 
            
            double memory_MB = (double)getMemory(pid) / 1024.0;

            if (cpuPercent > 0.01 || memory_MB > 0.01) {
                processes.push_back({pid, command, memory_MB, cpuPercent});
            }
        }

        // Sort the vector
        std::sort(processes.begin(), processes.end(), compareByCpu);

        // --- Display the data ---
        clearScreen(); // Clear the screen before printing

        // Print Header
        std::cout << std::left << std::setw(8) << "PID"
                  << std::setw(20) << "COMMAND"
                  << std::setw(12) << "MEM (MB)"
                  << std::setw(8) << "CPU %" << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << " (Press Ctrl+C to exit)" << std::endl;


        // Print from the sorted vector
        for (const auto& proc : processes) {
            std::cout << std::left << std::setw(8) << proc.pid
                      << std::setw(20) << proc.command.substr(0, 18)
                      << std::fixed << std::setprecision(2) << std::right
                      << std::setw(10) << proc.memory_MB << "  "
                      << std::setw(6) << proc.cpu_percent << "%"
                      << std::endl;
        }

        // --- Set up for the next loop ---
        // "Time 2" values become the "Time 1" values for the next iteration
        prevSysJiffies = currentSysJiffies;
        prevProcJiffies = currentProcJiffies;
    }

    return 0;
}
