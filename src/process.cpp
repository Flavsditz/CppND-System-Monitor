#include "process.h"

#include <string>

#include "linux_parser.h"

using std::string;

Process::Process(int pid) : pid(pid) {}

int Process::Pid() { return pid; }

float Process::CpuUtilization() const {
  long seconds = LinuxParser::UpTime(pid);
  long totalTime = LinuxParser::ActiveJiffies(pid);

  float utilization = float(totalTime) / float(seconds);

  return utilization;
}

string Process::Command() const { return LinuxParser::Command(pid); }

string Process::Ram() const { return LinuxParser::Ram(pid); }

string Process::User() const { return LinuxParser::User(pid); }

long int Process::UpTime() const { return LinuxParser::UpTime(pid); }

bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}