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

System::System() {
  m_kernel = LinuxParser::Kernel();
  m_os = LinuxParser::OperatingSystem();
  m_isTimeInJiffies = LinuxParser::TimeInJiffies(m_kernel);
}

Processor& System::Cpu() { return cpu_; }

vector<Process>& System::Processes() {
  vector<int> pids = LinuxParser::Pids();

  processes_.clear();
  for (int pid : pids) {
    Process process(pid, m_isTimeInJiffies);
    processes_.push_back(process);
  }

  std::sort(processes_.rbegin(), processes_.rend());
  return processes_;
}

std::string System::Kernel() { return m_kernel; }

float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() { return m_os; }

int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

long int System::UpTime() { return LinuxParser::UpTime(); }