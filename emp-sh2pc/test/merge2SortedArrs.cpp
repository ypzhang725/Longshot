#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
using namespace std;

std::vector<int> computePrefix(std::vector<int> vect){
  std::vector<int> res;
  res.push_back(vect[0]);
  for (int i = 1; i < vect.size(); i++) {
    res.push_back(res[i - 1] + vect[i]);
  }
  return res;
}

std::vector<int> slicing(vector<int>& arr, int X, int Y)
{
  // Starting and Ending iterators
  auto start = arr.begin() + X;
  auto end = arr.begin() + Y + 1;
  // To store the sliced vector
  std::vector<int> result(Y - X + 1);
  // Copy vector using copy function()
  copy(start, end, result.begin());
  // Return the final sliced vector
  return result;
}

std::vector<int> merge2SortedArr(std::vector<int> dp_main, std::vector<int> dp_cache, std::vector<int> main, std::vector<int> cache, int bins){
  if (main.size() == 0){
    return cache;
  }
  
  std::vector<int> merged;
  std::vector<int> dp_main_prefix = computePrefix(dp_main);
  std::vector<int> dp_cache_prefix = computePrefix(dp_cache);

  std::vector<int> insertedMain = slicing(main, 0, dp_main_prefix[0] - 1);
  std::vector<int> insertedCache = slicing(cache, 0, dp_cache_prefix[0] - 1);

  merged.insert(merged.end(), insertedMain.begin(), insertedMain.end());
  merged.insert(merged.end(), insertedCache.begin(), insertedCache.end());

  for (int i = 0; i < bins - 1; i++) {
    insertedMain = slicing(main, dp_main_prefix[i], dp_main_prefix[i+1] - 1);
    insertedCache = slicing(cache, dp_cache_prefix[i], dp_cache_prefix[i+1] - 1);
    merged.insert(merged.end(), insertedMain.begin(), insertedMain.end());
    merged.insert(merged.end(), insertedCache.begin(), insertedCache.end());
  }
  return merged;
}