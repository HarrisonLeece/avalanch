/* Contains the class data for the soundData and PSD objects

The soundData class stores information about the parsed .wav file.  It's utility is in safely extracting
bytecode from the .wav file and turning into integers that math can trivially be done upon in c++
The object is presently quite limited.  It takes mono and dual channel, but only 16 bit wav files.

The PSD object encapsulates all power spectrum density data and allows a vector to be made trivially in c++ with
vector<PSD>;

*/
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <variant>
#include <dirent.h>
#include <fstream>
#include "thread_pool.hpp"
#include "matplot/matplot.h"
#include <aubio/aubio.h>


using namespace std;

// Structure of the header of the .wav
typedef struct  WAV_HEADER
{
    /* RIFF Chunk Descriptor */
    uint8_t         RIFF[4];        // RIFF Header Magic header
    uint32_t        ChunkSize;      // RIFF Chunk Size
    uint8_t         WAVE[4];        // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t         fmt[4];         // FMT header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;  // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
} wav_hdr;


class soundData
{
  public:
    //Initialize all my 'instance variables'
    char * filePath;
    wav_hdr wavHeader;
    vector<int> left_channel;
    vector<int> right_channel;
    vector<int> mono_channel;
    //Troubleshooting; but it will saturate the whole console cache
    //print_vector_to_console(left_channel);

    //Declare class functions.  Remember soundData:: prefix to functions
    int getFileSize(FILE* inFile);
    void parse_header_and_body(char * fileName);
    vector<int> retreiveWaveChannel();
    void mono_parser();
    void stereo_parser();

};

// find the file size
int soundData::getFileSize(FILE* inFile)
{
    int fileSize = 0;
    fseek(inFile, 0, SEEK_END);

    fileSize = ftell(inFile);

    fseek(inFile, 0, SEEK_SET);
    return fileSize;
}


void soundData::parse_header_and_body(char * fileName)
{

  int headerSize = sizeof(wav_hdr), filelength = 0;


  FILE* wavFile = fopen(fileName, "r");
  if (wavFile == nullptr)
  {
      fprintf(stderr, "Unable to open wave file: %s\n", filePath);
      return;
  }

  //Read the header
  size_t blocksRead = fread(&wavHeader, 1, headerSize, wavFile);
  cout << "Header Read " << blocksRead << " bytes." << endl;



  if (blocksRead > 0)
  {
    //Read the data
    uint16_t bytesPerSample = wavHeader.bitsPerSample / 8;      //Number     of bytes per sample
    uint64_t numSamples = wavHeader.ChunkSize / bytesPerSample; //How many samples are in the wav file?
    //static const uint16_t BUFFER_SIZE = 4096; //Why is this hardcoded
    //int8_t* buffer = new int8_t[wavHeader.NumOfChan];

    ////////////////
    //Get the data from the wav
    ////////////////
    //
    //It would probably be better to make a list of vectors and make a for loop
    //WHich iterates #ofChanels times and get a vector in a list for each channel
    if (bytesPerSample>2){
      cout << "bytesPerSample: " << bytesPerSample << " is unsupported"  << endl;
      return;
    }
    int16_t * buffer  = new int16_t[1];


    int trigger = 0;
    //Every f read should read the data for one sample, and then store it to a vector
    if (wavHeader.NumOfChan == 2) {
        cout << "Number of Channels = 2; Parsing as stereo" << endl;
        int16_t * buffer  = new int16_t[1];
        //cout << bytesRead << endl;
        while (blocksRead>0) {
          //I think there is a glitch in here maybe?
          //originally was bytesRead = fread(buffer, bytesPerSample, bytesPerSample, wavFile);
          blocksRead = fread(buffer, bytesPerSample, 1, wavFile);
          //cout << "Read " << bytesRead << " bytes." << endl;
          if (blocksRead != 1) { trigger = 1; }
          int left = *buffer;
          left_channel.push_back(left);
          blocksRead = fread(buffer, bytesPerSample, 1, wavFile);
          //cout << "Read " << bytesRead << " bytes." << endl;
          int right = *buffer;
          right_channel.push_back(right);
          if (blocksRead != 1 || trigger == 1) { break; }



        }
    } else if (wavHeader.NumOfChan == 1) {
      cout << "Number of Channels = 1; Parsing as mono" << endl;
      while (blocksRead>0) {
        blocksRead = fread(buffer, 2, 2, wavFile);
        if (blocksRead != wavHeader.blockAlign) { break; }
        //cout << "Read " << bytesRead << " bytes." << endl;
        int left = *buffer;
        mono_channel.push_back(left);
      }

    }
  }


  filelength = getFileSize(wavFile);

  cout << "File is                    :" << filelength << " bytes." << endl;
  cout << "RIFF header                :" << wavHeader.RIFF[0] << wavHeader.RIFF[1] << wavHeader.RIFF[2] << wavHeader.RIFF[3] << endl;
  cout << "WAVE header                :" << wavHeader.WAVE[0] << wavHeader.WAVE[1] << wavHeader.WAVE[2] << wavHeader.WAVE[3] << endl;
  cout << "FMT                        :" << wavHeader.fmt[0] << wavHeader.fmt[1] << wavHeader.fmt[2] << wavHeader.fmt[3] << endl;
  cout << "Data size                  :" << wavHeader.ChunkSize << endl;

  // Display the sampling Rate from the header
  cout << "Sampling Rate              :" << wavHeader.SamplesPerSec << endl;
  cout << "Number of bits per Samp    :" << wavHeader.bitsPerSample << endl;
  cout << "Number of channels         :" << wavHeader.NumOfChan << endl;
  cout << "Number of bytes per second :" << wavHeader.bytesPerSec << endl;
  cout << "Data length                :" << wavHeader.Subchunk2Size << endl;
  cout << "Audio Format               :" << wavHeader.AudioFormat << endl;
  // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM

  cout << "Block align(Bytes per samp):" << wavHeader.blockAlign << endl;
  cout << "Data string                :" << wavHeader.Subchunk2ID[0] << wavHeader.Subchunk2ID[1] << wavHeader.Subchunk2ID[2] << wavHeader.Subchunk2ID[3] << endl;


  //Remember to close your file
  fclose(wavFile);
}

vector<int> soundData::retreiveWaveChannel(){
  if (!mono_channel.empty())
  {
    cout << "Returning Mono Channel Data" << endl;
    return mono_channel;
  } else if (!left_channel.empty()) {
    cout << "Returning Left Channel Data" << endl;
    return left_channel;
  } else {
    cout << "Error in soundData::retreiveWaveChannel, no wave data found in object" << endl;
    cout << "Did you read a .wav before calling this function?" << endl;
    vector<int> bullshit;
    return bullshit;
  }
}
