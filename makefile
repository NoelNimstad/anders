compile:
	gcc source/anders/*.c source/*.c -o anders -I./ -ISDL -Wl,-rpath,SDL -FSDL -framework SDL3 -lz

ffmpeg:
	ffmpeg -framerate 60 -i render/%08d.bmp -c:v libx264 -pix_fmt yuv420p output.mp4