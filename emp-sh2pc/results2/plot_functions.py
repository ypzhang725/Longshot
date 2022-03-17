import math
import matplotlib.pyplot as plt
import numpy as np
import re


def plotDP(T, arrayLeaf, arrayTree_sortSubroot, arrayTree_sortD, arrayLeaf_store, arrayTree_sortSubroot_store, arrayTree_sortD_store, xlable, ylable, title):
    xpoints = np.linspace(0, T, T, endpoint=True)
    y1points = arrayLeaf
    y2points = arrayTree_sortSubroot
    y3points = arrayTree_sortD
    
    y4points = arrayLeaf_store
    y5points = arrayTree_sortSubroot_store
    y6points = arrayTree_sortD_store

    #plt.subplot(1, 1, 1)
    plt.plot(xpoints, y1points, 'b^', label="CountDP_leaf")
    plt.plot(xpoints, y2points, 'r^', label="CountDP_tree_sortSubRoot")
    plt.plot(xpoints, y3points, 'g^', label="CountDP_tree_sortD")
    
    plt.plot(xpoints, y4points, 'bv', label="StoreDP_leaf")
    plt.plot(xpoints, y5points, 'rv', label="StoreDP_tree_sortSubRoot")
    plt.plot(xpoints, y6points, 'gv', label="StoreDP_tree_sortD")

    plt.xlabel(xlable)
    plt.ylabel(ylable)
    plt.legend(loc="upper left", prop={'size': 8})

    plt.title(title)

    plt.show()

#https://matplotlib.org/stable/api/_as_gen/matplotlib.pyplot.plot.html


def plotSort(T, arrayLeaf, arrayTree_sortSubroot, arrayTree_sortD, xlable, ylable, title):
    xpoints = np.linspace(0, T, T, endpoint=True)
    y1points = arrayLeaf
    y2points = arrayTree_sortSubroot
    y3points = arrayTree_sortD
    
    #plt.subplot(1, 1, 1)
    plt.plot(xpoints, y1points, 'b^', label="SortTime_leaf")
    plt.plot(xpoints, y2points, 'r^', label="SortTime_tree_sortSubRoot")
    plt.plot(xpoints, y3points, 'g^', label="SortTime_tree_sortD")

    plt.xlabel(xlable)
    plt.ylabel(ylable)
    plt.legend(loc="upper left", prop={'size': 8})

    plt.title(title)

    plt.show()


def readFileReturnMean(fileName):
    with open(fileName) as f:
        lines = f.readlines()
    runs = int(len(lines) / 7)
   # print(runs)
   # for i in lines:
   #     print(i)

    metricRunTimeDP = [None]*runs
    metricRunTimeDPSort = [None]*runs
    metricRunTimeDPMerge = [None]*runs
    metricDPError = [None]*runs
    metricDPStoreError = [None]*runs
    metricTTStoreError = [None]*runs
    
    for i in range(runs):
        metricRunTimeDP_string = lines[(i*6) + 1]
        metricRunTimeDP_string_result = re.search('metricRunTimeDP: (.*)\n', metricRunTimeDP_string).group(1)
        metricRunTimeDP[i] = [int(e) for e in metricRunTimeDP_string_result.split(",")]
        
        metricRunTimeDPSort_string = lines[(i*6) + 2]
        metricRunTimeDPSort_string_result = re.search('metricRunTimeDPSort: (.*)\n', metricRunTimeDPSort_string).group(1)
        metricRunTimeDPSort[i] = [int(e) for e in metricRunTimeDPSort_string_result.split(",")]
        
        metricRunTimeDPMerge_string = lines[(i*6) + 3]
        metricRunTimeDPMerge_string_result = re.search('metricRunTimeDPMerge: (.*)\n', metricRunTimeDPMerge_string).group(1)
        metricRunTimeDPMerge[i] = [int(e) for e in metricRunTimeDPMerge_string_result.split(",")]
        
        metricDPError_string = lines[(i*6) + 4]
        metricDPError_string_result = re.search('metricDPError: (.*)\n', metricDPError_string).group(1)
        metricDPError[i] = [int(e) for e in metricDPError_string_result.split(",")]
        
        metricDPStoreError_string = lines[(i*6) + 5]
        metricDPStoreError_string_result = re.search('metricDPStoreError: (.*)\n', metricDPStoreError_string).group(1)
        metricDPStoreError[i] = [int(e) for e in metricDPStoreError_string_result.split(",")]
        
        metricTTStoreError_string = lines[(i*6) + 6]
        metricTTStoreError_string_result = re.search('metricTTStoreError: (.*)\n', metricTTStoreError_string).group(1)
        metricTTStoreError[i] = [int(e) for e in metricTTStoreError_string_result.split(",")]
        
    metricRunTimeDP_mean = np.mean(np.array(metricRunTimeDP), axis = 0)
    metricRunTimeDPSort_mean = np.mean(np.array(metricRunTimeDPSort), axis = 0)
    metricRunTimeDPMerge_mean = np.mean(np.array(metricRunTimeDPMerge), axis = 0)
    metricDPError_mean = np.mean(np.array(metricDPError), axis = 0)
    metricDPStoreError_mean = np.mean(np.array(metricDPStoreError), axis = 0)
    metricTTStoreError_mean = np.mean(np.array(metricTTStoreError), axis = 0)
    
    d = dict(); 
    d['metricRunTimeDP_mean'] = metricRunTimeDP_mean
    d['metricRunTimeDPSort_mean'] = metricRunTimeDPSort_mean
    d['metricRunTimeDPMerge_mean'] = metricRunTimeDPMerge_mean
    d['metricDPError_mean'] = metricDPError_mean
    d['metricDPStoreError_mean'] = metricDPStoreError_mean
    d['metricTTStoreError_mean'] = metricTTStoreError_mean

    return d

    '''
    print(metricRunTimeDPSort)
    print(metricDPError)
    print(metricDPStoreError)
    print(metricRunTimeDPSort_mean)
    print(metricDPError_mean)
    print(metricDPStoreError_mean)
    return metricRunTimeDPSort_mean, metricDPError_mean, metricDPStoreError_mean
    '''


def plots(T, eps, N):
    # create file names
    T_string = str(T)+","
    eps_string = str(eps)+","
    N_string = str(N)
    fileNameLeaf = "leaftaxi_ss1.txt,"+T_string+eps_string+N_string+";1.txt"
    fileNameTreeSortRoot = "treetaxi_ss1.txt,"+T_string+eps_string+N_string+",0;1.txt"
    fileNameTreeSortD = "treetaxi_ss1.txt,"+T_string+eps_string+N_string+",2;1.txt"

    # read files, parse and return mean
    returnedMeanLeaf = readFileReturnMean(fileNameLeaf)
    returnedMeanSortRoot = readFileReturnMean(fileNameTreeSortRoot)
    returnedMeanSortD = readFileReturnMean(fileNameTreeSortD)

    # three metrics
        #leaf 
    LeafRunTimeDPSort_mean = returnedMeanLeaf["metricRunTimeDPSort_mean"]
    LeafDPError_mean = returnedMeanLeaf["metricDPError_mean"]
    LeafDPStoreError_mean = returnedMeanLeaf["metricDPStoreError_mean"]
        #tree_sortRoot
    SortRootRunTimeDPSort_mean = returnedMeanSortRoot["metricRunTimeDPSort_mean"]
    SortRootDPError_mean = returnedMeanSortRoot["metricDPError_mean"]
    SortRootDPStoreError_mean = returnedMeanSortRoot["metricDPStoreError_mean"]
        #tree_sortD
    SortDRunTimeDPSort_mean = returnedMeanSortD["metricRunTimeDPSort_mean"]
    SortDDPError_mean = returnedMeanSortD["metricDPError_mean"]
    SortDDPStoreError_mean = returnedMeanSortD["metricDPStoreError_mean"]

    title = "T:"+str(T)+"; eps:"+str(eps)+"; N:"+str(N)
    plotSort(T, LeafRunTimeDPSort_mean, SortRootRunTimeDPSort_mean, SortDRunTimeDPSort_mean, "time", "sortTime(s)", title)
    plotDP(T, LeafDPError_mean, SortRootDPError_mean, SortDDPError_mean, LeafDPStoreError_mean, SortRootDPStoreError_mean, SortDDPStoreError_mean, "time", "accuracy", title)




