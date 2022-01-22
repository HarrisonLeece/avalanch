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

using namespace std;


///////
// Plot waveforms or arbitrary vectors
///////
void plot_mono_channel(vector<int> channel)
{
    using namespace matplot;
    int length = channel.size();
    cout << "Running mono plotter" << endl;

    vector<double> tmp = linspace(0, length-1, length);
    vector<int> x(length);
    for (int i=0; i < length; i++){
      x[i]=int (tmp[i]);
    }
    try {
      plot(x, channel)->color({0.f, 0.7f, 0.9f});
      title("Single Channel Plot");
      xlabel("# samples");
      ylabel("Channel values");
    } catch (const runtime_error& e) {
      cout << "plot_stereo_channel; An error occured while plotting the graphs" << endl;
      cout << "sample indexes (variable x)" << endl;
      for (int i=0; i < length; i++){
        cout << x[i] << ", ";
      }
      cout << endl << "single channel values (variable channel)" << endl;
      for (int i=0; i < length; i++){
        cout << channel[i] << ", ";
      }
    }
    show();
}

void plot_stereo_channel(vector<int> left_channel, vector<int> right_channel)
{
    using namespace matplot;
    cout << "Running stereo plotter" << endl;
    int length = left_channel.size();
    if (length != right_channel.size()){
      cout << "Error in plot_stereo_channel; left and right channels size mismatch" << endl;
      cout << "left channel: "  << length << " samples; right channel: " << right_channel.size() << " samples" << endl;
      return;
    }
    //vector of integers, x, should be the length of the number of samples starting at zero
    vector<double> x = linspace(0, length-1, length);
    //print_vector_to_console_double(x);
    //In case we find out we have to cast the vector of doubles to int...
    //vector<int> x((length));
    //for (int i = 0; i < length; i++){
    //  x.at(i) = int (tmp[i]);
    //}
    //print_vector_to_console_int(x);
    try {
      hold(on);
      plot(x, left_channel)->color({0.f, 0.7f, 0.9f});
      plot(x,right_channel)->color({0.9f, 0.f, 0.1f});
      hold(off);
      title("Stereo Channels Plot");
      xlabel("# samples");
      ylabel("Channel values");
    } catch (const runtime_error& e) {
      cout << "plot_stereo_channel; An error occured while plotting the graphs" << endl;
      cout << "sample indexes (variable x)" << endl;
      for (int i=0; i < length; i++){
        cout << x[i] << ", ";
      }
      cout << endl << "left channel values (variable left_channel)" << endl;
      for (int i=0; i < length; i++){
        cout << left_channel[i] << ", ";
      }
      cout << endl << "right channel values (variable right_channel)" << endl;
      for (int i=0; i < length; i++){
        cout << right_channel[i] << ", ";
      }
    }
    show();
}
//


void animate_single_channel(vector<int> channel, auto anifig, int index, string flname = "waveform" )
{
    using namespace matplot;
    //anifig->clear(); // clear the plot
    int length = channel.size();
    cout << "Running single channel animator" << endl;

    //create suitable x axis linspace
    vector<double> tmp = linspace(0, length-1, length);
    vector<int> x(length);
    for (int i=0; i < length; i++){
      x[i]=int (tmp[i]);
    }

    try {
      auto p = plot(x, channel)->color({0.f, 0.7f, 0.9f}); //blue
      p.line_width(5);
      //title("Single Channel Plot");
      //xlabel("Sample");
      //ylabel("Channel values");

    } catch (const runtime_error& e) {
      cout << "animate_single_channel; An error occured while plotting the graphs" << endl;
      //print the x axis
      cout << "sample indexes (variable x): " << endl;
      for (int i=0; i < length; i++){
        cout << x[i] << ", ";
      }
      //print the y axis
      cout << endl << "single channel values (variable channel)" << endl;
      for (int i=0; i < length; i++){
        cout << channel[i] << ", ";
      }
    }
    string fileName = "./plot_saves/"+flname;
    fileName = fileName + to_string(index);
    save(fileName,"svg");
    anifig->draw();
}

void animate_single_channelFloat(vector<float> channel, auto anifig, int index, string flname = "waveform" )
{
    using namespace matplot;
    //anifig->clear(); // clear the plot
    int length = channel.size();
    cout << "Running single channel animator" << endl;

    //create suitable x axis linspace
    vector<double> tmp = linspace(0, length-1, length);
    vector<int> x(length);
    for (int i=0; i < length; i++){
      x[i]=int (tmp[i]);
    }

    try {
      auto p = plot(x, channel)->color({1.f, 0.0625f, 0.9375f}); //pink
      p.line_width(5);
      //title("Single Channel Plot");
      //xlabel("Sample");
      //ylabel("Channel values");

    } catch (const runtime_error& e) {
      cout << "animate_single_channel; An error occured while plotting the graphs" << endl;
      //print the x axis
      cout << "sample indexes (variable x): " << endl;
      for (int i=0; i < length; i++){
        cout << x[i] << ", ";
      }
      //print the y axis
      cout << endl << "single channel values (variable channel)" << endl;
      for (int i=0; i < length; i++){
        cout << channel[i] << ", ";
      }
    }
    string fileName = "./plot_saves/"+flname;
    fileName = fileName + to_string(index);
    save(fileName,"svg");
    anifig->draw();
}

///////
// Helper functions for plotting simple vectors of numbers
///////
void plot_vector_double(vector<double> in_vec){
  using namespace matplot;
  int length = in_vec.size();
  vector<double> x = linspace(0, length-1, length);

  plot(x, in_vec)->color({0.f, 0.7f, 0.9f});
  show();
}
void plot_vector_int(vector<int> in_vec){
  using namespace matplot;
  int length = in_vec.size();
  vector<double> x = linspace(0, length-1, length);

  plot(x, in_vec)->color({0.f, 0.7f, 0.9f});
  show();
}
////////
