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

def computeDPCountMark(originalData, originalDummyMarkers, epsilon, bins):
    # compute DP Count and mark records 
    trueCount = 0
    for count, value in enumerate(originalData):
        same = False
        for b in bins:
            if (value-1) == b:
                eq = True
                same = same or eq
        if same:
            trueCount = trueCount + 1
            originalDummyMarkers[count] = 0
    DPCount = int(trueCount + 2 * np.random.laplace(0, (1/epsilon), 1)) #np.random.laplace(0, (1/eps), numBins)
    TrueRecord = DPCount if DPCount <= trueCount else trueCount
     
    return DPCount, originalDummyMarkers, trueCount, TrueRecord

def sortOneBinDP(originalData, originalDummyMarkers, DPCount):
    sorted_data = [k for _,k in sorted(zip(originalDummyMarkers,originalData))]
    sorted_marker = [k for _,k in sorted(zip(originalDummyMarkers,originalDummyMarkers))]
    sorted_data = sorted_data[0:DPCount].copy()
    sorted_marker = sorted_marker[0:DPCount].copy()
    return sorted_data, sorted_marker

def processQuery(originalData, originalDummyMarkers, epsilon, bins):
    DPCount, originalDummyMarkers, trueCount, TrueRecord = computeDPCountMark(originalData, originalDummyMarkers, epsilon, bins)
    sorted_data, sorted_marker = sortOneBinDP(originalData, originalDummyMarkers, DPCount)       
    '''
    print("originalData", originalData)
    print("originalDummyMarkers", originalDummyMarkers)
    print("DPCount", DPCount)
    print("trueCount", trueCount)
    print("TrueRecord", TrueRecord)
    print("sorted_data", sorted_data)
    print("sorted_marker", sorted_marker)
    '''
    return DPCount, trueCount, TrueRecord  

def metricBaseline(bins, epsilon, T, numReal):
    metricDPErrorPoint = [0] * T          # |DP count - true count|
    metricDPStoreErrorPoint = [0] * T     # |DP count - true record|
    metricTTStoreErrorPoint = [0] * T     # |true count - true record|

    metricDPErrorRange = [0] * T          # |DPcount - true count|
    metricDPStoreErrorRange = [0] * T     # |DP count - true record|
    metricTTStoreErrorRange = [0] * T     # |true count - true record|

    originalData = {}
    originalDummyMarkers = {}
    df = readData('bin40.csv')
    for i in range(T):
        # for originalData
        records = [None] * numReal
        records[0: numReal] = df[i*numReal: (i+1)*numReal].copy()
        originalData[i] = records
        # for originalDummyMarker
        DummyMarker = [None] * numReal 
        DummyMarker[0: numReal] = [1] * numReal   
        originalDummyMarkers[i] = DummyMarker

        #process data
        tempOriginalData = []
        tempOriginalDummyMarkers = []
        for j in range(i+1):
            tempOriginalData.extend(originalData[j])
            tempOriginalDummyMarkers.extend(originalDummyMarkers[j])
        
        #point query 
        DPCountPoint = [0]*bins
        TrueCountPoint = [0]*bins
        TrueRecordPoint = [0]*bins
        for j in range(bins):
            binsPoint = [j, j]
            DPCount, trueCount, TrueRecord = processQuery(tempOriginalData.copy(), tempOriginalDummyMarkers.copy(), epsilon, binsPoint)
            DPCountPoint[j] = DPCount
            TrueCountPoint[j] = trueCount
            TrueRecordPoint[j] = TrueRecord
        DPStoreErrorPoint = 0
        DPErrorPoint = 0
        TTStoreErrorPoint = 0
        for (j, b) in enumerate(range(bins)): 
            DPStoreErrorPoint += abs(DPCountPoint[j] - TrueRecordPoint[j])
            DPErrorPoint += abs(DPCountPoint[j] - TrueCountPoint[j])
            TTStoreErrorPoint += abs(TrueCountPoint[j] - TrueRecordPoint[j])

        metricDPErrorPoint[i] = DPErrorPoint;
        metricDPStoreErrorPoint[i] = DPStoreErrorPoint;
        metricTTStoreErrorPoint[i] = TTStoreErrorPoint;


        #range query 
        rangeQuery= []
        for j in range(bins):   
            for k in range(j, bins, 1):
                vect = [j, k]
                rangeQuery.append(vect)
        querySize = len(rangeQuery)
        DPCountRange = [0]*querySize
        TrueCountRange = [0]*querySize
        TrueRecordRange = [0]*querySize
        idx_Range = 0
        for (j, binsPoint) in enumerate(rangeQuery): 
           # print("binsPoint: ", binsPoint)
            DPCount, trueCount, TrueRecord = processQuery(tempOriginalData.copy(), tempOriginalDummyMarkers.copy(), epsilon, binsPoint)
            DPCountRange[j] = DPCount
            TrueCountRange[j] = trueCount
            TrueRecordRange[j] = TrueRecord
        DPStoreErrorRange = 0
        DPErrorRange = 0
        TTStoreErrorRange = 0
        for (j, b) in enumerate(range(querySize)): 
            DPStoreErrorRange += abs(DPCountRange[j] - TrueRecordRange[j])
            DPErrorRange += abs(DPCountRange[j] - TrueCountRange[j])
            TTStoreErrorRange += abs(TrueCountRange[j] - TrueRecordRange[j])

        metricDPErrorRange[i] = DPErrorRange
        metricDPStoreErrorRange[i] = DPStoreErrorRange
        metricTTStoreErrorRange[i] = TTStoreErrorRange
    return metricDPErrorPoint, metricDPStoreErrorPoint, metricTTStoreErrorPoint, metricDPErrorRange, metricDPStoreErrorRange, metricTTStoreErrorRange       


bins = 40
epsilon = 1
T = 10
numReal = 1000  
runNum = 3

list_metricDPErrorPoint_Baseline = [None]*runNum
list_metricDPStoreErrorPoint_Baseline = [None]*runNum
list_metricTTStoreErrorPoint_Baseline = [None]*runNum
list_metricDPErrorRange_Baseline = [None]*runNum
list_metricDPStoreErrorRange_Baseline = [None]*runNum
list_metricTTStoreErrorRange_Baseline = [None]*runNum
for i in range(runNum):
    metricDPErrorPoint, metricDPStoreErrorPoint, metricTTStoreErrorPoint, metricDPErrorRange, metricDPStoreErrorRange, metricTTStoreErrorRange = metricBaseline(bins, epsilon, T, numReal)
    list_metricDPErrorPoint_Baseline[i] = metricDPErrorPoint
    list_metricDPStoreErrorPoint_Baseline[i] = metricDPStoreErrorPoint
    list_metricTTStoreErrorPoint_Baseline[i] = metricTTStoreErrorPoint
    list_metricDPErrorRange_Baseline[i] = metricDPErrorRange
    list_metricDPStoreErrorRange_Baseline[i] = metricDPStoreErrorRange
    list_metricTTStoreErrorRange_Baseline[i] = metricTTStoreErrorRange
mean_metricDPErrorPoint_Baseline = np.round(np.mean(list_metricDPErrorPoint_Baseline, axis = 0))
mean_metricDPStoreErrorPoint_Baseline = np.round(np.mean(list_metricDPStoreErrorPoint_Baseline, axis = 0))
mean_metricTTStoreErrorPoint_Baseline = np.round(np.mean(list_metricTTStoreErrorPoint_Baseline, axis = 0))
mean_metricDPErrorRange_Baseline = np.round(np.mean(list_metricDPErrorRange_Baseline, axis = 0))
mean_metricDPStoreErrorRange_Baseline = np.round(np.mean(list_metricDPStoreErrorRange_Baseline, axis = 0))
mean_metricTTStoreErrorRange_Baseline = np.round(np.mean(list_metricTTStoreErrorRange_Baseline, axis = 0))

fileName = "bin40ResultsPY/baseline-T:"+str(T)+",eps:"+str(epsilon)+",N:"+str(numReal)+".json"

with open(fileName, 'w') as f:
    entry = {}
    entry['list_metricDPErrorPoint_Baseline'] = mean_metricDPErrorPoint_Baseline.tolist()
    entry['list_metricDPStoreErrorPoint_Baseline'] = mean_metricDPStoreErrorPoint_Baseline.tolist()
    entry['list_metricTTStoreErrorPoint_Baseline'] = mean_metricTTStoreErrorPoint_Baseline.tolist()
    entry['list_metricDPErrorRange_Baseline'] = mean_metricDPErrorRange_Baseline.tolist()
    entry['list_metricDPStoreErrorRange_Baseline'] = mean_metricDPStoreErrorRange_Baseline.tolist()
    entry['list_metricTTStoreErrorRange_Baseline'] = mean_metricTTStoreErrorRange_Baseline.tolist()
    json.dump(entry, f, ensure_ascii=False)

