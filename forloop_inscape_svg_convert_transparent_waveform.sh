for i in $(seq 0 1 599 $END); 
	do 
		echo $i;
		inkscape --export-filename="./tempPNG/waveform${i}.png" --export-background-opacity=0 --export-type=png "./plot_saves/flaggerWaveform${i}.svg"
done
