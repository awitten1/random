
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <thread>
#include <fmt/core.h>
#include <iostream>

//
// apt install libfmt-dev
// g++ file_memory.cpp -std=c++11 -o file_memory `pkg-config --cflags --libs fmt`
//

#define BUF_SIZE 4096

size_t get_file_size(int fd);
std::string find_arg(int argc, char** argv, std::string arg);

void read_file_sequential(int fd, int bytes_per_second) {
  int sleep_time_ms = (BUF_SIZE * 1000) / bytes_per_second;
  char buf[BUF_SIZE];

  for (;;) {
    int ret = read(fd, buf, BUF_SIZE);
    if (ret < 0) {
      throw std::runtime_error{"failed to read from file"};
    }
    if (ret == 0) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));
  }
}

void read_file_random(int fd, int bytes_per_second) {
  int sleep_time_ms = (BUF_SIZE * 1000) / bytes_per_second;

  char buf[BUF_SIZE];
  size_t bytes_read = 0;
  size_t file_sz = get_file_size(fd);

  for (;;) {
    size_t page_offset = rand() % ((file_sz + BUF_SIZE - 1) / BUF_SIZE);
    size_t offset = page_offset * BUF_SIZE;
    int ret = pread(fd, buf, BUF_SIZE, offset);
    if (ret < 0) {
      throw std::runtime_error{"failed to read file"};
    }
    bytes_read += ret;
    if (bytes_read > file_sz) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));
  }
}

int main(int argc, char** argv) {
  std::string filename = find_arg(argc, argv, "--filename");
  std::string access_pattern = find_arg(argc, argv, "--access_pattern");
  std::string bytes_per_s = find_arg(argc, argv, "--bytes_per_second");
  if (access_pattern != "sequential" && access_pattern != "random") {
    throw std::runtime_error{"access pattern must be sequential or random"};
  }

  int fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    throw std::runtime_error{fmt::format("failed to open file {}. reason: {}", filename, strerror(errno))};
  }

  int bytes_per_second = std::stoi(bytes_per_s);
  std::cout << bytes_per_s << std::endl;
  if (access_pattern == "sequential") {
    read_file_sequential(fd, bytes_per_second);
  } else if (access_pattern == "random") {
    read_file_random(fd, bytes_per_second);
  }

  pause();
  return 0;
}


size_t get_file_size(int fd) {
  struct stat st;
  int ret = fstat(fd, &st);
  if (ret == -1) {
    throw std::runtime_error{fmt::format("failed to stat file. reason: {}", strerror(errno))};
  }

  return st.st_size;
}

std::string find_arg(int argc, char** argv, std::string arg) {
  for (int i = 0; i < argc; ++i) {
    if (std::string(argv[i]) == arg) {
      return std::string(*(argv + i + 1));
    }
  }
  throw std::runtime_error{fmt::format("must provide argument {}", arg)};
}

