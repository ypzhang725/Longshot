#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include "emp-sh2pc/emp-sh2pc.h"
using namespace emp;
using namespace std;

Integer * reconstructArray(std::vector<int> number){
  int size = number.size();
  Integer *A = new Integer[size];
  Integer *B = new Integer[size];
  Integer *res = new Integer[size];
  for(int i = 0; i < size; ++i){
    A[i] = Integer(32, number[i], ALICE);
    B[i] = Integer(32, number[i], BOB);
    res[i] = A[i] ^ B[i];
  }
  return res;   
}

// for debug
void printArray(Integer * array, int size){
  for (int i = 0; i < size; i++) {
    cout << array[i].reveal<int32_t>() << ' ';
  }
  cout <<  endl;
}

// for debug
int computeDummyNumber(Integer * array, int size){
  int res = 0;
  for (int i = 0; i < size; i++) {
    if (array[i].reveal<int32_t>() == 0) {
      res += 1;
    }
  }
  return res;
}

// for debug
void printArrayPlaintext(std::vector<int> array){
  for (int i = 0; i < int(array.size()); i++) {
    cout << array[i]<< ' ';
  }
  cout <<  endl;
}

std::vector<int> revealSh(Integer * sh, int bins, int party){ //todo: ALICE BOB
  std::vector<int> real(bins);
  for (int i = 0; i < bins; ++i) {
    real[i] = sh[i].reveal<int32_t>(party);
  }
  return real;
}

std::vector<int> revealSh64(Integer * sh, int bins, int party){ //todo: ALICE BOB
  std::vector<int> real(bins);
  for (int i = 0; i < bins; ++i) {
    real[i] = sh[i].reveal<int64_t>(party);
  }
  return real;
}

double lapGen(double beta){
    // https://www.johndcook.com/blog/2018/03/13/generating-laplace-random-variables/
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> expVar((1 / beta));
    double exp1 = expVar(gen);
    double exp2 = expVar(gen);     
    double res = exp1 - exp2;
    return res;
}

double uniformGen(){
    std::random_device rd;   
    std::mt19937 gen(rd());  
    std::uniform_int_distribution<> distrib(0, 100);
    int res = distrib(gen);
    return res;
}

std::vector<int> lapGenVector(int binNum, double beta){
    std::vector<int> res(binNum, 0);
    for(int i = 0; i < binNum; ++i) {
        res[i] = int(lapGen(beta));
    }
    return res;
}

std::vector<int> uniformGenVector(int binNum){
    std::vector<int> res(binNum, 0);
    for(int i = 0; i < binNum; ++i) {
        res[i] = uniformGen();
    }
    return res;
}
