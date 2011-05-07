// A simple stopwatch.
// Dave Hale, Colorado School of Mines, 04/16/2005

#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <ctime>

class Stopwatch {
public:

  // Constructs a stopwatch that is not running.
  Stopwatch() : running_(false), time_(0.0) {
  }

  // Starts this stopwatch, if not running.
  void start() {
    if (!running_) {
      running_ = true;
      start_ = std::clock();
    }
  }

  // Stops this stopwatch, if running.
  void stop() {
    if (running_) { 
      time_ = timeSinceStart();
      running_ = false;
    }
  }

  // Stops and resets this stopwatch.
  void reset() {
    stop();
    time_ = 0.0;
  }

  // Resets and starts this stopwatch.
  void restart() {
    reset();
    start();
  }

  // Returns stopwatch time, in seconds.
  double time() const {
    if (running_) {
      return time_+timeSinceStart();
    } else {
      return time_;
    }
  }

private:
  bool running_; // true, if stopwatch running
  clock_t start_; // time at which watch was started
  double time_; // elapsed process time, in seconds, when last stopped

  double timeSinceStart() const {
    return ((double)(std::clock()-start_))/CLOCKS_PER_SEC;
  }
};

#endif