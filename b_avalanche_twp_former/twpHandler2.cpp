#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sstream>
#include <string>
#include <fstream>

class twp {
  public:
    string path = "../twp_files/";
    string file = "output.twp";
    vector<int> waveform;
    vector<vector<float>> psdMetaVector;
    int fps;
    int sampFreq;
    int waveLength=0;
    int numPSD=0;
    int psdSize=0;
    int startPadding;
    int endPadding;

    void writeTWP();
    void readTWP();
    void addPSDtoTWP(vector<float> singlePSD);
    void addWavetoTWP(vector<int> wave);
    void setPaddings(int startPad, int endPad);
    void setFPS(int framesPerSec);
    void setSampFreq(int fs);
};

void twp::writeTWP(){
  psdSize = psdMetaVector[0].size();
  string outputFilePath = path + file;
  cout << "Writing TWP file to " << outputFilePath << endl;

  ofstream targetTWP (outputFilePath);
  if (targetTWP.is_open())
  {
    //Write the Header into the file first
    ////HEADER CODE

    targetTWP << fps << "\n";
    targetTWP << sampFreq << "\n";
    targetTWP << startPadding << '\n';
    targetTWP << endPadding << '\n';
    targetTWP << waveLength << '\n';
    targetTWP << numPSD << '\n';

    //Put waveform into the file
    for (int i = 0;  i < waveLength-1; i++)
    {
      targetTWP << waveform[i] << ',';
    }
    targetTWP << '\n';
    //Write psdMetaVector to file with a new line for each psd
    for (int i=0; i<numPSD; i++){
      for (int k = 0;  k < psdSize-1; k++)
      {
        targetTWP << psdMetaVector[i][k] << ',';
      }
      if (i < numPSD-1){
        targetTWP << '\n';
      }
    }

    targetTWP.close();
  }
}

void twp::readTWP(){

  string inputFilePath = path + file;
  cout << "Reading TWP file from " << inputFilePath << endl;

  ifstream targetTWP (inputFilePath);
  if (!targetTWP.is_open())
  {
      fprintf(stderr, "Unable to open TWP file: %s\n", inputFilePath);
      return;
  }

  string headerTemp;
  //Read in the HEADER
  //strictly speaking, not necessary, but I like it to make the twp file human readable
  getline(targetTWP, headerTemp);
  fps = stoi(headerTemp);
  getline(targetTWP, headerTemp);
  sampFreq = stoi(headerTemp);
  getline(targetTWP, headerTemp);
  startPadding = stoi(headerTemp);
  getline(targetTWP, headerTemp);
  endPadding = stoi(headerTemp);
  getline(targetTWP, headerTemp);
  waveLength = stoi(headerTemp);
  getline(targetTWP, headerTemp);
  numPSD = stoi(headerTemp);

  cout << "Header read" << endl;

  string delimiter =",";
  //initialize pos which is the location of the delimeter

  int ssInt;
  string unparsedWave;
  //if I understand std::getline correctly; should erase the extracted
  //line after putting the line into argument 2 (unparsedWave)
  getline(targetTWP, unparsedWave);
  istringstream ss(unparsedWave);
  //https://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
  for (int i; ss >> i;){
    waveform.push_back(i);
    if (ss.peek() == ','){
      ss.ignore();
    }
  }
  cout << "Waveform read" << endl;



  for (string line; getline(targetTWP, line);)
  {
    istringstream ss2(line);
    //std::cout << line << std::endl;
    vector<float> tmpPSD;
    for (float i; ss2 >> i;){
      tmpPSD.push_back(i);
      if (ss2.peek() == ','){
        ss2.ignore();
      }
    }
    psdMetaVector.push_back(tmpPSD);
  }
  psdSize = psdMetaVector.size();
  cout << psdMetaVector.back().size() << endl;
  targetTWP.close();
}

void twp::addWavetoTWP(vector<int> wave){
  waveform = wave;
  waveLength = wave.size();
}
void twp::addPSDtoTWP(vector<float> singlePSD){
  psdMetaVector.push_back(singlePSD);
  numPSD++;
}
void twp::setPaddings(int startPad, int endPad){
  startPadding = startPad;
  endPadding = endPad;
}
void twp::setFPS(int framesPerSec){
  fps = framesPerSec;
}
void twp::setSampFreq(int fs){
  sampFreq = fs;
}
