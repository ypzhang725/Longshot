#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
using namespace std;

std::vector<int> computePrefix(std::vector<int> vect){
  int vectSize = int(vect.size());
  std::vector<int> res(vectSize);
  res[0] = vect[0];
  for (int i = 1; i < int(vect.size()); i++) {
    res[i] = res[i - 1] + vect[i];
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

std::vector<int> computeTrueRecords(std::vector<int> dpHist, std::vector<int> dpStore, std::vector<int> dpStoreDummy){
  int binNum = dpHist.size();
  int recordNum = dpStore.size();
  Integer * dpStoreI = reconstructArray(dpStore);
  std::vector<int> dpStorePublic = revealSh(dpStoreI, recordNum, PUBLIC);
  Integer * dpStoreIDummy = reconstructArray(dpStoreDummy);
  std::vector<int> dpStorePublicDummy = revealSh(dpStoreIDummy, recordNum, PUBLIC);

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
       if (((dpStorePublic[j]-1) == i) && (dpStorePublicDummy[j] == 0)) { //dpStorePublic[j]: 1122334455
        num++;
      }
    }
    trueR[i] = num;
  }

  delete[] dpStoreI;
  delete[] dpStoreIDummy;

  return trueR;
}

std::vector<int> computeTrueRecordRange(std::vector<int> dpHist, std::vector<int> dpStore, std::vector<int> dpStoreDummy){
  int binNum = dpHist.size();
  int recordNum = dpStore.size();
  Integer * dpStoreI = reconstructArray(dpStore);
  std::vector<int> dpStorePublic = revealSh(dpStoreI, recordNum, PUBLIC);
  Integer * dpStoreIDummy = reconstructArray(dpStoreDummy);
  std::vector<int> dpStorePublicDummy = revealSh(dpStoreIDummy, recordNum, PUBLIC);

  // prefix 
  std::vector<int> dpHistPrefix_tmp = computePrefix(dpHist);
  dpHistPrefix_tmp[binNum-1] = recordNum;  // cut DP hist to be consistent with the dpStore; assume that only last bin
  std::vector<int> dpHistPrefix(binNum+1, 0); 
  for (int j = 0; j < binNum; j++) {
    dpHistPrefix[j+1] = dpHistPrefix_tmp[j];
  }

  // compute the number of true records 
  std::vector<int> trueR;
  int idx = 0;
  for (int j = 0; j < binNum; j++) {   
    for (int k = j; k < binNum; k++) {    // all range queries [j, k]
      trueR.push_back(0);
      for (int l = dpHistPrefix[j]; l < dpHistPrefix[k+1]; l++){   // for these records 
        for (int m = j; m <= k; m++){
          if (((dpStorePublic[l]-1) == m) && (dpStorePublicDummy[l] == 0)) { //dpStorePublic[j]: 1122334455
          trueR[idx]++;
          }
        }
      }
      idx++;
    }
  }

  delete[] dpStoreI;
  delete[] dpStoreIDummy;
  
  return trueR; 
}

std::pair<std::vector<std::vector<int> >, std::vector<int> > seperateD(std::vector<std::vector<int> > dpMergedPrevious, std::vector<std::vector<int> > dataMergedPrevious, int d, int bins){
  std::vector<std::vector<int> > vectFirst(bins);
  std::vector<int> vectSecond; 
  int intervals = dpMergedPrevious.size();

  // preprocess preefix --> cut last bin if no enough records 
  // preprocess prefixsum
  std::vector<std::vector<int> > dpHistPrefixIntrevals(intervals); 
  for (int i = 0; i < intervals; i++){  // for each interval
    int sizeInterval = dataMergedPrevious[i].size();
    dpMergedPrevious[i][bins-1] = sizeInterval;
    std::vector<int> dpHistPrefix(bins+1, 0); 
    for (int j = 0; j < bins; j++) {
      dpHistPrefix[j+1] = dpMergedPrevious[i][j];
    }
    dpHistPrefixIntrevals[i] = dpHistPrefix;
  }

  for (int i = 0; i < bins; i++){  // for each bin 
    std::vector<int> first;
    for (int j = 0; j < intervals; j++){  // for each interval
      std::vector<int> begining = slicing(dataMergedPrevious[j], dpHistPrefixIntrevals[j][i], dpHistPrefixIntrevals[j][i+1] - d - 1);
      int left = (dpHistPrefixIntrevals[j][i] < dpHistPrefixIntrevals[j][i+1] - d) ? dpHistPrefixIntrevals[j][i+1] - d: dpHistPrefixIntrevals[j][i];
      std::vector<int> ending = slicing(dataMergedPrevious[j], left, dpHistPrefixIntrevals[j][i+1] - 1);
      first.insert(first.end(), begining.begin(), begining.end());  // the first n-d
      vectSecond.insert(vectSecond.end(), ending.begin(), ending.end());   // the last d 
    }
    vectFirst[i] = first;
  }  

  return std::make_pair(vectFirst, vectSecond);
}

std::vector<std::vector<int> > seperateBin(std::vector<int> seperatedRecord, std::vector<int> sortDPdHist){
  int numBin = sortDPdHist.size();
  std::vector<int> sortDPdPrefix_tmp = computePrefix(sortDPdHist);
  std::vector<int> sortDPdPrefix(numBin+1, 0); 
  for (int j = 0; j < numBin; j++) {
    sortDPdPrefix[j+1] = sortDPdPrefix_tmp[j];
  }
  std::vector<std::vector<int> > seperatedBin(numBin);
  for (int j = 0; j < numBin; j++) {
    std::vector<int> toSortMergedPrevious = slicing(seperatedRecord, sortDPdPrefix[j], sortDPdPrefix[j+1] - 1);
    seperatedBin[j] = toSortMergedPrevious;
  }
  return seperatedBin;
}
