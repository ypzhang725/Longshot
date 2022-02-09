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
  // construct tree?
  string option_string = argv[5]; // tree or 
  int option = atoi(option_string.c_str()); // 1, 2, 3, 4 how to DP histgrams; 1:leaf; 4:tree; 3:treeminus
  // how to sort main data
  string dpMerge_string = argv[6];;    // 1: merge; 0: sort all
  int dpMerge = atoi(dpMerge_string.c_str());
  // how to make it consistent
  int inconsistConst = 1; //  
  // constant dp noise
  bool constantDP = true; 
  // privacy budget
  double eps = 10;
  // bin number 
  int bins = 0;
  // !warning: if there are not enough dummy records, then sortDP and copy2two are incorrect
  int num_real = 0;  
  int num_dummy = 0; // make sure there are enough dummy records
   // nyc taxi dataset: 1271413 rows; 4 bins; payment_type
  if ((fileName_real == "taxi_ss1.txt") || (fileName_real == "taxi_ss2.txt")) {
    bins = 4; // bin number
    num_real = 1000; 
    num_dummy = 100; 
  } else {
    bins = 5; // bin number
    num_real = 10;  
    num_dummy = 10; 
    std::vector<int> vect_ = vect;
    for (int i = 0; i < t; i++) { 
      vect.insert(vect.end(), vect_.begin(), vect_.end());
    }
  }
  
 


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
  int mainSize = 0; //debug
  // for each update: 
  for (int i = 0; i < t; i++) {
    cout<< "index: " << i << endl;
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
      if (option != 1) {
        double levels = log2(t);  // double or int?
        eps = 1 / levels;
      }
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
     //     cout << "intervalDP: " << intervalDP <<  endl;
          dpAllNodes.push_back(inconsistDPHists[intervalDP]);
        }
      }
      // step3.2: make these nodes on this tree consistent and return the DP hist for the root
      dp = enforceConsistencyH(dpAllNodes, gapAgain);
      string intervalDP = std::to_string(rootLeftAgain) + ',' + std::to_string(i);
      dpHists[intervalDP] = nonNegative(dp);
      //debug
      for (int j = 0; j < bins; j++) {
      cout << dp[j] << ' ';
      }
      cout << endl;
      //debug
    }

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
    if (gapAgain <= 6) {
      // option 1: the subtree is small, so resort the root 
      cout << "intervalPrevious------------------------------------: " << intervalPrevious.size() << endl;
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
      cout << "ssizeSort------------------------------------: " << sizeSort << endl;
      //debug
      for (int j = 0; j < bins; j++) {
        cout << dpRoot[j] << ' ';
      }
      cout << endl;
      //debug
      // sort according to the DP hist of root 
      std::vector<int> encodedRecords, dummyMarker, notEncordedRecords;
      // do we need to re-encrypt????
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
      Integer *sortedRecordsortedRecord = reconstructArray(mainData[intervalRootDP]);
      Integer *leftRecordleftRecord = reconstructArray(leftCacheData);
      cout << "sortedRecordsortedRecord" << ' ';
      printArray(sortedRecordsortedRecord, mainData[intervalRootDP].size());
      cout << "leftRecordleftRecord" << ' ';
      printArray(leftRecordleftRecord, leftCacheData.size());

      Integer *sortedDummysortedDummy = reconstructArray(mainDummyMarker[intervalRootDP]);
      Integer *leftDummyleftDummy = reconstructArray(leftCacheDummyMarker);
      cout << "sortedDummysortedDummy" << ' ';
      printArray(sortedDummysortedDummy, mainDummyMarker[intervalRootDP].size());
      cout << "leftDummyleftDummy" << ' ';
      printArray(leftDummyleftDummy, leftCacheDummyMarker.size());

      Integer *sortedRecordsortedRecordEncodedNot = reconstructArray(mainDataEncodedNot[intervalRootDP]);
      Integer *leftRecordleftRecordEncodedNot = reconstructArray(leftCacheDataEncodedNot);
      cout << "sortedRecordsortedRecordEncodedNot" << ' ';
      printArray(sortedRecordsortedRecordEncodedNot, mainDataEncodedNot[intervalRootDP].size());
      cout << "leftRecordleftRecordEncodedNot" << ' ';
      printArray(leftRecordleftRecordEncodedNot, leftCacheDataEncodedNot.size());
      cout << "sortedRecord.size(): " << mainData[intervalRootDP].size() << endl;
      // debug
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
      std::vector<int> dataMergedPrevious = mainData[intervalPrevious[0]];
      std::vector<int> dataEncodedNotMergedPrevious = mainDataEncodedNot[intervalPrevious[0]];
      std::vector<int> dummyMarkerMergedPrevious = mainDummyMarker[intervalPrevious[0]];
      std::vector<int> dpMergedPrevious = dpHists[intervalPrevious[0]];
      // merge these previous nodes using their DP histograms
      for (int j = 1; j < intervalPrevious.size() ; j++){
        dataMergedPrevious = merge2SortedArr(dpMergedPrevious, dpHists[intervalPrevious[j]], dataMergedPrevious, mainData[intervalPrevious[j]], bins);
        dataEncodedNotMergedPrevious = merge2SortedArr(dpMergedPrevious, dpHists[intervalPrevious[j]], dataEncodedNotMergedPrevious, mainDataEncodedNot[intervalPrevious[j]], bins);
        dummyMarkerMergedPrevious = merge2SortedArr(dpMergedPrevious, dpHists[intervalPrevious[j]], dummyMarkerMergedPrevious, mainDummyMarker[intervalPrevious[j]], bins);
        dpMergedPrevious = addTwoVectors(dpMergedPrevious, dpHists[intervalPrevious[j]]);
      }
      // debug
      Integer *sortedRecordsortedRecord = reconstructArray(dataMergedPrevious);
      cout << "sortedRecordsortedRecord" << ' ';
      printArray(sortedRecordsortedRecord, dataMergedPrevious.size());
     
      Integer *sortedDummysortedDummy = reconstructArray(dummyMarkerMergedPrevious);
      cout << "sortedDummysortedDummy" << ' ';
      printArray(sortedDummysortedDummy, dummyMarkerMergedPrevious.size());

      Integer *sortedRecordsortedRecordEncodedNot = reconstructArray(dataEncodedNotMergedPrevious);
      cout << "sortedRecordsortedRecordEncodedNot" << ' ';
      printArray(sortedRecordsortedRecordEncodedNot, dataEncodedNotMergedPrevious.size());
      cout << "sortedRecord.size(): " << dataMergedPrevious.size() << endl;
      // for each bin, put the records for this bin of previous node and cache together, and sort
      
      for (int j = 0; j < bins; j++) {
        
      }
    }




    /*if (dpMerge == 1) {
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
      mainSize += totalRecords;
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
      cout << "sortedRecord.size(): " << sortedRecord.size() << endl;
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
      Integer *mainmain = reconstructArray(mainData);
      cout << "mainmain: " << ' ';
      printArray(mainmain, mainData.size());
    }
    // alternative to step3 + step4: re-sort all
    else if (dpMerge == 0) {
    // dp hist of all
    // sort DP
    // main + left cache + current records+cache 
      leftCacheData.insert(leftCacheData.end(), originalData[i].begin(), originalData[i].end());
      leftCacheDataEncodedNot.insert(leftCacheDataEncodedNot.end(), originalDataEncodedNot[i].begin(), originalDataEncodedNot[i].end());
      leftCacheDummyMarker.insert(leftCacheDummyMarker.end(), originalDummyMarkers[i].begin(), originalDummyMarkers[i].end());
      mainData.insert(mainData.end(), leftCacheData.begin(), leftCacheData.end());
      mainDataEncodedNot.insert(mainDataEncodedNot.end(), leftCacheDataEncodedNot.begin(), leftCacheDataEncodedNot.end());
      mainDummyMarker.insert(mainDummyMarker.end(), leftCacheDummyMarker.begin(), leftCacheDummyMarker.end());
      int sizeCache = mainData.size();
      std::vector<int> encodedRecords, dummyMarker, notEncordedRecords;
      std::vector<int> dp_main(bins, 0);
      for (int j = 0; j <= i; j++) { 
        dp_main = addTwoVectors(dp_main, dpHists[j]);
      }
      std::tie(encodedRecords, dummyMarker, notEncordedRecords) = sortDP(party, mainData, mainDummyMarker, mainDataEncodedNot, dp_main, sizeCache, bins);

      // total DP count = #records we want to retrieve --> sorted main + left cache 
      // DP histogram for main data store
      int totalRecords = accumulate(dp_main.begin(), dp_main.end(), 0);
      std::pair<std::vector<int>, std::vector<int> > seperatedRecord = copy2two(encodedRecords, totalRecords);
      std::pair<std::vector<int>, std::vector<int> > seperatedDummyMarker = copy2two(dummyMarker, totalRecords);
      std::pair<std::vector<int>, std::vector<int> > seperatedRecordEncodedNot = copy2two(notEncordedRecords, totalRecords);

      mainData = seperatedRecord.first;
      leftCacheData = seperatedRecord.second;
      mainDummyMarker = seperatedDummyMarker.first;
      leftCacheDummyMarker = seperatedDummyMarker.second;
      mainDataEncodedNot = seperatedRecordEncodedNot.first;
      leftCacheDataEncodedNot = seperatedRecordEncodedNot.second;
    }*/
  }

 /* //debug
  cout << "******************************************* " << endl;
  std::vector<int> dp_main(bins, 0);
  for (int j = 0; j < t; j++) { 
    dp_main = addTwoVectors(dp_main, dpHists[j]);
  }
  std::vector<std::vector<int> > trueHistgrams;
  for (int j = 0; j < t; j++) { 
    trueHistgrams.push_back(trueHists[j]);
  }
  std::vector<int> true_main = computeTrueNumber(trueHistgrams, bins);
  cout << "DP count" << endl;
  for (int j = 0; j < bins; j++) { 
    cout << dp_main[j] << "; ";
  }
  cout << endl;
  cout << "true count" << endl;
  for (int j = 0; j < bins; j++) { 
    cout << true_main[j] << "; ";
  }
  cout << endl;
  int totalRecordsDP = accumulate(dp_main.begin(), dp_main.end(), 0);
  int totalRecordsTrue = accumulate(true_main.begin(), true_main.end(), 0);
  cout << "**********totalRecords_DP: " << totalRecordsDP << endl;
  cout << "**********totalRecords_True: " << totalRecordsTrue << endl;
  cout << "**********mainData.size(): " << mainData.size() << endl;
  cout << "**********mainDataLeft.size(): " << leftCacheData.size() << endl;
  cout << "mainDummyMarker.size(): " << mainDummyMarker.size() << endl;
  cout << "mainDataEncodedNot.size(): " << mainDataEncodedNot.size() << endl;
  Integer *mainmain = reconstructArray(mainData);
  Integer *mainDummyMarkermain = reconstructArray(mainDummyMarker);
  Integer *mainmainEncodedNot = reconstructArray(mainDataEncodedNot);
  cout << "mainmain: " << ' ';
  printArray(mainmain, mainData.size());
  cout << "mainDummyMarkermain: " << ' ';
  printArray(mainDummyMarkermain, mainDummyMarker.size());
  cout << "mainmainEncodedNot: " << ' ';
  printArray(mainmainEncodedNot, mainDataEncodedNot.size());
  int dummyNum = computeDummyNumber(mainDummyMarkermain, mainDummyMarker.size());
  cout << "**********totalRecords_dummy: " << dummyNum << endl;
  Integer *leftCacheDummyMarker_Int = reconstructArray(leftCacheDummyMarker);
  int dummyNumLeft = computeDummyNumber(leftCacheDummyMarker_Int, leftCacheDummyMarker.size());
  cout << "**********totalRecordsLeft_dummy: " << dummyNumLeft << endl;
  cout << "**********computed true(main+left): " << (mainData.size() + leftCacheData.size() - dummyNum - dummyNumLeft) << endl;
  //debug
*/
  finalize_semi_honest();
  delete io;
}
