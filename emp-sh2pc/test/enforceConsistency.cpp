#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
using namespace std;

std::vector<int> addTwoVectors(std::vector<int> vect1, std::vector<int> vect2){
  std::vector<int> res;
  for (int i = 0; i < vect1.size(); i++) {
    res.push_back(vect1[i] + vect2[i]);
  }
  return res;
}

std::vector<int> minusTwoVectors(std::vector<int> vect1, std::vector<int> vect2){
  std::vector<int> res;
  for (int i = 0; i < vect1.size(); i++) {
    res.push_back(vect1[i] - vect2[i]);
  }
  return res;
}

int nodesSubtree(int i){
  i += 1;
  int j = 1;
  int k = i;
  int rootLeft = i;
  while (k % 2 == 0) {
    rootLeft = i - std::pow(2, j) + 1;
    k = k / 2;
    j += 1;
  }
  return rootLeft - 1;
}

std::vector<int> nonNegative(std::vector<int> vect){
  std::vector<int> res;
  for (int i = 0; i < vect.size(); i++) {
    if (vect[i] < 0){
      res.push_back(0);
    } else {
      res.push_back(vect[i]);
    }
  }
  return res;
}

std::vector<int> enforceConsistency(std::vector<std::vector<int> > dpNodes, std::vector<std::vector<int> > constant){
  int binSize = dpNodes[0].size();
  int nodeSize = dpNodes.size();
  int constSize = constant.size();

  if (constSize == 0) {
    return dpNodes[nodeSize - 1];
  }
  std::vector<int> dp;
  for (int i = 0; i < binSize; i++) {
    int dpCounts = 0;
    for (int j = 0; j < nodeSize; j++) {
      dpCounts += dpNodes[j][i];
    }

    int constCounts = 0;
    for (int j = 0; j < constSize; j++) {
      constCounts += constant[j][i] * (j + 1);
    }

    int dpConsistent = int((dpCounts - constCounts) / nodeSize);
    dp.push_back(dpConsistent);
  }     

  return dp;
}

