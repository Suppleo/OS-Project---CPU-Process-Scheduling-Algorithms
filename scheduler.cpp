#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <sstream>

// Structure to represent a process
struct Process {
    int id;
    int arrival_time;
    std::vector<int> bursts;  // Alternating CPU and R bursts
    int current_burst;
    int remaining_time;
    bool just_from_io;  // Flag to track if the process just came from I/O

    Process(int _id, int _arrival_time) : id(_id), arrival_time(_arrival_time), current_burst(0), remaining_time(0), just_from_io(false) {}
};

// Global variables
std::vector<Process> processes;
std::vector<int> cpu_schedule;
std::vector<int> r_schedule;
int current_time = 0;
size_t completed = 0;
Process* current_cpu_process = nullptr;
Process* current_io_process = nullptr;
bool io_exec_flag = true; // Assign to R schedule only if this is true

// Function to read input file
void read_input_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening input file: " + filename);
    }

    int num_processes;
    if (!(file >> num_processes)) {
        throw std::runtime_error("Error reading number of processes from file");
    }
    std::cout << "Number of processes: " << num_processes << std::endl;

    // Consume the newline character after reading num_processes
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < num_processes; i++) {
        std::string line;
        if (!std::getline(file, line)) {
            throw std::runtime_error("Error reading line for process " + std::to_string(i+1));
        }

        std::istringstream iss(line);
        int arrival_time;
        if (!(iss >> arrival_time)) {
            throw std::runtime_error("Error reading arrival time for process " + std::to_string(i+1));
        }
        
        Process process(i + 1, arrival_time);
        std::cout << "Process " << i+1 << " arrival time: " << arrival_time << std::endl;

        int burst;
        bool first_burst = true;
        while (iss >> burst) {
            process.bursts.push_back(burst);
            std::cout << "Burst: " << burst << " ";
            if (first_burst) {
                process.remaining_time = burst;
                first_burst = false;
            }
        }
        std::cout << std::endl;

        if (process.bursts.empty()) {
            throw std::runtime_error("No bursts read for process " + std::to_string(i+1));
        }

        processes.push_back(process);
    }

    std::cout << "Finished reading input file" << std::endl;
    file.close();
}

// Function to write output file
void write_output_file(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening output file");
    }

    for (int cpu : cpu_schedule) {
        file << (cpu == 0 ? "- " : std::to_string(cpu) + " ");
    }
    file << "\n";

    for (int r : r_schedule) {
        file << (r == 0 ? "- " : std::to_string(r) + " ");
    }
    file << "\n";

    file.close();
}

// First-Come, First-Served (FCFS) scheduler
void fcfs_scheduler() {
    std::queue<Process*> cpu_queue;
    std::queue<Process*> io_queue;
    Process* temp_cpu_process = nullptr;

    while (completed < processes.size() || !cpu_queue.empty() || !io_queue.empty() || current_cpu_process != nullptr || current_io_process != nullptr) {
        // Check for newly arrived processes
        for (auto& process : processes) {
            if (process.arrival_time == current_time && process.current_burst < process.bursts.size()) {
                cpu_queue.push(&process);
            }
        }

        // Check any process remains after conflict in the entrance of the ready queue scenario
        if (temp_cpu_process != nullptr){
            cpu_queue.push(temp_cpu_process);
            temp_cpu_process = nullptr;
        }
        // Handle CPU scheduling
        if (current_cpu_process == nullptr && !cpu_queue.empty()) {
            current_cpu_process = cpu_queue.front();
            cpu_queue.pop();
        }

        if (current_cpu_process != nullptr) {
            cpu_schedule.push_back(current_cpu_process->id);
            current_cpu_process->bursts[current_cpu_process->current_burst]--;

            if (current_cpu_process->bursts[current_cpu_process->current_burst] == 0) {
                current_cpu_process->current_burst++;
                if (current_cpu_process->current_burst < current_cpu_process->bursts.size()) {
                    if (current_io_process == nullptr) {
                        io_exec_flag = false;
                    }
                    io_queue.push(current_cpu_process);
                } else {
                    completed++;
                }
                current_cpu_process = nullptr;
            }
        } else {
            cpu_schedule.push_back(0);
        }

        // Handle I/O scheduling
        if (current_io_process == nullptr && !io_queue.empty()) {
            current_io_process = io_queue.front();
            io_queue.pop();
        }

        if (current_io_process != nullptr && io_exec_flag) {
            r_schedule.push_back(current_io_process->id);
            current_io_process->bursts[current_io_process->current_burst]--;

            if (current_io_process->bursts[current_io_process->current_burst] == 0) {
                current_io_process->current_burst++;
                if (current_io_process->current_burst < current_io_process->bursts.size()) {
                    // Check if a new process will arrive in the next time unit
                    bool new_process_next = false;
                    for (const auto& process : processes) {
                        if (process.arrival_time == current_time + 1 && process.current_burst < process.bursts.size()) {
                            new_process_next = true;
                            break;
                        }
                    }
                    
                    if (new_process_next) {
                        temp_cpu_process = current_io_process;
                    } else {
                        cpu_queue.push(current_io_process);
                    }
                } else {
                    completed++;
                }
                current_io_process = nullptr;
            }
        } else {
            r_schedule.push_back(0);
        }

        current_time++;
        io_exec_flag = true;
    }
}

// Round Robin (RR) scheduler
void rr_scheduler(int quantum) {
    std::queue<Process*> cpu_queue;
    std::queue<Process*> io_queue;
    int time_in_quantum = 0;
    Process* temp_cpu_process = nullptr;

    while (completed < processes.size() || !cpu_queue.empty() || !io_queue.empty() || current_cpu_process != nullptr || current_io_process != nullptr) {
        // Check for newly arrived processes
        for (auto& process : processes) {
            if (process.arrival_time == current_time && process.current_burst < process.bursts.size()) {
                cpu_queue.push(&process);
            }
        }

        // Check any process remains after conflict in the entrance of the ready queue scenario
        if (temp_cpu_process != nullptr){
            cpu_queue.push(temp_cpu_process);
            temp_cpu_process = nullptr;
        }

        // Handle CPU scheduling
        if (current_cpu_process == nullptr && !cpu_queue.empty()) {
            current_cpu_process = cpu_queue.front();
            cpu_queue.pop();
            time_in_quantum = 0;
        }

        if (current_cpu_process != nullptr) {
            cpu_schedule.push_back(current_cpu_process->id);
            current_cpu_process->bursts[current_cpu_process->current_burst]--;
            time_in_quantum++;

            if (current_cpu_process->bursts[current_cpu_process->current_burst] == 0) {
                current_cpu_process->current_burst++;
                if (current_cpu_process->current_burst < current_cpu_process->bursts.size()) {
                    if (current_io_process == nullptr) {
                        io_exec_flag = false;
                    }
                    io_queue.push(current_cpu_process);
                } else {
                    completed++;
                }
                current_cpu_process = nullptr;
                time_in_quantum = 0;
            } else if (time_in_quantum == quantum) {
                // Check if a new process will arrive in the next time unit
                bool new_process_next = false;
                for (const auto& process : processes) {
                    if (process.arrival_time == current_time + 1 && process.current_burst < process.bursts.size()) {
                        new_process_next = true;
                        break;
                    }
                }
                
                if (new_process_next) {
                    temp_cpu_process = current_cpu_process;
                } else {
                    cpu_queue.push(current_cpu_process);
                }

                current_cpu_process = nullptr;
                time_in_quantum = 0;
            }
        } else {
            cpu_schedule.push_back(0);
        }

        // Handle I/O scheduling
        if (current_io_process == nullptr && !io_queue.empty()) {
            current_io_process = io_queue.front();
            io_queue.pop();
        }

        if (current_io_process != nullptr && io_exec_flag) {
            r_schedule.push_back(current_io_process->id);
            current_io_process->bursts[current_io_process->current_burst]--;

            if (current_io_process->bursts[current_io_process->current_burst] == 0) {
                current_io_process->current_burst++;
                if (current_io_process->current_burst < current_io_process->bursts.size()) {
                    // Check if a new process will arrive in the next time unit
                    bool new_process_next = false;
                    for (const auto& process : processes) {
                        if (process.arrival_time == current_time + 1 && process.current_burst < process.bursts.size()) {
                            new_process_next = true;
                            break;
                        }
                    }
                    
                    if (new_process_next) {
                        temp_cpu_process = current_io_process;                   
                    } else {
                        cpu_queue.push(current_io_process);
                    }
                } else {
                    completed++;
                }
                current_io_process = nullptr;
            }
        } else {
            r_schedule.push_back(0);
        }

        current_time++;
        io_exec_flag = true;
    }
}

// Shortest Job First (SJF) scheduler
void sjf_scheduler() {
    std::vector<Process*> cpu_queue;
    std::queue<Process*> io_queue;

    auto compare_sjf = [](Process* a, Process* b) {
        if (a->bursts[a->current_burst] == b->bursts[b->current_burst]) {
            // If burst times are equal, use the new priority system
            if (a->arrival_time == current_time && b->arrival_time != current_time)
                return false; // a has higher priority
            if (b->arrival_time == current_time && a->arrival_time != current_time)
                return true; // b has higher priority
            if (a->just_from_io && !b->just_from_io)
                return false; // a just moved from I/O, higher priority
            if (b->just_from_io && !a->just_from_io)
                return true; // b just moved from I/O, higher priority
            return a->id > b->id; // If all else is equal, choose the process with lower ID
        }
        return a->bursts[a->current_burst] > b->bursts[b->current_burst];
    };

    while (completed < processes.size() || !cpu_queue.empty() || !io_queue.empty() || current_cpu_process != nullptr || current_io_process != nullptr) {
        // Check for newly arrived processes
        for (auto& process : processes) {
            if (process.arrival_time == current_time && process.current_burst < process.bursts.size()) {
                cpu_queue.push_back(&process);
                std::push_heap(cpu_queue.begin(), cpu_queue.end(), compare_sjf);
            }
        }

        // Handle CPU scheduling
        if (current_cpu_process == nullptr && !cpu_queue.empty()) {
            std::pop_heap(cpu_queue.begin(), cpu_queue.end(), compare_sjf);
            current_cpu_process = cpu_queue.back();
            cpu_queue.pop_back();
            current_cpu_process->just_from_io = false;  // Reset the flag when process starts CPU burst
        }

        if (current_cpu_process != nullptr) {
            cpu_schedule.push_back(current_cpu_process->id);
            current_cpu_process->bursts[current_cpu_process->current_burst]--;

            if (current_cpu_process->bursts[current_cpu_process->current_burst] == 0) {
                current_cpu_process->current_burst++;
                if (current_cpu_process->current_burst < current_cpu_process->bursts.size()) {
                    if (current_io_process == nullptr) {
                        io_exec_flag = false;
                    }
                    io_queue.push(current_cpu_process);
                } else {
                    completed++;
                }
                current_cpu_process = nullptr;
            }
        } else {
            cpu_schedule.push_back(0);
        }

        // Handle I/O scheduling (FCFS)
        if (current_io_process == nullptr && !io_queue.empty()) {
            current_io_process = io_queue.front();
            io_queue.pop();
        }

        if (current_io_process != nullptr && io_exec_flag) {
            r_schedule.push_back(current_io_process->id);
            current_io_process->bursts[current_io_process->current_burst]--;

            if (current_io_process->bursts[current_io_process->current_burst] == 0) {
                current_io_process->current_burst++;
                if (current_io_process->current_burst < current_io_process->bursts.size()) {
                    current_io_process->just_from_io = true;  // Set the flag when process finishes I/O burst
                    cpu_queue.push_back(current_io_process);
                    std::push_heap(cpu_queue.begin(), cpu_queue.end(), compare_sjf);
                } else {
                    completed++;
                }
                current_io_process = nullptr;
            }
        } else {
            r_schedule.push_back(0);
        }

        current_time++;
        io_exec_flag = true;
    }
}

// Shortest Remaining Time Next (SRTN) scheduler
void srtn_scheduler() {
    std::vector<Process*> cpu_queue;
    std::queue<Process*> io_queue;

    auto compare_srtn = [](Process* a, Process* b) {
        if (a->remaining_time == b->remaining_time) {
            if (a->arrival_time == current_time && b->arrival_time != current_time)
                return false; // a has higher priority
            if (b->arrival_time == current_time && a->arrival_time != current_time)
                return true; // b has higher priority
            if (a->just_from_io && !b->just_from_io)
                return false; // a just moved from I/O, higher priority
            if (b->just_from_io && !a->just_from_io)
                return true; // b just moved from I/O, higher priority
            return a->id > b->id; // If all else is equal, choose the process with lower ID
        }
        return a->remaining_time > b->remaining_time; // Shorter remaining time has higher priority
    };

    // Initialize remaining time for all processes
    for (auto& process : processes) {
        process.remaining_time = process.bursts[0];
    }

    while (completed < processes.size() || !cpu_queue.empty() || !io_queue.empty() || current_cpu_process != nullptr || current_io_process != nullptr) {
        // Check for newly arrived processes
        for (auto& process : processes) {
            if (process.arrival_time == current_time && process.current_burst < process.bursts.size()) {
                cpu_queue.push_back(&process);
                std::push_heap(cpu_queue.begin(), cpu_queue.end(), compare_srtn);
            }
        }

        // Handle CPU scheduling
        if (!cpu_queue.empty()) {
            Process* shortest_process = cpu_queue.front(); // The shortest process is at the front of the heap
            
            if (current_cpu_process == nullptr || compare_srtn(current_cpu_process, shortest_process)) {
                if (current_cpu_process != nullptr) {
                    cpu_queue.push_back(current_cpu_process);
                    std::push_heap(cpu_queue.begin(), cpu_queue.end(), compare_srtn);
                }
                std::pop_heap(cpu_queue.begin(), cpu_queue.end(), compare_srtn);
                current_cpu_process = cpu_queue.back();
                cpu_queue.pop_back();
                current_cpu_process->just_from_io = false;  // Reset the flag when process starts CPU burst
            } else {
                std::push_heap(cpu_queue.begin(), cpu_queue.end(), compare_srtn);
            }
        }

        if (current_cpu_process != nullptr) {
            cpu_schedule.push_back(current_cpu_process->id);
            current_cpu_process->bursts[current_cpu_process->current_burst]--;
            current_cpu_process->remaining_time--;

            if (current_cpu_process->bursts[current_cpu_process->current_burst] == 0) {
                current_cpu_process->current_burst++;
                if (current_cpu_process->current_burst < current_cpu_process->bursts.size()) {
                    if (current_io_process == nullptr) {
                        io_exec_flag = false;
                    }
                    io_queue.push(current_cpu_process);
                    current_cpu_process->remaining_time = current_cpu_process->bursts[current_cpu_process->current_burst];
                } else {
                    completed++;
                }
                current_cpu_process = nullptr;
            }
        } else {
            cpu_schedule.push_back(0);
        }

        // Handle I/O scheduling (FCFS)
        if (current_io_process == nullptr && !io_queue.empty()) {
            current_io_process = io_queue.front();
            io_queue.pop();
        }

        if (current_io_process != nullptr && io_exec_flag) {
            r_schedule.push_back(current_io_process->id);
            current_io_process->bursts[current_io_process->current_burst]--;

            if (current_io_process->bursts[current_io_process->current_burst] == 0) {
                current_io_process->current_burst++;
                if (current_io_process->current_burst < current_io_process->bursts.size()) {
                    current_io_process->remaining_time = current_io_process->bursts[current_io_process->current_burst];
                    current_io_process->just_from_io = true;  // Set the flag when process finishes I/O burst
                    cpu_queue.push_back(current_io_process);
                    std::push_heap(cpu_queue.begin(), cpu_queue.end(), compare_srtn);
                } else {
                    completed++;
                }
                current_io_process = nullptr;
            }
        } else {
            r_schedule.push_back(0);
        }

        current_time++;
        io_exec_flag = true;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4 || argc >= 6) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <algorithm> [quantum]\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    int algorithm = std::stoi(argv[3]);
    int quantum = (argc == 5) ? std::stoi(argv[4]) : 2;

    try {
        read_input_file(input_file);

        std::cout << "Read input file successfully" << std::endl;

        switch (algorithm) {
            case 1:
                fcfs_scheduler();
                break;
            case 2:
                rr_scheduler(quantum);
                break;
            case 3:
                sjf_scheduler();
                break;
            case 4:
                srtn_scheduler();
                break;
            default:
                throw std::runtime_error("Invalid algorithm specified");
        }

        std::cout << "Finished scheduling" << std::endl;

        write_output_file(output_file);

        std::cout << "Wrote output file successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}