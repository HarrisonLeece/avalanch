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
#include <iostream>
#include <fstream>


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
    void parse_header_and_body(const char * fileName);
    vector<int> retreiveWaveChannel();
    void mono_parser(uint16_t bytesPerSamp, fstream &wav);
    void stereo_parser(uint16_t bytesPerSamp, fstream &wav);
    void read_header(fstream &wav);
};

void soundData::read_header(fstream &wav){
  //RIFF header
  wav.read((char*)&wavHeader.RIFF, sizeof(wavHeader.RIFF));
  //cout<<wavHeader.RIFF[0]<<wavHeader.RIFF[1]<<wavHeader.RIFF[2]<<wavHeader.RIFF[3]<<endl;
  //ChunkSize
  wav.read((char*)&wavHeader.ChunkSize, sizeof(wavHeader.ChunkSize));
  //cout << wavHeader.ChunkSize << endl;
  //Wave header
  wav.read((char*)&wavHeader.WAVE, sizeof(wavHeader.WAVE));
  //cout<<wavHeader.WAVE[0]<<wavHeader.WAVE[1]<<wavHeader.WAVE[2]<<wavHeader.WAVE[3]<<endl;
  //fmt (check to make sure this is not junk and if it is... logic)
  while(wav.peek()!='f'){
    wav.ignore(1);
  }
  wav.read((char*)&wavHeader.fmt, sizeof(wavHeader.fmt));
  //Data size (excluding the 16 bits from RIFF and WAVE)
  wav.read((char*)&wavHeader.Subchunk1Size, sizeof(wavHeader.Subchunk1Size));
  //Audio Format
  wav.read((char*)&wavHeader.AudioFormat, sizeof(wavHeader.AudioFormat));
  //Number of channels
  wav.read((char*)&wavHeader.NumOfChan, sizeof(wavHeader.NumOfChan));
  //Sample rate
  wav.read((char*)&wavHeader.SamplesPerSec, sizeof(wavHeader.SamplesPerSec));
  //Number of bytes per second (byte rate)
  wav.read((char*)&wavHeader.bytesPerSec, sizeof(wavHeader.bytesPerSec));
  //Block align (Bytes per Sample) (one channel, I think)
  wav.read((char*)&wavHeader.blockAlign, sizeof(wavHeader.blockAlign));
  //bitsPerSample
  wav.read((char*)&wavHeader.bitsPerSample, sizeof(wavHeader.bitsPerSample));
  //data length (IDFK)
  wav.read((char*)&wavHeader.Subchunk2ID, sizeof(wavHeader.Subchunk2ID));
  //Subchunk2Size
  wav.read((char*)&wavHeader.Subchunk2Size, sizeof(wavHeader.Subchunk2Size));
}

// find the file size
int soundData::getFileSize(FILE* inFile)
{
    int fileSize = 0;
    fseek(inFile, 0, SEEK_END);

    fileSize = ftell(inFile);

    fseek(inFile, 0, SEEK_SET);
    return fileSize;
}

//if num channels = 1
void soundData::mono_parser(uint16_t bytesPerSamp, fstream &wav){
  //Switching for bytes per sample
  size_t blocksRead = 1;
  int trigger = 0;

  if (bytesPerSamp==1){
    int8_t * buffer  = new int8_t[1];
    while (!wav.eof()) {
      wav.read((char*)buffer, sizeof(*buffer));
      if (wav.eof()) { break; }
      int left = *buffer;
      mono_channel.push_back(left);
    }
  } else if (bytesPerSamp==2) {
    int16_t * buffer  = new int16_t[1];
    while (!wav.eof()) {
      wav.read((char*)buffer, sizeof(*buffer));
      if (wav.eof()) { break; }
      int left = *buffer;
      mono_channel.push_back(left);
    }
  } else if (bytesPerSamp==3) {
    cerr << "Number of bytes " << bytesPerSamp << " unsupported" << endl;
  } else if (bytesPerSamp==4) {
    int32_t * buffer  = new int32_t[1];
    while (blocksRead>0) {
      wav.read((char*)buffer, sizeof(*buffer));
      if (wav.eof()) { break; }
      int left = *buffer;
      mono_channel.push_back(left);
    }
  } else {
    cerr << "Number of bytes " << bytesPerSamp << " unsupported" << endl;
  }

}

//if num channels = 2
void soundData::stereo_parser(uint16_t bytesPerSamp, fstream &wav){
  //Switching for bytes per sample
  if (bytesPerSamp==1){
    cerr << "Number of bytes " << bytesPerSamp << " unsupported in stereo" << endl;
  } else if (bytesPerSamp==2) {
    int16_t * buffer  = new int16_t[1];
    //cout << bytesRead << endl;
    while (!wav.eof()) {
      if (wav.eof()) { break; }
      wav.read((char*)buffer, sizeof(*buffer));
      int left = *buffer;
      left_channel.push_back(left);
      if (wav.eof()) { break; } //This is just in case right and left channel has mismatching data for some reason 0_o
      wav.read((char*)buffer, sizeof(*buffer));
      int right = *buffer;
      right_channel.push_back(right);
    }
  } else if (bytesPerSamp==3) {
    cerr << "Number of bytes " << bytesPerSamp << " unsupported in stereo" << endl;
  } else if (bytesPerSamp==4) {
    int32_t * buffer  = new int32_t[1];
    //cout << bytesRead << endl;
    while (!wav.eof()) {
      if (wav.eof()) { break; }
      wav.read((char*)buffer, sizeof(*buffer));
      int left = *buffer;
      left_channel.push_back(left);
      if (wav.eof()) { break; } //This is just in case right and left channel has mismatching data for some reason 0_o
      wav.read((char*)buffer, sizeof(*buffer));
      int right = *buffer;
      right_channel.push_back(right);
    }
  } else {
    cerr << "Number of bytes " << bytesPerSamp << " unsupported in stereo" << endl;
  }
}

void soundData::parse_header_and_body(const char * fileName)
{

  int filelength = 0;

  fstream wavStream;
  wavStream.open(fileName, std::fstream::in | std::fstream::binary);
  //Read the header
  read_header(wavStream);

  //size_t blocksRead = fread(&wavHeader, 1, headerSize, wavFile);
  size_t blocksRead = 0;
  cout << "Header Read " << blocksRead << " bytes." << endl;
  bool testing = false;
  if (!testing)
  {
    //Read the data
    uint16_t bytesPerSample = wavHeader.bitsPerSample / 8;      //Number     of bytes per sample
    cout << "Bytes per sample " << bytesPerSample << endl;
    uint64_t numSamples = wavHeader.ChunkSize / bytesPerSample; //How many samples are in the wav file?

    //static const uint16_t BUFFER_SIZE = 4096; //Why is this hardcoded
    //int8_t* buffer = new int8_t[wavHeader.NumOfChan];

    ////////////////
    //Get the data from the wav
    ////////////////

    //Every f read should read the data for one sample, and then store it to a vector
    if (wavHeader.NumOfChan == 2) {
      cout << "Number of Channels = 2; Parsing as stereo" << endl;
      stereo_parser(bytesPerSample, wavStream);
    } else if (wavHeader.NumOfChan == 1) {
      cout << "Number of Channels = 1; Parsing as mono" << endl;
      mono_parser(bytesPerSample, wavStream);
    }
  } else {
    cerr << "Error parsing the header" << endl;
    terminate();
  }


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
  cout << "Subchunk2Size              :" << wavHeader.Subchunk2Size << endl;

  //Remember to close your file
  wavStream.close();
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
