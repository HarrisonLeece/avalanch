#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include "thread_pool.hpp"
#include "matplot/matplot.h"
#include <aubio/aubio.h>
#include <cmath>
#include <assert.h>
//#include "vectorUtils.cpp"
#include "pocketfft_hdronly.h"



class energyBands
{
  public:
    vector<vector<double>> energyBandVec;
    vector<double> energyMem; //energyMem.size() = numBands
    int numBands = 32; // How many energy bands to divide the fft into
    int energyHistory = 42; //How many frames of energy data should be kept for the moving average of band specific engergy
    double variance = 250; //The multiple by which a currentEnergy must exceed energyMem in order to be known as a beat
    vector<double> currentEnergy; // initialize empty


    void setBandMemVariance(int band, int mem, int variance); //default is numBands =32, energyHistory = 42
    vector<int> detectBeat(); //returns a vector of size numBands populated by an integer, based on which band a beat was detected
    void addPSD(vector<float> onePSD); //Break onePSD into numBands bands and push back energyBandVec with a vector of length numBands
    void setEnergyMemory();
};

//Set some variables which an be tuned to improve sensitivity to beats
//Called once when object is created to
void energyBands::setBandMemVariance(int band, int mem, int vari)
{
  numBands = band;
  assert(numBands%2 == 0);
  energyHistory = mem;
  variance = vari;
  currentEnergy;
  cout << "Set numBands,energyHistory,variance: " << numBands << "," << energyHistory << "," << variance << endl;
  assert(currentEnergy.empty());

}

void energyBands::setEnergyMemory()
{
  if (energyBandVec[0].size() == 0) //This unfortunate if statement spaggheti is because sliceVectorDouble will out of bounds if this wasn't here, because I need to slice before the added PSD in order to prevent capture of a potential beat into the avergage value
  {
    for (int i = 0; i < numBands; i++)
    {
      energyMem[i] = 0;
    }
  }
  else if (energyBandVec[0].size() > energyHistory+1) //
  {
    vector<double> slicedVec;
    for (int i = 0; i < numBands; i++)
    {
      //my sliceVectorDouble includes the start and end index.  For energyHistory = 42 and energyBandVec[0].size()=43
      //sliceVectorDouble(energyBandVec[i],0,41) where index 42 contains the current energy
      slicedVec = sliceVectorDouble(energyBandVec[i], (energyBandVec[i].size()-energyHistory-1), energyBandVec[i].size()-2);
      energyMem[i] = sumVector<double>(slicedVec)/energyHistory;
    }
  }
  else if (energyBandVec[0].size() <= energyHistory+1) // This is probably wrong but probably won't matter
  {
    for (int i = 0; i < numBands; i++)
    {
      energyMem[i] = (sumVector<double>(energyBandVec[i]))/energyBandVec[i].size();
    }
  }
  //Now energyMem should be a vector which contains only the average energy of frames
  //memHistory frames previous to the current frame, but not including the energies of detected beats
  //The logic for removing beats from vectors comes from the detectBeat function
}

//After this function exits, energyMem should be up to date
void energyBands::addPSD(vector<float> psd)
{
  cout << "addPSD started" << endl;
  //if this is your first added PSD, initialize the energyBandVec with numBands empty vectors
  if (energyBandVec.empty())
  {
    vector<double> energyBandVecTemp;
    for (int i=0;i<numBands;i++)
    {
      energyBandVec.push_back(energyBandVecTemp);
      currentEnergy.push_back(0);
      energyMem.push_back(0);
    }
    cout << "energyBandVec is fully initialized"  << endl;
    cout << "currentEnergy vector fully initialized"  << endl;
    cout << "energyMem vector is fully initialized" << endl;
  }
  assert(currentEnergy.size() == numBands);

  //Break psd into numBands equispaced section
  //length of each band
  int bandLength = psd.size()/numBands;
  assert (bandLength>0);

  vector<float> tempVec; //holds the psd data points
  double energy;

  //Slice input power spectrum, sum into energy (per band), add energy per band into
  //the energyBandVec. Place current energy into currentEnergy vector
  for (int i=0; i < numBands; i++)
  {
    //slice
    tempVec = sliceVectorFloat(psd,i*bandLength,((i+1)*bandLength)-1);
    //find the total energy https://en.wikipedia.org/wiki/Parseval%27s_theorem
    energy = (2.0/bandLength)*sumVector<float>(tempVec);
    energyBandVec[i].push_back(energy);
    currentEnergy[i]=energy;
  }

  //logic for setting energyMemory (which is the average of energyHistory frames)
  setEnergyMemory();

}

//returns a vector of length numBands
//The integer returned will be equal to the subBand which triggered
vector<int> energyBands::detectBeat()
{
  bool isDiagnostic = true;
  vector<int> out;
  for (int band = 0; band < numBands; band++)
  {
    //We also need to pop the energyBandVec so that multiple beats within the energyHistory window can still be detected without error
    if (currentEnergy[band] > energyMem[band] * variance && energyMem[band] != 0)
    {
      //Load our out vector with an integer, indicating beat detection
      out.push_back(band+1);//This is plus 1, so that index 0 becomes 1 in the out vector
      energyBandVec[band].pop_back();//remove the current beat from energyBandVector
      energyBandVec[band].push_back(energyMem[band]*variance*.95);//Replace with the average energy for this band
      if (isDiagnostic)
      {cout << "Band " << band+1 << " above threshold by ratio: " << currentEnergy[band]/energyMem[band] << endl;
       cout << currentEnergy[band] << endl;
       cout << energyMem[band] << endl;}
    } else {
      out.push_back(0);
      if (isDiagnostic)
      {cout << "Band " << band+1 << " below threshold by ratio: " << currentEnergy[band]/energyMem[band] << endl;
       cout << currentEnergy[band] << endl;
       cout << energyMem[band] << endl;}
    }

  }
  return out;
}
