#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  explicit Process(int pid, bool isTimeInJiffies);

  int Pid();
  std::string User() const;
  std::string Command() const;
  float CpuUtilization() const;
  std::string Ram() const;
  long int UpTime() const;
  bool operator<(Process const& a) const;

 private:
  int pid;
  bool m_isTimeInJiffies;
  float m_cpu_utilization;
  void calculateCpuUtilization();
};

#endif