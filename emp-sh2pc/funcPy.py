import pandas as pd
import numpy as np
import math
import time
import json

def readData(fileName):
    df = pd.read_csv(fileName)   # [1271413 rows x 1 columns]
    df = df.to_numpy().flatten()
    df = df.tolist()
    for i in range(4):
        df.extend(df)
    return df
    
def nonNegative(value):
    return [int(round(v)) if v>0 else 0 for v in value]

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
    vectSecond = []
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
        for j in range(intervals):  # for each interval
            left = dpHistPrefixIntrevals[j][i+1] - d if (dpHistPrefixIntrevals[j][i] < dpHistPrefixIntrevals[j][i+1] - d) else dpHistPrefixIntrevals[j][i]
            begining = dataMergedPrevious[j][dpHistPrefixIntrevals[j][i]: left].copy()
            ending = dataMergedPrevious[j][left: dpHistPrefixIntrevals[j][i+1]].copy()
            first.extend(begining)  # the first n-d
            vectSecond.extend(ending)   # the last d 
        vectFirst[i] = first.copy()

    return (vectFirst, vectSecond)  

def seperateBin(sorted_data, sortDPdHist):
    numBin = len(sortDPdHist)
    seperatedData = [None]*numBin
    dpHistPrefix_tmp = computePrefix(sortDPdHist)
    dpHistPrefix_tmp[numBin-1] = len(sorted_data)  
    dpHistPrefix = [0]*(numBin+1)
    for j in range(numBin):
        dpHistPrefix[j+1] = dpHistPrefix_tmp[j]
    for j in range(numBin):
        seperatedData[j] = sorted_data[dpHistPrefix[j]: dpHistPrefix[j+1]].copy()

    return seperatedData 


# for each time unit, insert real and dummy records, and compute true histogram
def originalDataMarkerHistsTree(T, numReal, num_Dummy, numBins, df):
    originalData = {}
    originalDummyMarkers = {}
    trueHists = [None] * T
    for i in range(T):
        numDummy = num_Dummy
        if (i%2 == 1):
            numDummy = 0
        # for originalData
        records = [None] * (numReal+numDummy)
        records[0: numReal] = df[i*numReal: (i+1)*numReal].copy()
        records[numReal: numReal+numDummy] = [1000] * numDummy #todo change mpc code
        originalData[i] = records
        # for originalDummyMarker
        DummyMarker = [None] * (numReal+numDummy)
        DummyMarker[0: numReal] = [1] * numReal
        DummyMarker[numReal: numReal+numDummy] = [2] * numDummy #todo change mpc code
        originalDummyMarkers[i] = DummyMarker
        # compute trueHists
        counts, bins = np.histogram(records, bins=np.arange(1,numBins+2)) #[1,2,3,4,5] -> 4bins
        trueHists[i] = counts 
 #   print(originalData)
 #   print(originalDummyMarkers)
 #   print(trueHists)
    return originalData, originalDummyMarkers, trueHists

def originalDataMarkerHistsLeaf(p, eps, T, numReal, numBins, df):
    originalData = {}
    originalDummyMarkers = {}
    trueHists = [None] * T
    dummy_leaf = 0
    t_ = math.log((1/p), math.e)
    b = 1/eps
    for i in range(T):
        a = round(2*b*math.sqrt((i+1)*math.log((1/p), math.e)))
        if (a > i*b):
            numDummy = round((1/eps) * t_) * numBins
        else:
            numDummy = round(a*numBins-dummy_leaf) if round(a*numBins-dummy_leaf) >= 0 else 0 
        dummy_leaf += numDummy
        
        # for originalData
        records = [None] * (numReal+numDummy)
        records[0: numReal] = df[i*numReal: (i+1)*numReal].copy()
        records[numReal: numReal+numDummy] = [1000] * numDummy #todo change mpc code
        originalData[i] = records
        # for originalDummyMarker
        DummyMarker = [None] * (numReal+numDummy)
        DummyMarker[0: numReal] = [1] * numReal
        DummyMarker[numReal: numReal+numDummy] = [2] * numDummy #todo change mpc code
        originalDummyMarkers[i] = DummyMarker
        # compute trueHists
        counts, bins = np.histogram(records, bins=np.arange(1,numBins+2)) #[1,2,3,4,5] -> 4bins
        trueHists[i] = counts 
 #   print(originalData)
 #   print(originalDummyMarkers)
 #   print(trueHists)
    return originalData, originalDummyMarkers, trueHists
 
    
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

def computeTrueRecordsRange(dpHist, dpStore):
    binNum = len(dpHist)
    recordNum = len(dpStore) 
            
    dpHistPrefix_tmp = computePrefix(dpHist)
    dpHistPrefix_tmp[binNum-1] = recordNum  
    dpHistPrefix = [0]*(binNum+1)
    for j in range(binNum):
        dpHistPrefix[j+1] = dpHistPrefix_tmp[j]
    trueR = []
    idx = 0 
    for j in range(binNum):
        for k in range(j, binNum, 1): # all range queries [j, k]
            trueR.append(0) 
            for l in range(dpHistPrefix[j], dpHistPrefix[k+1], 1):
                for m in range(j, (k+1), 1): 
                    if ((dpStore[l]-1) == m):
                        trueR[idx] = trueR[idx]+1

            idx = idx+1
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
            dpNode = np.array([float(round(e)) for e in dpNode])
           # print(dpNode)
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

def sortTree(num_dummy, sortOption, gapAgainThreshold, T, numBins, dpHists, originalData, originalDummyMarkers, eps):
    t = math.log((1/0.005), math.e)
    d = math.ceil((1/eps) * t)
    leftCacheData = []
    leftCacheDummyMarker = []
    mainData = {}
    mainDummyMarker = {}
    trueRecordNum = [None] * T
    trueRecordNumRange = [None] * T
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
            rootLeftSort += int(gapSort / 2)
            gapSort /= 2
        #print(intervalPrevious)
        # step4.3: get the sorted array of the root node
        # option0: if sortOption == 0 or gapAgain <= x
        # option1: else if sortOption == 1 
        # option2: else sortOption == 2 
        
        
        numDrop = (len(intervalPrevious) - 1) if len(intervalPrevious) > 0 else 0
        dropDummy = num_dummy * numDrop
       # print("dropDummy: ", dropDummy)
    
        if (sortOption == 0 or gapAgainAgain <= gapAgainThreshold):
            dataToSort = leftCacheData.copy()
            dummyMarkerToSort = leftCacheDummyMarker.copy();
            for interval in intervalPrevious:
                dataToSort.extend(mainData[interval])
                dummyMarkerToSort.extend(mainDummyMarker[interval])
            dataToSort.extend(originalData[i])
            dummyMarkerToSort.extend(originalDummyMarkers[i])


         #   print(dpRoot)
            bins = computeBin(dataToSort,dummyMarkerToSort,dpRoot)
            sorted_data = [k for _,k in sorted(zip(bins,dataToSort))]
            sorted_marker = [k for _,k in sorted(zip(bins,dummyMarkerToSort))]
            totalRecords = sum(dpRoot)
            dataToSortSize = len(dataToSort)
            sizeDroppedDummy = dataToSortSize - dropDummy
            mainData[intervalRootDP] = sorted_data[0:totalRecords].copy()
            leftCacheData = sorted_data[totalRecords:sizeDroppedDummy].copy()
            mainDummyMarker[intervalRootDP] = sorted_marker[0:totalRecords].copy()
            leftCacheDummyMarker = sorted_marker[totalRecords:sizeDroppedDummy].copy()
            
            #runtime
            # 2m(n*#leafs) + O(n*#leafs  * (log(n*#leafs)  ^ 2))
            # have 
            runtimeSortSize = dataToSortSize*math.log(dataToSortSize, math.e)*math.log(dataToSortSize, math.e)
            runTimeDPSort[i] = round(runtimeSortSize)
           # print(mainData)
           # print(leftCacheData)
          #  print(mainDummyMarker)
          #  print(leftCacheDummyMarker)
        else:  # (sortOption == 2)
           # print(dpRoot)
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

            encodedRecordsSecond.extend(dataCache)
            dummyMarkerSecond.extend(dummyMarkerCache)
            
            sortDPdHist = [None]*numBins
            for j in range(numBins):
                # sort previous node for each bin and cache--> sorted for this bin + leftCache
              #  sizeSort = toSortMergedPrevious.size();
                # compute the number of records we need to retrieve;  
                # ??? what if the sum of n-d for all intervals is larger than dpRoot[j]? 
                # d should be not too small!
                sortDPd = dpRoot[j]
                for k in range(int(len(intervalPrevious))):
                    leftAfterCutD = 0 if ((dpHists[intervalPrevious[k]][j] - d) < 0) else (dpHists[intervalPrevious[k]][j] - d);
                    sortDPd = sortDPd - leftAfterCutD
                sortDPd = 0 if (sortDPd < 0) else sortDPd   # todo: increase d if sortDPd<0
         #       print("sortDPd", sortDPd)
         #       print("toSortMergedPrevious", sorted(toSortMergedPrevious))
                # sort 
                sortDPdHist[j] = sortDPd
           
            bins = computeBin(encodedRecordsSecond, dummyMarkerSecond, sortDPdHist)
            sorted_data = [k for _,k in sorted(zip(bins,encodedRecordsSecond))]
            sorted_marker = [k for _,k in sorted(zip(bins,dummyMarkerSecond))]
            
            mainData[intervalRootDP] = []
            mainDummyMarker[intervalRootDP] = []
            
            totalRecords = sum(sortDPdHist)
            dataToSortSize = len(encodedRecordsSecond)
            sizeDroppedDummy = dataToSortSize - dropDummy
            
            # sorted root d for this bin + left cache
            seperatedBinsRecord = seperateBin(sorted_data[0:totalRecords], sortDPdHist)
            seperatedBinsDummyMarker = seperateBin(sorted_marker[0:totalRecords], sortDPdHist)
            for j in range(numBins):
                # n-d
                mainData[intervalRootDP].extend(encodedRecordsFirst[j].copy())
                mainDummyMarker[intervalRootDP].extend(dummyMarkerFirst[j].copy())
                
                mainData[intervalRootDP].extend(seperatedBinsRecord[j].copy())
                mainDummyMarker[intervalRootDP].extend(seperatedBinsDummyMarker[j].copy())
            
            leftCacheData = sorted_data[totalRecords:sizeDroppedDummy].copy()
            leftCacheDummyMarker = sorted_marker[totalRecords:sizeDroppedDummy].copy()
            #runtime
            # O(n*#leafs  * (log(n*#leafs)  ^ 2))
            runtimeSortSize = dataToSortSize*math.log(dataToSortSize, math.e)*math.log(dataToSortSize, math.e)
            runTimeDPSort[i] = round(runtimeSortSize)
            
          #      print("dataCache", dataCache)
         #   print(mainData)
         #   print(leftCacheData)
         #   print(mainDummyMarker)
         #   print(leftCacheDummyMarker)

        for interval in intervalPrevious:
            mainData.pop(interval, None)
            mainDummyMarker.pop(interval, None)
        
        dummyRecordNumCache[i] = computeDummyRecordsCache(leftCacheDummyMarker)
        intervalss = intervalRangeQ(i) 
        rangeQuery= []
        for j in range(numBins):   
            for k in range(j, numBins, 1):
                vect = [j, k]
                rangeQuery.append(vect)
        querySize = len(rangeQuery)
      #  print(intervalss)
        trueR = np.array([0]*numBins)
        RangetrueR = np.array([0]*querySize)
        for interval in intervalss:
            trueR += computeTrueRecords(dpHists[interval], mainData[interval])
            RangetrueR += computeTrueRecordsRange(dpHists[interval], mainData[interval])
        trueRecordNum[i] = trueR
        trueRecordNumRange[i] = RangetrueR

    return trueRecordNum, trueRecordNumRange, runTimeDPSort, dummyRecordNumCache
        
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
    trueRecordNumRange = [None] * T
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
        rangeQuery= []
        for j in range(numBins):   
            for k in range(j, numBins, 1):
                vect = [j, k]
                rangeQuery.append(vect)
        querySize = len(rangeQuery)
        trueR = np.array([0]*numBins)
        RangetrueR = np.array([0]*querySize)
        for j in range(i+1):
            jStr = str(j) + ',' + str(j)
            trueR += computeTrueRecords(dpHists[jStr], mainData[jStr])
            RangetrueR += computeTrueRecordsRange(dpHists[jStr], mainData[jStr])
        trueRecordNum[j] = trueR
        trueRecordNumRange[i] = RangetrueR
    return trueRecordNum, trueRecordNumRange, runTimeDPSort, dummyRecordNumCache

