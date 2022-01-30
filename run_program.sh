export LD_LIBRARY_PATH=/home/harrison/Documents/computer/cpp_packages/shared_object_lib/
WAV=./wavs/input.wav
if test -f "$WAV"; then
    echo "$WAV exists."
else 
    echo "$WAV does not exist.  Place wav file in /wavs/ and/or rename file to input.wav"
    exit
fi

#Ensure a location to place twp_files exists
if [ -d "./twp_files" ] 
then
    echo "Directory ./twp_files exists." 
else
    echo "Error: Directory ./twp_files does not exist.  Creating directory."
    mkdir twp_files
fi

#Create twp files from a .wav
cd ./a_avalanche_audio_parser/
./build_and_run.sh
wait
cd ..
#Check for a .twp file
TWP=./twp_files/output.twp
if test -f "$TWP"; then
    echo "$TWP exists."
else 
    echo "$TWP does not exist after running audio parser.  Something went wrong in the c++ program"
    exit
fi

#Check that there is a place for waveforms to go to
if [ -d "./waveforms" ] 
then
    echo "Directory ./waveforms exists." 
else
    echo "Error: Directory ./waveforms does not exist.  Creating directory."
    mkdir waveforms
fi

#Check that there is a place for PSDs to go to
if [ -d "./psds" ] 
then
    echo "Directory ./psds exists." 
else
    echo "Error: Directory ./psds does not exist.  Creating directory."
    mkdir psds
fi

#Check that there is a place for pre-inkscape processed .svg files to go to within ./b_avalanche_twp_former
if [ -d "./b_avalanche_twp_former/plot_saves" ] 
then
    echo "Directory ./b_avalanche_twp_former/plot_saves exists."
    if [ "$(ls -A ./b_avalanche_twp_former/plot_saves)" ]; then
     	echo "./b_avalanche_twp_former/plot_saves is not Empty"
     	echo "Deleting files in 30 seconds, unless script is cancelled"
	for i in {30..1..5};do echo -n "$i." && sleep 5; done
     	rm ./b_avalanche_twp_former/plot_saves/*
    else
    	echo "./b_avalanche_twp_former/plot_saves is Empty, proceeding..."
    fi
else
    echo "Error: Directory ./b_avalanche_twp_former/plot_saves does not exist.  Creating directory."
    mkdir ./b_avalanche_twp_former/plot_saves
fi

#Create the plots
cd ./b_avalanche_twp_former
./runAudioFlagger.sh
wait
cd ..

#Process the .svgs into transparent alpha pngs
./forloop_inkscape_svg_convert_transparent.sh
