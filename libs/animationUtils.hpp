#pragma once
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include "matplot/matplot.h"

void animate_single_channel(vector<int> channel, auto anifig, int index, string flname = "waveform", vector<float> clr = {0.f,0.7f,1.0f} )
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
      auto p = plot(x, channel)->color({clr[0],clr[1],clr[2]}); //blue
      p.line_width(8);
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

void animate_single_channelFloat(vector<float> channel, auto anifig, int index, string flname = "waveform", vector<float> clr = {1.f, 0.0625f, 0.9375f} )
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
      auto p = plot(x, channel)->color({clr[0],clr[1],clr[2]}); //pink
      p.line_width(8);
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
