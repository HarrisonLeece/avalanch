ffmpeg  -framerate 60 -start_number 0 -i ./plot_saves/flaggerPSD%d.jpeg -c:v vp9 -crf 11 -b:v 4M -vf "format=rgba" ./videos/psd.webm
