
#include <chrono>
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <thread>

#define PAGE_SIZE 4096

int main(int argc, char** argv) {
  if (argc < 2) {
    throw std::runtime_error{"must provide filename as first argument"};
  }
  std::string filename = std::string(argv[1]);
  int fd = open(filename.c_str(), O_RDONLY);

  if (fd < 0) {
    throw std::runtime_error{"failed to open file"};
  }

  struct stat st;
  int ret = fstat(fd, &st);
  if (ret == -1) {
    throw std::runtime_error{"failed to stat file"};
  }

  size_t file_sz = st.st_size;
  int bytes_per_second = std::stoi(argv[2]);
  int sleep_time_ms = (PAGE_SIZE * 1000) / bytes_per_second;
  char buf[PAGE_SIZE];

  for (;;) {
    size_t page_offset = rand() % ((file_sz + PAGE_SIZE - 1) / PAGE_SIZE);
    size_t offset = page_offset * PAGE_SIZE;
    ret = pread(fd, buf, PAGE_SIZE, offset);
    if (ret < 0) {
      throw std::runtime_error{"failed to read file"};
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));
  }

  return 0;
}

