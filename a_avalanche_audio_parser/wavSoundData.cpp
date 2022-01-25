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
    //http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
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

// Structure of the header of the .wav
typedef struct  smpl_HEADER
{
    //https://sites.google.com/site/musicgapi/technical-documents/wav-file-format#smpl
    /* smpl format Chunk Descriptor */
    uint32_t manufacturer;
    uint32_t product;
    uint32_t sample_period;
    uint32_t midi_unity_note; //Even tho its 4bytes, value is between 0 and 127
    uint32_t midi_pitch_fraction;
    uint32_t smpte_format;
    uint32_t smpte_offset;
    uint32_t num_sample_loops;
    uint32_t sampler_data;
} smpl_hdr;

typedef struct  data_HEADER
{
    //https://sites.google.com/site/musicgapi/technical-documents/wav-file-format#smpl
    /* smpl format Chunk Descriptor */
    uint32_t manufacturer;
    uint32_t product;
} data_hdr;

typedef struct  WAV_HEADER_EXTENSIBLE
{
    //http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
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
} wav_hdr_ex;


class soundData
{
  public:
    //Initialize all my 'instance variables'
    char * filePath;
    wav_hdr wavHeader;
    smpl_hdr smplData; //For SubChunk2ID = 'smpl'
    vector<int> left_channel;
    vector<int> right_channel;
    vector<int> mono_channel;
    vector<float> mono_channelf;
    vector<float> left_channelf;
    vector<float> right_channelf;
    //Troubleshooting; but it will saturate the whole console cache
    //print_vector_to_console(left_channel);

    bool float32 = true;

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
  if (wavHeader.AudioFormat == 1 ) {
    return;
  } else if (wavHeader.AudioFormat == 3 && wavHeader.Subchunk2ID[0] == 'd' && wavHeader.Subchunk2ID[1] == 'a' && wavHeader.Subchunk2ID[2] == 't' && wavHeader.Subchunk2ID[3] == 'a') {
        return;
  }
  else if (wavHeader.AudioFormat != 1 && wavHeader.Subchunk2ID[0] == 's' && wavHeader.Subchunk2ID[1] == 'm' && wavHeader.Subchunk2ID[2] == 'p' && wavHeader.Subchunk2ID[3] == 'l')
  {
    cout << "Additional processing is needed as this header indicates an audio format not equal to 1 (PCM)" << endl;
    cout << "smpl format detected.  Used to encode extra midi data to wav" << endl;

    wav.read((char*)&smplData.manufacturer, sizeof(smplData.manufacturer));
    wav.read((char*)&smplData.product, sizeof(smplData.product));
    wav.read((char*)&smplData.sample_period, sizeof(smplData.sample_period));
    wav.read((char*)&smplData.midi_unity_note, sizeof(smplData.midi_unity_note));
    wav.read((char*)&smplData.midi_pitch_fraction, sizeof(smplData.midi_pitch_fraction));
    wav.read((char*)&smplData.smpte_format, sizeof(smplData.smpte_format));
    wav.read((char*)&smplData.smpte_offset, sizeof(smplData.smpte_offset));
    wav.read((char*)&smplData.num_sample_loops, sizeof(smplData.num_sample_loops));
    wav.read((char*)&smplData.sampler_data, sizeof(smplData.sampler_data));
    cout << "Sampler data: " << smplData.sampler_data << endl << endl;
  }
  else {
    cout << "As of yet unsupported extensible wav data detected" << endl;
    cout<<wavHeader.AudioFormat << endl;
    cout<<wavHeader.Subchunk2ID[0]<<wavHeader.Subchunk2ID[1]<<wavHeader.Subchunk2ID[2]<<wavHeader.Subchunk2ID[3]<<endl;
    cout <<"Attempt to manually ignore useless bytes by editing line "<< __LINE__+3 << " in wavSoundData" << endl;
  }
  bool remove_manually = false;
  size_t remove_bytes = 60;
  uint8_t extraData[remove_bytes];

  if(remove_manually){
    cout << "removing extra data of size " << remove_bytes << " bytes" << endl;
    wav.read((char*)&extraData, sizeof(extraData));
  }
}

// find the file size
int soundData::getFileSize(FILE* inFile)
{
    int fileSize = 60;
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
  } else if (bytesPerSamp==4 && !float32) {
    uint32_t * buffer  = new uint32_t[1];
    while (blocksRead>0) {
      wav.read((char*)buffer, sizeof(*buffer));
      if (wav.eof()) { break; }
      int left = *buffer;
      mono_channel.push_back(left);
    }
  } else if (bytesPerSamp==4 && float32) {
    float * buffer  = new float[1];
    while (blocksRead>0) {
      wav.read((char*)buffer, sizeof(*buffer));
      if (wav.eof()) { break; }
      float left = *buffer;
      mono_channelf.push_back(left);
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
  } else if (bytesPerSamp==4 && !float32) {
    cout << "Running bytesPerSampe == 4 integer " << endl;
    float * buffer  = new float[1];
    while(!wav.eof()){
      wav.read((char*)buffer, sizeof(*buffer));
      float left = *buffer;
      left_channel.push_back(left);
      if (wav.eof()) { break; } //This is just in case right and left channel has mismatching data for some reason 0_o

      wav.read((char*)buffer, sizeof(*buffer));
      int right = *buffer;
      right_channel.push_back(right);
    }
  } else if (bytesPerSamp==4 && float32) {
    cout << "Running bytesPerSampe == 4 float " << endl;
    float * buffer  = new float[1];
    while (!wav.eof()) {
      wav.read((char*)buffer, sizeof(*buffer));
      float left = *buffer;
      left_channelf.push_back(left);
      if (wav.eof()) { break; } //This is just in case right and left channel has mismatching data for some reason 0_o

      wav.read((char*)buffer, sizeof(*buffer));
      float right = *buffer;
      right_channelf.push_back(right);
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

  cout << "RIFF header                :" << wavHeader.RIFF[0] << wavHeader.RIFF[1] << wavHeader.RIFF[2] << wavHeader.RIFF[3] << endl;
  cout << "Data size                  :" << wavHeader.ChunkSize << endl;
  cout << "WAVE header                :" << wavHeader.WAVE[0] << wavHeader.WAVE[1] << wavHeader.WAVE[2] << wavHeader.WAVE[3] << endl;
  cout << "FMT                        :" << wavHeader.fmt[0] << wavHeader.fmt[1] << wavHeader.fmt[2] << wavHeader.fmt[3] << endl;
  cout << "Subchunk1 size (fmt chunk) :" << wavHeader.Subchunk1Size << endl;
  // Display the sampling Rate from the header
  cout << "Sampling Rate              :" << wavHeader.SamplesPerSec << endl;
  cout << "Number of bits per Samp    :" << wavHeader.bitsPerSample << endl;
  cout << "Number of channels         :" << wavHeader.NumOfChan << endl;
  cout << "Number of bytes per second :" << wavHeader.bytesPerSec << endl;
  cout << "Audio Format               :" << wavHeader.AudioFormat << endl;
  // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM

  cout << "Block align(Bytes per samp):" << wavHeader.blockAlign << endl;
  cout << "Data string                :" << wavHeader.Subchunk2ID[0] << wavHeader.Subchunk2ID[1] << wavHeader.Subchunk2ID[2] << wavHeader.Subchunk2ID[3] << endl;
  cout << "Subchunk2Size              :" << wavHeader.Subchunk2Size << endl;

  //Remember to close your file
  wavStream.close();
  cout << "Closed wavStream" << endl;
}

vector<int> convert_vecf_to_veci(vector<float> f_vec){
  #include <limits>
  #include <algorithm>
  auto minmax = minmax_element(f_vec.begin(), f_vec.end()); // C++11+
  double max = *minmax.second;
  double test = abs(*minmax.first); //The minimum value in the vector
  if (test>max){
    max = test;
  }
  double scale_factor = (numeric_limits<int>::max())/(max);
  vector<int> i_vec;
  for (int index = 0; index < f_vec.size(); index++) {
    //cout << scale_factor << " :scale factor ||| adjusted value: " << (int) (scale_factor*f_vec[index]) << endl;
    i_vec.push_back((int) (scale_factor*f_vec[index]));
  }
  return i_vec;
}

vector<int> soundData::retreiveWaveChannel(){
  if (!mono_channel.empty())
  {
    cout << "Returning Mono Channel Data" << endl;
    return mono_channel;
  } else if (!left_channel.empty()) {
    cout << "Returning Left Channel Data" << endl;
    return left_channel;
  } else if (!left_channelf.empty()){
    cout << "Returning Left Channel Data, of FLOAT data type, cast to int" << endl;
    vector<int> result = convert_vecf_to_veci(left_channelf);
    return result;
  } else {
    cout << "Error in soundData::retreiveWaveChannel, no wave data found in object" << endl;
    cout << "Did you read a .wav before calling this function?" << endl;
    vector<int> bullshit;
    return bullshit;
  }
}
