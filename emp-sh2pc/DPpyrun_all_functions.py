import pandas as pd
import numpy as np
import math
import time
import json


def readData(fileName):
    df = pd.read_csv(fileName)   # [1271413 rows x 1 columns]
    df = df.to_numpy().flatten()
    df = df.tolist()
    for i in range(3):
        df.extend(df)
    return df
    
def nonNegative(value):
    return [round(v) if v>0 else 0 for v in value]

def DPTreeH(DPAllNodes, gap):
    height = int(math.log(gap, 2)) + 1 #the height of the tree
    if height == 1:
        return DPAllNodes[0]
    succ = DPAllNodes[0: gap].copy() #leaf nodes
    pt = 0+gap #the index of the first node in the second level
    for i in range(2, height+1, 1): #the level in the paper
        gap = int(gap/2) #the number of nodes in this level
        succ_new = [0 for i in range(gap)] #compute the sum of succ for each node in this level

        for j in range(gap): #
            succ_new[j] = succ[j*2] + succ[j*2+1]
      
        now = DPAllNodes[pt: pt + gap].copy() #the dp nodes for this level

        consist = [0 for i in range(gap)] #compute the #z[v] in paper
        a = (pow(2, i) - pow(2, i-1)) / (pow(2, i) - 1)
        b = (pow(2, i-1) - 1) / (pow(2, i) - 1)
        
        for j in range(gap):
            consist[j] =  a*now[j] + b*succ_new[j]

        pt = pt + gap #point to the first node of the next level 
        succ = consist # needed for next level 
    return consist[0]

def nodesSubtree(i):
    i += 1
    j = 1
    k = i
    rootLeft = i
    while (k % 2 == 0):
        rootLeft = i - 2**j + 1
        k = k / 2
        j += 1    
    return rootLeft - 1

def retrieveNodes(i):
    left = nodesSubtree(i)
    gap = i - left + 1
    height = int(math.log(gap, 2)) + 1
    for i in range(height):
        print("i-----", i)
        inteval = pow(2, i)
        print("inteval", inteval)
        for j in range(left, right+1, inteval):
            print(str(j) + "," + str(j+inteval-1))
            
# sort according to the DP hist of root 
def computeBin(data, markers, dp):
    counter = dp.copy()
    size = len(data)
    binNum = len(dp)
    bins = [binNum] * size
    # real records 
    for i in range(size):
        bin_num = data[i] - 1
        if markers[i] == 1:  
            if counter[bin_num] > 0:
                bins[i] = bin_num
                counter[bin_num] = counter[bin_num] - 1
    # dummy records 
    for i in range(size):
        if markers[i] == 2:
            for j in range(binNum):
                if bins[i] == binNum and counter[j] > 0 :
                    bins[i] = j
                    counter[j] = counter[j] - 1
    return bins

# sort according to the DP hist of root 
def computeBinJ(data, markers, sortDPd, j):
    counter = sortDPd
    size = len(data)
    bins = [1] * size
    # real records 
    for i in range(size):
        bin_num = data[i] - 1
        if markers[i] == 1 and bin_num == j and counter > 0:
            bins[i] = 0
            counter = counter - 1
    # dummy records 
    for i in range(size):
        if markers[i] == 2 and counter > 0:
            bins[i] = 0
            counter = counter - 1
            continue
    return bins



# using list comprehension + sum() + list slicing
# prefix sum list
def computePrefix(test_list):
    res = [sum(test_list[ : i + 1]) for i in range(len(test_list))]
    return res
    
def seperateD(dpMergedPrevious, dataMergedPrevious, d, numBin):
    vectFirst = [None] * numBin
    vectSecond = [None] * numBin
    intervals = len(dpMergedPrevious)
    # preprocess preefix --> cut last bin if no enough records 
    # preprocess prefixsum
    dpHistPrefixIntrevals = [None] * intervals
    for i in range(intervals):  # for each interval
        sizeInterval = len(dataMergedPrevious[i])
        dpMergedPrevious[i][numBin-1] = sizeInterval #?
        dpHistPrefix = [0] * (numBin+1) 
        for j in range(numBin):
            dpHistPrefix[j+1] = dpMergedPrevious[i][j]
        dpHistPrefixIntrevals[i] = dpHistPrefix
    for i in range(numBin): # for each bin 
        first = []
        second = []
        for j in range(intervals):  # for each interval
            left = dpHistPrefixIntrevals[j][i+1] - d if (dpHistPrefixIntrevals[j][i] < dpHistPrefixIntrevals[j][i+1] - d) else dpHistPrefixIntrevals[j][i]
            begining = dataMergedPrevious[j][dpHistPrefixIntrevals[j][i]: left].copy()
            ending = dataMergedPrevious[j][left: dpHistPrefixIntrevals[j][i+1]].copy()
            first.extend(begining)  # the first n-d
            second.extend(ending)   # the last d 
        vectFirst[i] = first.copy()
        vectSecond[i] = second.copy()

    return (vectFirst, vectSecond)  

# for each time unit, insert real and dummy records, and compute true histogram
def originalDataMarkerHists(T, numReal, numDummy, numBins, df):
    originalData = {}
    originalDummyMarkers = {}
    trueHists = [None] * T
    for i in range(T):
        # for originalData
        records = [None] * (numReal+numDummy)
        records[0: numReal] = df[i*numReal: (i+1)*numReal].copy()
        records[numReal: numReal+numDummy] = [10] * numDummy #todo change mpc code
        originalData[i] = records
        # for originalDummyMarker
        DummyMarker = [None] * (numReal+numDummy)
        DummyMarker[0: numReal] = [1] * numReal
        DummyMarker[numReal: numReal+numDummy] = [2] * numDummy #todo change mpc code
        originalDummyMarkers[i] = DummyMarker
        # compute trueHists
        counts, bins = np.histogram(records, bins=np.arange(1,numBins+2)) #[1,2,3,4,5] -> 4bins
        trueHists[i] = counts 
    return originalData, originalDummyMarkers, trueHists
    #print(originalData)
    #print(originalDummyMarkers)
    #print(trueHists)
    
def computeTrueRecords(dpHist, dpStore):
    binNum = len(dpHist)
    recordNum = len(dpStore) 
            
    dpHistPrefix_tmp = computePrefix(dpHist)
    dpHistPrefix_tmp[binNum-1] = recordNum  
    dpHistPrefix = [0]*(binNum+1)
    for j in range(binNum):
        dpHistPrefix[j+1] = dpHistPrefix_tmp[j]
    trueR = [0]*binNum
    for i in range(binNum):
        num = 0
        for j in range(dpHistPrefix[i], dpHistPrefix[i+1], 1):
            if ((dpStore[j]-1) == i): #dpStorePublic[j]: 1122334455
                num = num+1
        trueR[i] = num
    return trueR 

def computeDummyRecordsCache(cache):
    num = 0
    size = len(cache)
    for i in range(size):
        if cache[i] == 2:
            num = num+1
    return num 

def intervalRangeQ(i):  
    intervalss = []
    rightI = i
    while (rightI >= 0):
        rootLeftI = nodesSubtree(rightI)
        intervalRootDPI = str(rootLeftI) + ',' + str(rightI)
        intervalss.append(intervalRootDPI)
        rightI = rootLeftI - 1
    return intervalss

    

# step2: dpHistGen for the root of the subtree
def DPTimeTree(T, trueHists, eps, numBins):
    dpHists = {}
    inconsistDPHists = {}
    
    for i in range(T):
      #  print(str(i)+"********************************")
        # step2.1 and step2.2: generate DP hists of nodes on the current path (current leaf to root)
        rootLeft = nodesSubtree(i)
        gap = i - rootLeft + 1
       # print("rootLeft", rootLeft)
       # print("gap", gap)

        while ((gap / 2 >= 1) or (gap >= 1)):
            # step2.1: true hists of nodes on the path
            trueHistgrams = [0 for j in range(numBins)]
            for j in range(rootLeft, (i + 1)): 
                trueHistgrams += trueHists[j]
            # step2.2: DP hists of nodes on the path  
           # dpNode = trueHistgrams + 2#np.random.laplace(0, 1 / epsTree, 1)[0]
            dpNode = trueHistgrams + np.random.laplace(0, (1/eps), numBins)
            intervalDP = str(rootLeft) + ',' + str(i)
          #  print("intervalDP", intervalDP)
            inconsistDPHists[intervalDP] = dpNode

            rootLeft += int(gap / 2)
            gap /= 2

        # step3: compute the consistent DP histogram of the root of the subtree
        # step3.1: DP hists of all nodes in the subtree

        dpAllNodes = []
        rootLeftAgain = nodesSubtree(i)
        gapAgain = i - rootLeftAgain + 1
        height = int(math.log(gapAgain, 2)) + 1
        for j in range(height):
          #  print("j-----", j)
            inteval = pow(2, j)
           # print("inteval", inteval)
            for k in range(rootLeftAgain, i+1, inteval):
                intervalDP = str(k) + "," + str(k+inteval-1)
                dpAllNodes.append(inconsistDPHists[intervalDP])
        dp = DPTreeH(dpAllNodes, gapAgain)
        intervalDP = str(rootLeftAgain) + ',' + str(i)
        dpHists[intervalDP] = nonNegative(dp)
    return dpHists
    #print(trueHists)
    #print(dpHists)
    #print(inconsistDPHists) 

    
def sortTree(sortOption, gapAgainThreshold, T, numBins, dpHists, originalData, originalDummyMarkers, eps):
    t = math.log((1/0.005), math.e)
    d = math.ceil((1/eps) * t)
    leftCacheData = []
    leftCacheDummyMarker = []
    mainData = {}
    mainDummyMarker = {}
    trueRecordNum = [None] * T
    runTimeDPSort = [None] * T
    dummyRecordNumCache = [None] * T
    
    # step4: get the sorted array of the root node 
    for i in range(T):
     #   print(str(i)+"********************************")
        # step4.1: retrieve the DP histogram of the root node 
        rootLeftSort = nodesSubtree(i)
        intervalRootDP = str(rootLeftSort) + ',' + str(i)
        dpRoot = dpHists[intervalRootDP]
        gapAgainAgain = i - rootLeftSort + 1
        # step4.2: compute the interval of previous nodes
        intervalPrevious = []
        gapSort = i - rootLeftSort + 1
        while ((gapSort / 2 >= 1) or (gapSort >= 1)):
            if (rootLeftSort < int(rootLeftSort + (gapSort / 2))):
                interval = str(rootLeftSort) + ',' + str(int(rootLeftSort + (gapSort / 2)) - 1)
                intervalPrevious.append(interval)
            rootLeftSort += int(gapSort / 2);
            gapSort /= 2;
        #print(intervalPrevious)
        # step4.3: get the sorted array of the root node
        # option0: if sortOption == 0 or gapAgain <= x
        # option1: else if sortOption == 1 
        # option2: else sortOption == 2 
        if (sortOption == 0 or gapAgainAgain <= gapAgainThreshold):
            dataToSort = leftCacheData.copy()
            dummyMarkerToSort = leftCacheDummyMarker.copy();
            for interval in intervalPrevious:
                dataToSort.extend(mainData[interval])
                dummyMarkerToSort.extend(mainDummyMarker[interval])
            dataToSort.extend(originalData[i])
            dummyMarkerToSort.extend(originalDummyMarkers[i])


          #  print(dpRoot)
            bins = computeBin(dataToSort,dummyMarkerToSort,dpRoot)
            sorted_data = [k for _,k in sorted(zip(bins,dataToSort))]
            sorted_marker = [k for _,k in sorted(zip(bins,dummyMarkerToSort))]
            totalRecords = sum(dpRoot)
            dataToSortSize = len(dataToSort)
            mainData[intervalRootDP] = sorted_data[0:totalRecords].copy()
            leftCacheData = sorted_data[totalRecords:dataToSortSize].copy()
            mainDummyMarker[intervalRootDP] = sorted_marker[0:totalRecords].copy()
            leftCacheDummyMarker = sorted_marker[totalRecords:dataToSortSize].copy()
            
            #runtime
            # 2m(n*#leafs) + O(n*#leafs  * (log(n*#leafs)  ^ 2))
            runtimeSortSize = 2*numBins*dataToSortSize + dataToSortSize*math.log(dataToSortSize, math.e)*math.log(dataToSortSize, math.e)
            runTimeDPSort[i] = round(runtimeSortSize)
       #     print(mainData)
       #     print(leftCacheData)
       #     print(mainDummyMarker)
       #     print(leftCacheDummyMarker)
        else:  # (sortOption == 2)
        #    print(dpRoot)
            dataCache = leftCacheData.copy()
            dummyMarkerCache = leftCacheDummyMarker.copy()
            dataCache.extend(originalData[i])
            dummyMarkerCache.extend(originalDummyMarkers[i])
        #    print("elsedataCache", sorted(dataCache))

            # retrieve the data and DP histograms of previous nodes in this subtree
            # for each bin, we need to put n-d records for each interval together.  
            intervalSize = int(len(intervalPrevious))
            dataMergedPrevious = [None]*intervalSize
            dummyMarkerMergedPrevious = [None]*intervalSize
            dpMergedPrevious = [None]*intervalSize
            for j in range(intervalSize):
                dataMergedPrevious[j] = mainData[intervalPrevious[j]]
                dummyMarkerMergedPrevious[j] = mainDummyMarker[intervalPrevious[j]]
                dpMergedPrevious[j] = computePrefix(dpHists[intervalPrevious[j]])

            encodedRecordsFirst, encodedRecordsSecond = seperateD(dpMergedPrevious, dataMergedPrevious, d, numBins);
            dummyMarkerFirst, dummyMarkerSecond = seperateD(dpMergedPrevious, dummyMarkerMergedPrevious, d, numBins);

            mainData[intervalRootDP] = []
            mainDummyMarker[intervalRootDP] = []
            runtimeSortSize = 0
            for j in range(numBins):
                toSortMergedPrevious = encodedRecordsSecond[j].copy()
                toSortMarkerMergedPrevious = dummyMarkerSecond[j].copy()
                toSortMergedPrevious.extend(dataCache)
                toSortMarkerMergedPrevious.extend(dummyMarkerCache)

                # sort previous node for each bin and cache--> sorted for this bin + leftCache
              #  sizeSort = toSortMergedPrevious.size();
                # compute the number of records we need to retrieve;  
                # ??? what if the sum of n-d for all intervals is larger than dpRoot[j]? 
                # d should be not too small!
                sortDPd = dpRoot[j]
                dataToSortSize = len(toSortMergedPrevious)
                for k in range(int(len(intervalPrevious))):
                    leftAfterCutD = 0 if ((dpHists[intervalPrevious[k]][j] - d) < 0) else (dpHists[intervalPrevious[k]][j] - d);
                    sortDPd = sortDPd - leftAfterCutD
                sortDPd = 0 if (sortDPd < 0) else sortDPd   # todo: increase d if sortDPd<0
         #       print("sortDPd", sortDPd)
         #       print("toSortMergedPrevious", sorted(toSortMergedPrevious))
                # sort 
                bins = computeBinJ(toSortMergedPrevious, toSortMarkerMergedPrevious, sortDPd, j)
                sorted_data = [k for _,k in sorted(zip(bins,toSortMergedPrevious))]
                sorted_marker = [k for _,k in sorted(zip(bins,toSortMarkerMergedPrevious))]
                # n-d
                mainData[intervalRootDP].extend(encodedRecordsFirst[j].copy())
                mainDummyMarker[intervalRootDP].extend(dummyMarkerFirst[j].copy())
                # sorted root d for this bin + left cache
                mainData[intervalRootDP].extend(sorted_data[0:sortDPd].copy())
                dataCache = sorted_data[sortDPd:dataToSortSize].copy()
                mainDummyMarker[intervalRootDP].extend(sorted_marker[0:sortDPd].copy())
                dummyMarkerCache = sorted_marker[sortDPd:dataToSortSize].copy()
                
                runtimeSortSize += 2*1*dataToSortSize + dataToSortSize*math.log(dataToSortSize, math.e)*math.log(dataToSortSize, math.e)

          #      print("dataCache", dataCache)

            leftCacheData = dataCache.copy()
            leftCacheDummyMarker = dummyMarkerCache.copy()
            
            runTimeDPSort[i] = round(runtimeSortSize)
       #     print(mainData)
       #     print(leftCacheData)
       #     print(mainDummyMarker)
       #     print(leftCacheDummyMarker)

        for interval in intervalPrevious:
            mainData.pop(interval, None)
            mainDummyMarker.pop(interval, None)
        
        dummyRecordNumCache[i] = computeDummyRecordsCache(leftCacheDummyMarker)
        intervalss = intervalRangeQ(i) 
      #  print(intervalss)
        trueR = np.array([0]*numBins)
        for interval in intervalss:
            trueR += computeTrueRecords(dpHists[interval], mainData[interval])
        trueRecordNum[i] = trueR
        
    return trueRecordNum, runTimeDPSort, dummyRecordNumCache
        


# step2: dpHistGen for the root of the subtree
def DPTimeLeaf(T, trueHists, eps, numBins):
    dpHists = {}
    for i in range(T):
      #  print(str(i)+"********************************")
        trueHistgrams = trueHists[i]
        dp = trueHistgrams + np.random.laplace(0, (1/eps), numBins)
        intervalDP = str(i) + ',' + str(i)
        dpHists[intervalDP] = nonNegative(dp)
    return dpHists
    #print(trueHists)
    #print(dpHists)
    #print(inconsistDPHists) 


def sortLeaf(T, numBins, dpHists, originalData, originalDummyMarkers):
    leftCacheData = []
    leftCacheDummyMarker = []
    mainData = {}
    mainDummyMarker = {}
    trueRecordNum = [None] * T
    runTimeDPSort = [None] * T
    dummyRecordNumCache = [None] * T
    # step4: get the sorted array of the root node 
    for i in range(T):
      #  print(str(i)+"********************************")
        dataToSort = leftCacheData.copy()
        dummyMarkerToSort = leftCacheDummyMarker.copy()
        dataToSort.extend(originalData[i])
        dummyMarkerToSort.extend(originalDummyMarkers[i])
        
        iStr = str(i) + ',' + str(i)
        dpH = dpHists[iStr]
        bins = computeBin(dataToSort,dummyMarkerToSort,dpH)
        sorted_data = [k for _,k in sorted(zip(bins,dataToSort))]
        sorted_marker = [k for _,k in sorted(zip(bins,dummyMarkerToSort))]
        totalRecords = sum(dpH)
        dataToSortSize = len(dataToSort)

        mainData[iStr] = sorted_data[0:totalRecords].copy()
        leftCacheData = sorted_data[totalRecords:dataToSortSize].copy()
        mainDummyMarker[iStr] = sorted_marker[0:totalRecords].copy()
        leftCacheDummyMarker = sorted_marker[totalRecords:dataToSortSize].copy()
        
        runtimeSortSize = 2*numBins*dataToSortSize + dataToSortSize*math.log(dataToSortSize, math.e)*math.log(dataToSortSize, math.e)
        runTimeDPSort[i] = round(runtimeSortSize)
        dummyRecordNumCache[i] = computeDummyRecordsCache(leftCacheDummyMarker)
      #  print(mainData)
      #  print(leftCacheData)
      #  print(mainDummyMarker)
      #  print(leftCacheDummyMarker)
        
        trueR = np.array([0]*numBins)
        for j in range(i+1):
            jStr = str(j) + ',' + str(j)
            trueR += computeTrueRecords(dpHists[jStr], mainData[jStr])
        trueRecordNum[j] = trueR

    return trueRecordNum, runTimeDPSort, dummyRecordNumCache



def metrics(treeorLeaf, T, epsAll, numReal, sortOption):
    df = readData('nycTaxiData_payment_type.csv')
    numBins = 4
    p = 0.05
    t = math.log((1/p), math.e)
    if treeorLeaf == "tree":
        #tree 
        level = math.log(T, 2)
        eps = epsAll/level  
        numDummy = math.ceil((1/eps) * t) * numBins
        originalData, originalDummyMarkers, trueHists = originalDataMarkerHists(T, numReal, numDummy, numBins, df)
        dpHists = DPTimeTree(T, trueHists, eps, numBins)
        gapAgainThreshold = 1 
#        trueRecordNum, runTimeDPSort, dummyRecordNumCache = sortTree(sortOption, gapAgainThreshold, T, numBins, dpHists, originalData, originalDummyMarkers, eps) # original?
        #print(trueRecordNum)

    else:
        #leaf
        eps = epsAll
        numDummy = math.ceil((1/eps) * t) * numBins
        originalData, originalDummyMarkers, trueHists = originalDataMarkerHists(T, numReal, numDummy, numBins, df)

        dpHistsLeaf = DPTimeLeaf(T, trueHists, eps, numBins)
 #       trueRecordNum, runTimeDPSort, dummyRecordNumCache = sortLeaf(T, numBins, dpHistsLeaf, originalData, originalDummyMarkers)

    DPCount = [None]*T
    trueCount = [None]*T
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
    
   # print("DPCount: ", DPCount)
 #   print("trueCount: ", trueCount)
 #   print("trueRecordNum: ", trueRecordNum)
    
    metricDPError = np.sum(np.abs(np.array(DPCount) - np.array(trueCount)), axis =1)
  #  metricDPStoreError = np.sum(np.abs(np.array(DPCount) - np.array(trueRecordNum)), axis =1)
  #  metricTTStoreError = np.sum(np.abs(np.array(trueCount) - np.array(trueRecordNum)), axis =1)
    
    return metricDPError#, metricDPStoreError, metricTTStoreError, runTimeDPSort, dummyRecordNumCache



def run_all(T_list, epsAll_list, numReal_list, runNum):
    for T in T_list:
        for epsAll in epsAll_list:
            for numReal in numReal_list:
                start_time = time.time()
                list_metricDPError_leaf = [None]*runNum
              #  list_metricDPStoreError_leaf = [None]*runNum
             #   list_metricTTStoreError_leaf = [None]*runNum
             #   list_runTimeDPSort_leaf = [None]*runNum
              #  list_dummyRecordNumCache_leaf = [None]*runNum
                for i in range(runNum):
                    print("leaf -- T: "+str(T)+"; epsAll: "+str(epsAll)+"; N: "+str(numReal)+"; run: "+str(i))
                    metricDPError_leaf = metrics("leaf", T, epsAll, numReal, None)
                    leaf_end_time = time.time()
                    list_metricDPError_leaf[i] = metricDPError_leaf
                  #  list_metricDPStoreError_leaf[i] = metricDPStoreError_leaf
                  #  list_metricTTStoreError_leaf[i] = metricTTStoreError_leaf
                  #  list_runTimeDPSort_leaf[i] = runTimeDPSort_leaf
                  #  list_dummyRecordNumCache_leaf[i] = dummyRecordNumCache_leaf
                print("leaf***************")
                print("--- %s seconds ---" % (leaf_end_time - start_time))

                mean_metricDPError_leaf = np.round(np.mean(list_metricDPError_leaf, axis = 0))
              #  mean_metricDPStoreError_leaf = np.round(np.mean(list_metricDPStoreError_leaf, axis = 0))
              #  mean_metricTTStoreError_leaf = np.round(np.mean(list_metricTTStoreError_leaf, axis = 0))
              #  mean_runTimeDPSort_leaf = np.round(np.mean(list_runTimeDPSort_leaf, axis = 0))
              #  mean_dummyRecordNumCache_leaf = np.round(np.mean(list_dummyRecordNumCache_leaf, axis = 0))



                list_metricDPError_treeD = [None]*runNum
             #   list_metricDPStoreError_treeD = [None]*runNum
             #   list_metricTTStoreError_treeD = [None]*runNum
             #   list_runTimeDPSort_treeD = [None]*runNum
             #   list_dummyRecordNumCache_treeD = [None]*runNum
                for i in range(runNum):
                    print("treeD -- T: "+str(T)+"; epsAll: "+str(epsAll)+"; N: "+str(numReal)+"; run: "+str(i))
                    metricDPError_treeD = metrics("tree", T, epsAll, numReal, 2)
                    treeD_end_time = time.time()
                    list_metricDPError_treeD[i] = metricDPError_treeD
                 #   list_metricDPStoreError_treeD[i] = metricDPStoreError_treeD
                 #   list_metricTTStoreError_treeD[i] = metricTTStoreError_treeD
                 #   list_runTimeDPSort_treeD[i] = runTimeDPSort_treeD
                 #   list_dummyRecordNumCache_treeD[i] = dummyRecordNumCache_treeD
                print("treeD***************")
                print("--- %s seconds ---" % (treeD_end_time - start_time))
                mean_metricDPError_treeD = np.round(np.mean(list_metricDPError_treeD, axis = 0))
             #   mean_metricDPStoreError_treeD = np.round(np.mean(list_metricDPStoreError_treeD, axis = 0))
             #   mean_metricTTStoreError_treeD = np.round(np.mean(list_metricTTStoreError_treeD, axis = 0))
             #   mean_runTimeDPSort_treeD = np.round(np.mean(list_runTimeDPSort_treeD, axis = 0))
             #   mean_dummyRecordNumCache_treeD = np.round(np.mean(list_dummyRecordNumCache_treeD, axis = 0))


                list_metricDPError_treeA = [None]*runNum
              #  list_metricDPStoreError_treeA = [None]*runNum
              #  list_metricTTStoreError_treeA = [None]*runNum
              #  list_runTimeDPSort_treeA = [None]*runNum
              #  list_dummyRecordNumCache_treeA = [None]*runNum
                for i in range(runNum):
                    print("treeA -- T: "+str(T)+"; epsAll: "+str(epsAll)+"; N: "+str(numReal)+"; run: "+str(i))
                    metricDPError_treeA = metrics("tree", T, epsAll, numReal, 0)
                    treeA_end_time = time.time()
                    list_metricDPError_treeA[i] = metricDPError_treeA
                #    list_metricDPStoreError_treeA[i] = metricDPStoreError_treeA
                #    list_metricTTStoreError_treeA[i] = metricTTStoreError_treeA
                #    list_runTimeDPSort_treeA[i] = runTimeDPSort_treeA
                #    list_dummyRecordNumCache_treeA[i] = dummyRecordNumCache_treeA
                print("treeA***************")
                print("--- %s seconds ---" % (treeA_end_time - start_time))
                mean_metricDPError_treeA = np.round(np.mean(list_metricDPError_treeA, axis = 0))
              #  mean_metricDPStoreError_treeA = np.round(np.mean(list_metricDPStoreError_treeA, axis = 0))
              #  mean_metricTTStoreError_treeA = np.round(np.mean(list_metricTTStoreError_treeA, axis = 0))
              #  mean_runTimeDPSort_treeA = np.round(np.mean(list_runTimeDPSort_treeA, axis = 0))
              #  mean_dummyRecordNumCache_treeA = np.round(np.mean(list_dummyRecordNumCache_treeA, axis = 0))


                fileName = "newResultsPY/DP-T:"+str(T)+",eps:"+str(epsAll)+",N:"+str(numReal)+".json"

                with open(fileName, 'w') as f:
                    entry = {}
                    entry['list_metricDPError_leaf'] = mean_metricDPError_leaf.tolist()
               #     entry['list_metricDPStoreError_leaf'] = mean_metricDPStoreError_leaf.tolist()
               #     entry['list_metricTTStoreError_leaf'] = mean_metricTTStoreError_leaf.tolist()
               #     entry['list_runTimeDPSort_leaf'] = mean_runTimeDPSort_leaf.tolist()
               #     entry['list_dummyRecordNumCache_leaf'] = mean_dummyRecordNumCache_leaf.tolist()

                    entry['list_metricDPError_treeA'] = mean_metricDPError_treeA.tolist()
               #     entry['list_metricDPStoreError_treeA'] = mean_metricDPStoreError_treeA.tolist()
               #     entry['list_metricTTStoreError_treeA'] = mean_metricTTStoreError_treeA.tolist()
               #     entry['list_runTimeDPSort_treeA'] = mean_runTimeDPSort_treeA.tolist()
               #     entry['list_dummyRecordNumCache_treeA'] = mean_dummyRecordNumCache_treeA.tolist()

                    entry['list_metricDPError_treeD'] = mean_metricDPError_treeD.tolist()
               #     entry['list_metricDPStoreError_treeD'] = mean_metricDPStoreError_treeD.tolist()
               #     entry['list_metricTTStoreError_treeD'] = mean_metricTTStoreError_treeD.tolist()
               #     entry['list_runTimeDPSort_treeD'] = mean_runTimeDPSort_treeD.tolist()
               #     entry['list_dummyRecordNumCache_treeD'] = mean_dummyRecordNumCache_treeD.tolist()
                    json.dump(entry, f, ensure_ascii=False)


T_list = [10000]
epsAll_list = [10, 1, 0.1]
numReal_list = [100, 1000, 10000]
runNum = 3

run_all(T_list, epsAll_list, numReal_list, runNum)
