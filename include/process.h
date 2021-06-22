#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  explicit Process(int pid);

  int Pid();
  std::string User();      // TODO: See src/process.cpp
  std::string Command();   // TODO: See src/process.cpp
  float CpuUtilization();  // TODO: See src/process.cpp
  std::string Ram();       // TODO: See src/process.cpp
  long int UpTime();
  bool operator<(Process const& a) const;  // TODO: See src/process.cpp

 private:
  int pid;
};

#endif