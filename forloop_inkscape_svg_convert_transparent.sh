echo "Enter the number (integer) of frames in the video (should be half of plot_saves).  This input will be used for both PSD and Waveform processing"
read frames
if [[ ! $frames =~ ^[0-9]+$ ]] ; then
    echo "Not a valid input"
    exit
fi
echo "Beginning PSD transformation"
for i in $(seq 0 1 $frames $END); 
	do 
		echo $i;
		inkscape --export-filename="./psds/psd${i}.png" --export-background-opacity=0 --export-type=png "./b_avalanche_twp_former/plot_saves/flaggerPSD${i}.svg" &
		if (($i % 10 == 0))
		then 
			wait
		fi
done
wait
echo "Beginning wavefrom transformation"
for i in $(seq 0 1 $frames $END); 
	do 
		echo $i;
		inkscape --export-filename="./waveforms/waveform${i}.png" --export-background-opacity=0 --export-type=png "./b_avalanche_twp_former/plot_saves/flaggerWaveform${i}.svg" &
		if (($i % 10 == 0)) 
		then 
			wait		
		fi
done
wait
