#!/usr/bin/env bash

sudo apt-get update

readonly ubuntu_release="$(grep "DISTRIB_RELEASE" /etc/lsb-release | sed -e 's/DISTRIB_RELEASE=//')"
if [[ "${ubuntu_release}" == "22.04" ]]; then
  sudo apt-get install -qq \
    g++ cmake \
    libeigen3-dev freeglut3-dev libopencv-dev \
    qtdeclarative5-dev libqt5multimedia5 \
    protobuf-compiler libprotobuf-dev \
    libdc1394-25 libdc1394-dev \
    libv4l-0
elif [[ "${ubuntu_release}" == "20.04" ]]; then
  sudo apt-get install -qq \
    g++ cmake \
    libeigen3-dev freeglut3-dev libopencv-dev \
    qtdeclarative5-dev libqt5multimedia5 \
    protobuf-compiler libprotobuf-dev \
    libdc1394-22 libdc1394-22-dev \
    libv4l-0
else
  echo "Ubuntu release not supported: ${ubuntu_release}" >&2
  exit 1
fi
