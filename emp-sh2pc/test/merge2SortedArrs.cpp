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
  for (int i = 1; i < int(vect.size()); i++) {
    res.push_back(res[i - 1] + vect[i]);
  }
  return res;
}

std::vector<int> slicing(vector<int>& arr, int X, int Y)
{
  if (X > Y){
    return {};
  }
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

std::vector<int> computeTrueRecords(std::vector<int> dpHist, std::vector<int> dpStore){
  int binNum = dpHist.size();
  int recordNum = dpStore.size();
  Integer * dpStoreI = reconstructArray(dpStore);
  std::vector<int> dpStorePublic = revealSh(dpStoreI, recordNum, PUBLIC);
   
  // prefix 
  std::vector<int> dpHistPrefix_tmp = computePrefix(dpHist);
  dpHistPrefix_tmp[binNum-1] = recordNum;  // cut DP hist to be consistent with the dpStore; assume that only last bin
  std::vector<int> dpHistPrefix(binNum+1, 0); 
  for (int j = 0; j < binNum; j++) {
    dpHistPrefix[j+1] = dpHistPrefix_tmp[j];
  }
    
  // compute the number of true records 
  std::vector<int> trueR(binNum, 0);
  for (int i = 0; i < binNum; i++){
    int num = 0;
    for (int j = dpHistPrefix[i]; j < dpHistPrefix[i+1]; j++){
       if ((dpStorePublic[j]-1) == i){ //dpStorePublic[j]: 1122334455
        num++;
      }
    }
    trueR[i] = num;
  }
  return trueR;
}

std::pair<std::vector<std::vector<int> >, std::vector<std::vector<int> > > seperateD(std::vector<std::vector<int> > dpMergedPrevious, std::vector<std::vector<int> > dataMergedPrevious, int d, int bins){
  std::vector<std::vector<int> > vectFirst;
  std::vector<std::vector<int> > vectSecond; 
  int intervals = dpMergedPrevious.size();

  // preprocess preefix --> cut last bin if no enough records 
  // preprocess prefixsum
  std::vector<std::vector<int> > dpHistPrefixIntrevals; 
  for (int i = 0; i < intervals; i++){  // for each interval
    int sizeInterval = dataMergedPrevious[i].size();
    dpMergedPrevious[i][bins] = sizeInterval;
    std::vector<int> dpHistPrefix(bins+1, 0); 
    for (int j = 0; j < bins; j++) {
      dpHistPrefix[j+1] = dpMergedPrevious[i][j];
    }
    dpHistPrefixIntrevals.push_back(dpHistPrefix);
  }

  for (int i = 0; i < bins; i++){  // for each bin 
    std::vector<int> first;
    std::vector<int> second;
    for (int j = 0; j < intervals; j++){  // for each interval
      std::vector<int> begining = slicing(dataMergedPrevious[j], dpHistPrefixIntrevals[j][i], dpHistPrefixIntrevals[j][i+1] - d - 1);
      int left = (dpHistPrefixIntrevals[j][i] < dpHistPrefixIntrevals[j][i+1] - d) ? dpHistPrefixIntrevals[j][i+1] - d: dpHistPrefixIntrevals[j][i];
      std::vector<int> ending = slicing(dataMergedPrevious[j], left, dpHistPrefixIntrevals[j][i+1] - 1);
      first.insert(first.end(), begining.begin(), begining.end());  // the first n-d
      second.insert(second.end(), ending.begin(), ending.end());   // the last d 
    }
    vectFirst.push_back(first);
    vectSecond.push_back(second);
  }  

  return std::make_pair(vectFirst, vectSecond);
}
