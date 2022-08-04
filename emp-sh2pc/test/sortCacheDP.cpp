#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include "emp-sh2pc/emp-sh2pc.h"
using namespace emp;
using namespace std;

// for debug
Integer memconcat(Integer int1, Integer int2){
  Integer res(64, 0, ALICE);
  memcpy(res.bits.data(), int1.bits.data(), 32* sizeof(block));
  memcpy(res.bits.data()+32, int2.bits.data(), 32 * sizeof(block));
  return res;
}

Integer memconcat(Integer int1, Integer int2, Integer int3){
  Integer res(64, 0, ALICE);
  memcpy(res.bits.data(), int3.bits.data(), 16 * sizeof(block));
  memcpy(res.bits.data()+16, int2.bits.data(), 16 * sizeof(block));
  memcpy(res.bits.data()+16+16, int1.bits.data(), 32 * sizeof(block));
  return res;
}

// for debug
  /*Integer *res_2 = new Integer[size];
  for(int i = 0; i < size; ++i){
    res_2[i] = memconcat(res[i], res_d[i]);
  }
  
  sort(res_b, size, res_2);
  for(int i = 0; i < size; ++i){
    std::pair<Integer, Integer> res_temp = memseperate(res_2[i]);
    res[i] = res_temp.first;
    res_d[i] = res_temp.second;
  }*/

  // for debug

// for debug
std::pair<Integer, Integer> memseperate(Integer res){
  Integer res1(32, 0, ALICE);
  Integer res2(32, 0, BOB);
  memcpy(res1.bits.data(), res.bits.data(), 32* sizeof(block));
  memcpy(res2.bits.data(), res.bits.data()+32, 32 * sizeof(block));
  return std::make_pair(res1, res2);
}

Integer * toIntegerArray(std::vector<int> num, int party){
  int size = num.size();
  Integer * numInt = new Integer[size];
  for(int i = 0; i < size; ++i){
    numInt[i] = Integer(32, num[i], party);
  }
  return numInt;
}

Integer * assignBin(Integer *res, Integer *res_d, int size, int bins, std::vector<int> counter){
  // initialize dp counter
  Integer *counter_A = toIntegerArray(counter, ALICE);
  Integer *counter_B = toIntegerArray(counter, BOB);
  Integer two(32, 2, PUBLIC);
  Integer *res_counter = new Integer[bins];
  for(int i = 0; i < bins; ++i){
    res_counter[i] = (counter_A[i] + counter_B[i]) / two;
  }
  // initialize bin mark                                                                                        
  Integer *res_b = new Integer[size];
  for(int i = 0; i < size; ++i){
    res_b[i] = Integer(32, bins*2+1, ALICE); 
  }
  
  Integer zero(32, 0, PUBLIC);
  Integer one(32, 1, PUBLIC);
  // first round: real records                                                                       
  for(int i = 0; i < size; ++i){
    Bit eq_real = res_d[i] == zero;   
    res_b[i] = If(eq_real, res_b[i]-one, res_b[i]);  
    Integer bin_num = res[i] - one;                         
    for(int j = 0; j < bins; ++j){  
      Bit eq_bin = bin_num == Integer(32, j, PUBLIC);    // ALICE OR PUBLIC?
      Bit eq_count = res_counter[j] > zero;     
      Bit eq_bin_count_real = eq_real & eq_bin & eq_count;
      res_b[i] = If(eq_bin_count_real, bin_num*two, res_b[i]);  
      res_counter[j] = If(eq_bin_count_real, res_counter[j] - one, res_counter[j]);    
    }
  }    
 // cout << "assigned bin number for each records after first round" << ' ';
 // printArray(res_b, size);                                   
  // second round: dummy records   
  // warning: the dummy records should be enough, otherwise real records will be retrieved as dummy                                                                  
  for(int i = 0; i < size; ++i){   
    Bit eq_dummy = res_d[i] == one;
    for(int j = 0; j < bins; ++j){  
      Bit eq_bin_last = res_b[i] == Integer(32, bins*2+1, PUBLIC);    // ALICE OR PUBLIC?
      Bit eq_count = res_counter[j] > zero;  
      Bit eq_bin_count_dummy = eq_dummy & eq_bin_last & eq_count;   
      res_b[i] = If(eq_bin_count_dummy, Integer(32, j, ALICE)*two+one, res_b[i]);  
      res_counter[j] = If(eq_bin_count_dummy, res_counter[j] - one, res_counter[j]);                         
    }
  }
 // cout << "assigned bin number for each records" << ' ';
 // printArray(res_b, size);
  return res_b;
}

/*std::pair<Integer, Integer> *sort2(Integer *res, Integer *res_d, Integer *res_b, int size){
  std::pair<Integer, Integer> *res_seperate;
  Integer *res_2 = new Integer[size];
  for(int i = 0; i < size; ++i){
    res_2[i] = memconcat(res[i], res_d[i]);
  }

  sort(res_b, size, res_2);
  for(int i = 0; i < size; ++i){
    res_seperate[i] = memseperate(res_2[i]);
  }
  return res_seperate;
    
}*/

std::pair<std::vector<int>, std::vector<int> > copy2two(std::vector<int> num, int index, int dropDummy){
  int sizeOriginal = num.size();
  int sizeDroppedDummy = sizeOriginal - dropDummy;
  // warning: to remove bugs, the DP count is not larger than the total number of records.
  // actually we need to padd more records!
  if (index > sizeDroppedDummy) {     
    index = sizeDroppedDummy;
  }
  std::vector<int> vect1;
  std::vector<int> vect2;
  for(int i = 0; i < index; ++i){
    vect1.push_back(num[i]);
  }
  for(int i = index; i < sizeDroppedDummy; ++i){
    vect2.push_back(num[i]);
  }
  return std::make_pair(vect1, vect2);
}

Integer* copyArray(Integer* array, int size){
  Integer * copy = new Integer[size];
  for(int i = 0; i < size; ++i){
    copy[i] = array[i];
  }
  return copy; 
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int> > sortDP(int party, std::vector<int> encodedData, std::vector<int> dummyMarker, std::vector<int> notEncodedData, std::vector<int> dpHist, int size, int bins) {
 // cout<< "secret shares" << ' ';
 // printArrayPlaintext(encodedData);

  // reconstruct random     
  std::vector<int> randomVect_res = uniformGenVector(size);                                                            
  Integer *sh1_res = reconstructArray(randomVect_res);
  std::vector<int> randomVect_res_d = uniformGenVector(size);                                                            
  Integer *sh1_res_d = reconstructArray(randomVect_res_d);
  std::vector<int> randomVect_res_NotEn = uniformGenVector(size);                                                            
  Integer *sh1_res_NotEn = reconstructArray(randomVect_res_NotEn);

  std::vector<int> counter = dpHist; 
  // reconstruct original data                                                                      
  Integer *res = reconstructArray(encodedData);
  // reconstruct dummy mark                                                                         
  Integer *res_d = reconstructArray(dummyMarker);
  // reconstruct original data not encoded     
  Integer *res_NotEn = reconstructArray(notEncodedData);
  Integer *res_b = assignBin(res, res_d, size, bins, counter);

  Integer * res_b_copy = copyArray(res_b, size);
  Integer * res_b_copy2 = copyArray(res_b, size);
  Integer * res_b_copy3 = copyArray(res_b, size);

  sort(res_b_copy, size, res);
  sort(res_b_copy2, size, res_d);
  sort(res_b_copy3, size, res_NotEn);

  /*cout << "after sort" << ' ';
  cout << "original records" << ' ';
  printArray(res, size);
  cout << "dummy marker" << ' ';
  printArray(res_d, size);
  cout << "assigned bin" << ' ';
  printArray(res_b_copy, size);*/

  // generate secret shares      
  Integer *sh2_res = generateSh2(sh1_res, res, size);
  Integer *sh2_res_d = generateSh2(sh1_res_d, res_d, size);
  Integer *sh2_res_NotEn = generateSh2(sh1_res_NotEn, res_NotEn, size);

  std::vector<int> A_reveal = revealSh(sh1_res, size, ALICE);
  std::vector<int> B_reveal = revealSh(sh2_res, size, BOB);
  std::vector<int> D_A_reveal = revealSh(sh1_res_d, size, ALICE);
  std::vector<int> D_B_reveal = revealSh(sh2_res_d, size, BOB);
  std::vector<int> A_reveal_notEn = revealSh(sh1_res_NotEn, size, ALICE);
  std::vector<int> B_reveal_notEn = revealSh(sh2_res_NotEn, size, BOB);

  if (party == ALICE) {
    return std::make_tuple(A_reveal, D_A_reveal, A_reveal_notEn);
  }
  else {
    return std::make_tuple(B_reveal, D_B_reveal, B_reveal_notEn);
  }

}

std::tuple<std::vector<int>, std::vector<int> > sortDPNew(int party, std::vector<int> encodedData, std::vector<int> dummyMarker, std::vector<int> dpHist, int size, int bins, int num_dummy_bin) {
 // cout<< "secret shares" << ' ';
 // printArrayPlaintext(encodedData);

  // reconstruct random     
  std::vector<int> randomVect_res = uniformGenVector(size);                                                            
  Integer *sh1_res = reconstructArray(randomVect_res);
  std::vector<int> randomVect_res_d = uniformGenVector(size);                                                            
  Integer *sh1_res_d = reconstructArray(randomVect_res_d);
  std::vector<int> randomVect_res_NotEn = uniformGenVector(size);                                                            
 // Integer *sh1_res_NotEn = reconstructArray(randomVect_res_NotEn);

  std::vector<int> counter = dpHist; 

  // reconstruct original data                                                                      
  Integer *res = reconstructArray(encodedData);
  // reconstruct dummy mark                                                                         
  Integer *res_d = reconstructArray(dummyMarker);
  // reconstruct original data not encoded     
//  Integer *res_NotEn = reconstructArray(notEncodedData);

  Integer zero(32, 0, PUBLIC);
  Integer one(32, 1, PUBLIC);
  Integer two(32, 2, PUBLIC);

  /*std::vector<int> dpStorePublicO = revealSh(res, size, PUBLIC);
  std::vector<int> dpStorePublicDummyO = revealSh(res_d, size, PUBLIC);
  for (int j = 0; j < size; j++) {
    cout << dpStorePublicO[j] << ' ';
  }
  cout << endl;
  for (int j = 0; j < size; j++) {
    cout << dpStorePublicDummyO[j] << ' ';
  }
  cout << endl;
  cout << num_dummy_bin << endl;*/
  // step0: reassign dummy data value 
  Integer cnt_dummy_bin(32, num_dummy_bin, BOB);   // assume we have a fixed number of dummy for each bin
  Integer num_bin(32, 1, BOB);   // from 1
  for(int i = 0; i < size; ++i){
    Bit eq_dummy = res_d[i] == one;  
    Bit cnt_dummy_zero = cnt_dummy_bin == zero;  
    cnt_dummy_bin = If(cnt_dummy_zero, Integer(32, num_dummy_bin, PUBLIC), cnt_dummy_bin); 
    Bit bins_enough = num_bin <= Integer(32, bins, BOB);
    num_bin = If(cnt_dummy_zero, num_bin + one, num_bin); 
    Bit assign = eq_dummy & bins_enough;
    res[i] = If(assign, num_bin, res[i]); 
    cnt_dummy_bin = If(assign, cnt_dummy_bin - one, cnt_dummy_bin); 
  }

  /*std::vector<int> dpStorePublic = revealSh(res, size, PUBLIC);
  std::vector<int> dpStorePublicDummy = revealSh(res_d, size, PUBLIC);
  for (int j = 0; j < size; j++) {
    cout << dpStorePublic[j] << ' ';
  }
  cout << endl;
  for (int j = 0; j < size; j++) {
    cout << dpStorePublicDummy[j] << ' ';
  }
  cout << endl;*/

  // step1: <dataValue, isDummy> --> <dataValue, isDummy, isCounter, counter>, append m values 
  // isDummy: real: 0; dummy: 1   
  // isCounter: counter: 0; record: 1
  int newSize = size+bins;
  Integer *isCounter =new Integer[newSize];
  Integer *counterValue = new Integer[newSize];
  Integer *isDummy = new Integer[newSize];
  Integer *dataValue = new Integer[newSize];
 // Integer *dataValueNotEn = new Integer[newSize];
  Integer *binValue = new Integer[newSize];
  
  for(int i = 0; i < size; ++i){
    isCounter[i] = Integer(16, 1, BOB);
    counterValue[i] = Integer(32, 0, BOB);
    dataValue[i] = res[i];
//    dataValueNotEn[i] = res_NotEn[i];
    Bit eq_real = res_d[i] == zero;   
    binValue[i] = If(eq_real, Integer(32, (bins+1)*2-1, BOB), Integer(32, (bins+1)*2, BOB));  
    isDummy[i] = If(eq_real, Integer(16, 0, BOB), Integer(16, 1, BOB));  
  }

  for(int i = size; i < newSize; ++i){
    isCounter[i] = Integer(16, 0, BOB);
    counterValue[i] = Integer(32, counter[i-size], BOB);
    isDummy[i] = Integer(16, 1, BOB);
    dataValue[i] = Integer(32, i-size+1, BOB);
 //   dataValueNotEn[i] = Integer(32, i-size+1, BOB);
    binValue[i] = Integer(32, (bins+1)*2+1, BOB);
  }

  /*std::vector<int> dataValue_ = revealSh(dataValue, newSize, PUBLIC);
  std::vector<int> isDummy_ = revealSh(isDummy, newSize, PUBLIC);
  std::vector<int> counterValue_ = revealSh(counterValue, newSize, PUBLIC);
  std::vector<int> isCounter_ = revealSh(isCounter, newSize, PUBLIC);
  for (int j = 0; j < newSize; j++) {
    cout << dataValue_[j] << ' ';
  }
  cout << endl;
  for (int j = 0; j < newSize; j++) {
    cout << isDummy_[j] << ' ';
  }
  cout << endl;
  for (int j = 0; j < newSize; j++) {
    cout << counterValue_[j] << ' ';
  }
  cout << endl;
  for (int j = 0; j < newSize; j++) {
    cout << isCounter_[j] << ' ';
  }
  cout << endl;*/

  // step2: sort by (dataValue, isCounter, isDummy)
  Integer *sortKey = new Integer[newSize];
  for(int i = 0; i < newSize; ++i){
    sortKey[i] = memconcat(dataValue[i], isCounter[i], isDummy[i]);
  }
 /* std::vector<int> sortKey_ = revealSh64(sortKey, newSize, PUBLIC);
  cout << "sortKey_" << endl;
  for (int j = 0; j < newSize; j++) {
    cout << sortKey_[j] << ' ';
  }
  cout << endl;*/
  Integer * sortKey_copy = copyArray(sortKey, newSize);
//  Integer * sortKey_copy2 = copyArray(sortKey, newSize);
  Integer * sortKey_copy3 = copyArray(sortKey, newSize);
  Integer * sortKey_copy4 = copyArray(sortKey, newSize);
  Integer * sortKey_copy5 = copyArray(sortKey, newSize);
  Integer * sortKey_copy6 = copyArray(sortKey, newSize);

  sort(sortKey_copy, newSize, dataValue);
 // sort(sortKey_copy2, newSize, dataValueNotEn);
  sort(sortKey_copy3, newSize, isDummy);
  sort(sortKey_copy4, newSize, isCounter);
  sort(sortKey_copy5, newSize, counterValue);
  sort(sortKey_copy6, newSize, binValue);

  /*dataValue_ = revealSh(dataValue, newSize, PUBLIC);
  isDummy_ = revealSh(isDummy, newSize, PUBLIC);
  counterValue_ = revealSh(counterValue, newSize, PUBLIC);
  isCounter_ = revealSh(isCounter, newSize, PUBLIC);
  cout << "after sort" << endl;
  for (int j = 0; j < newSize; j++) {
    cout << dataValue_[j] << ' ';
  }
  cout << endl;
  for (int j = 0; j < newSize; j++) {
    cout << isDummy_[j] << ' ';
  }
  cout << endl;
  for (int j = 0; j < newSize; j++) {
    cout << counterValue_[j] << ' ';
  }
  cout << endl;
  for (int j = 0; j < newSize; j++) {
    cout << isCounter_[j] << ' ';
  }
  cout << endl;
  std::vector<int> binValue_ = revealSh(binValue, newSize, PUBLIC);
  cout << "binValue" << endl;
  for (int j = 0; j < newSize; j++) {
    cout << binValue_[j] << ' ';
  }*/

  // step3: linear scan to assign binValue
  Integer cnt = counterValue[0];
  for(int i = 0; i < newSize; ++i){
    Bit eq_counter_not = isCounter[i] == one;  
    cnt = If(eq_counter_not, cnt - one, counterValue[i]); 
    Bit bigger_zero = cnt >= zero;  
    Bit eq_real = isDummy[i] == zero;  
    Bit real_bin = bigger_zero & eq_counter_not & eq_real;
    Bit dummy_bin = bigger_zero & eq_counter_not & !eq_real;
    binValue[i] = If(real_bin, dataValue[i] * two, binValue[i]); 
    binValue[i] = If(dummy_bin, dataValue[i] * two + one, binValue[i]); 
  }

  /*binValue_ = revealSh(binValue, newSize, PUBLIC);
  cout << "binValue" << endl;
  for (int j = 0; j < newSize; j++) {
    cout << binValue_[j] << ' ';
  }*/

  // step4: sort by binValue
  Integer * res_b_copy = copyArray(binValue, newSize);
//  Integer * res_b_copy2 = copyArray(binValue, newSize);
  Integer * res_b_copy3 = copyArray(binValue, newSize);

  sort(res_b_copy, newSize, dataValue);
//  sort(res_b_copy2, newSize, dataValueNotEn);
  sort(res_b_copy3, newSize, isDummy);

  // step5: drop m counters 
  for(int i = 0; i < size; ++i){
    res[i] = dataValue[i];
    res_d[i] = isDummy[i];
 //   res_NotEn[i] = dataValueNotEn[i];
  }

  /*cout << "after sort" << ' ';
  cout << "original records" << ' ';
  printArray(res, size);
  cout << "dummy marker" << ' ';
  printArray(res_d, size);
  cout << "assigned bin" << ' ';
  printArray(res_b_copy, size);*/

  // generate secret shares      
  Integer *sh2_res = generateSh2(sh1_res, res, size);
  Integer *sh2_res_d = generateSh2(sh1_res_d, res_d, size);
 // Integer *sh2_res_NotEn = generateSh2(sh1_res_NotEn, res_NotEn, size);

  std::vector<int> A_reveal = revealSh(sh1_res, size, ALICE);
  std::vector<int> B_reveal = revealSh(sh2_res, size, BOB);
  std::vector<int> D_A_reveal = revealSh(sh1_res_d, size, ALICE);
  std::vector<int> D_B_reveal = revealSh(sh2_res_d, size, BOB);
//  std::vector<int> A_reveal_notEn = revealSh(sh1_res_NotEn, size, ALICE);
//  std::vector<int> B_reveal_notEn = revealSh(sh2_res_NotEn, size, BOB);

  if (party == ALICE) {
    return std::make_tuple(A_reveal, D_A_reveal);
  }
  else {
    return std::make_tuple(B_reveal, D_B_reveal);
  }

}

Integer * assignBinCompaction(Integer *res, Integer *res_d, int size, int binNumber, std::vector<int> counter){
  // initialize dp counter(only one element)
  Integer *counter_A = toIntegerArray(counter, ALICE);
  Integer *counter_B = toIntegerArray(counter, BOB);
  Integer two(32, 2, PUBLIC);
  Integer *res_counter = new Integer[1];
  res_counter[0] = (counter_A[0] + counter_B[0]) / two;
  
  Integer one(32, 1, PUBLIC);
  Integer zero(32, 0, PUBLIC);

  // initialize bin mark                                                                                        
  Integer *res_b = new Integer[size];
  for(int i = 0; i < size; ++i){
    res_b[i] = one; 
  }
  
  // first round: real records                                                                       
  for(int i = 0; i < size; ++i){
    Bit eq_real = res_d[i] == zero;   
    //res_b[i] = If(eq_real, res_b[i] + one, res_b[i]);  
    Integer bin_num = res[i] - one;    // from 0!!!!                     
    Bit eq_bin = bin_num == Integer(32, binNumber, PUBLIC);    // PUBLIC?
    Bit eq_count = res_counter[0] > zero;     
    Bit eq_bin_count_real = eq_real & eq_bin & eq_count;
    res_b[i] = If(eq_bin_count_real, zero, res_b[i]);  
    res_counter[0] = If(eq_bin_count_real, res_counter[0] - one, res_counter[0]);    
  }    
 // cout << "assigned bin number for each records after first round" << ' ';
 // printArray(res_b, size);                                   
  // second round: dummy records   
  // warning: the dummy records should be enough, otherwise real records will be retrieved as dummy                                                                  
  for(int i = 0; i < size; ++i){   
    Bit eq_dummy = res_d[i] == one;
    Bit eq_bin = res_b[i] == one;    // ALICE OR PUBLIC?
    Bit eq_count = res_counter[0] > zero;  
    Bit eq_bin_count_dummy = eq_dummy & eq_bin & eq_count;   
    res_b[i] = If(eq_bin_count_dummy, zero, res_b[i]);  
    res_counter[0] = If(eq_bin_count_dummy, res_counter[0] - one, res_counter[0]);                         
  }
 // cout << "assigned bin number for each records" << ' ';
 // printArray(res_b, size);
  return res_b;
}

std::tuple<std::vector<int>, std::vector<int> > sortBinDP(int party, std::vector<int> encodedData, std::vector<int> dummyMarker, int dpCount, int size, int binNum) {
 // cout<< "secret shares" << ' ';
 // printArrayPlaintext(encodedData);

  // reconstruct random     
  std::vector<int> randomVect_res = uniformGenVector(size);                                                            
  Integer *sh1_res = reconstructArray(randomVect_res);
  std::vector<int> randomVect_res_d = uniformGenVector(size);                                                            
  Integer *sh1_res_d = reconstructArray(randomVect_res_d);
 // std::vector<int> randomVect_res_NotEn = uniformGenVector(size);                                                            
 // Integer *sh1_res_NotEn = reconstructArray(randomVect_res_NotEn);

  std::vector<int> counter(1, dpCount); 
  // reconstruct original data                                                                      
  Integer *res = reconstructArray(encodedData);
  // reconstruct dummy mark                                                                         
  Integer *res_d = reconstructArray(dummyMarker);
  // reconstruct original data not encoded     
//  Integer *res_NotEn = reconstructArray(notEncodedData);
  // tag the records for this binNum as 0, otherwise as 1
  Integer *res_b = assignBinCompaction(res, res_d, size, binNum, counter);

  Integer * res_b_copy = copyArray(res_b, size);
  Integer * res_b_copy2 = copyArray(res_b, size);
 // Integer * res_b_copy3 = copyArray(res_b, size);

  sort(res_b_copy, size, res);
  sort(res_b_copy2, size, res_d);
 // sort(res_b_copy3, size, res_NotEn);

  /*cout << "after sort" << ' ';
  cout << "original records" << ' ';
  printArray(res, size);
  cout << "dummy marker" << ' ';
  printArray(res_d, size);
  cout << "assigned bin" << ' ';
  printArray(res_b_copy, size);*/

  // generate secret shares      
  Integer *sh2_res = generateSh2(sh1_res, res, size);
  Integer *sh2_res_d = generateSh2(sh1_res_d, res_d, size);
//  Integer *sh2_res_NotEn = generateSh2(sh1_res_NotEn, res_NotEn, size);

  std::vector<int> A_reveal = revealSh(sh1_res, size, ALICE);
  std::vector<int> B_reveal = revealSh(sh2_res, size, BOB);
  std::vector<int> D_A_reveal = revealSh(sh1_res_d, size, ALICE);
  std::vector<int> D_B_reveal = revealSh(sh2_res_d, size, BOB);
//  std::vector<int> A_reveal_notEn = revealSh(sh1_res_NotEn, size, ALICE);
//  std::vector<int> B_reveal_notEn = revealSh(sh2_res_NotEn, size, BOB);

  if (party == ALICE) {
    return std::make_tuple(A_reveal, D_A_reveal);
  }
  else {
    return std::make_tuple(B_reveal, D_B_reveal);
  }
}

std::tuple<std::vector<int>, std::vector<int> > sortOneBinDP(int party, std::vector<int> encodedData, std::vector<int> dummyMarker, std::vector<int> markers, int dpCount, int size) {
 // cout<< "secret shares" << ' ';
 // printArrayPlaintext(encodedData);

  // reconstruct random     
  std::vector<int> randomVect_res = uniformGenVector(size);                                                            
  Integer *sh1_res = reconstructArray(randomVect_res);
  std::vector<int> randomVect_res_d = uniformGenVector(size);                                                            
  Integer *sh1_res_d = reconstructArray(randomVect_res_d);
 // std::vector<int> randomVect_res_NotEn = uniformGenVector(size);                                                            
 // Integer *sh1_res_NotEn = reconstructArray(randomVect_res_NotEn);

  // reconstruct original data                                                                      
  Integer *res = reconstructArray(encodedData);
  // reconstruct dummy mark                                                                         
  Integer *res_d = reconstructArray(dummyMarker);
  // reconstruct original data not encoded     
 // Integer *res_NotEn = reconstructArray(notEncodedData);
  // tag the records for this binNum as 0, otherwise as 1
  Integer *res_b = reconstructArray(markers);

  Integer * res_b_copy = copyArray(res_b, size);
  Integer * res_b_copy2 = copyArray(res_b, size);
//  Integer * res_b_copy3 = copyArray(res_b, size);

  sort(res_b_copy, size, res);
  sort(res_b_copy2, size, res_d);
 // sort(res_b_copy3, size, res_NotEn);

  /*cout << "after sort" << ' ';
  cout << "original records" << ' ';
  printArray(res, size);
  cout << "dummy marker" << ' ';
  printArray(res_d, size);
  cout << "assigned bin" << ' ';
  printArray(res_b_copy, size);*/

  // generate secret shares      
  Integer *sh2_res = generateSh2(sh1_res, res, size);
  Integer *sh2_res_d = generateSh2(sh1_res_d, res_d, size);
//  Integer *sh2_res_NotEn = generateSh2(sh1_res_NotEn, res_NotEn, size);

  std::vector<int> A_reveal = revealSh(sh1_res, size, ALICE);
  std::vector<int> B_reveal = revealSh(sh2_res, size, BOB);
  std::vector<int> D_A_reveal = revealSh(sh1_res_d, size, ALICE);
  std::vector<int> D_B_reveal = revealSh(sh2_res_d, size, BOB);
//  std::vector<int> A_reveal_notEn = revealSh(sh1_res_NotEn, size, ALICE);
 // std::vector<int> B_reveal_notEn = revealSh(sh2_res_NotEn, size, BOB);

  delete[] sh1_res;
  delete[] sh1_res_d;
  delete[] res;
  delete[] res_d;
  delete[] res_b;
  delete[] res_b_copy;
  delete[] res_b_copy2;
  delete[] sh2_res;
  delete[] sh2_res_d;

  if (party == ALICE) {
    return std::make_tuple(A_reveal, D_A_reveal);
  }
  else {
    return std::make_tuple(B_reveal, D_B_reveal);
  }
}
