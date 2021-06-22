#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  long totalJiffies{0};
  long idleJiffies{0};

  void StoreSnapshot(long idle, long total);
};

#endif