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


std::tuple<std::vector<int>, std::vector<int>, std::vector<int>, int>  processQuery(std::vector<int> resultBins, bool debugPrint, double eps, bool constantDP, int party, std::vector<int> originalData, std::vector<int> originalDummyMarkers, std::vector<int> originalDataEncodedNot) {
  // step1: compute DP count and mark records 
  int size = originalData.size(); 
  std::vector<int> lapVect;   // todo: check the correctness of lap + move it to each option
  if (constantDP) { 
    std::vector<int> lapVect_(1, 0);
    // std::vector<int> lapVect_(bins, 1);
    lapVect = lapVect_;
  } else {
    lapVect = lapGenVector(1, 1 / eps); 
  } 

  std::vector<int> randomVect = uniformGenVector(size);
  std::pair<int, std::vector<int> > results = computeDPCountMark(party, originalData, originalDummyMarkers, randomVect, resultBins, lapVect);
  int DPCount = results.first;
  std::vector<int> resultDummyMarkers = results.second;
  
  // step2: sort and retrive first DP count records 
  std::vector<int> ansOriginalData_, ansOriginalDummyMarkers_, ansOriginalDataEncodedNot_;
  tie(ansOriginalData_, ansOriginalDummyMarkers_, ansOriginalDataEncodedNot_) = sortOneBinDP(party, originalData, originalDummyMarkers, originalDataEncodedNot, resultDummyMarkers, DPCount, size);
  int count = (DPCount < size) ? DPCount - 1: size - 1;  // - 1 for slicing
  std::vector<int> ansOriginalData = slicing(ansOriginalData_, 0, count);
  std::vector<int> ansOriginalDummyMarkers = slicing(ansOriginalDummyMarkers_, 0, count);
  std::vector<int> ansOriginalDataEncodedNot = slicing(ansOriginalDataEncodedNot_, 0, count);

  //debug
  if (debugPrint) {
      cout << "dpCount " << DPCount << endl;
      Integer *ansOriginalData_ = reconstructArray(ansOriginalData);
      cout << "ansOriginalData: " << ' ';
      printArray(ansOriginalData_, ansOriginalData.size());

      Integer *ansOriginalDummyMarkers_ = reconstructArray(ansOriginalDummyMarkers);
      cout << "ansOriginalDummyMarkers: " << ' ';
      printArray(ansOriginalDummyMarkers_, ansOriginalDummyMarkers.size());
  }
  //debug
  return std::make_tuple(ansOriginalData, ansOriginalDummyMarkers, ansOriginalDataEncodedNot, DPCount);
}

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
  bool constantDP = true; 
  // print 
  bool debugPrint = true;
  // privacy budget
  string eps_string = argv[5]; // eps
  double eps = std::stod(eps_string);
  // bin number 
  int bins = 0;
  // !warning: if there are not enough dummy records, then sortDP and copy2two are incorrect
  int num_real = 0;  
  string N_string = argv[6]; // num of reals for each cache
   // nyc taxi dataset: 1271413 rows; 4 bins; payment_type
  if ((fileName_real == "taxi_ss1.txt") || (fileName_real == "taxi_ss2.txt")) {
    bins = 4; // bin number
    num_real = std::stod(N_string);
  } else {
    bins = 5; // bin number
    num_real = 10;  
    std::vector<int> vect_ = vect;
    for (int i = 0; i < t; i++) { 
      vect.insert(vect.end(), vect_.begin(), vect_.end());
    }
  }
  
  string fileNameOutIndex = argv[7]; // out
  string fileNameOut = "./results2/leaf"+fileName_real+","+t_string+","+eps_string+","+N_string+";"+fileNameOutIndex+".txt";
  cout << "fileName: " << fileName_real << "  T: " << t_string << "  eps: " << eps_string << "  N: " << N_string << " out:" << fileNameOutIndex << endl;
  // prepare input data: original data contains real and dummy records
  // trigger update for each t 
  std::vector<std::vector<int> > originalData(t);  // encoded real + dummy 
  std::vector<std::vector<int> > originalDataEncodedNot(t);  // not encoded real + dummy
  std::vector<std::vector<int> > originalDummyMarkers(t);  // dummy markers for real + dummy

  for (int i = 0; i < t; i++) { 
    std::vector<int> v_originalData(vect.begin() + (i*num_real), vect.begin() + ((i+1)*num_real)); // original real data
    std::vector<int> v_originalDataEncoded;  // encoded 
    std::vector<int> v_originalDummyMarkers;    

    int size = num_real ;  // real 
    std::vector<int> randomVect = uniformGenVector(size);

    if (party == ALICE) {
      std::vector<int> realMarkers(num_real, 211);  // real(0s)
      // std::vector<int> dummyMarker(num_dummy, 625);  // dummy(1s)
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), realMarkers.begin(), realMarkers.end()); //v_originalDummyMarkers: 1s+0s
    } else {
      std::vector<int> realMarkers(num_real, 211);  // real(0s)
      // std::vector<int> dummyMarker(num_dummy, 624);  // dummy(1s)
      v_originalDummyMarkers.insert(v_originalDummyMarkers.end(), realMarkers.begin(), realMarkers.end()); //v_originalDummyMarkers: 1s+0s
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
  std::vector<double> metricDPError(t);  // |DP count - true count|
  std::vector<double> metricDPStoreError(t);  // |DP count - true record|
  std::vector<double> metricTTStoreError(t);  // |true count - true record|
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
    auto startPointQ = high_resolution_clock::now();
    for (int j = 0; j < bins; j++) {   // point queries 
      std::vector<int> resultBins{j-1, j};   // given a query 
      std::vector<int> ansOriginalData, ansOriginalDummyMarkers, ansOriginalDataEncodedNot;
      int DPCounter;
      tie(ansOriginalData, ansOriginalDummyMarkers, ansOriginalDataEncodedNot, DPCounter) = processQuery(resultBins, debugPrint, eps, constantDP, party, originalData[i], originalDummyMarkers[i], originalDataEncodedNot[i]);
    }
    auto afterPointQ = high_resolution_clock::now();

    auto durationDPSort = duration_cast<microseconds>(afterPointQ - startPointQ);
    // cout << "RunTime: durationDP: " << durationDP.count() << ";  durationDPSort: " << durationDPSort.count() <<endl;
    metricRunTimeDP[i] = durationDPSort.count() / 1000;
    // metric and runtime 


    /*
    // metric 1: run time
    auto afterDPSort = high_resolution_clock::now();
    auto durationDP = duration_cast<microseconds>(afterDP - start);
    auto durationDPSort = duration_cast<microseconds>(afterDPSort - afterDP);
   // cout << "RunTime: durationDP: " << durationDP.count() << ";  durationDPSort: " << durationDPSort.count() <<endl;
    metricRunTimeDP[i] = durationDP.count() / 1000;
    metricRunTimeDPSort[i] = durationDPSort.count() / 1000000;

    // metric 2: DP accuracy for query 0 -- i
    // step1: add DP histograms that cover 0 -- i 
    std::vector<string> intervalss;
    std::vector<int> dpI(bins, 0);
    for (int j = 0; j <= i; j++) { 
      dpI = addTwoVectors(dpI, dpHists[j]);
    }
    // step2: compute true histograms that cover 0 -- i
    std::vector<std::vector<int> > trueHistgramsT(i+1);
    for (int j = 0; j <= i; j++) { 
     trueHistgramsT[j] = trueHists[j];
    }
    std::vector<int> trueI = computeTrueNumber(trueHistgramsT, bins);
    // step3: compute the error for DP count 
    double DPCountError = 0;
    for (int j = 0; j < bins; j++) { 
      DPCountError += abs(dpI[j] - trueI[j]);
    }
   // cout << "DPCountError: " << DPCountError << endl;
    metricDPError[i] = DPCountError;  

    // metric 3: sort errors = (dp count - true records)  
    // step1: compute the #true for each interval's data and then add them up 
    std::vector<int> trueR = computeTrueRecords(dpI, mainData); 
      
    // step2: compute the error for DP store 
    double DPStoreError = 0;
    for (int j = 0; j < bins; j++) { 
      DPStoreError += abs(dpI[j] - trueR[j]);
    }
   // cout << "DPStoreError: " << DPStoreError << endl;
    metricDPStoreError[i] = DPStoreError;

    // metric errors = (true count - true records)  
    double TTStoreError = 0;
    for (int j = 0; j < bins; j++) { 
      TTStoreError += abs(trueI[j] - trueR[j]);
    }
    metricTTStoreError[i] = TTStoreError;

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
    */
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
  cout << "metricTTStoreError: ";
  outFile << "metricTTStoreError: ";
  for (int i = 0; i < t; i++) {
    cout << metricTTStoreError[i];
    outFile << metricTTStoreError[i];
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
