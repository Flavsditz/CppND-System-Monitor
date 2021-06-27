#include "process.h"

#include <string>

#include "linux_parser.h"

using std::string;

Process::Process(int pid) : pid(pid) { calculateCpuUtilization(); }

int Process::Pid() { return pid; }

void Process::calculateCpuUtilization() {
  long seconds = LinuxParser::UpTime(pid);
  long totalTime = LinuxParser::ActiveJiffies(pid);

  m_cpu_utilization = float(totalTime) / float(seconds);
}

float Process::CpuUtilization() const { return m_cpu_utilization; }

string Process::Command() const { return LinuxParser::Command(pid); }

string Process::Ram() const { return LinuxParser::Ram(pid); }

string Process::User() const { return LinuxParser::User(pid); }

long int Process::UpTime() const { return LinuxParser::UpTime(pid); }

bool Process::operator<(Process const& a) const {
  return m_cpu_utilization < a.CpuUtilization();
}