#include <atomic>
#include <chrono>

inline void spinsleep(long int useconds)
{
  std::atomic<long> value(0);

  auto start = std::chrono::system_clock::now();
  auto end = std::chrono::system_clock::now();
  long int dur = 0;
  
  do {
    for(int i = 0; i<1000; ++i)++value;
    end = std::chrono::system_clock::now();
    dur = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
  } while (dur < useconds);
}

