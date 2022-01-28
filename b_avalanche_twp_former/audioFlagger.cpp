#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include "thread_pool.hpp"
//Things for plottings
#include "matplot/matplot.h"
#include <aubio/aubio.h>
#include "testingUtils.cpp" //in libs
#include "animationUtils.hpp" //in libs, strictly for making animations for production use
#include <cmath>
#include <assert.h>
#include "vectorUtils.cpp"
//#include "wavSoundData.cpp"
#include "twpHandler2.cpp"
#include "pocketfft_hdronly.h"
#include "energyBands2.cpp"
//#include "flgHandler.cpp"

using namespace pocketfft;
using namespace std;
/*
In this section, take the data parsed to the twp file, read
and use techniques to find notable time zones of music

*/
//fourier transform//Go from waveform to PSD
vector<float> forwardFFT(vector<int> waveformVec){
  assert(log2(waveformVec.size())==int(log2(waveformVec.size()))); //Ensure that the waveformVecs size is a power of 2
  //initialize and allocate memory for the output vector
  vector<float> outputVec(waveformVec.size()/2,0.0);
  //FFT


  return outputVec; // Does not work yet
}
//inverse fourier transform// Go from PSD to waveform
vector<int> inverseFFT(vector<float> frequencyVec){
  //First we have to convert psd to fft
  for (int i = 0; i < frequencyVec.size(); i++){
    frequencyVec[i] = sqrt(frequencyVec[i]);
  }
  //inverseFFT might require the imaginary component to be useful.
  vector<int> outputWaveform(frequencyVec.size()*2,0); //We should get 2 data points of a waveform for every frequency bin
  return outputWaveform; //DOes not work yet

}
//TWP information recovery
float hertzPerPsdBin(int psdSize, int samplesPerSec)
{
  //Because the aubio FFT implementation gives only the positive side of the FFT, we need to recover the true length of the time period
  int divisor = psdSize*2;
  float hertzPerPsd = float((samplesPerSec)/divisor);
  return hertzPerPsd;
}
//filters
vector<float> bandFiltration(vector<float> psdInput, float hzPerBin, float hzStart, float hzEnd) // Filter by frequency
{
  assert(hzStart > 0 && hzEnd > hzStart);
  int numStartBin = round(hzStart/hzPerBin);
  int indeciesToTraverse = round((hzEnd-hzStart)/hzPerBin);
  //add zeros for the cut content
  vector<float> beg (numStartBin, 0.0);
  vector<float> end (psdInput.size()-numStartBin - indeciesToTraverse,0.0);
  vector<float> passedBand = sliceVectorFloat(psdInput, numStartBin, numStartBin+indeciesToTraverse);
  return passedBand;
}

//Drum hits

//diagnostic fxns
void animatePSDs(vector<vector<float>> psdMatrix, int length, int padding, vector<float> color) //length is the length of the animation.  !!!Should equal the size of the frames and times vectors!!!
{
  cout << "This wack psd is sized " << psdMatrix.back().size() << endl;
  cout << psdMatrix.size() << endl;
  print_vector_to_console_float(psdMatrix.back());
  auto animationFigure = matplot::figure();
  animationFigure -> size(2*666,2*777);

  string animationName = "flaggerPSD";
  vector<float> psd;
  int psdSize = psdMatrix[0].size();
  vector<float> fakePSD(psdSize, 0.0); //populate a vector with zeros; With the length fo one PSD
  cout << "animating PSDs" << endl;
  for (int i = 0; i < length; i++)
  {
    cout << "loop index i: " << i;
    if ( i >= padding && i < length-(2*padding)) //I can actually compute padding from just the length and matrix size but Ill save that for the rewrite
    {
      cout << " - psd Number: " << i-padding << endl;

      assert (psdMatrix[i-padding].size() == psdSize); // so no bad aloc errors
      psd=psdMatrix[i-padding]; //This will segfault if i messed up the frame timing
      psd = sliceVectorFloat(psd, 5); //Cut off the zero hertz bin
      animate_single_channelFloat(psd, animationFigure, i, animationName, color);
    } else
    {
      animate_single_channelFloat(fakePSD, animationFigure, i, animationName, color);
    }
  }
}
void animateWaves(int waveWidth, vector<int> waveform, int samplesPerFrame, vector<float> color) //wave width is how much of the wave is displayed per frame.  Out of bound frames are handled by adding zeros
{
  auto animationFigure = matplot::figure();
  animationFigure -> size(2*888,2*444);

  string animationName = "flaggerWaveform";
  int padLength = ceil(waveWidth/(2.0*samplesPerFrame)); //Related to padding, like in PSD but is a different value based on how much of the waveform is displayed at a time
  int targetSample; //initialize this variable outside the loop
  cout << "animating waves" << endl;
  for (int i = 0; i < waveform.size()/samplesPerFrame; i++) //waveform.size() should be already be cut, so this divides evenly, by the time this is called; therefore division by samplesPerFrame gives how many frames are in the waveform
  {
    vector<int> tempWaveData(waveWidth);
    cout << "Rendering frame: " << i << endl;
    targetSample = i*samplesPerFrame;
    cout << "Target Sample: " << targetSample << endl;
    if (i>=padLength && i < (waveform.size()/samplesPerFrame - padLength)){
      copy(waveform.begin()+targetSample-(waveWidth/2 - 1), waveform.begin()+targetSample+waveWidth/2, tempWaveData.begin()); // I think I nailed it +/- some extra frames
      animate_single_channel(tempWaveData, animationFigure, i, animationName, color);
    } else {
      // logic to fudge out of bounds frames here
      if (i<padLength) //pad the firstpart with zeros, last part with valid wav data
      {
        cout << "Padding front of vector with zeros" << endl;
        int adder = (padLength-i)*samplesPerFrame;
        int diff = i*samplesPerFrame;
        vector<int> frontTmpData = sliceVectorInt(waveform, 0, waveWidth/2 + diff-1);
        vector<int> waveBegPad(adder-samplesPerFrame,0); // Add this to the begining, so the waveform starts in the middle of the plot
        //concatenate something to waveBegPad (waveBegPad is the begining)
        frontTmpData.insert(frontTmpData.begin(), waveBegPad.begin(), waveBegPad.end());
        animate_single_channel(frontTmpData, animationFigure, i, animationName, color);
      } else //pad the end with zeros
      {
        cout << "Padding end of vector with zeros to finish animation" << endl;
        int diff = (padLength - waveform.size()/samplesPerFrame + i)*samplesPerFrame;
        vector<int> tempWaveData(waveWidth);
        copy(waveform.begin()+targetSample-(waveWidth/2 - 1), waveform.begin()+targetSample+(waveWidth/2-(diff)), tempWaveData.begin()); // I think I nailed it +/- a couple of extra samples
        vector<int> waveEndPad(diff,0);
        //tempWaveData.push_back(waveEndPad);
        animate_single_channel(tempWaveData, animationFigure, i, animationName, color);
      }
    }
  }
}


int main()
{
  twp timeWavePower;
  timeWavePower.readTWP();
  assert (timeWavePower.numPSD == timeWavePower.psdMetaVector.size());
  cout << "TWP read" << endl;
  cout << "PSD Matrix size: " << timeWavePower.psdMetaVector.size() << endl;
  cout << "This wack psd is sized " << timeWavePower.psdMetaVector.back().size() << endl;
  //print_vector_to_console_float(timeWavePower.psdMetaVector[594]);


  //Initialize some important variables
  int sampPerFrame = timeWavePower.sampFreq/timeWavePower.fps;
  //number of extra frames, so that the number of samples divides easily into number of frames
  //when thes extra samples are subtracted from the number of samples,
  int extraSamples= timeWavePower.waveform.size()%sampPerFrame;
  //trim the waveform within the object.  Subtract 1 because my sliceVectorInt function includes the end index's data
  timeWavePower.waveform = sliceVectorInt(timeWavePower.waveform,0,(timeWavePower.waveform.size()-extraSamples)-1);
  //////////Maybe have to 'buy back' a padding frame. Think harder about this later
  //test that things were sliced correctly
  assert(timeWavePower.waveform.size()%sampPerFrame == 0);
  cout << "Waveform sliced to proper size" << endl;
  cout << "Waveform size: " << timeWavePower.waveform.size() << endl;

  //Create a vector representing times, extending from 0 seconds to the end of the audio clip (calculated index/fps
  //Create a vector with a size series of displayable waves in the sound file (includes 'padding', using index 0 will out of bounds for fft)
  //The vector at index 0 will equal 0. Index 1 will equal a number of frames, such as 735 for a 60 fps video.
  vector<double> times;
  vector<int> frames;
  double tempD;
  for (double i=0.0; i < timeWavePower.waveform.size()/sampPerFrame; i++){
    tempD = i/timeWavePower.fps;
    times.push_back(tempD);
  }
  cout << "times vector finished" << endl;

  assert(times.size()==timeWavePower.waveform.size()/sampPerFrame); //includes one extra frame at the end
  assert(times[0]==0);
  assert(times[1]==1.0/timeWavePower.fps);
  assert(times.back()==(timeWavePower.waveform.size()/sampPerFrame-1)*times[1]);
  cout << "Time vector data is formed correctly" << endl;

  int temp;
  for (int n = 0; n < times.size(); n++){
    temp = (sampPerFrame) * n;
    frames.push_back(temp);
  }
  cout << "Frames vector finished" << endl;

  //////Assert a bunch of conditions are satisfied before proceeding to prevent runtime errors
  //Check that frames was initialized per specifications (frame[0]=0, frame[1]=timeWavePower.sampFreq/timeWavePower.fps, frame[frame.back()]=(timeWavePower.sampFreq/timeWavePower.fps)*frame.size()-1... etc)
  assert(frames.size() == times.size());
  assert(frames[0] == 0);
  assert(frames[1] == sampPerFrame);
  assert(frames.back() == (sampPerFrame)*(frames.size()-1));

  cout << "Frames data formed correctly" << endl;

  //Check that the time vector aligns with frames.  time*sampFreq should equal number of frames
  //Because 60FPS = .01666.... repeating, carefully rounding might be necessary
  assert(times[0] == 0 && times[1] * timeWavePower.sampFreq == frames[1] && times.back() * timeWavePower.sampFreq == frames.back());
  cout << "times and frames align." << endl << endl;

  //initialize variables
  vector<float> powerSpectrum;
  vector<float> energyMemory;
  vector<vector<int>> beatsVec;
  int energyMemoryAverage;
  energyBands ebOne;

  //Tunable parameters
  int waveformParam = 4096; //The number of frames captured for a single frame of display
  int frameAdjustment = 0; //Shift data left or right to account for mismatch between audio and visuals
  float beatThreshhold = 2.5; //The parameter which controls what energy difference is required to register a beat
  int numSubBands = 32; // How many subbands to divide the fft into.  Must be even
  int energyHistory = 34;

  ebOne.setBandMemVariance(numSubBands, energyHistory, beatThreshhold);

  //Unit tests
  bool testing = false;
  if (testing){
    cout << "tests running: " << endl;
    //Just 3 seconds of just kicks
    //3 seconds of a sine wave @60Hz with kicks
    //3 seconds of sine wave@60Hz with kicks and snares
    //10 second clip of a song
    return 1;
  }

  int plotting=2;
  while (plotting != 1 || plotting != 0){
    cout << "Enter 1 to plot, enter 0 to proceed to beat detection logic" << endl;
    cin >> plotting;
  }

  if (plotting)
  {
    float r = -1;
    float g = -1;
    float b = -1;
    while (r<0 || g<0 || b<0){
      cout << "Enter PSD Red channel value: " << endl;
      cin >> r;
      cout << "Enter PSD Green channel value: " << endl;
      cin >> g;
      cout << "Enter PSD Blue channel value: " << endl;
      cin >> b;
      if ((r>=0 && r<=1) && (g>=0 && g<=1) && (b>=0 && b<=1)) {
        cout << "Interpreted decimal input" << endl;
      } else if ((r>1 && r<=255) || (g>1 && g<=255) || (b>1 && b<=255)){
        cout << "Interpreted 1byte integer inputs.  Converting." << endl;
        r = r/255.0; g = g/255.0; b = b/255.0;
      } else {
        r = -1;
        cout << "Cannot understand input; re-requesting" << endl;
      }
    }
    vector<float> psdColor = {r,g,b};
    r = -1; g = -1; b = -1;
    while (r<0 || g<0 || b<0){
      cout << "Enter Waveform Red channel value: " << endl;
      cin >> r;
      cout << "Enter Waveform Green channel value: " << endl;
      cin >> g;
      cout << "Enter Waveform Blue channel value: " << endl;
      cin >> b;
      if ((r>=0 && r<=1) && (g>=0 && g<=1) && (b>=0 && b<=1)) {
        cout << "Interpreted decimal input" << endl;
      } else if ((r>1 && r<=255) || (g>1 && g<=255) || (b>1 && b<=255)){
        cout << "Interpreted 1byte integer inputs.  Converting." << endl;
        r = r/255.0; g = g/255.0; b = b/255.0;
      } else {
        r = -1;
        cout << "Cannot understand input; re-requesting" << endl;
      }
    }
    vector<float> waveColor = {r,g,b};
    cout << "Num PSDs in psdMetaVector " << timeWavePower.numPSD << endl;
    cout << "End padding " << timeWavePower.endPadding << endl;

    animatePSDs(timeWavePower.psdMetaVector, frames.size(),timeWavePower.startPadding, psdColor);
    animateWaves(waveformParam, timeWavePower.waveform, sampPerFrame, waveColor);


    return 44;
  }

  //instantiate a vecto of ints outside of loop to hold beats
  vector<int> beatsTemp;
  //construct a padding vector to pad out frames where PSD data is out of range
  vector<int> noBeatsPad;
  for(int i=0;i<numSubBands;i++) { noBeatsPad.push_back(0);}

  int diagnosticIterator = 0;

  //instantiate flag object
  //flg flagObj;

  cout << "Starting beat detection loop" << endl;
  //Loop through all
  for (int i=0; i < frames.size(); i++)
  {
    //if valid PSD exist basically
    if (i >= timeWavePower.startPadding && i < frames.size() - timeWavePower.endPadding - 1)
    {
      //increment the diagnosticIterator
      diagnosticIterator++;
      //cout << i << endl;
      //cout << timeWavePower.startPadding << endl;
      //Add PSD to PSD;
      ebOne.addPSD(timeWavePower.psdMetaVector[i-timeWavePower.startPadding]);
      //ebOne.addPSD(timeWavePower.psdMetaVector[5]);

      beatsTemp = ebOne.detectBeat(); // its kinda nice not to push this back to beatsVec right away for diagnostic
      //print any non-zero data points
      cout << "Frame number: " << i << endl;
      cout << "Energy bands flagged for beats: ";
      for (int i=0;i<beatsTemp.size(); i++){if (beatsTemp[i]>0) { cout << beatsTemp[i] << ",";}}
      cout << endl;
      beatsVec.push_back(beatsTemp);
    } else
    {
      cout << "No valid PSD to detect beats with: " << endl;
      beatsVec.push_back(noBeatsPad);
      //flagObj.addBeatsToFLG(beatsVec); //Add beats to the BeatsVec variable of the flg object
    }
  }
  //ensure all of the PSDs were added to the energyBand object
  //I don't compare numPSD to size of beatsVec because I have plans to delay beat detection past frame padding
  //assert(diagnosticIterator == timeWavePower.numPSD-"Padding numbers here");
  cout << diagnosticIterator << endl;
  print_vector_to_console_int(beatsVec[292]);

  //Now write all data to a file
  //first load all relevant parameters from the TWP to the FLG object
  //flagObj.importTWP(timeWavePower);
  //Write to disk
  //flagObj.writeFLG();

  return 0;
}
