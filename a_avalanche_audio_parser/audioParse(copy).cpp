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
#include "testingUtils.cpp"
#include <cmath>
#include <math.h>
#include "wavSoundData.cpp"
#include "twpHandler2.cpp"
#include "vectorUtils.cpp"
#include "pocketfft_hdronly.h"

using namespace std;

//Turn aubio vector into a c++ vector
vector<float> convert_fft_to_vector(cvec_t input) {
  //initializ the output vector of length of the complex vec
  vector<float> output_vec(input.length);
  for (int i = 0; i < input.length-1; i++){
    output_vec[i]=input.norm[i];
  }
  return output_vec;
}


vector<float> create_single_fft_vector(vector<int> input_vector, int PSDPOWER){
  uint_t n_iters = 1; // number of iterations
  uint_t win_s = pow(2,PSDPOWER); // window size
  fvec_t * in = new_fvec (win_s); // input buffer
  cvec_t * fftgrain = new_cvec (win_s); // fft norm and phase
  fvec_t * out = new_fvec (win_s); // output buffer
  vector<float> magnitudes(win_s); //initalization of the magnitudes vector
  // create fft object
  aubio_fft_t * fft = new_aubio_fft(win_s);
  if (!fft) {
    cout << "big whoopsies in audioParse.cpp;create_single_fft_vector " << endl;
    goto beach;
  }
  for (int i = 0; i < win_s; i++){
    // fill input with some data
    in->data[i] = input_vector[i];
  }

  /*
  for (int i = 0; i < n_iters; i++) {
    cout << i << endl;
    // execute stft
    aubio_fft_do (fft,in,fftgrain);
    cvec_print(fftgrain);
    // execute inverse fourier transform
    aubio_fft_rdo(fft,fftgrain,out);
  }
  */
  aubio_fft_do (fft,in,fftgrain);
  cvec_print(fftgrain);
  // cleam up
  //fvec_print(out);
  del_aubio_fft(fft);

  magnitudes = convert_fft_to_vector(*fftgrain);

  //for troubleshooting
  //plot_vector_double(magnitudes);



beach:
  del_fvec(in);
  del_cvec(fftgrain);
  del_fvec(out);
  aubio_cleanup();
  return magnitudes;
}

//Take a converted fft_vector and turn it into a power spectral density
vector<float> power_spec_density(vector<int> inputWave, int pSDPOWER) {
  vector<float> outputPSD;
  vector<float> tmpVec = create_single_fft_vector(inputWave, pSDPOWER);
  for (int i = 0; i < tmpVec.size()-1; i++){
    //Square input_vec[i] to get PSD from FFT raw output
    outputPSD.push_back(pow(tmpVec[i],2.0));
  }
  return outputPSD;
}

/*
Tests the fft, power spectrum and waveform
on sine signals
*/
void unitTests(int psdInSize)
{
  using namespace matplot; //we gon' do a lot of plotting
  char* filePath;
  filePath = "../tests/100Hz_44100Hz_16bit_05sec.wav";
  int psdInSizePower = int(log2(psdInSize));
  cout << psdInSizePower << endl;
  vector<int> waveform(psdInSize);
  vector<float> psd(psdInSize/2);


  //load the 100hz data so I can access some file info
  soundData clip_100hz;
  clip_100hz.parse_header_and_body(filePath);
  double modifier = (clip_100hz.wavHeader.SamplesPerSec)/pow(2,psdInSizePower+1);
  vector<double> psdX = linspace(0,(psdInSize/2-1)*modifier,psdInSize/2);
  vector<double> waveX = linspace(0,psdInSize-1, psdInSize);



  auto f1 = figure(true);
  f1->width(f1->width() * 2);
  f1->height(f1->height() * 1);
  f1->x_position(10);
  f1->y_position(10);
  hold(on);

  waveform = sliceVectorInt(clip_100hz.retreiveWaveChannel(),0,psdInSize-1);

  psd = power_spec_density(waveform, psdInSizePower);

  //100 hz tests
  subplot(2, 1, 0);
  xlabel("Amplitude");

  ylabel("Sample");
  title("100hz waveform");
  plot(waveX,waveform)->color({0.f, 0.7f, 0.9f});

  subplot(2, 1, 1);
  xlabel("Frequency");
  xlim({0,200});
  ylabel("Value");
  title("100hz PSD");
  plot(psdX,psd)->color({0.9f, 0.f, 0.1f});

  f1->draw();

  //250hz plot setup
  auto f2 = figure(true);
  f2->width(f2->width() * 2);
  f2->height(f2->height() * 1);
  f2->x_position(10);
  f2->y_position(10);

  filePath = "../tests/250Hz_44100Hz_16bit_05sec.wav";
  soundData clip_250hz;
  clip_250hz.parse_header_and_body(filePath);
  waveform = sliceVectorInt(clip_250hz.retreiveWaveChannel(),0,psdInSize-1);
  psd = power_spec_density(waveform,psdInSizePower);

  //250 hz tests
  subplot(2, 1, 0);
  xlabel("Amplitude");

  ylabel("Value");
  title("250hz waveform");
  plot(waveX,waveform)->color({0.f, 0.7f, 0.9f});

  subplot(2, 1, 1);
  xlabel("Frequency");
  xlim({0,300});
  ylabel("Value");
  title("250hz PSD");
  plot(psdX,psd)->color({0.9f, 0.f, 0.1f});

  f2->draw();

  //1kHz plot setup
  auto f3 = figure(true);
  f3->width(f3->width() * 2);
  f3->height(f3->height() * 1);
  f3->x_position(10);
  f3->y_position(10);

  filePath = "../tests/1kHz_44100Hz_16bit_05sec.wav";
  soundData clip_1khz;
  clip_1khz.parse_header_and_body(filePath);
  waveform = sliceVectorInt(clip_1khz.retreiveWaveChannel(),0,psdInSize-1);
  psd = power_spec_density(waveform,psdInSizePower);

  //1k hz tests
  subplot(2, 1, 0);
  xlabel("Amplitude");
  ylabel("Value");
  title("1khz waveform");
  plot(waveX,waveform)->color({0.f, 0.7f, 0.9f});

  subplot(2, 1, 1);
  xlabel("Frequency");
  xlim({0,1100});
  ylabel("Value");
  title("1khz PSD");
  plot(psdX,psd)->color({0.9f, 0.f, 0.1f});

  f3->draw();

  //10kHz plot setup

  auto f4 = figure(true);
  f4->width(f4->width() * 2);
  f4->height(f4->height() * 1);
  f4->x_position(10);
  f4->y_position(10);

  filePath = "../tests/10kHz_44100Hz_16bit_05sec.wav";
  soundData clip_10khz;
  clip_10khz.parse_header_and_body(filePath);
  waveform = sliceVectorInt(clip_10khz.retreiveWaveChannel(),0,psdInSize-1);
  psd = power_spec_density(waveform,psdInSizePower);

  //10k hz tests
  subplot(2, 1, 0);
  xlabel("Amplitude");
  ylabel("Value");
  title("10khz waveform");
  plot(waveX,waveform)->color({0.f, 0.7f, 0.9f});

  subplot(2, 1, 1);
  xlabel("Frequency");
  ylabel("Value");
  xlim({0,11000});
  title("10khz PSD");
  plot(psdX,psd)->color({0.9f, 0.f, 0.1f});

  f4->draw();

  show();

}

/*
Tests the fft, power spectrum and waveform
on sine signals
*/
void unitTests32bit(int psdInSize)
{
  using namespace matplot; //we gon' do a lot of plotting
  char* filePath;
  filePath = "../tests/110Hz_44100Hz_32bit_05sec.wav";
  int psdInSizePower = int(log2(psdInSize));
  cout << psdInSizePower << endl;
  vector<int> waveform(psdInSize);
  vector<float> psd(psdInSize/2);


  //load the 100hz data so I can access some file info
  soundData clip_110hz;
  clip_110hz.parse_header_and_body(filePath);
  double modifier = (clip_110hz.wavHeader.SamplesPerSec)/pow(2,psdInSizePower+1);
  vector<double> psdX = linspace(0,(psdInSize/2-1)*modifier,psdInSize/2);
  vector<double> waveX = linspace(0,psdInSize-1, psdInSize);



  auto f1 = figure(true);
  f1->width(f1->width() * 2);
  f1->height(f1->height() * 1);
  f1->x_position(10);
  f1->y_position(10);
  hold(on);

  waveform = sliceVectorInt(clip_110hz.retreiveWaveChannel(),0,psdInSize-1);

  psd = power_spec_density(waveform, psdInSizePower);

  //100 hz tests
  subplot(2, 1, 0);
  xlabel("Amplitude");

  ylabel("Sample");
  title("110hz waveform");
  plot(waveX,waveform)->color({0.f, 0.7f, 0.9f});

  subplot(2, 1, 1);
  xlabel("Frequency");
  xlim({0,200});
  ylabel("Value");
  title("110hz PSD");
  plot(psdX,psd)->color({0.9f, 0.f, 0.1f});

  f1->draw();

  show();
}

int main()
{
    //60 and 30 divide easily into typical Samp/sec specifications
    const int FRAMESPERSEC = 60;
    //2^14 = 16384 = ~8000 hz
    const int PSDPOWER = 11;

    //16384 =2^14 produces ~8300 ffts bins
    int samplesPerPSD = pow(2,PSDPOWER);

    bool isTesting = true;
    if (isTesting) {
      cout << "Tests starting" << endl;
      unitTests(samplesPerPSD);
      cout << "tests complete" << endl;
      //unitTests32bit(samplesPerPSD);
      return 2;
    }


    char* filePath;
    filePath = "../wavs/test1.wav";

    //create object to hold a bunch of important information
    //see wavSoundData.cpp file to implementation
    soundData test_clip;
    test_clip.parse_header_and_body(filePath);



    //get single fft data to check that its working correctly
    //vector<float> test_fft_vec = create_single_fft_vector(test_clip.left_channel, samplesPerPSD);

    //SampleRate divided by frames per second
    int samplesPerFrame = test_clip.wavHeader.SamplesPerSec/FRAMESPERSEC;


    int bytesPerSample = test_clip.wavHeader.bitsPerSample/8;
    //Define soundDataSample outside if statements
    //Subchunk2Size is a size in bits, if there are 2 channels, half the
    //subchunk2Size and divide by the number of bytesPerSample to get
    int soundDataSamples;
    if (test_clip.wavHeader.NumOfChan == 1) {
        soundDataSamples = int (test_clip.wavHeader.Subchunk2Size)/bytesPerSample;
    } else if (test_clip.wavHeader.NumOfChan == 2) {
        soundDataSamples = int (test_clip.wavHeader.Subchunk2Size/2)/bytesPerSample;
    }

    //total number of frames in the sound data, without regard for PSD beginning
    //and end buffer zones
    int numFramesInSound = floor(soundDataSamples/samplesPerFrame);
    //Because we are reaching samplesPerPSD/2 - 1 back in time, we have to delay
    //The capture of PSDs a number of frames.  This shrinks the total number of
    //frames in the capture
    //This one has a -1 because the total number of sampled frames is 2^PSDPOWER
    //That number includes the 'current frame' x,
    //Imagine if 2^3 = 8 samples  <-4,-3,-2,-1,x,1,2,3,4> is incorrect
    // <-3,-2,-1,x,1,2,3,4> is 8 samples
    //int startFrameDelay = floor((samplesPerPSD/2 - 1)/samplesPerFrame); // This might have to be cieling
    int startFrameDelay = ceil((samplesPerPSD/2 - 1)/(double(samplesPerFrame)));
    //Now this is the number of frames by which the loop will end early
    //to stop making PSDs when we will go too far forward into the .wav
    //int endFrameHalt = floor((samplesPerPSD/2)/samplesPerFrame);
    int endFrameHalt = ceil((samplesPerPSD/2)/(double(samplesPerFrame)));



    //Initialize outside of loops
    int targetSample;
    vector<int> tempWaveData(samplesPerPSD);



    //How many samples are in the left_channel?  For a 10s clip @ 44,100 samp/second
    //=441,000  For troubleshooting
    //cout << "Samples in test_clip.left_channel?: " << test_clip.left_channel.size() << endl;


    bool testAnimation = false;
    /////////////////////////////
    //All of this commented sction plots the full waveform and then animates
    //All frames of data are saved
    /////////////////////////////
    if (testAnimation){
      //How many channels are there?  Interpret data accordingly
      if (test_clip.wavHeader.NumOfChan==1) {
        plot_mono_channel(test_clip.mono_channel);
        auto animationFigure = matplot::figure();
        for (int i = startFrameDelay+1; i < numFramesInSound-endFrameHalt-1; i++){
          cout << i << endl;
          targetSample = i*samplesPerFrame;
          cout << "Target Sample: " << targetSample << endl;

          copy(test_clip.mono_channel.begin()+targetSample-(samplesPerPSD/2 - 1), test_clip.mono_channel.begin()+targetSample+samplesPerPSD/2, tempWaveData.begin());
          //animate_single_channel(tempWaveData, animationFigure, i);
        }
      } else if (test_clip.wavHeader.NumOfChan==2) {
        plot_stereo_channel(test_clip.left_channel, test_clip.right_channel);
        auto animationFigure = matplot::figure();
        for (int i = startFrameDelay+1; i < numFramesInSound-endFrameHalt-1; i++){
          cout << i << endl;
          targetSample = i*samplesPerFrame;
          cout << "Target Sample: " << targetSample << endl;
          //cout << "left channel size: " << test_clip.left_channel.size() << endl;

          copy(test_clip.left_channel.begin()+targetSample-(samplesPerPSD/2 - 1), test_clip.left_channel.begin()+targetSample+samplesPerPSD/2, tempWaveData.begin());

          animate_single_channel(tempWaveData, animationFigure,i);
        }
      }
      cout << "Safely exited the animation loop" << endl;
    }



    //initialize a vector to write ffts to with proper size
    vector<float> fft_vec(samplesPerPSD/2);
    twp timeWavePower;

    //Set the paddings so that the written file knows how to sync video to PSDs
    timeWavePower.setFPS(FRAMESPERSEC);
    timeWavePower.setSampFreq(test_clip.wavHeader.SamplesPerSec);
    timeWavePower.setPaddings(startFrameDelay, endFrameHalt);
    timeWavePower.addWavetoTWP(test_clip.left_channel);


    for (int i = startFrameDelay; i<numFramesInSound-endFrameHalt; i++) {
      targetSample = i*samplesPerFrame;
      copy(test_clip.left_channel.begin()+targetSample-(samplesPerPSD/2 - 1), test_clip.left_channel.begin()+targetSample+samplesPerPSD/2, tempWaveData.begin());
      fft_vec = create_single_fft_vector(tempWaveData, PSDPOWER);
      timeWavePower.addPSDtoTWP(fft_vec);
    }

    cout << int(test_clip.wavHeader.SamplesPerSec) << endl;
    //Now that data is loaded into the object, write to file
    timeWavePower.writeTWP();
    cout << "Number of PSDs: " << timeWavePower.numPSD << endl;
    cout << startFrameDelay << endl;
    cout <<  endFrameHalt << endl;


    return 0;
}
