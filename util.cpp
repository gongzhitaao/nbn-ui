#include "util.h"

int bisearch_le(int key, const int *arr, int size)
{
  int low = 0, high = size - 1;
  if (key >= arr[high]) return high;

  while (low < high) {
    int mid = (low + high) / 2;
    if (arr[mid] > key) high = mid;
    else low = mid + 1;
  }
  return high - 1;
}

int bisearch_ge(int key, const int *arr, int size)
{
  int low = 0, high = size - 1;
  if (key > arr[high]) return size;
  while (low < high) {
    int mid = (low + high) / 2;
    if (arr[mid] >= key) high = mid;
    else low = mid + 1;
  }
  return high;
}
