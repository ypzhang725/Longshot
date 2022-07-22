import sys; sys.path.insert(0, '.')
from funcPy import *

def metrics(treeorLeaf, T, epsAll, numReal, sortOption):
    df = readData('bin40.csv')
    numBins = 40
    p = 0.01
    t = math.log((1/p), math.e)
    if treeorLeaf == "tree":
        #tree 
        level = math.log(T, 2)
        eps = epsAll/level  
        numDummy = math.ceil((1/eps) * t) * numBins
        originalData, originalDummyMarkers, trueHists = originalDataMarkerHistsTree(T, numReal, numDummy, numBins, df)
        dpHists = DPTimeTree(T, trueHists, eps, numBins)
        gapAgainThreshold = 1 
        trueRecordNum, trueRecordNumRange, runTimeDPSort, dummyRecordNumCache = sortTree(numDummy, sortOption, gapAgainThreshold, T, numBins, dpHists, originalData, originalDummyMarkers, eps) # original?
        #print(trueRecordNum)

    else:
        #leaf
        p = 0.0001
        eps = epsAll
        originalData, originalDummyMarkers, trueHists = originalDataMarkerHistsLeaf(p, eps, T, numReal, numBins, df)
        dpHistsLeaf = DPTimeLeaf(T, trueHists, eps, numBins)
        trueRecordNum, trueRecordNumRange, runTimeDPSort, dummyRecordNumCache = sortLeaf(T, numBins, dpHistsLeaf, originalData, originalDummyMarkers)

    DPCount = [None]*T
    trueCount = [None]*T
    DPCountRange = [None]*T
    trueCountRange = [None]*T
    for i in range(T):
        if (treeorLeaf=="tree"):
            intervalss = intervalRangeQ(i) 
            DPI = np.array([0]*numBins)
            for interval in intervalss:
                DPI += dpHists[interval]
            DPCount[i] = DPI
        else:
            DPI = np.array([0]*numBins)
            for j in range(i+1):
                DPI += dpHistsLeaf[str(j)+','+str(j)]
            DPCount[i] = DPI

        trueI = np.array([0]*numBins)
        for j in range(i+1):
            trueI += trueHists[j]
        trueCount[i] = trueI

        DPCountRange_ = []
        trueCountRange_ = []
        idx = 0 
        for j in range(numBins):
            for k in range(j, numBins, 1):
                DPCountRange_.append(0) 
                trueCountRange_.append(0) 
                for l in range(j, (k+1), 1):
                    DPCountRange_[idx] = DPCountRange_[idx] + DPI[l]
                    trueCountRange_[idx] = trueCountRange_[idx] + trueI[l]
                idx = idx+1
        DPCountRange[i] = DPCountRange_
        trueCountRange[i] = trueCountRange_
    
   # print("DPCount: ", DPCount)
 #   print("trueCount: ", trueCount)
 #   print("trueRecordNum: ", trueRecordNum)
    
   # metricDPError = np.sum(np.abs(np.array(DPCount) - np.array(trueCount)), axis =1)
   # metricDPStoreError = np.sum(np.abs(np.array(DPCount) - np.array(trueRecordNum)), axis =1)
   # metricTTStoreError = np.sum(np.abs(np.array(trueCount) - np.array(trueRecordNum)), axis =1)

    metricDPError = np.mean(np.abs(np.array(DPCount) - np.array(trueCount)), axis =1)
    metricDPStoreError = np.mean(np.abs(np.array(DPCount) - np.array(trueRecordNum)), axis =1)
    metricTTStoreError = np.mean(np.abs(np.array(trueCount) - np.array(trueRecordNum)), axis =1)

    metricDPErrorRange = np.mean(np.abs(np.array(DPCountRange) - np.array(trueCountRange)), axis =1)
    metricDPStoreErrorRange = np.mean(np.abs(np.array(DPCountRange) - np.array(trueRecordNumRange)), axis =1)
    metricTTStoreErrorRange = np.mean(np.abs(np.array(trueCountRange) - np.array(trueRecordNumRange)), axis =1)
  
    
    return metricDPError, metricDPStoreError, metricTTStoreError, metricDPErrorRange, metricDPStoreErrorRange, metricTTStoreErrorRange, runTimeDPSort, dummyRecordNumCache



def run_all(T_list, epsAll_list, numReal_list, runNum):
    for T in T_list:
        for epsAll in epsAll_list:
            for numReal in numReal_list:
                start_time = time.time()
                list_metricDPError_leaf = [None]*runNum
                list_metricDPStoreError_leaf = [None]*runNum
                list_metricTTStoreError_leaf = [None]*runNum
                list_metricDPError_leaf_range = [None]*runNum
                list_metricDPStoreError_leaf_range = [None]*runNum
                list_metricTTStoreError_leaf_range = [None]*runNum
                list_runTimeDPSort_leaf = [None]*runNum
                list_dummyRecordNumCache_leaf = [None]*runNum
                for i in range(runNum):
                    print("leaf -- T: "+str(T)+"; epsAll: "+str(epsAll)+"; N: "+str(numReal)+"; run: "+str(i))
                    metricDPError_leaf, metricDPStoreError_leaf, metricTTStoreError_leaf, metricDPErrorRange_leaf, metricDPStoreErrorRange_leaf, metricTTStoreErrorRange_leaf, runTimeDPSort_leaf, dummyRecordNumCache_leaf = metrics("leaf", T, epsAll, numReal, None)
                    leaf_end_time = time.time()
                    list_metricDPError_leaf[i] = metricDPError_leaf
                    list_metricDPStoreError_leaf[i] = metricDPStoreError_leaf
                    list_metricTTStoreError_leaf[i] = metricTTStoreError_leaf
                    list_metricDPError_leaf_range[i] = metricDPErrorRange_leaf
                    list_metricDPStoreError_leaf_range[i] = metricDPStoreErrorRange_leaf
                    list_metricTTStoreError_leaf_range[i] = metricTTStoreErrorRange_leaf
                    list_runTimeDPSort_leaf[i] = runTimeDPSort_leaf
                    list_dummyRecordNumCache_leaf[i] = dummyRecordNumCache_leaf
                print("leaf***************")
                print("--- %s seconds ---" % (leaf_end_time - start_time))

                mean_metricDPError_leaf = np.round(np.mean(list_metricDPError_leaf, axis = 0))
                mean_metricDPStoreError_leaf = np.round(np.mean(list_metricDPStoreError_leaf, axis = 0))
                mean_metricTTStoreError_leaf = np.round(np.mean(list_metricTTStoreError_leaf, axis = 0))
                mean_runTimeDPSort_leaf = np.round(np.mean(list_runTimeDPSort_leaf, axis = 0))
                mean_dummyRecordNumCache_leaf = np.round(np.mean(list_dummyRecordNumCache_leaf, axis = 0))
                mean_metricDPErrorRange_leaf = np.round(np.mean(list_metricDPError_leaf_range, axis = 0))
                mean_metricDPStoreErrorRange_leaf = np.round(np.mean(list_metricDPStoreError_leaf_range, axis = 0))
                mean_metricTTStoreErrorRange_leaf = np.round(np.mean(list_metricTTStoreError_leaf_range, axis = 0))


                list_metricDPError_treeD = [None]*runNum
                list_metricDPStoreError_treeD = [None]*runNum
                list_metricTTStoreError_treeD = [None]*runNum
                list_metricDPError_treeD_range = [None]*runNum
                list_metricDPStoreError_treeD_range = [None]*runNum
                list_metricTTStoreError_treeD_range = [None]*runNum
                list_runTimeDPSort_treeD = [None]*runNum
                list_dummyRecordNumCache_treeD = [None]*runNum
                for i in range(runNum):
                    print("treeD -- T: "+str(T)+"; epsAll: "+str(epsAll)+"; N: "+str(numReal)+"; run: "+str(i))
                    metricDPError_treeD, metricDPStoreError_treeD, metricTTStoreError_treeD, metricDPErrorRange_treeD, metricDPStoreErrorRange_treeD, metricTTStoreErrorRange_treeD, runTimeDPSort_treeD, dummyRecordNumCache_treeD = metrics("tree", T, epsAll, numReal, 2)
                    treeD_end_time = time.time()
                    list_metricDPError_treeD[i] = metricDPError_treeD
                    list_metricDPStoreError_treeD[i] = metricDPStoreError_treeD
                    list_metricTTStoreError_treeD[i] = metricTTStoreError_treeD
                    list_metricDPError_treeD_range[i] = metricDPErrorRange_treeD
                    list_metricDPStoreError_treeD_range[i] = metricDPStoreErrorRange_treeD
                    list_metricTTStoreError_treeD_range[i] = metricTTStoreErrorRange_treeD
                    list_runTimeDPSort_treeD[i] = runTimeDPSort_treeD
                    list_dummyRecordNumCache_treeD[i] = dummyRecordNumCache_treeD
                print("treeD***************")
                print("--- %s seconds ---" % (treeD_end_time - start_time))
                mean_metricDPError_treeD = np.round(np.mean(list_metricDPError_treeD, axis = 0))
                mean_metricDPStoreError_treeD = np.round(np.mean(list_metricDPStoreError_treeD, axis = 0))
                mean_metricTTStoreError_treeD = np.round(np.mean(list_metricTTStoreError_treeD, axis = 0))
                mean_runTimeDPSort_treeD = np.round(np.mean(list_runTimeDPSort_treeD, axis = 0))
                mean_dummyRecordNumCache_treeD = np.round(np.mean(list_dummyRecordNumCache_treeD, axis = 0))
                mean_metricDPErrorRange_treeD = np.round(np.mean(list_metricDPError_treeD_range, axis = 0))
                mean_metricDPStoreErrorRange_treeD = np.round(np.mean(list_metricDPStoreError_treeD_range, axis = 0))
                mean_metricTTStoreErrorRange_treeD = np.round(np.mean(list_metricTTStoreError_treeD_range, axis = 0))


                list_metricDPError_treeA = [None]*runNum
                list_metricDPStoreError_treeA = [None]*runNum
                list_metricTTStoreError_treeA = [None]*runNum
                list_metricDPError_treeA_range = [None]*runNum
                list_metricDPStoreError_treeA_range = [None]*runNum
                list_metricTTStoreError_treeA_range = [None]*runNum
                list_runTimeDPSort_treeA = [None]*runNum
                list_dummyRecordNumCache_treeA = [None]*runNum
                for i in range(runNum):
                    print("treeA -- T: "+str(T)+"; epsAll: "+str(epsAll)+"; N: "+str(numReal)+"; run: "+str(i))
                    metricDPError_treeA, metricDPStoreError_treeA, metricTTStoreError_treeA, metricDPErrorRange_treeA, metricDPStoreErrorRange_treeA, metricTTStoreErrorRange_treeA, runTimeDPSort_treeA, dummyRecordNumCache_treeA = metrics("tree", T, epsAll, numReal, 0)
                    treeA_end_time = time.time()
                    list_metricDPError_treeA[i] = metricDPError_treeA
                    list_metricDPStoreError_treeA[i] = metricDPStoreError_treeA
                    list_metricTTStoreError_treeA[i] = metricTTStoreError_treeA
                    list_metricDPError_treeA_range[i] = metricDPErrorRange_treeA
                    list_metricDPStoreError_treeA_range[i] = metricDPStoreErrorRange_treeA
                    list_metricTTStoreError_treeA_range[i] = metricTTStoreErrorRange_treeA
                    list_runTimeDPSort_treeA[i] = runTimeDPSort_treeA
                    list_dummyRecordNumCache_treeA[i] = dummyRecordNumCache_treeA
                print("treeA***************")
                print("--- %s seconds ---" % (treeA_end_time - start_time))
                mean_metricDPError_treeA = np.round(np.mean(list_metricDPError_treeA, axis = 0))
                mean_metricDPStoreError_treeA = np.round(np.mean(list_metricDPStoreError_treeA, axis = 0))
                mean_metricTTStoreError_treeA = np.round(np.mean(list_metricTTStoreError_treeA, axis = 0))
                mean_runTimeDPSort_treeA = np.round(np.mean(list_runTimeDPSort_treeA, axis = 0))
                mean_dummyRecordNumCache_treeA = np.round(np.mean(list_dummyRecordNumCache_treeA, axis = 0))
                mean_metricDPErrorRange_treeA = np.round(np.mean(list_metricDPError_treeA_range, axis = 0))
                mean_metricDPStoreErrorRange_treeA = np.round(np.mean(list_metricDPStoreError_treeA_range, axis = 0))
                mean_metricTTStoreErrorRange_treeA = np.round(np.mean(list_metricTTStoreError_treeA_range, axis = 0))

<<<<<<< HEAD
=======

>>>>>>> 3791fa09472d8a814f393a5aaae0eccdcd80bd5d
                fileName = "resultsJuly/all-T:"+str(T)+",eps:"+str(epsAll)+",N:"+str(numReal)+".json"

                with open(fileName, 'w') as f:
                    entry = {}
                    entry['list_metricDPError_leaf'] = mean_metricDPError_leaf.tolist()
                    entry['list_metricDPStoreError_leaf'] = mean_metricDPStoreError_leaf.tolist()
                    entry['list_metricTTStoreError_leaf'] = mean_metricTTStoreError_leaf.tolist()

                    entry['list_metricDPErrorRange_leaf'] = mean_metricDPErrorRange_leaf.tolist()
                    entry['list_metricDPStoreErrorRange_leaf'] = mean_metricDPStoreErrorRange_leaf.tolist()
                    entry['list_metricTTStoreErrorRange_leaf'] = mean_metricTTStoreErrorRange_leaf.tolist()

                    entry['list_runTimeDPSort_leaf'] = mean_runTimeDPSort_leaf.tolist()
                    entry['list_dummyRecordNumCache_leaf'] = mean_dummyRecordNumCache_leaf.tolist()



                    entry['list_metricDPError_treeA'] = mean_metricDPError_treeA.tolist()
                    entry['list_metricDPStoreError_treeA'] = mean_metricDPStoreError_treeA.tolist()
                    entry['list_metricTTStoreError_treeA'] = mean_metricTTStoreError_treeA.tolist()

                    entry['list_metricDPErrorRange_treeA'] = mean_metricDPErrorRange_treeA.tolist()
                    entry['list_metricDPStoreErrorRange_treeA'] = mean_metricDPStoreErrorRange_treeA.tolist()
                    entry['list_metricTTStoreErrorRange_treeA'] = mean_metricTTStoreErrorRange_treeA.tolist()

                    entry['list_runTimeDPSort_treeA'] = mean_runTimeDPSort_treeA.tolist()
                    entry['list_dummyRecordNumCache_treeA'] = mean_dummyRecordNumCache_treeA.tolist()



                    entry['list_metricDPError_treeD'] = mean_metricDPError_treeD.tolist()
                    entry['list_metricDPStoreError_treeD'] = mean_metricDPStoreError_treeD.tolist()
                    entry['list_metricTTStoreError_treeD'] = mean_metricTTStoreError_treeD.tolist()

                    entry['list_metricDPErrorRange_treeD'] = mean_metricDPErrorRange_treeD.tolist()
                    entry['list_metricDPStoreErrorRange_treeD'] = mean_metricDPStoreErrorRange_treeD.tolist()
                    entry['list_metricTTStoreErrorRange_treeD'] = mean_metricTTStoreErrorRange_treeD.tolist()

                    entry['list_runTimeDPSort_treeD'] = mean_runTimeDPSort_treeD.tolist()
                    entry['list_dummyRecordNumCache_treeD'] = mean_dummyRecordNumCache_treeD.tolist()
                    json.dump(entry, f, ensure_ascii=False)


<<<<<<< HEAD
T_list = [10]
epsAll_list = [1]
numReal_list = [10]
runNum = 3
=======
T_list = [1000]
epsAll_list = [1, 10, 0.1 ]
numReal_list = [1000, 100]
runNum = 2
>>>>>>> 3791fa09472d8a814f393a5aaae0eccdcd80bd5d
'''
T_list = [1000]
epsAll_list = [1]
numReal_list = [1000]
runNum = 10
'''
run_all(T_list, epsAll_list, numReal_list, runNum)
