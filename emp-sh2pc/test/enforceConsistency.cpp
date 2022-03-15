#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
using namespace std;

std::vector<int> addTwoVectors(std::vector<int> vect1, std::vector<int> vect2){
  int vectSize = int(vect1.size());
  std::vector<int> res(vectSize);
  for (int i = 0; i < vectSize; i++) {
    res[i] = vect1[i] + vect2[i];
  }
  return res;
}

std::vector<int> minusTwoVectors(std::vector<int> vect1, std::vector<int> vect2){
  int vectSize = int(vect1.size());
  std::vector<int> res(vectSize);
  for (int i = 0; i < vectSize; i++) {
    res[i] = vect1[i] - vect2[i];
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
  int vectSize = int(vect.size());
  std::vector<int> res(vectSize);
  for (int i = 0; i < vectSize; i++) {
    if (vect[i] < 0){
      res[i] = 0;
    } else {
      res[i] = vect[i];
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
  std::vector<int> dp(binSize);
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
    dp[i] = dpConsistent;
  }     

  return dp;
}

std::vector<double> slicingDouble(vector<double>& arr, int X, int Y)
{
  // Starting and Ending iterators
  auto start = arr.begin() + X;
  auto end = arr.begin() + Y + 1;
  // To store the sliced vector
  std::vector<double> result(Y - X + 1);
  // Copy vector using copy function()
  copy(start, end, result.begin());
  // Return the final sliced vector
  return result;
}

std::vector<int> enforceConsistencyH(std::vector<std::vector<int> > dpNodes, int gap){
  int binSize = dpNodes[0].size();
  int nodeSize = dpNodes.size();
  int height = log2(gap) + 1;
  if (height == 1){
    return dpNodes[0];
  }
  std::vector<int> dp(binSize);
  for (int i = 0; i < binSize; i++) {
    std::vector<double> dpNodesBin(nodeSize, 0);
    for (int j = 0; j < nodeSize; j++){
      dpNodesBin[j] = double(dpNodes[j][i]);
    }  
    int gap_again = gap;
    std::vector<double> succ = slicingDouble(dpNodesBin, 0, gap_again - 1);
     int pt = gap_again;
    std::vector<double> consist(gap_again, 0); 
    std::vector<double> succ_new(gap_again, 0); 

    for (int j = 2; j < (height+1); j++){
      gap_again = int(gap_again/2);
      for (int k = 0; k < gap_again; k++){
        succ_new[k] = succ[k*2] + succ[k*2+1];
      }
      std::vector<double> now = slicingDouble(dpNodesBin, pt, pt + gap_again - 1);
      double a = (std::pow(2, j) - std::pow(2, j - 1)) / (std::pow(2, j) - 1);
      double b = (std::pow(2, j - 1) - 1) / (std::pow(2, j) - 1);
      for (int k = 0; k < gap_again; k++){
        consist[k] = a * now[k] + b * succ_new[k];
      }
      /*
      cout << "now: ";
      for (int k = 0; k < gap_again; k++){
        cout<<now[k]<<",";
      }
      cout << endl;;
      cout << "succ_new: ";
      for (int k = 0; k < gap_again; k++){
        cout<<succ_new[k]<<",";
      }
      cout << endl;
      cout << "consist: ";
      for (int k = 0; k < gap_again; k++){
        cout<<consist[k]<<",";
      }
      cout<<endl;*/
      
      pt += gap_again;
      succ = consist;
    }
    
    int dpConsistent = int(consist[0]);  //here double to int 124 -> 123
   // cout << "dpConsistent int: " << dpConsistent <<"," << endl;
   // cout << "dpConsistent double: " << consist[0] <<"," << endl;;
    dp[i] = dpConsistent;
  }     
  return dp;
}


