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


std::tuple<std::vector<int>, std::vector<int>, std::vector<int>, int, int>  processQuery(std::vector<int> resultBins, bool debugPrint, double eps, bool constantDP, int party, std::vector<int> originalData, std::vector<int> originalDummyMarkers, std::vector<int> originalDataEncodedNot) {
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
  int DPCount, TrueCount;
  std::vector<int> resultDummyMarkers;
  // note that the returned trueCounter is only for test purpose
  std::tie(DPCount, resultDummyMarkers, TrueCount) = computeDPCountMark(party, originalData, originalDummyMarkers, randomVect, resultBins, lapVect);
  // step2: sort and retrive first DP count records 
  std::vector<int> ansOriginalData_, ansOriginalDummyMarkers_, ansOriginalDataEncodedNot_;
  std::tie(ansOriginalData_, ansOriginalDummyMarkers_, ansOriginalDataEncodedNot_) = sortOneBinDP(party, originalData, originalDummyMarkers, originalDataEncodedNot, resultDummyMarkers, DPCount, size);
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
  return std::make_tuple(ansOriginalData, ansOriginalDummyMarkers, ansOriginalDataEncodedNot, DPCount, TrueCount);
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
  std::vector<double> metricRunTimePoint(t);
  std::vector<double> metricDPErrorPoint(t);  // |DP count - true count|
  std::vector<double> metricDPStoreErrorPoint(t);  // |DP count - true record|
  std::vector<double> metricTTStoreErrorPoint(t);  // |true count - true record|
  std::vector<double> metricRunTimeRange(t);
  std::vector<double> metricDPErrorRange(t);  // |DP count - true count|
  std::vector<double> metricDPStoreErrorRange(t);  // |DP count - true record|
  std::vector<double> metricTTStoreErrorRange(t);  // |true count - true record|
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
    // point queries -------------------------------------------------------------------------------------------
    auto startPointQ = high_resolution_clock::now();
    // step1: get DP count and DP records
    std::vector<int> DPCountPoint(bins, 0);
    std::vector<int> TrueCountPoint(bins, 0);
    std::vector<int> TrueRecordPoint(bins, 0);
  
    for (int j = 0; j < bins; j++) {   
      std::vector<int> resultBins{j, j};   // given a query 
      std::vector<int> ansOriginalData, ansOriginalDummyMarkers, ansOriginalDataEncodedNot;
      int DPCounter, TrueCounter;
      // note that the returned trueCounter is only for test purpose
      std::tie(ansOriginalData, ansOriginalDummyMarkers, ansOriginalDataEncodedNot, DPCounter, TrueCounter) = processQuery(resultBins, debugPrint, eps, constantDP, party, originalData[i], originalDummyMarkers[i], originalDataEncodedNot[i]);
      DPCountPoint[j] = DPCounter;
      TrueCountPoint[j] = TrueCounter;
      // process returned records to filter out dummy -- this is only for trusted clients 
      std::vector<int> resultTrueRecords = returnTrueRecords(party, ansOriginalData, ansOriginalDummyMarkers, TrueCounter);
      TrueRecordPoint[j] = resultTrueRecords.size();
    }
    auto afterPointQ = high_resolution_clock::now();
    // metric 1: query process time for all point queries 
    auto durationPointQ = duration_cast<microseconds>(afterPointQ - startPointQ);
    // cout << "RunTime: durationDP: " << durationDP.count() << ";  durationDPSort: " << durationDPSort.count() <<endl;
    // metric 2: accuracy time for all point queries 
    double DPStoreErrorPoint = 0;
    double DPErrorPoint = 0;
    double TTStoreErrorPoint = 0;
    for (int j = 0; j < bins; j++) { 
      DPStoreErrorPoint += abs(DPCountPoint[j] - TrueRecordPoint[j]);
      DPErrorPoint += abs(DPCountPoint[j] - TrueCountPoint[j]);
      TTStoreErrorPoint += abs(TrueCountPoint[j] - TrueRecordPoint[j]);
    }
    // for all time point
    metricRunTimePoint[i] = durationPointQ.count() / 1000;
    metricDPErrorPoint[i] = DPErrorPoint;
    metricDPStoreErrorPoint[i] = DPStoreErrorPoint;
    metricTTStoreErrorPoint[i] = TTStoreErrorPoint;
    
    // range queries -------------------------------------------------------------------------------------------
    auto startRangeQ = high_resolution_clock::now();
    // step1: get DP count and DP records
    std::vector<std::vector<int> > rangeQuery;
    for (int j = 0; j < bins; j++) {   
      for (int k = j; k < bins; k++) {   
        vector<int> vect{j, k};
        rangeQuery.push_back(vect);
      }
    }
    int querySize = rangeQuery.size();
    std::vector<int> DPCountRange(querySize, 0);
    std::vector<int> TrueCountRange(querySize, 0);
    std::vector<int> TrueRecordRange(querySize, 0);
   
    for (int j = 0; j < querySize; j++) {   
      std::vector<int> resultBins = rangeQuery[j];   // given a query 
      std::vector<int> ansOriginalData, ansOriginalDummyMarkers, ansOriginalDataEncodedNot;
      int DPCounter, TrueCounter;
      // note that the returned trueCounter is only for test purpose
      std::tie(ansOriginalData, ansOriginalDummyMarkers, ansOriginalDataEncodedNot, DPCounter, TrueCounter) = processQuery(resultBins, debugPrint, eps, constantDP, party, originalData[i], originalDummyMarkers[i], originalDataEncodedNot[i]);
      DPCountRange[j] = DPCounter;
      TrueCountRange[j] = TrueCounter;
      // process returned records to filter out dummy -- this is only for trusted clients 
      std::vector<int> resultTrueRecords = returnTrueRecords(party, ansOriginalData, ansOriginalDummyMarkers, TrueCounter);
      TrueRecordRange[j] = resultTrueRecords.size();
     }
    auto afterRangeQ = high_resolution_clock::now();
    // metric 1: query process time for all point queries 
    auto durationRangeQ  = duration_cast<microseconds>(afterRangeQ  - startRangeQ );
    // cout << "RunTime: durationDP: " << durationDP.count() << ";  durationDPSort: " << durationDPSort.count() <<endl;
    // metric 2: accuracy time for all point queries 
    double DPStoreErrorRange = 0;
    double DPErrorRange = 0;
    double TTStoreErrorRange = 0;
    for (int j = 0; j < querySize; j++) { 
      DPStoreErrorRange += abs(DPCountRange[j] - TrueRecordRange[j]);
      DPErrorRange += abs(DPCountRange[j] - TrueCountRange[j]);
      TTStoreErrorRange += abs(TrueCountRange[j] - TrueRecordRange[j]);
    }
    // for all time point
    metricRunTimeRange[i] = durationRangeQ.count() / 1000;
    metricDPErrorRange[i] = DPErrorRange;
    metricDPStoreErrorRange[i] = DPStoreErrorRange;
    metricTTStoreErrorRange[i] = TTStoreErrorRange;
  }
  

  std::ofstream outFile;
  outFile.open(fileNameOut, std::ios_base::app);//std::ios_base::app
 
  cout << "********************************************************************* " << endl;
  outFile << "********************************************************************* " << endl;
  cout << "metricRunTimePoint: ";
  outFile << "metricRunTimePoint: ";
  for (int i = 0; i < t; i++) {
    cout << metricRunTimePoint[i];
    outFile << metricRunTimePoint[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  
  cout << "metricDPErrorPoint: ";
  outFile << "metricDPErrorPoint: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPErrorPoint[i];
    outFile << metricDPErrorPoint[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  cout << "metricDPStoreErrorPoint: ";
  outFile << "metricDPStoreErrorPoint: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPStoreErrorPoint[i];
    outFile << metricDPStoreErrorPoint[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl; 
  cout << "metricTTStoreErrorPoint: ";
  outFile << "metricTTStoreErrorPoint: ";
  for (int i = 0; i < t; i++) {
    cout << metricTTStoreErrorPoint[i];
    outFile << metricTTStoreErrorPoint[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;


  cout << "********************************************************************* " << endl;
  outFile << "********************************************************************* " << endl;
  cout << "metricRunTimeRange: ";
  outFile << "metricRunTimeRange: ";
  for (int i = 0; i < t; i++) {
    cout << metricRunTimeRange[i];
    outFile << metricRunTimeRange[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  
  cout << "metricDPErrorRange: ";
  outFile << "metricDPErrorRange: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPErrorRange[i];
    outFile << metricDPErrorRange[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl;
  cout << "metricDPStoreErrorRange: ";
  outFile << "metricDPStoreErrorRange: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPStoreErrorRange[i];
    outFile << metricDPStoreErrorRange[i];
    if (i != t-1){
     cout << ", ";
     outFile << ", ";
    }
  } 
  cout << endl;
  outFile << endl; 
  cout << "metricTTStoreErrorRange: ";
  outFile << "metricTTStoreErrorRange: ";
  for (int i = 0; i < t; i++) {
    cout << metricTTStoreErrorRange[i];
    outFile << metricTTStoreErrorRange[i];
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
