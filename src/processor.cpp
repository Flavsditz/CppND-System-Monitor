#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() {
  // Return the aggregate CPU utilization
  long total_old, total_new;
  long idle_old, idle_new;

  total_new = LinuxParser::Jiffies();
  idle_new = LinuxParser::IdleJiffies();

  total_old = totalJiffies;
  idle_old = idleJiffies;

  StoreSnapshot(idle_new, total_new);

  float diffTotal = float(total_new) - float(total_old);
  float diffIdle = float(idle_new) - float(idle_old);

  float utilization = (diffTotal - diffIdle) / diffTotal;
  return utilization;
}

void Processor::StoreSnapshot(long idle, long total){
  totalJiffies = total;
  idleJiffies = idle;
}