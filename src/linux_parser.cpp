#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "format.h"
namespace fs = std::filesystem;

using std::cout;
using std::regex;
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
          break;
        }
      }
      if (key == "PRETTY_NAME") {
        break;
      }
    }
  }
  filestream.close();

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
  stream.close();

  TimeInJiffies(kernel);
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
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

  filestream.close();

  float used = stof(total) - stof(free);
  return used / stof(total);
}

long LinuxParser::UpTime() {
  string line;
  string value{"0"};
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    linestream >> value;
  }
  filestream.close();

  return stol(value);
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
  stream.close();

  // make sure parsing was correct and values was read
  long utime = LinuxParser::GetLongOutOfVector(values, 13);
  long stime = LinuxParser::GetLongOutOfVector(values, 14);
  long cutime = LinuxParser::GetLongOutOfVector(values, 15);
  long cstime = LinuxParser::GetLongOutOfVector(values, 16);

  totalTime = utime + stime + cutime + cstime;
  return totalTime / sysconf(_SC_CLK_TCK);
}

long LinuxParser::GetLongOutOfVector(vector<string> values, int position) {
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
  filestream.close();

  return values;
}

int LinuxParser::TotalProcesses() { return ParseProcesses("processes"); }

int LinuxParser::RunningProcesses() { return ParseProcesses("procs_running"); }

int LinuxParser::ParseProcesses(string key) {
  string line;
  string tmp{"0"};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);

      linestream >> tmp;
      if (tmp == key) {
        linestream >> tmp;
        break;
      }
    }
  }
  filestream.close();
  return stoi(tmp);
}

string LinuxParser::Command(int pid) {
  string line;
  string value;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  filestream.close();

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

      // We are using VmData since it shows the physical memory instead of the
      // virtual memory size (Given by "VmSize")
      if (key == "VmData:") {
        linestream >> value;
        break;
      }
    }
  }
  filestream.close();

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
  filestream.close();

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
  filestream.close();

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
  filestream.close();

  // The value we want is the 22nd value on the line we just parsed
  if (isTimeInJiffies) {
    long seconds = UpTime() - stol(values[21]);
    return seconds;
  } else {
    long seconds = UpTime() - stol(values[21]) / sysconf(_SC_CLK_TCK);
    return seconds;
  }
}

void LinuxParser::TimeInJiffies(const std::string& kernelString) {
  std::istringstream iss(kernelString);
  std::vector<std::string> tokens;
  std::string token;
  while (std::getline(iss, token, '.')) {
    if (!token.empty())
      tokens.push_back(token);
  }

  // Check if kernel version is smaller than 2.6 (which means time is in Jiffies)
  if(stoi(tokens[0]) > 2){
    isTimeInJiffies = false;
  } else {
    if(stoi(tokens[1]) < 6){
      isTimeInJiffies = true;
    } else {
      isTimeInJiffies = false;
    }
  }
}