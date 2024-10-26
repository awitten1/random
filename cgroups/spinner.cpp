#include <thread>
#include <string>

int main(int argc, char** argv) {
  auto spin = []() {
    while(true);
  };
  int num_threads = std::stoi(argv[1]);
  for (int i = 0; i < num_threads - 1; ++i) {
    std::thread t(spin);
    t.detach();
  }
  spin();
  return 0;
}