#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

Processor& System::Cpu() { return cpu_; }

vector<Process>& System::Processes() {
  vector<int> pids = LinuxParser::Pids();

  processes_.clear();
  for (int pid : pids) {
    Process process(pid);
    processes_.push_back(process);
  }

  std::sort(processes_.rbegin(), processes_.rend());
  return processes_;
}

System::System() {
  m_kernel = LinuxParser::Kernel();
  m_os = LinuxParser::OperatingSystem();
  m_memUtil = LinuxParser::MemoryUtilization();
  m_runningProcs = LinuxParser::RunningProcesses();
  m_totalProcs = LinuxParser::TotalProcesses();
}

std::string System::Kernel() { return m_kernel; }

float System::MemoryUtilization() { return m_memUtil; }

std::string System::OperatingSystem() { return m_os; }

int System::RunningProcesses() { return m_runningProcs; }

int System::TotalProcesses() { return m_totalProcs; }

long int System::UpTime() { return LinuxParser::UpTime(); }
