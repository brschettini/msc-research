#!/bin/bash

# Tools
sudo apt update
sudo apt install \
	build-essential \
	cmake \
	pkg-config \
	meshlab \
	gthumb \
	ffmpeg \
	libopencv-dev \
	libfreenect-dev \
	libfreenect0.5 \
	libomp-dev \
	libboost-all-dev \
	libsoil-dev \
	freeglut3-dev \
	libglew-dev \
	libxxf86vm-dev -y

sudo ln -s /usr/lib/x86_64-linux-gnu/libglut.a /usr/lib/libglut.a
sudo ln -s /usr/lib/x86_64-linux-gnu/libglut.so /usr/lib/libglut.so
sudo ln -s /usr/lib/x86_64-linux-gnu/libglut.so.3 /usr/lib/libglut.so.3
sudo ln -s /usr/lib/x86_64-linux-gnu/libglut.so.3.9.0 /usr/lib/libglut.so.3.9.0

sudo apt install snap
sudo snap install meshlab
