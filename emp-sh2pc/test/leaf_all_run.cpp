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

// todo: remove push_back
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
  bool debugPrint = true;
  // privacy budget
  string eps_string = argv[5]; // eps
  double eps = std::stod(eps_string);
  // bin number 
  int bins = 0;
  // !warning: if there are not enough dummy records, then sortDP and copy2two are incorrect
  int num_real = 0;  
  int num_dummy = 0; // make sure there are enough dummy records
  string N_string = argv[6]; // num of reals for each cache
   // nyc taxi dataset: 1271413 rows; 4 bins; payment_type
  if ((fileName_real == "taxi_ss1.txt") || (fileName_real == "taxi_ss2.txt")) {
    bins = 4; // bin number
    num_real = std::stod(N_string);
    //num_dummy = 10;
    double b = 1 / eps;
    double t = log((1/0.1));    // Pr[|Y| ≥ t · b] = exp(−t) = 0.1.
    num_dummy = bins * int(b * t);
  } else {
    bins = 5; // bin number
    num_real = 10;  
    num_dummy = 100; 
    std::vector<int> vect_ = vect;
    for (int i = 0; i < t; i++) { 
      vect.insert(vect.end(), vect_.begin(), vect_.end());
    }
  }
  
  string fileNameOutIndex = argv[7]; // out
  string fileNameOut = "./results/leaf"+fileName_real+","+t_string+","+eps_string+","+N_string+";"+fileNameOutIndex+".txt";
  cout << "fileName: " << fileName_real << "  T: " << t_string << "  eps: " << eps_string << "  N: " << N_string << " out:" << fileNameOutIndex << endl;

  // prepare input data: original data contains real and dummy records
  // trigger update for each t 
  std::vector<std::vector<int> > originalData;  // encoded real + dummy 
  std::vector<std::vector<int> > originalDataEncodedNot;  // not encoded real + dummy
  std::vector<std::vector<int> > originalDummyMarkers;  // dummy markers for real + dummy
  for (int i = 0; i < t; i++) { 
    std::vector<int> v_originalData(vect.begin() + (i*num_real), vect.begin() + ((i+1)*num_real)); // original real data
    std::vector<int> v_originalDataEncoded;  // encoded 
    std::vector<int> v_originalDummyMarkers;
    int size = num_real + num_dummy;  // real + dummy
    std::vector<int> randomVect = uniformGenVector(size);

    if (party == ALICE) {
      std::vector<int> dummyRecord(num_dummy, 446);  // dummy(0s)
      v_originalData.insert(v_originalData.end(), dummyRecord.begin(), dummyRecord.end()); // v_originalData: real+dummy

      std::vector<int> realMarkers(num_real, 625);  // real(1s)
      std::vector<int> dummyMarker(num_dummy, 211);  // dummy(0s)
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), realMarkers.begin(), realMarkers.end()); //v_originalDummyMarkers: 1s+0s
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), dummyMarker.begin(), dummyMarker.end()); //v_originalDummyMarkers: 1s+0s
    } else {
      std::vector<int> dummyRecord(num_dummy, 446);  // dummy(0s)
      v_originalData.insert(v_originalData.end(), dummyRecord.begin(), dummyRecord.end()); // v_originalData: real+dummy

      std::vector<int> realMarkers(num_real, 624);  // real(1s)
      std::vector<int> dummyMarker(num_dummy, 211);  // dummy(0s)
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), realMarkers.begin(), realMarkers.end()); //v_originalDummyMarkers: 1s+0s
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), dummyMarker.begin(), dummyMarker.end()); //v_originalDummyMarkers: 1s+0s
    }

    // only categorical; for numerical, need to specify range and bin size
    v_originalDataEncoded = encodeData(party, size, randomVect, v_originalData, v_originalDummyMarkers);

    originalData.push_back(v_originalDataEncoded);
    originalDataEncodedNot.push_back(v_originalData);
    originalDummyMarkers.push_back(v_originalDummyMarkers);
  }
  
  // metric
  std::vector<double> metricRunTimeDP;
  std::vector<double> metricRunTimeDPSort;
  std::vector<double> metricDPError;
  std::vector<double> metricDPStoreError;

  // secure part 
  std::vector<int> mainData;
  std::vector<int> mainDataEncodedNot;
  std::vector<int> mainDummyMarker;
  std::vector<std::vector<int> > trueHists;
  std::vector<std::vector<int> > dpHists;
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
    trueHists.push_back(sh);

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
    std::vector<std::vector<int> > trueHistgrams;
    trueHistgrams.push_back(trueHists[i]);
    dp = dpHistGen(party, trueHistgrams, lapVect, bins);  
    dpHists.push_back(nonNegative(dp));

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

    if (true) {
      // step3: sortCacheUsingDP
      // previously left records + new records in the cache --> sort using DP histogram
      leftCacheData.insert(leftCacheData.end(), originalData[i].begin(), originalData[i].end());
      leftCacheDataEncodedNot.insert(leftCacheDataEncodedNot.end(), originalDataEncodedNot[i].begin(), originalDataEncodedNot[i].end());
      leftCacheDummyMarker.insert(leftCacheDummyMarker.end(), originalDummyMarkers[i].begin(), originalDummyMarkers[i].end());
      int sizeCache = leftCacheData.size();
      std::vector<int> encodedRecords, dummyMarker, notEncordedRecords;
      std::tie(encodedRecords, dummyMarker, notEncordedRecords) = sortDP(party, leftCacheData, leftCacheDummyMarker, leftCacheDataEncodedNot, dpHists[i], sizeCache, bins);

      // total DP count = #records we want to retrieve --> sorted cache + left cache 
      int totalRecords = accumulate(dpHists[i].begin(), dpHists[i].end(), 0);
      std::pair<std::vector<int>, std::vector<int> > seperatedRecord = copy2two(encodedRecords, totalRecords);
      std::pair<std::vector<int>, std::vector<int> > seperatedDummyMarker = copy2two(dummyMarker, totalRecords);
      std::pair<std::vector<int>, std::vector<int> > seperatedRecordEncodedNot = copy2two(notEncordedRecords, totalRecords);

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
      std::vector<int> dp_main(bins, 0);
      for (int j = 0; j < i; j++) { 
        dp_main = addTwoVectors(dp_main, dpHists[j]);
      }
      // merge main and cache
      mainData = merge2SortedArr(dp_main, dpHists[i], mainData, sortedRecord, bins);
      mainDummyMarker = merge2SortedArr(dp_main, dpHists[i], mainDummyMarker, sortedDummy, bins);
      mainDataEncodedNot = merge2SortedArr(dp_main, dpHists[i], mainDataEncodedNot, sortedRecordEncodedNot, bins);
      //debug
      if (debugPrint) {
        Integer *mainmain = reconstructArray(mainData);
        cout << "mainmain: " << ' ';
        printArray(mainmain, mainData.size());
      }
    }

    // metric 1: run time
    auto afterDPSort = high_resolution_clock::now();
    auto durationDP = duration_cast<microseconds>(afterDP - start);
    auto durationDPSort = duration_cast<microseconds>(afterDPSort - afterDP);
   // cout << "RunTime: durationDP: " << durationDP.count() << ";  durationDPSort: " << durationDPSort.count() <<endl;
    metricRunTimeDP.push_back(durationDP.count() / 1000);
    metricRunTimeDPSort.push_back(durationDPSort.count() / 1000000);

    // metric 2: DP accuracy for query 0 -- i
    // step1: add DP histograms that cover 0 -- i 
    std::vector<string> intervalss;
    std::vector<int> dpI(bins, 0);
    for (int j = 0; j <= i; j++) { 
      dpI = addTwoVectors(dpI, dpHists[j]);
    }
    // step2: compute true histograms that cover 0 -- i
    std::vector<std::vector<int> > trueHistgramsT;
    for (int j = 0; j <= i; j++) { 
     trueHistgramsT.push_back(trueHists[j]);
    }
    std::vector<int> trueI = computeTrueNumber(trueHistgramsT, bins);
    // step3: compute the error for DP count 
    double DPCountError = 0;
    for (int j = 0; j < bins; j++) { 
      DPCountError += abs(dpI[j] - trueI[j]);
    }
   // cout << "DPCountError: " << DPCountError << endl;
    metricDPError.push_back(DPCountError);  

    // metric 3: sort errors = (dp count - true records)  
    // step1: compute the #true for each interval's data and then add them up 
    std::vector<int> trueR = computeTrueRecords(dpI, mainData); 
      
    // step2: compute the error for DP store 
    double DPStoreError = 0;
    for (int j = 0; j < bins; j++) { 
      DPStoreError += abs(dpI[j] - trueR[j]);
    }
   // cout << "DPStoreError: " << DPStoreError << endl;
    metricDPStoreError.push_back(DPStoreError);

    //debug
    if (debugPrint) {
      cout << "dp count: ";
      for (int j = 0; j < bins; j++) {
        cout << dpI[j] << ' ';
      }
      cout << endl;
      cout << "true count: ";
      for (int j = 0; j < bins; j++) {
        cout << trueI[j] << ' ';
      }
      cout << endl;
      cout << "true record: ";
      for (int j = 0; j < bins; j++) {
        cout << trueR[j] << ' ';
      }
      cout << endl;
    }
    //debug
  }

  std::ofstream outFile;
  outFile.open(fileNameOut, std::ios_base::app);//std::ios_base::app
 
  cout << "********************************************************************* " << endl;
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
  cout << "metricDPError: ";
  outFile << "metricDPError: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPError[i];
    outFile << metricDPError[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  cout << "metricDPStoreError: ";
  outFile << "metricDPStoreError: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPStoreError[i];
    outFile << metricDPStoreError[i];
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
