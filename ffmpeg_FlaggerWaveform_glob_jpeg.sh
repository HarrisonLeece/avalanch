ffmpeg  -framerate 60 -start_number 12 -i ./plot_saves/flaggerWaveform%d.jpeg -c:v vp9 -crf 11 -b:v 4M -vf "format=rgba" ./videos/flagger_wave.webm
