#ifndef UTIL_H_
#define UTIL_H_

#pragma once

#include <cmath>
#include <chrono>

class Timer
{
 public:
  Timer() : beg_(clock_::now()) {}
  void reset() { beg_ = clock_::now(); }
  double elapsed() const {
    return std::chrono::duration_cast<second_>
        (clock_::now() - beg_).count(); }

 private:
  typedef std::chrono::high_resolution_clock clock_;
  typedef std::chrono::duration<double, std::ratio<1> > second_;
  std::chrono::time_point<clock_> beg_;
};

// The func_d is the derivative of func.  The func_s is the
// symmetric version of func.  And note for sigmoid and tanh, the
// derivative is based on output of sigmoid and tanh respectively.

inline double linear(double x, double k) { return k * x; }
inline double linear_d(double /* x */, double k) { return k; }

// They are not supposed to be used for training, so no derivatives
// are defined here.
inline double threshold(double x, double /* k */) { return x < 0 ? 0 : 1; }
inline double threshold_s(double x, double /* k */) { return x < 0 ? -1 : 1; }

inline double sigmoid(double x, double k) { return 1 / (1 + std::exp(-k * x)); }
inline double sigmoid_d(double y, double k) { return k * y * (1 - y); }

inline double tanh(double x, double k) { return std::tanh(k * x); }
inline double tanh_d(double y, double k) { return k * (1 - y * y); }

// Search for position such that all elements that lie before are less
// or equal to *key*.
int bisearch_le(int key, const int *arr, int size);

// Search for position such that all elements that lie after are
// greater or equal to *key*.
int bisearch_ge(int key, const int *arr, int size);

#endif
