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
  bool constantDP = true; 
  // print 
  bool debugPrint = true;
  // privacy budget
  double epsALL = 10;
  double levels = log2(t);  // double or int?
  double eps = epsALL / levels;
  // bin number 
  int bins = 0;
  // !warning: if there are not enough dummy records, then sortDP and copy2two are incorrect
  int num_real = 0;  
  int num_dummy = 0; // make sure there are enough dummy records
   // nyc taxi dataset: 1271413 rows; 4 bins; payment_type
  if ((fileName_real == "taxi_ss1.txt") || (fileName_real == "taxi_ss2.txt")) {
    bins = 4; // bin number
    num_real = 1000; 
    //num_dummy = 10;
    double b = 1 / eps;
    double t = log((1/0.1));    // Pr[|Y| ≥ t · b] = exp(−t) = 0.1.
    num_dummy = bins * int(b * t);
  } else {
    bins = 5; // bin number
    num_real = 10;  
    num_dummy = 10; 
    std::vector<int> vect_ = vect;
    for (int i = 0; i < t; i++) { 
      vect.insert(vect.end(), vect_.begin(), vect_.end());
    }
  }

  // how to sort
  // 0: sort subroot together; 1: sort each bin seperatly; 2: sort each bin and only the last d records 
  int sortOption = 2; 
  // option2: d --> depends on epsilon???
  double b = 1 / eps;
  double t_ = log((1/0.1));    // Pr[|Y| ≥ t · b] = exp(−t) = 0.1.
  int d = int(b * t_);
  int gapAgainThreshold = 1;

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
  std::map<std::string, std::vector<int> > mainData;
  std::map<std::string, std::vector<int> > mainDataEncodedNot;
  std::map<std::string, std::vector<int> > mainDummyMarker;
  std::vector<std::vector<int> > trueHists;
  std::map<std::string, std::vector<int> > dpHists;
  std::map<std::string, std::vector<int> > inconsistDPHists;
  std::vector<int> leftCacheData;
  std::vector<int> leftCacheDataEncodedNot;
  std::vector<int> leftCacheDummyMarker;
  // for each update: 
  for (int i = 0; i < t; i++) {
    cout<< "index---------------------------------------------------------------------------: " << i << endl;
    auto start = high_resolution_clock::now();
    // step1: trueHistGen for the path
    int size = originalData[i].size();   
    std::vector<int> randomVect = uniformGenVector(bins);
    std::vector<int> sh = trueHistGen(party, originalData[i], originalDummyMarkers[i], randomVect, size, bins); 
    trueHists.push_back(sh);

    // DP noise 
    std::vector<int> lapVect;   // todo: check the correctness of lap + move it to each option
    if (constantDP) { 
      std::vector<int> lapVect_(bins, 0);
     // std::vector<int> lapVect_(bins, 1);
      lapVect = lapVect_;
    } else {
      lapVect = lapGenVector(bins, 1 / eps); 
    }
    // step2: dpHistGen for the root of the subtree
    std::vector<int> dp;
    if (true) { // b = 2  todo: check nonNegative
      // step2.1 and step2.2: generate DP hists of nodes on the current path (current leaf to root)
      int rootLeft = nodesSubtree(i);
      int gap = i - rootLeft + 1;
   //   cout << "rootLeft: " << rootLeft << " gap: " <<  gap << " i: " << i << endl;
      while ((gap / 2 >= 1) or (gap >= 1)) {  
        // step2.1: true hists of nodes on the path
        std::vector<std::vector<int> > trueHistgrams;
        for (int j = rootLeft; j <= i; j++) { 
          trueHistgrams.push_back(trueHists[j]);
        }
        // step2.2: DP hists of nodes on the path
        std::vector<int> dpNode = dpHistGen(party, trueHistgrams, lapVect, bins);
        string intervalDP = std::to_string(rootLeft) + ',' + std::to_string(i);
        inconsistDPHists[intervalDP] = nonNegative(dpNode);

        rootLeft += (gap / 2);
        gap /= 2;
      }
    // step3: compute the consistent DP histogram of the root of the subtree
    // step3.1: DP hists of all nodes in the subtree
      std::vector<std::vector<int> > dpAllNodes; // all nodes on the subtree
      int rootLeftAgain= nodesSubtree(i);
      int gapAgain = i - rootLeftAgain + 1;
      int height = int(log2(gapAgain)) + 1;
    //  cout << "log2(gap_: " << log2(gapAgain) << " i:" << i << endl;
     // cout << "rootLeft_: " << rootLeftAgain << " gap_: " <<  gapAgain << " height: " << height << endl;
      for (int j = 0; j < height; j++) { 
        int interval = std::pow(2, j);
     //   cout << "interval: " << interval <<  endl;
        for (int k = rootLeftAgain; k < (i+1); (k += interval)) { 
          string intervalDP = std::to_string(k) + ',' + std::to_string(k + interval - 1);
        //  cout << "intervalDP: " << intervalDP <<  endl;
         /* for (int j = 0; j < bins; j++) {
            cout << inconsistDPHists[intervalDP][j] << ' ';
          }
          cout << endl;*/
          dpAllNodes.push_back(inconsistDPHists[intervalDP]);
        }
      }
      // step3.2: make these nodes on this tree consistent and return the DP hist for the root
      dp = enforceConsistencyH(dpAllNodes, gapAgain);
      string intervalDP = std::to_string(rootLeftAgain) + ',' + std::to_string(i);
      dpHists[intervalDP] = nonNegative(dp);
      //debug
      if (debugPrint) {
        cout << "Consistent root: ";
        for (int j = 0; j < bins; j++) {
        cout << dp[j] << ' ';
        }
        cout << endl;
      }
      //debug
    }
    auto afterDP = high_resolution_clock::now();
    // step4: get the sorted array of the root node 
    // step4.1: retrieve the DP histogram of the root node 
    int rootLeft = nodesSubtree(i);
    string intervalRootDP = std::to_string(rootLeft) + ',' + std::to_string(i);
    std::vector<int> dpRoot = dpHists[intervalRootDP];
    int gapAgain = i - rootLeft + 1;
    // step4.2: compute the interval of previous nodes
    std::vector<string> intervalPrevious;
    int gap = i - rootLeft + 1;
    while ((gap / 2 >= 1) or (gap >= 1)) {
      if (rootLeft < int(rootLeft + (gap / 2))){
        string interval = std::to_string(rootLeft) + ',' + std::to_string(int(rootLeft + (gap / 2)) - 1);
        intervalPrevious.push_back(interval);
      }
      rootLeft += (gap / 2);
      gap /= 2;
    }
  
    // step4.3: get the sorted array of the root node
    // option0: if sortOption == 0 or gapAgain <= x
    // option1: else if sortOption == 1 
    // option2: else sortOption == 2 
    // gapAgain <= 6 -->  to be a parameter 
    if (sortOption == 0 or gapAgain <= gapAgainThreshold) {
      // option 1: the subtree is small, so resort the root 
    //  cout << "intervalPrevious------------------------------------: " << intervalPrevious.size() << endl;
      // previously left records in cache + previous nodes --> sort using DP histogram of root node
      // put these data together 
      std::vector<int> dataToSort = leftCacheData;
      std::vector<int> dataEncodedNotToSort = leftCacheDataEncodedNot;
      std::vector<int> dummyMarkerToSort = leftCacheDummyMarker;
      for (string& interval: intervalPrevious){
        dataToSort.insert(dataToSort.end(), mainData[interval].begin(), mainData[interval].end());
        dataEncodedNotToSort.insert(dataEncodedNotToSort.end(), mainDataEncodedNot[interval].begin(), mainDataEncodedNot[interval].end());
        dummyMarkerToSort.insert(dummyMarkerToSort.end(), mainDummyMarker[interval].begin(), mainDummyMarker[interval].end());
      }
      dataToSort.insert(dataToSort.end(), originalData[i].begin(), originalData[i].end());
      dataEncodedNotToSort.insert(dataEncodedNotToSort.end(), originalDataEncodedNot[i].begin(), originalDataEncodedNot[i].end());
      dummyMarkerToSort.insert(dummyMarkerToSort.end(), originalDummyMarkers[i].begin(), originalDummyMarkers[i].end());

      int sizeSort = dataToSort.size();
      //debug
      if (debugPrint) {
        cout << "ssizeSort " << sizeSort << endl;
        cout << "Consistent root sort: ";
        for (int j = 0; j < bins; j++) {
          cout << dpRoot[j] << ' ';
        }
        cout << endl;
      }
      //debug

      // sort according to the DP hist of root 
      std::vector<int> encodedRecords, dummyMarker, notEncordedRecords;
      std::tie(encodedRecords, dummyMarker, notEncordedRecords) = sortDP(party, dataToSort, dummyMarkerToSort, dataEncodedNotToSort, dpRoot, sizeSort, bins);
      // total DP count = #records we want to retrieve --> sorted root + left cache 
      int totalRecords = accumulate(dpRoot.begin(), dpRoot.end(), 0);
      std::pair<std::vector<int>, std::vector<int> > seperatedRecord = copy2two(encodedRecords, totalRecords);
      std::pair<std::vector<int>, std::vector<int> > seperatedDummyMarker = copy2two(dummyMarker, totalRecords);
      std::pair<std::vector<int>, std::vector<int> > seperatedRecordEncodedNot = copy2two(notEncordedRecords, totalRecords);
      // sorted root + left cache
      mainData[intervalRootDP] = seperatedRecord.first;
      leftCacheData = seperatedRecord.second;
      mainDummyMarker[intervalRootDP] = seperatedDummyMarker.first;
      leftCacheDummyMarker = seperatedDummyMarker.second;
      mainDataEncodedNot[intervalRootDP] = seperatedRecordEncodedNot.first;
      leftCacheDataEncodedNot = seperatedRecordEncodedNot.second;

      // debug
      if (debugPrint) {
        Integer *sortedRecordsortedRecord = reconstructArray(mainData[intervalRootDP]);
        Integer *leftRecordleftRecord = reconstructArray(leftCacheData);
        cout << "sortedRecords for " << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecord, mainData[intervalRootDP].size());
        cout << "leftRecord for " << intervalRootDP << ": ";
        printArray(leftRecordleftRecord, leftCacheData.size());

       /* Integer *sortedDummysortedDummy = reconstructArray(mainDummyMarker[intervalRootDP]);
        Integer *leftDummyleftDummy = reconstructArray(leftCacheDummyMarker);
        cout << "sortedDummy for " << intervalRootDP << ": ";
        printArray(sortedDummysortedDummy, mainDummyMarker[intervalRootDP].size());
        cout << "leftDummy for " << intervalRootDP << ": ";
        printArray(leftDummyleftDummy, leftCacheDummyMarker.size());

        Integer *sortedRecordsortedRecordEncodedNot = reconstructArray(mainDataEncodedNot[intervalRootDP]);
        Integer *leftRecordleftRecordEncodedNot = reconstructArray(leftCacheDataEncodedNot);
        cout << "sortedRecordEncodedNot for " << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecordEncodedNot, mainDataEncodedNot[intervalRootDP].size());
        cout << "leftRecordEncodedNot for " << intervalRootDP << ": ";
        printArray(leftRecordleftRecordEncodedNot, leftCacheDataEncodedNot.size());
        cout << "sortedRecord.size(): " << mainData[intervalRootDP].size() << endl;*/
      }
      // debug
    } else if (sortOption == 1) {
    // option1: 
      // #leaf nodes in this subtree should be at least one
      // cache <-- left cache + current leaf node
      std::vector<int> dataCache = leftCacheData;
      std::vector<int> dataEncodedNotCache = leftCacheDataEncodedNot;
      std::vector<int> dummyMarkerCache = leftCacheDummyMarker;
      dataCache.insert(dataCache.end(), originalData[i].begin(), originalData[i].end());
      dataEncodedNotCache.insert(dataEncodedNotCache.end(), originalDataEncodedNot[i].begin(), originalDataEncodedNot[i].end());
      dummyMarkerCache.insert(dummyMarkerCache.end(), originalDummyMarkers[i].begin(), originalDummyMarkers[i].end());
      // retrieve the data and DP histograms of previous nodes in this subtree
      std::vector<int> dataMergedPrevious = mainData[intervalPrevious[0]];
      std::vector<int> dataEncodedNotMergedPrevious = mainDataEncodedNot[intervalPrevious[0]];
      std::vector<int> dummyMarkerMergedPrevious = mainDummyMarker[intervalPrevious[0]];
      std::vector<int> dpMergedPrevious = dpHists[intervalPrevious[0]];
      // merge these previous nodes using their DP histograms
      for (int j = 1; j < int(intervalPrevious.size()) ; j++){
        dataMergedPrevious = merge2SortedArr(dpMergedPrevious, dpHists[intervalPrevious[j]], dataMergedPrevious, mainData[intervalPrevious[j]], bins);
        dataEncodedNotMergedPrevious = merge2SortedArr(dpMergedPrevious, dpHists[intervalPrevious[j]], dataEncodedNotMergedPrevious, mainDataEncodedNot[intervalPrevious[j]], bins);
        dummyMarkerMergedPrevious = merge2SortedArr(dpMergedPrevious, dpHists[intervalPrevious[j]], dummyMarkerMergedPrevious, mainDummyMarker[intervalPrevious[j]], bins);
        dpMergedPrevious = addTwoVectors(dpMergedPrevious, dpHists[intervalPrevious[j]]);
      }
      // debug
     /* if (debugPrint) {
        Integer *sortedRecordsortedRecord = reconstructArray(dataMergedPrevious);
        cout << "previous sortedRecords for" << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecord, dataMergedPrevious.size());
      
        Integer *sortedDummysortedDummy = reconstructArray(dummyMarkerMergedPrevious);
        cout << "previous sortedDummy for " << intervalRootDP << ": ";
        printArray(sortedDummysortedDummy, dummyMarkerMergedPrevious.size());

        Integer *sortedRecordsortedRecordEncodedNot = reconstructArray(dataEncodedNotMergedPrevious);
        cout << "previous sortedRecordEncodedNot for " << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecordEncodedNot, dataEncodedNotMergedPrevious.size());
        cout << "previous sortedRecord.size(): " << dataMergedPrevious.size() << endl;
      }*/
      // debug 
      std::vector<int> dpMergedPreviousPrefix_tmp = computePrefix(dpMergedPrevious);
      std::vector<int> dpMergedPreviousPrefix(bins+1, 0); 
      for (int j = 0; j < bins; j++) {
        dpMergedPreviousPrefix[j+1] = dpMergedPreviousPrefix_tmp[j];
      }
       // for each bin, put the records for this bin of previous node and cache together, and sort
      for (int j = 0; j < bins; j++) {
        std::vector<int> toSortMergedPrevious = slicing(dataMergedPrevious, dpMergedPreviousPrefix[j], dpMergedPreviousPrefix[j+1] - 1);
        std::vector<int> toSortEncodedNotMergedPrevious = slicing(dataEncodedNotMergedPrevious, dpMergedPreviousPrefix[j], dpMergedPreviousPrefix[j+1] - 1);
        std::vector<int> toSortMarkerMergedPrevious = slicing(dummyMarkerMergedPrevious, dpMergedPreviousPrefix[j], dpMergedPreviousPrefix[j+1] - 1);
        toSortMergedPrevious.insert(toSortMergedPrevious.end(), dataCache.begin(), dataCache.end());
        toSortEncodedNotMergedPrevious.insert(toSortEncodedNotMergedPrevious.end(), dataEncodedNotCache.begin(), dataEncodedNotCache.end());
        toSortMarkerMergedPrevious.insert(toSortMarkerMergedPrevious.end(), dummyMarkerCache.begin(), dummyMarkerCache.end());
        // sort previous node for each bin and cache--> sorted for this bin + leftCache
        std::vector<int> encodedRecords, dummyMarker, notEncordedRecords;
        int sizeSort = toSortMergedPrevious.size();
        std::tie(encodedRecords, dummyMarker, notEncordedRecords) = sortBinDP(party, toSortMergedPrevious, toSortMarkerMergedPrevious, toSortEncodedNotMergedPrevious, dpRoot[j], sizeSort, j);
        std::pair<std::vector<int>, std::vector<int> > seperatedRecord = copy2two(encodedRecords, dpRoot[j]);
        std::pair<std::vector<int>, std::vector<int> > seperatedDummyMarker = copy2two(dummyMarker, dpRoot[j]);
        std::pair<std::vector<int>, std::vector<int> > seperatedRecordEncodedNot = copy2two(notEncordedRecords, dpRoot[j]);
        // sorted root for this bin + left cache
        mainData[intervalRootDP].insert(mainData[intervalRootDP].end(), seperatedRecord.first.begin(), seperatedRecord.first.end());
        dataCache = seperatedRecord.second;
        mainDummyMarker[intervalRootDP].insert(mainDummyMarker[intervalRootDP].end(), seperatedDummyMarker.first.begin(), seperatedDummyMarker.first.end());
        dummyMarkerCache = seperatedDummyMarker.second;
        mainDataEncodedNot[intervalRootDP].insert(mainDataEncodedNot[intervalRootDP].end(), seperatedRecordEncodedNot.first.begin(), seperatedRecordEncodedNot.first.end());
        dataEncodedNotCache = seperatedRecordEncodedNot.second;
      }

      leftCacheData = dataCache;
      leftCacheDummyMarker = dummyMarkerCache;
      leftCacheDataEncodedNot = dataEncodedNotCache;

      // debug
      if (debugPrint) {
        Integer *sortedRecordsortedRecord = reconstructArray(mainData[intervalRootDP]);
        Integer *leftRecordleftRecord = reconstructArray(leftCacheData);
        cout << "sortedRecords for " << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecord, mainData[intervalRootDP].size());
        cout << "leftRecord for " << intervalRootDP << ": ";
        printArray(leftRecordleftRecord, leftCacheData.size());

       /* Integer *sortedDummysortedDummy = reconstructArray(mainDummyMarker[intervalRootDP]);
        Integer *leftDummyleftDummy = reconstructArray(leftCacheDummyMarker);
        cout << "sortedDummy for " << intervalRootDP << ": ";
        printArray(sortedDummysortedDummy, mainDummyMarker[intervalRootDP].size());
        cout << "leftDummy for " << intervalRootDP << ": ";
        printArray(leftDummyleftDummy, leftCacheDummyMarker.size());

        Integer *sortedRecordsortedRecordEncodedNot = reconstructArray(mainDataEncodedNot[intervalRootDP]);
        Integer *leftRecordleftRecordEncodedNot = reconstructArray(leftCacheDataEncodedNot);
        cout << "sortedRecordEncodedNot for " << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecordEncodedNot, mainDataEncodedNot[intervalRootDP].size());
        cout << "leftRecordEncodedNot for " << intervalRootDP << ": ";
        printArray(leftRecordleftRecordEncodedNot, leftCacheDataEncodedNot.size());
        cout << "sortedRecord.size(): " << mainData[intervalRootDP].size() << endl;*/
    
      // debug
      }
      // leftCache = leftCache 
    } else {
      // option2: 
      // #leaf nodes in this subtree should be at least one
      // cache <-- left cache + current leaf node
      std::vector<int> dataCache = leftCacheData;
      std::vector<int> dataEncodedNotCache = leftCacheDataEncodedNot;
      std::vector<int> dummyMarkerCache = leftCacheDummyMarker;
      dataCache.insert(dataCache.end(), originalData[i].begin(), originalData[i].end());
      dataEncodedNotCache.insert(dataEncodedNotCache.end(), originalDataEncodedNot[i].begin(), originalDataEncodedNot[i].end());
      dummyMarkerCache.insert(dummyMarkerCache.end(), originalDummyMarkers[i].begin(), originalDummyMarkers[i].end());
      
      // retrieve the data and DP histograms of previous nodes in this subtree
      // for each bin, we need to put n-d records for each interval together.  
      std::vector<std::vector<int> > dataMergedPrevious;
      std::vector<std::vector<int> > dataEncodedNotMergedPrevious;
      std::vector<std::vector<int> > dummyMarkerMergedPrevious;
      std::vector<std::vector<int> > dpMergedPrevious;
      for (int j = 0; j < int(intervalPrevious.size()) ; j++){
        dataMergedPrevious.push_back(mainData[intervalPrevious[j]]);
        dataEncodedNotMergedPrevious.push_back(mainDataEncodedNot[intervalPrevious[j]]);
        dummyMarkerMergedPrevious.push_back(mainDummyMarker[intervalPrevious[j]]);
        dpMergedPrevious.push_back(computePrefix(dpHists[intervalPrevious[j]]));
      }
      std::vector<std::vector<int> > encodedRecordsFirst, notEncordedRecordsFirst, dummyMarkerFirst;
      std::vector<std::vector<int> > encodedRecordsSecond, notEncordedRecordsSecond, dummyMarkerSecond;
      std::tie(encodedRecordsFirst, encodedRecordsSecond) = seperateD(dpMergedPrevious, dataMergedPrevious, d, bins);
      std::tie(notEncordedRecordsFirst, notEncordedRecordsSecond) = seperateD(dpMergedPrevious, dataEncodedNotMergedPrevious, d, bins);
      std::tie(dummyMarkerFirst, dummyMarkerSecond) = seperateD(dpMergedPrevious, dummyMarkerMergedPrevious, d, bins);


      // debug
     /* if (debugPrint) {
        Integer *sortedRecordsortedRecord = reconstructArray(dataMergedPrevious);
        cout << "previous sortedRecords for" << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecord, dataMergedPrevious.size());
      
        Integer *sortedDummysortedDummy = reconstructArray(dummyMarkerMergedPrevious);
        cout << "previous sortedDummy for " << intervalRootDP << ": ";
        printArray(sortedDummysortedDummy, dummyMarkerMergedPrevious.size());

        Integer *sortedRecordsortedRecordEncodedNot = reconstructArray(dataEncodedNotMergedPrevious);
        cout << "previous sortedRecordEncodedNot for " << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecordEncodedNot, dataEncodedNotMergedPrevious.size());
        cout << "previous sortedRecord.size(): " << dataMergedPrevious.size() << endl;
      }*/
      // debug 
       // for each bin, put the records for this bin of previous node and cache together, and sort
      for (int j = 0; j < bins; j++) {
        std::vector<int> toSortMergedPrevious = encodedRecordsSecond[j];
        std::vector<int> toSortEncodedNotMergedPrevious = notEncordedRecordsSecond[j];
        std::vector<int> toSortMarkerMergedPrevious = dummyMarkerSecond[j];
        toSortMergedPrevious.insert(toSortMergedPrevious.end(), dataCache.begin(), dataCache.end());
        toSortEncodedNotMergedPrevious.insert(toSortEncodedNotMergedPrevious.end(), dataEncodedNotCache.begin(), dataEncodedNotCache.end());
        toSortMarkerMergedPrevious.insert(toSortMarkerMergedPrevious.end(), dummyMarkerCache.begin(), dummyMarkerCache.end());
        // sort previous node for each bin and cache--> sorted for this bin + leftCache
        std::vector<int> encodedRecords, dummyMarker, notEncordedRecords;
        int sizeSort = toSortMergedPrevious.size();
        // compute the number of records we need to retrieve;  
        // ??? what if the sum of n-d for all intervals is larger than dpRoot[j]? 
        // d should be not too small!
        int sortDPd = dpRoot[j];
        for (int k = 0; k < int(intervalPrevious.size()); k++) {
          int leftAfterCutD = ((dpHists[intervalPrevious[k]][j] - d) < 0) ? 0: (dpHists[intervalPrevious[k]][j] - d);
          sortDPd = sortDPd - leftAfterCutD;
        }
        sortDPd = (sortDPd < 0) ? 0: sortDPd;   // todo: increase d if sortDPd<0
        // sort 
        std::tie(encodedRecords, dummyMarker, notEncordedRecords) = sortBinDP(party, toSortMergedPrevious, toSortMarkerMergedPrevious, toSortEncodedNotMergedPrevious, sortDPd, sizeSort, j);
        std::pair<std::vector<int>, std::vector<int> > seperatedRecord = copy2two(encodedRecords, sortDPd);
        std::pair<std::vector<int>, std::vector<int> > seperatedDummyMarker = copy2two(dummyMarker, sortDPd);
        std::pair<std::vector<int>, std::vector<int> > seperatedRecordEncodedNot = copy2two(notEncordedRecords, sortDPd);
        // n-d
        mainData[intervalRootDP].insert(mainData[intervalRootDP].end(), encodedRecordsFirst[j].begin(), encodedRecordsFirst[j].end());
        mainDummyMarker[intervalRootDP].insert(mainDummyMarker[intervalRootDP].end(), notEncordedRecordsFirst[j].begin(), notEncordedRecordsFirst[j].end());
        mainDataEncodedNot[intervalRootDP].insert(mainDataEncodedNot[intervalRootDP].end(), dummyMarkerFirst[j].begin(), dummyMarkerFirst[j].end());
        // sorted root d for this bin + left cache
        mainData[intervalRootDP].insert(mainData[intervalRootDP].end(), seperatedRecord.first.begin(), seperatedRecord.first.end());
        dataCache = seperatedRecord.second;
        mainDummyMarker[intervalRootDP].insert(mainDummyMarker[intervalRootDP].end(), seperatedDummyMarker.first.begin(), seperatedDummyMarker.first.end());
        dummyMarkerCache = seperatedDummyMarker.second;
        mainDataEncodedNot[intervalRootDP].insert(mainDataEncodedNot[intervalRootDP].end(), seperatedRecordEncodedNot.first.begin(), seperatedRecordEncodedNot.first.end());
        dataEncodedNotCache = seperatedRecordEncodedNot.second;
      }

      leftCacheData = dataCache;
      leftCacheDummyMarker = dummyMarkerCache;
      leftCacheDataEncodedNot = dataEncodedNotCache;

      // debug
      if (debugPrint) {
        Integer *sortedRecordsortedRecord = reconstructArray(mainData[intervalRootDP]);
        Integer *leftRecordleftRecord = reconstructArray(leftCacheData);
        cout << "sortedRecords for " << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecord, mainData[intervalRootDP].size());
        cout << "leftRecord for " << intervalRootDP << ": ";
        printArray(leftRecordleftRecord, leftCacheData.size());

       /* Integer *sortedDummysortedDummy = reconstructArray(mainDummyMarker[intervalRootDP]);
        Integer *leftDummyleftDummy = reconstructArray(leftCacheDummyMarker);
        cout << "sortedDummy for " << intervalRootDP << ": ";
        printArray(sortedDummysortedDummy, mainDummyMarker[intervalRootDP].size());
        cout << "leftDummy for " << intervalRootDP << ": ";
        printArray(leftDummyleftDummy, leftCacheDummyMarker.size());

        Integer *sortedRecordsortedRecordEncodedNot = reconstructArray(mainDataEncodedNot[intervalRootDP]);
        Integer *leftRecordleftRecordEncodedNot = reconstructArray(leftCacheDataEncodedNot);
        cout << "sortedRecordEncodedNot for " << intervalRootDP << ": ";
        printArray(sortedRecordsortedRecordEncodedNot, mainDataEncodedNot[intervalRootDP].size());
        cout << "leftRecordEncodedNot for " << intervalRootDP << ": ";
        printArray(leftRecordleftRecordEncodedNot, leftCacheDataEncodedNot.size());
        cout << "sortedRecord.size(): " << mainData[intervalRootDP].size() << endl;*/
    
      // debug
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
    int rightI = i;
    while (rightI >= 0) {
      int rootLeftI = nodesSubtree(rightI);
      string intervalRootDPI = std::to_string(rootLeftI) + ',' + std::to_string(rightI);
   //   cout << "intervalRootDPI: " << intervalRootDPI << endl;
      intervalss.push_back(intervalRootDPI);
     
      rightI = rootLeftI - 1;
    } 
    for (int j = 0; j < int(intervalss.size()); j++) { 
      dpI = addTwoVectors(dpI, dpHists[intervalss[j]]);
    }
    // step2: compute true histograms that cover 0 -- i
    std::vector<std::vector<int> > trueHistgrams;
    for (int j = 0; j <= i; j++) { 
     trueHistgrams.push_back(trueHists[j]);
    }
    std::vector<int> trueI = computeTrueNumber(trueHistgrams, bins);
    // step3: compute the error for DP count 
    double DPCountError = 0;
    for (int j = 0; j < bins; j++) { 
      DPCountError += abs(dpI[j] - trueI[j]);
    }
  //  cout << "DPCountError: " << DPCountError << endl;
    metricDPError.push_back(DPCountError);  

    // metric 3: sort errors = (dp count - true records)  
    // step1: compute the #true for each interval's data and then add them up 
    std::vector<int> trueR(bins, 0);
    for (int j = 0; j < int(intervalss.size()); j++) { 
      std::vector<int> trueRecords = computeTrueRecords(dpHists[intervalss[j]], mainData[intervalss[j]]); 
      trueR = addTwoVectors(trueR, trueRecords);
    }
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
  cout << "********************************************************************* " << endl;
  cout << "metricRunTimeDP: ";
  for (int i = 0; i < t; i++) {
    cout << metricRunTimeDP[i];
    if (i != t-1){
     cout << ", ";
    }
  } 
  cout << endl;
  cout << "metricRunTimeDPSort: ";
  for (int i = 0; i < t; i++) {
    cout << metricRunTimeDPSort[i]; 
    if (i != t-1){
     cout << ", ";
    }
  } 
  cout << endl;
  cout << "metricDPError: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPError[i];
    if (i != t-1){
     cout << ", ";
    }
  } 
  cout << endl;
  cout << "metricDPStoreError: ";
  for (int i = 0; i < t; i++) {
    cout << metricDPStoreError[i];
    if (i != t-1){
     cout << ", ";
    }
  } 
  cout << endl;

  finalize_semi_honest();
  delete io;
}
