#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <cmath>
#include <math.h>
#include <vector>
using namespace std;
///////
// Helper functions for printing simple vectors of numbers
///////
void print_vector_to_console_int(vector<int> my_vector)
{
  int length = my_vector.size();
  for (int i=0; i < length; i++){
    cout << my_vector[i] << ", ";
  }
  cout << endl;
}
void print_vector_to_console_float(vector<float> my_vector)
{
  int length = my_vector.size();
  for (int i=0; i < length; i++){
    cout << my_vector[i] << ", ";
  }
  cout << endl;
}
void print_vector_to_console_double(vector<double> my_vector)
{
  int length = my_vector.size();
  for (int i=0; i < length; i++){
    cout << my_vector[i] << ", ";
  }
  cout << endl;
}

//////
//Slicing functions.  Includes beginning and end indexes
//////

vector<int> sliceVectorInt(vector<int> v1, int start=0, int end=NULL)
{
  //if end was not passed, go from the index 0+start to end
  if (!end){
    try{
      return vector<int>(v1.begin() + start, v1.end());
    } catch (const runtime_error& e) {
      cout << "Error in vectorUtils:sliceVectorInt; if statement"  << endl;
      cout << "likely cause: start index greater than Vector's length"  << endl;
    }
  } else {
    try {
      return vector<int>(v1.begin() + start, v1.begin()+(end+1));
    } catch (const runtime_error& e) {
      cout << "Error in vectorUtils:sliceVectorInt; if statement"  << endl;
      cout << "likely cause: start or /probably/ end index greater than Vector's length"  << endl;
    }
  }
}
vector<float> sliceVectorFloat(vector<float> v1, int start=0, int end=NULL)
{
  //if end was not passed, go from the index 0+start to end
  if (!end){
    try{
      return vector<float>(v1.begin() + start, v1.end());
    } catch (const runtime_error& e) {
      cout << "Error in vectorUtils:sliceVectorFloat; if statement"  << endl;
      cout << "likely cause: start index greater than Vector's length"  << endl;
    }
  } else {
    try {
      return vector<float>(v1.begin() + start, v1.begin()+(end+1));
    } catch (const runtime_error& e) {
      cout << "Error in vectorUtils:sliceVectorFloat; if statement"  << endl;
      cout << "likely cause: start or /probably/ end index greater than Vector's length"  << endl;
    }
  }
}
vector<double> sliceVectorDouble(vector<double> v1, int start=0, int end=NULL)
{
  //if end was not passed, go from the index 0+start to end
  if (!end){
    try{
      return vector<double>(v1.begin() + start, v1.end());
    } catch (const runtime_error& e) {
      cout << "Error in vectorUtils:sliceVectorDouble; if statement"  << endl;
      cout << "likely cause: start index greater than Vector's length"  << endl;
    }
  } else {
    try {
      return vector<double>(v1.begin() + start, v1.begin()+(end+1));
    } catch (const runtime_error& e) {
      cout << "Error in vectorUtils:sliceVectorDouble; if statement"  << endl;
      cout << "likely cause: start or /probably/ end index greater than Vector's length"  << endl;
    }
  }
}
void unitTests(){
  using namespace std;
  vector<int> vInts{1,2,3,4,5,6,7,8,9,10};
  vector<float> vFloats{1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9,10.11};
  vector<double> vDoubles{1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9,10.11};

  vector<int> intsOut;
  vector<float> floatsOut;
  vector<double> doublesOut;


  intsOut = sliceVectorInt(vInts,0,9);
  cout << "This should print 1-10:" << endl;
  print_vector_to_console_int(intsOut);
  floatsOut = sliceVectorFloat(vFloats,3,7);
  cout << "This should print 4-8:" << endl;
  print_vector_to_console_float(floatsOut);
  doublesOut = sliceVectorDouble(vDoubles,1,8);
  cout << "This should print 2-9:" << endl;
  print_vector_to_console_double(doublesOut);
}

//This should take any type
template <typename T>
T sumVector(const std::vector<T> inVec)
{
  T sum=0;
  for (int i = 0; i < inVec.size(); i++)
  {
    sum+=inVec[i];
  }
  return sum;
}
void unitTestsSumVector(){
  using namespace std;
  vector<int> vInts{1,2,3,4,5,6,7,8,9,10}; // sum is 55
  vector<float> vFloats{1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9,10.11}; // sums to 59.61
  vector<double> vDoubles{1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9,10.11}; // sums to 59.61

  int sumInt =       sumVector<int>(vInts);
  float sumFloat =   sumVector<float>(vFloats);
  double sumDouble = sumVector<double>(vDoubles);


  cout << "This sum should be 55:" << endl;
  cout << sumInt << endl;
  cout << "This sum should be 59.61:" << endl;
  cout << sumFloat << endl;
  cout << "This sum should be 59.61:" << endl;
  cout << sumDouble << endl;

}
