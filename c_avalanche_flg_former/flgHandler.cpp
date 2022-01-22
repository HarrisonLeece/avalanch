#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>

class flg {
  public:
    string path = "./flag_files/";
    string file = "plucky.flg";

    int frameRate; //Framerate of the video.  Not arbitrary, numPSD is defined by this parameter in audioParse.cpp

    vector<int> waveform; //Wave data.  Easy
    int waveLength=0;//How long the wave data is
    vector<vector<float>> psdMetaVector; //Matrix of all psds.  Note:  The first (index 0) psd does not align with time or frame = 0.  See padding size for number of indexes to skip
    int numPSD=0; //number of PSDs contained within psdMetaVector
    int psdSize=0; //side of each PSD
    int startPadding; //By how many frames computation or display must be delayed to show valid PSDs
    int endPadding; //By how many frames computation or display on PSDs must be terminated early in order to  remain valid
    ////The above 2 parameters are determined by psdSize

    //
    vector<double> times; //Vector of times starting at 0
    vector<int> frames; //Vector of frames starting at 0
    vector<vector<int>> beatsVec; //vector of beats vectors

    //For encoding arbitrary flags
    int numFlags; //Number of channels containing data

    void writeFLG();
    void readFLG();
    void addPSDtoFLG(vector<float> singlePSD);
    void addWavetoFLG(vector<int> wave);
    void importTWP(vector<float> singlePSD);
    void addBeatsToFLG(vector<int> beatVecTemp);

};
void flg::writeFLG(){
  string outputFilePath = path + file;
  cout << "Writing FLG file to " << outputFilePath << endl;

  ofstream targetFLG (outputFilePath);
  if (!targetFLG.is_open())
  {
    fprintf(stderr, "Unable to write FLG file: %s\n", inputFilePath);
    return;
  }
  //Write any header data
  targetFLG << fps << "\n";

  //for loop to deposit time vector to csv
  for (int i = 0;  i < times.size()-1; i++)
  {
    targetFLG << times[i] << ',';
  }
  targetFLG << '\n';
  for (int i = 0;  i < frames.size()-1; i++)
  {
    targetFLG << frames[i] << ',';
  }
  targetFLG << '\n'
}

void flg::readFLG(){
  string inputFilePath = path + file;
  cout << "Reading FLG file from " << inputFilePath << endl;

  ifstream targetFLG (inputFilePath);
  if (!targetFLG.is_open())
  {
      fprintf(stderr, "Unable to open FLG file: %s\n", inputFilePath);
      return;
  }
  cout << << endl;
}
void flg::importTWP(vector<float> singlePSD){

}
void flg::addBeatsToFLG(vector<int> beatVecTemp){
  beatVec.push_back(beatVecTemp);
}
