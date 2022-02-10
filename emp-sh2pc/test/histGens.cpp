#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include "emp-sh2pc/emp-sh2pc.h"
#include "tools.cpp"
using namespace emp;
using namespace std;



Integer * addedNoise(std::vector<int> number){
  int size = number.size();
  Integer *A = new Integer[size];
  Integer *B = new Integer[size];
  Integer *res = new Integer[size];
  for(int i = 0; i < size; ++i){
    A[i] = Integer(32, number[i], ALICE);
    B[i] = Integer(32, number[i], BOB);
    res[i] = A[i] + B[i];
  }
  return res;
}



Integer * computeHist(Integer * res_d, Integer * res, int bins, int size){
  Integer *res_h = new Integer[bins];
  for(int i = 0; i < bins; ++i){
    res_h[i] = Integer(32, 0, ALICE);
  }
  Integer one(32, 1, BOB);
  for(int i = 0; i < size; ++i){
    Bit eq_real = res_d[i] == one;   
    Integer bin_num = res[i] - one;                                                    
      for(int j = 0; j < bins; ++j){ //each bin    
        Bit eq_bin = bin_num == Integer(32, j, PUBLIC);    // ALICE OR PUBLIC?                                                 
          Bit eq_bin_count_real = eq_real & eq_bin;
          res_h[j] = If(eq_bin_count_real, res_h[j] + one, res_h[j]);  
    }
  }
  return res_h;
}

Integer * generateSh2(Integer * sh1, Integer * res_h, int bins){
  Integer *sh2 = new Integer[bins];
  for(int i = 0; i < bins; ++i) {
    sh2[i] = sh1[i] ^ res_h[i];
  }
  return sh2;
}

std::vector<int> encodeData(int party, int size, std::vector<int> randomVect, std::vector<int> v_originalData, std::vector<int> v_originalDummyMarkers){
  // reconstruct original data
  Integer *res = reconstructArray(v_originalData);
  // reconstruct dummy mark
 // Integer *res_d = reconstructArray(v_originalDummyMarkers);  // not need to check, since assume dummy value is 0;
  // reconstruct random number
  Integer *sh1 = reconstructArray(randomVect);

  Integer *sh2 = new Integer[size];
  Integer *encoded = new Integer[size];

  for (int i = 0; i < size; i++){   // categorical: encoded = original; numerical: please specify the range and bin size;
    encoded[i] = res[i];
  }
  sh2 = generateSh2(sh1, encoded, size);

  std::vector<int> realSh1 = revealSh(sh1, size, ALICE);
  std::vector<int> realSh2 = revealSh(sh2, size, BOB);

  if (party == ALICE) {
    return realSh1;
  }
  else {
    return realSh2;
  }

}

//std::pair<Integer, Integer>
std::vector<int> trueHistGen(int party, std::vector<int> number, std::vector<int> number2, std::vector<int> number3
, int size, int bins) {
  // reconstruct original data
  Integer *res = reconstructArray(number);
 
  // reconstruct dummy mark
  Integer *res_d = reconstructArray(number2);

  // reconstruct random number
  Integer *sh1 = reconstructArray(number3);

  // compute histogram
  Integer *res_h = computeHist(res_d, res, bins, size);
    
  /*cout << "random number" << ' ';
  printArray(sh1, bins);

  cout << "original data" << ' ';
  printArray(res, size);

  cout << "real or dummy" << ' ';
  printArray(res_d, size);
  
  cout << "counter for each bin" << ' ';
  for (int i = 0; i < bins; i++) {
    cout << res_h[i].reveal<int32_t>() << ' ';
  }
  cout << endl;*/

  // generate secret shares      
  Integer *sh2 = generateSh2(sh1, res_h, bins);
 
  // for debug
 /* cout << "counter for each bin" << ' ';
  for (int i = 0; i < bins; i++) {
    cout << (sh1[i] ^ sh2[i]).reveal<int32_t>() << ' ';
  }
  cout <<  endl;*/

  std::vector<int> realSh1 = revealSh(sh1, bins, ALICE);
  std::vector<int> realSh2 = revealSh(sh2, bins, BOB);
  
  /*cout << "sh1" << ' ';
  printArray(sh1, bins);
  cout << "sh2" << ' ';
  printArray(sh2, bins);*/

  if (party == ALICE) {
    return realSh1;
  }
  else {
    return realSh2;
  }
}

// for debug
std::vector<int> computeTrueNumber(std::vector<std::vector<int> > number, int bins){
  std::vector<int> res_num(bins, 0);
  Integer *res = new Integer[bins];
  for(int i = 0; i < bins; ++i){
    res[i] = Integer(32, 0, ALICE);
  }
  for (int j = 0; j < number.size(); j++) { 
    Integer *hist = reconstructArray(number[j]);
    for(int i = 0; i < bins; ++i){
      res[i] = hist[i] + res[i];
    }
  }
  for (int i = 0; i < bins; i++) {
    res_num[i] = res[i].reveal<int32_t>();
  }
  return res_num;
}


std::vector<int> dpHistGen(int party, std::vector<std::vector<int> > number, std::vector<int> number2, int bins) {
  // reconstruct true historgam
  Integer *res = new Integer[bins];
  for(int i = 0; i < bins; ++i){
    res[i] = Integer(32, 0, ALICE);
  }
  for (int j = 0; j < number.size(); j++) { 
    Integer *hist = reconstructArray(number[j]);
    for(int i = 0; i < bins; ++i){
      res[i] = hist[i] + res[i];
    }
  }
  // reconstruct dp noise
  Integer *res_lap = addedNoise(number2);

  /*cout << "true hist" << ' ';
  printArray(res, bins);*/
  
  // add noise
  for(int i = 0; i < bins; ++i){
    res[i] = res[i] + res_lap[i];
  }

  /*cout << "added noise" << ' ';
  printArray(res_lap, bins);

  cout << "after adding noise" << ' ';
  printArray(res, bins);*/
  
  std::vector<int> dpHist;
  for (int i = 0; i < bins; i++) {
    dpHist.push_back(res[i].reveal<int32_t>());
  }
  return dpHist;
}


