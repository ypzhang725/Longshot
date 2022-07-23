#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include<numeric>
#include <cmath>
#include "emp-sh2pc/emp-sh2pc.h"
#include "histGens.cpp"
#include "enforceConsistency.cpp"
#include "sortCacheDP.cpp"
#include "merge2SortedArrs.cpp"
#include <chrono>
using namespace std::chrono;
using namespace emp;
using namespace std;

std::vector<int> readInputs(string fileName){
  fstream sharesFile ;
  std::vector<int> vect;
  sharesFile.open(fileName, std::ios::in); //open a file to perform read operation using file object
  if (sharesFile.is_open()){   //checking whether the file is open
    std::string tp;
    while(getline(sharesFile, tp)){ //read data from file object and put it into string.
      std::stringstream ss(tp);
      for (int i; ss >> i;) {
        vect.push_back(i);
      }
    }
    sharesFile.close(); //close the file object.
  }
/*  cout << fileName << ' ';
  for (int i = 0; i < vect.size(); i++) {
    cout << vect[i] << ' ';
  }
  cout <<  endl;*/
  return vect;
}

int main(int argc, char** argv) {
  int port, party;
  parse_party_and_port(argv, &party, &port);
  NetIO * io = new NetIO(party==ALICE ? nullptr : "127.0.0.1", port);
  setup_semi_honest(io, party);

  // read real data from external file.
  string fileName_real = argv[3]; // original
  std::vector<int> vect = readInputs(fileName_real); // original
  // the number of updates
  string t_string = argv[4]; // t 
  int t = atoi(t_string.c_str());  //  the number of updates
  // constant dp noise
  bool constantDP = false; 
  // print 
  bool debugPrint = false;
  // privacy budget
  string eps_string = argv[5]; // eps
  double eps = std::stod(eps_string);
  // bin number 
  int bins = 0;
  // !warning: if there are not enough dummy records, then sortDP and copy2two are incorrect
  int num_real = 0;  
  int num_dummy_bin = 0; // make sure there are enough dummy records
  string N_string = argv[6]; // num of reals for each cache
   // nyc taxi dataset: 1271413 rows; 4 bins; payment_type
  if ((fileName_real == "taxi_ss1.txt") || (fileName_real == "taxi_ss2.txt")) {
    bins = 4; // bin number
    num_real = std::stod(N_string);
    //num_dummy = 10;
    double b = 1 / eps;
    double t = log((1/0.01));    // Pr[|Y| ≥ t · b] = exp(−t) = 0.1.
    num_dummy_bin = int(b * t);
  } else if ((fileName_real == "bin40_ss1.txt") || (fileName_real == "bin40_ss2.txt")) {
    bins = 40; // bin number
    num_real = std::stod(N_string);
    //num_dummy = 10;
    double b = 1 / eps;
    double t = log((1/0.01));    // Pr[|Y| ≥ t · b] = exp(−t) = 0.1.
    num_dummy_bin = int(b * t);
  } else {
    bins = 5; // bin number
    num_real = 10;  
    num_dummy_bin = 20; 
    std::vector<int> vect_ = vect;
    for (int i = 0; i < t; i++) { 
      vect.insert(vect.end(), vect_.begin(), vect_.end());
    }
  }
  
  string fileNameOutIndex = argv[7]; // out
  string fileNameOut = "./resultsEMP/leaf"+fileName_real+","+t_string+","+eps_string+","+N_string+";"+fileNameOutIndex+".txt";
  cout << "fileName: " << fileName_real << "  T: " << t_string << "  eps: " << eps_string << "  N: " << N_string << " out:" << fileNameOutIndex << endl;
  // prepare input data: original data contains real and dummy records
  // trigger update for each t 
  std::vector<std::vector<int> > originalData(t);  // encoded real + dummy 
  std::vector<std::vector<int> > originalDataEncodedNot(t);  // not encoded real + dummy
  std::vector<std::vector<int> > originalDummyMarkers(t);  // dummy markers for real + dummy
  int dummy_leaf = 0;
  double b = 1 / eps;
  double p = 0.01;
  double tt = log((1/p));    // Pr[|Y| ≥ t · b] = exp(−t) = 0.1.
  for (int i = 0; i < t; i++) { 
    std::vector<int> v_originalData(vect.begin() + (i*num_real), vect.begin() + ((i+1)*num_real)); // original real data
    std::vector<int> v_originalDataEncoded;  // encoded 
    std::vector<int> v_originalDummyMarkers;    

    int num_dummy = 0;      // redefine
    int a = round(2 * b * sqrt((i + 1) * tt));
    if (a > i * b){
        num_dummy_bin = round(b * tt);
    } else{
      if (round(a - dummy_leaf) >= 0){
        num_dummy_bin = round(a - dummy_leaf);
      } 
    } 
    dummy_leaf += num_dummy_bin;
    num_dummy = num_dummy_bin * bins;
    cout << "dummy_leaf: " << dummy_leaf << "  num_dummy: " << num_dummy<< endl;

    int size = num_real + num_dummy;  // real + dummy
    std::vector<int> randomVect = uniformGenVector(size);

    if (party == ALICE) {
      for (int j = 0; j < bins; j++) {    
        std::vector<int> dummyRecord(num_dummy_bin, 446);  // dummy(0s)
        v_originalData.insert(v_originalData.end(), dummyRecord.begin(), dummyRecord.end()); // v_originalData: real+dummy
      }
      std::vector<int> realMarkers(num_real, 211);  // real(0s)
      std::vector<int> dummyMarker(num_dummy, 625);  // dummy(1s)
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), realMarkers.begin(), realMarkers.end()); //v_originalDummyMarkers: 1s+0s
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), dummyMarker.begin(), dummyMarker.end()); //v_originalDummyMarkers: 1s+0s
    } else {
      for (int j = 0; j < bins; j++) {    
        std::vector<int> dummyRecord(num_dummy_bin, 446 ^ (j+1));  // dummy(1,2,3...)
        v_originalData.insert(v_originalData.end(), dummyRecord.begin(), dummyRecord.end()); // v_originalData: real+dummy
      }
      std::vector<int> realMarkers(num_real, 211);  // real(0s)
      std::vector<int> dummyMarker(num_dummy, 624);  // dummy(1s)
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), realMarkers.begin(), realMarkers.end()); //v_originalDummyMarkers: 1s+0s
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), dummyMarker.begin(), dummyMarker.end()); //v_originalDummyMarkers: 1s+0s
    }

    // only categorical; for numerical, need to specify range and bin size
    v_originalDataEncoded = encodeData(party, size, randomVect, v_originalData, v_originalDummyMarkers);

    originalData[i] = v_originalDataEncoded;
    originalDataEncodedNot[i] = v_originalData;
    originalDummyMarkers[i] = v_originalDummyMarkers;
  }
  
  // metric
  std::vector<double> metricRunTimeDP(t);
  std::vector<double> metricRunTimeDPMerge(t);
  std::vector<double> metricRunTimeDPSort(t);

  std::vector<double> metricRunTimePointQuery(t);
  std::vector<double> metricDPErrorP(t);  // |DP count - true count|
  std::vector<double> metricDPStoreErrorP(t);  // |DP count - true record|
  std::vector<double> metricTTStoreErrorP(t);  // |true count - true record|
  std::vector<double> metricRunTimeRangeQuery(t);
  std::vector<double> metricDPErrorR(t);  // |DP count - true count|
  std::vector<double> metricDPStoreErrorR(t);  // |DP count - true record|
  std::vector<double> metricTTStoreErrorR(t);  // |true count - true record|

  std::vector<double> metricss(t);
  // secure part 
  std::vector<int> mainData(t);
  std::vector<int> mainDataEncodedNot(t);
  std::vector<int> mainDummyMarker(t);
  std::vector<std::vector<int> > trueHists(t);
  std::vector<std::vector<int> > dpHists(t);
  std::map<std::string, std::vector<int> > inconsistDPHists;
  std::vector<int> leftCacheData;
  std::vector<int> leftCacheDataEncodedNot;
  std::vector<int> leftCacheDummyMarker;
 // int mainSize = 0;
  // for each update: 
  for (int i = 0; i < t; i++) {
    cout<< "index---------------------------------------------------------------------------: " << i << endl;
    auto start = high_resolution_clock::now();
    // step1: trueHistGen
    int size = originalData[i].size();   
    std::vector<int> randomVect = uniformGenVector(bins);
    std::vector<int> sh = trueHistGen(party, originalData[i], originalDummyMarkers[i], randomVect, size, bins); 
    trueHists[i] = sh;

    // step2: dpHistGen
    // option1: add noise to leaf node
    std::vector<int> lapVect;   // todo: check the correctness of lap + move it to each option
    if (constantDP) { 
      std::vector<int> lapVect_(bins, 0);
     // std::vector<int> lapVect_(bins, 1);
      lapVect = lapVect_;
    } else {
      lapVect = lapGenVector(bins, 1 / eps); 
    }
    std::vector<int> dp;
    std::vector<std::vector<int> > trueHistgrams(1);
    trueHistgrams[0] = trueHists[i];
    dp = dpHistGen(party, trueHistgrams, lapVect, bins);  
    dpHists[i] = nonNegative(dp);

    auto afterDP = high_resolution_clock::now();

    //debug
    if (debugPrint) {
        cout << "dpHists" << ' ';
        for (int j = 0; j < bins; j++) {
        cout << dpHists[i][j] << ' ';
        }
        cout <<  endl;
    }
    //debug

     
    // step3: sortCacheUsingDP
    // previously left records + new records in the cache --> sort using DP histogram
    leftCacheData.insert(leftCacheData.end(), originalData[i].begin(), originalData[i].end());
    leftCacheDataEncodedNot.insert(leftCacheDataEncodedNot.end(), originalDataEncodedNot[i].begin(), originalDataEncodedNot[i].end());
    leftCacheDummyMarker.insert(leftCacheDummyMarker.end(), originalDummyMarkers[i].begin(), originalDummyMarkers[i].end());
    int sizeCache = leftCacheData.size();
    std::vector<int> encodedRecords, dummyMarker, notEncordedRecords;
    auto ssBefore = high_resolution_clock::now();
  //  std::tie(encodedRecords, dummyMarker, notEncordedRecords) = sortDP(party, leftCacheData, leftCacheDummyMarker, leftCacheDataEncodedNot, dpHists[i], sizeCache, bins);
    std::tie(encodedRecords, dummyMarker, notEncordedRecords) = sortDPNew(party, leftCacheData, leftCacheDummyMarker, leftCacheDataEncodedNot, dpHists[i], sizeCache, bins, num_dummy_bin);
    auto ssAfter = high_resolution_clock::now();
    auto durationss = duration_cast<microseconds>(ssAfter - ssBefore);
    metricss[i] = durationss.count();
    cout << "ss" << durationss.count() << " num: "<< leftCacheData.size()<<endl;  
    // total DP count = #records we want to retrieve --> sorted cache + left cache 
    int totalRecords = accumulate(dpHists[i].begin(), dpHists[i].end(), 0);
    std::pair<std::vector<int>, std::vector<int> > seperatedRecord = copy2two(encodedRecords, totalRecords, 0);
    std::pair<std::vector<int>, std::vector<int> > seperatedDummyMarker = copy2two(dummyMarker, totalRecords, 0);
    std::pair<std::vector<int>, std::vector<int> > seperatedRecordEncodedNot = copy2two(notEncordedRecords, totalRecords, 0);

    std::vector<int> sortedRecord = seperatedRecord.first;
    leftCacheData = seperatedRecord.second;
    std::vector<int> sortedDummy = seperatedDummyMarker.first;
    leftCacheDummyMarker = seperatedDummyMarker.second;
    std::vector<int> sortedRecordEncodedNot = seperatedRecordEncodedNot.first;
    leftCacheDataEncodedNot = seperatedRecordEncodedNot.second;

    //debug
    if (debugPrint) {      
        /*mainSize += totalRecords;
      Integer *sortedRecordsortedRecord = reconstructArray(sortedRecord);
      Integer *leftRecordleftRecord = reconstructArray(leftCacheData);
      cout << "sortedRecordsortedRecord" << ' ';
      printArray(sortedRecordsortedRecord, sortedRecord.size());
      cout << "leftRecordleftRecord" << ' ';
      printArray(leftRecordleftRecord, leftCacheData.size());
        Integer *sortedDummysortedDummy = reconstructArray(sortedDummy);
      Integer *leftDummyleftDummy = reconstructArray(leftCacheDummyMarker);
      cout << "sortedDummysortedDummy" << ' ';
      printArray(sortedDummysortedDummy, sortedDummy.size());
      cout << "leftDummyleftDummy" << ' ';
      printArray(leftDummyleftDummy, leftCacheDummyMarker.size());
      Integer *sortedRecordsortedRecordEncodedNot = reconstructArray(sortedRecordEncodedNot);
      Integer *leftRecordleftRecordEncodedNot = reconstructArray(leftCacheDataEncodedNot);
      cout << "sortedRecordsortedRecordEncodedNot" << ' ';
      printArray(sortedRecordsortedRecordEncodedNot, sortedRecordEncodedNot.size());
      cout << "leftRecordleftRecordEncodedNot" << ' ';
      printArray(leftRecordleftRecordEncodedNot, leftCacheDataEncodedNot.size());
      cout << "sortedRecord.size(): " << sortedRecord.size() << endl;*/
    }
    //debug

    // step4: dpMerge
    // DP histogram for main data store
    auto DPMergeBefore = high_resolution_clock::now();
    std::vector<int> dp_main(bins, 0);
    for (int j = 0; j < i; j++) { 
      dp_main = addTwoVectors(dp_main, dpHists[j]);
    }
    // merge main and cache
    mainData = merge2SortedArr(dp_main, dpHists[i], mainData, sortedRecord, bins);
    mainDummyMarker = merge2SortedArr(dp_main, dpHists[i], mainDummyMarker, sortedDummy, bins);
    mainDataEncodedNot = merge2SortedArr(dp_main, dpHists[i], mainDataEncodedNot, sortedRecordEncodedNot, bins);
    auto DPMergeAfter = high_resolution_clock::now();
    auto durationDPMerge = duration_cast<microseconds>(DPMergeAfter - DPMergeBefore);
    metricRunTimeDPMerge[i] = durationDPMerge.count();

    //debug
    if (debugPrint) {
      Integer *mainmain = reconstructArray(mainData);
      cout << "mainmain: " << ' ';
      printArray(mainmain, mainData.size());
      Integer *mainmaindummy = reconstructArray(mainDummyMarker);
      cout << "mainmaindummy: " << ' ';
      printArray(mainmaindummy, mainDummyMarker.size());
    }
    
    // metric 1: run time
    //auto afterDPSort = high_resolution_clock::now();
    auto durationDP = duration_cast<microseconds>(afterDP - start);
    auto durationDPSort = duration_cast<microseconds>(DPMergeBefore - afterDP);  
   // cout << "RunTime: durationDP: " << durationDP.count() << ";  durationDPSort: " << durationDPSort.count() <<endl;
    metricRunTimeDP[i] = durationDP.count() / 1000000;
    metricRunTimeDPSort[i] = durationDPSort.count() / 1000000;

    // point query ======= 
    // metric 2: DP accuracy for Point query 0 -- i
    // add DP histograms that cover 0 -- i 
    auto PointQueryBefore = high_resolution_clock::now();
    std::vector<string> intervalss;
    std::vector<int> PdpI(bins, 0);
    for (int j = 0; j <= i; j++) { 
      PdpI = addTwoVectors(PdpI, dpHists[j]);
    }
    // compute the #true for each interval's data and then add them up 
    std::vector<int> PtrueR = computeTrueRecords(PdpI, mainData, mainDummyMarker); 
    auto PointQueryAfter = high_resolution_clock::now();

    // range query =======
    auto RangeQueryBefore = high_resolution_clock::now();
    std::vector<int> RdpI;
    int idx = 0;
    for (int j = 0; j < bins; j++) {   
      for (int k = j; k < bins; k++) {   
        RdpI.push_back(0);
        for (int l = j; l <= k; l++){
          RdpI[idx] = RdpI[idx] + PdpI[l];
        }
        idx++;
      }
    } 
    int rangeQuerySize = RdpI.size();
    std::vector<int> RtrueR = computeTrueRecordRange(PdpI, mainData, mainDummyMarker); 
    auto RangeQueryAfter = high_resolution_clock::now();

    auto durationPointQuery= duration_cast<microseconds>(PointQueryAfter - PointQueryBefore);
    metricRunTimePointQuery[i] = (durationPointQuery.count() / bins) / 1000;
    auto durationRangeQuery= duration_cast<microseconds>(RangeQueryAfter - RangeQueryBefore);
    metricRunTimeRangeQuery[i] = (durationRangeQuery.count() / rangeQuerySize) / 1000;

    // compute true histograms that cover 0 -- i
    std::vector<std::vector<int> > trueHistgramsT(i+1);
    for (int j = 0; j <= i; j++) { 
     trueHistgramsT[j] = trueHists[j];
    }
    std::vector<int> PtrueI = computeTrueNumber(trueHistgramsT, bins);
    std::vector<int> RtrueI;
    int idx2 = 0;
    for (int j = 0; j < bins; j++) {   
      for (int k = j; k < bins; k++) {   
        RtrueI.push_back(0);
        for (int l = j; l <= k; l++){
          RtrueI[idx2] = RtrueI[idx2] + PtrueI[l];
        }
        idx2++;
      }
    }
    
    // compute the error for DP count 
    double DPCountErrorP = 0;
    double DPCountErrorR = 0;
    for (int j = 0; j < bins; j++) { 
      DPCountErrorP += abs(PdpI[j] - PtrueI[j]);
    }
    for (int j = 0; j < rangeQuerySize; j++) { 
      DPCountErrorR += abs(RdpI[j] - RtrueI[j]);
    }
   // cout << "DPCountError: " << DPCountError << endl;
    metricDPErrorP[i] = DPCountErrorP / bins;  
    metricDPErrorR[i] = DPCountErrorR / rangeQuerySize;  

    // metric sort errors = (dp count - true records)  
    // compute the error for DP store 
    double DPStoreErrorP = 0;
    double DPStoreErrorR = 0;
    for (int j = 0; j < bins; j++) { 
      DPStoreErrorP += abs(PdpI[j] - PtrueR[j]);
    }
    for (int j = 0; j < rangeQuerySize; j++) { 
      DPStoreErrorR += abs(RdpI[j] - RtrueR[j]);
    }
   // cout << "DPStoreError: " << DPStoreError << endl;
    metricDPStoreErrorP[i] = DPStoreErrorP / bins;  
    metricDPStoreErrorR[i] = DPStoreErrorR / rangeQuerySize;  

    // metric errors = (true count - true records)  
    double TTStoreErrorP = 0;
    double TTStoreErrorR = 0;
    for (int j = 0; j < bins; j++) { 
      TTStoreErrorP += abs(PtrueI[j] - PtrueR[j]);
    }
    for (int j = 0; j < rangeQuerySize; j++) { 
      TTStoreErrorR += abs(RtrueI[j] - RtrueR[j]);
    }
    metricTTStoreErrorP[i] = TTStoreErrorP / bins;  
    metricTTStoreErrorR[i] = TTStoreErrorR / rangeQuerySize;  

    //debug
    if (debugPrint) {
      cout << "dp count P: ";
      for (int j = 0; j < bins; j++) {
        cout << PdpI[j] << ' ';
      }
      cout << endl;
      cout << "true count P: ";
      for (int j = 0; j < bins; j++) {
        cout << PtrueI[j] << ' ';
      }
      cout << endl;
      cout << "true record P: ";
      for (int j = 0; j < bins; j++) {
        cout << PtrueR[j] << ' ';
      }
      cout << endl;
      cout << "dp count R: ";
      for (int j = 0; j < rangeQuerySize; j++) {
        cout << RdpI[j] << ' ';
      }
      cout << endl;
      cout << "true count R: ";
      for (int j = 0; j < rangeQuerySize; j++) {
        cout << RtrueI[j] << ' ';
      }
      cout << endl;
      cout << "true record R: ";
      for (int j = 0; j < rangeQuerySize; j++) {
        cout << RtrueR[j] << ' ';
      }
      cout << endl;
    }
    //debug
  }

  std::ofstream outFile;
  outFile.open(fileNameOut, std::ios_base::app);//std::ios_base::app
 
  cout << "********************************************************************* " << endl;
  outFile << "********************************************************************* " << endl;
  cout << "metricRunTimeDP: ";
  outFile << "metricRunTimeDP: ";
  for (int i = 0; i < t; i++) {
    cout << metricRunTimeDP[i];
    outFile << metricRunTimeDP[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  cout << "metricRunTimeDPSort: ";
  outFile << "metricRunTimeDPSort: ";
  for (int i = 0; i < t; i++) {
    cout << metricRunTimeDPSort[i]; 
    outFile << metricRunTimeDPSort[i]; 
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  cout << "metricRunTimeDPMerge: ";
  outFile << "metricRunTimeDPMerge: ";
  for (int i = 0; i < t; i++) {
    cout << metricRunTimeDPMerge[i];
    outFile << metricRunTimeDPMerge[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;


  // query processing runtime 
  cout << "metricRunTimePointQuery: ";
  outFile << "metricRunTimePointQuery: ";
  for (int i = 0; i < t; i++) {
    cout << metricRunTimePointQuery[i];
    outFile << metricRunTimePointQuery[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  cout << "metricRunTimeRangeQuery: ";
  outFile << "metricRunTimeRangeQuery: ";
  for (int i = 0; i < t; i++) {
    cout << metricRunTimeRangeQuery[i];
    outFile << metricRunTimeRangeQuery[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  // query processing error  
  cout << "metricDPErrorP: ";
  outFile << "metricDPErrorP: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPErrorP[i];
    outFile << metricDPErrorP[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  cout << "metricDPStoreErrorP: ";
  outFile << "metricDPStoreErrorP: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPStoreErrorP[i];
    outFile << metricDPStoreErrorP[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl; 
  cout << "metricTTStoreErrorP: ";
  outFile << "metricTTStoreErrorP: ";
  for (int i = 0; i < t; i++) {
    cout << metricTTStoreErrorP[i];
    outFile << metricTTStoreErrorP[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;

  cout << "metricDPErrorR: ";
  outFile << "metricDPErrorR: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPErrorR[i];
    outFile << metricDPErrorR[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  cout << "metricDPStoreErrorR: ";
  outFile << "metricDPStoreErrorR: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPStoreErrorR[i];
    outFile << metricDPStoreErrorR[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl; 
  cout << "metricTTStoreErrorR: ";
  outFile << "metricTTStoreErrorR: ";
  for (int i = 0; i < t; i++) {
    cout << metricTTStoreErrorR[i];
    outFile << metricTTStoreErrorR[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;

  finalize_semi_honest();
  delete io;
}
