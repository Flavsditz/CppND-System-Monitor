#include "linux_parser.h"

#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "format.h"
namespace fs = std::filesystem;

using std::cout;
using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;

  fs::path path = kProcDirectory;

  for (auto& p : fs::directory_iterator(path)) {
    if (fs::is_directory(p.path())) {
      std::string fileName = p.path().filename();
      if (std::all_of(fileName.begin(), fileName.end(), isdigit)) {
        int pid = stoi(fileName);
        pids.push_back(pid);
      }
    }
  }

  return pids;
}

float LinuxParser::MemoryUtilization() {
  string line;
  string total, free;
  string tmp;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);

      linestream >> tmp;
      if (tmp == "MemTotal:") {
        linestream >> total;
      } else if (tmp == "MemFree:") {
        linestream >> free;
        break;
      }
    }
  }

  float used = stof(total) - stof(free);
  return used / stof(total);
}

long LinuxParser::UpTime() {
  string line;
  string value;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    linestream >> value;
    return stol(value);
  }

  return 0;
}

long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

long LinuxParser::ActiveJiffies(int pid) {
  long totalTime;

  string line, value;
  vector<string> values;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    while (linestream >> value) {
      values.push_back(value);
    }
  }

  // make sure parsing was correct and values was read
  long utime = LinuxParser::GetLongOutOfVector(values, 13);
  long stime = LinuxParser::GetLongOutOfVector(values, 14);
  long cutime = LinuxParser::GetLongOutOfVector(values, 15);
  long cstime = LinuxParser::GetLongOutOfVector(values, 16);

  totalTime = utime + stime + cutime + cstime;
  return totalTime / sysconf(_SC_CLK_TCK);
}

long LinuxParser::GetLongOutOfVector(vector<string> values, int position){
  if (std::all_of(values[position].begin(), values[position].end(), isdigit)) {
    return stol(values[position]);
  }
  return 0;
}

long LinuxParser::ActiveJiffies() {
  vector<string> jiffies = LinuxParser::CpuUtilization();

  return stol(jiffies[kUser_]) + stol(jiffies[kNice_]) +
         stol(jiffies[kSystem_]) + stol(jiffies[kIRQ_]) +
         stol(jiffies[kSoftIRQ_]) + stol(jiffies[kSteal_]);
}

long LinuxParser::IdleJiffies() {
  vector<string> jiffies = LinuxParser::CpuUtilization();

  return stol(jiffies[kIdle_]) + stol(jiffies[kIOwait_]);
}

vector<string> LinuxParser::CpuUtilization() {
  string line, cpuNumber;
  vector<string> values{10};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    // Although we could simply get all values and "push_back" into the array
    // this shows what is being stored in which position
    linestream >> cpuNumber >> values[kUser_] >> values[kNice_] >>
        values[kSystem_] >> values[kIdle_] >> values[kIOwait_] >>
        values[kIRQ_] >> values[kSoftIRQ_] >> values[kSteal_] >>
        values[kGuest_] >> values[kGuestNice_];
  }

  return values;
}

int LinuxParser::TotalProcesses() { return ParseProcesses("processes"); }

int LinuxParser::RunningProcesses() { return ParseProcesses("procs_running"); }

int LinuxParser::ParseProcesses(string key) {
  string line;
  string tmp;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);

      linestream >> tmp;
      if (tmp == key) {
        linestream >> tmp;
        return stoi(tmp);
      }
    }
  }

  return 0;
}

string LinuxParser::Command(int pid) {
  string line;
  string value;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line;
}

string LinuxParser::Ram(int pid) {
  string line;
  string key;
  string value{"0"};

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);

      linestream >> key;
      if (key == "VmSize:") {
        linestream >> value;
        break;
      }
    }
  }

  // Convert value to MB
  float mbValue = stof(value) / 1000;

  return Format::PrecisionFloat(mbValue, 2);
}

string LinuxParser::Uid(int pid) {
  string line;
  string value;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> value;

      if (value == "Uid:") {
        linestream >> value;
        break;
      }
    }
  }

  return value;
}

string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);

  string username, x, lineUID;
  string line;

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      linestream >> username >> x >> lineUID;
      if (lineUID == uid) {
        break;
      }
    }
  }

  return username;
}

long LinuxParser::UpTime(int pid) {
  string line;
  string value;
  vector<string> values;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    while (linestream >> value) {
      values.push_back(value);
    }
  }

  // The value we want is the 22nd value on the line we just parsed
  long seconds = stol(values[21]) / sysconf(_SC_CLK_TCK);
  return seconds;
}