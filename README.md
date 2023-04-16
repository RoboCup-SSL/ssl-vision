[![CircleCI](https://circleci.com/gh/RoboCup-SSL/ssl-vision/tree/master.svg?style=svg)](https://circleci.com/gh/RoboCup-SSL/ssl-vision/tree/master)

# RoboCup Small Size League Shared Vision System

In the past, RoboCup Small Size League rules allowed every team to set up their own global vision system as a primary sensor. 
This option beared several organizational limitations and costs for the teams, thus impairing the league's progress. 
Additionally, most teams had converged on similar solutions and produced few significant research results to this global vision problem over the last years.

In 2009, the league committees decided to migrate to a shared vision system (including hardware) that could be used by all teams. 
This system - named SSL-Vision - is currently developed by volunteers from participating teams.

To find more in-depth and up-to-date information about SSL-Vision (including installation and configuration), 
please visit the [Wiki Documentation Page](https://github.com/RoboCup-SSL/ssl-vision/wiki).
 
## Software Requirements

Following dependencies are required to build the software:

 * g++
 * QT >= 4.3 with opengl and networking support
 * cmake
 * Eigen3
 * Google protocol buffers (protoc)
 * OpenGL
 * GLU
 * libjpeg
 * libpng
 * OpenCV >= 3
 * libdc1394 Version >= 2.0
 * video for linux 2 (v4l)

To get all of these packages in (k)ubuntu, run the `InstallPackagesUbuntu.sh` script.

Or, in archlinux, run the `InstallPackagesArch.sh` script.

## Test data

If you do not have a camera available and want to test or improve ssl-vision, you can use the `CaptureFromFile` capture module to play back images.
To install some test data, run: `make install_test_data`. It will download several test images to [test-data](./test-data).
The default configuration of ssl-vision will peak this up automatically.

## Supported cameras

Multiple cameras are supported:

 * 1394B / Firewire 800 (backward compatible with 1394A)
 * Basic usb camera support via the [Video for Linux (V4L)](http://linuxtv.org/downloads/v4l-dvb-apis/) drivers
 * Matrix-Vision BlueFox (USB 2.0) and BlueFox3 (USB 3.0) cameras via [mvIMPACT Acquire SDK](http://www.matrix-vision.com/software-drivers-en.html)
 * Basler cameras via the [Pylon Software Suite](https://www.baslerweb.com/en/products/software/basler-pylon-camera-software-suite/) (currently supports 7.2.1)
 * FLIR cameras via the [SPINNAKER and FLYCAP SDK](https://www.flir.com/support-center/iis/machine-vision/downloads/spinnaker-sdk-flycapture-and-firmware-download/)
 
To enable support for one or more of those cameras, install the corresponding SDK (linked above and described in more details below) first.
Then build with the corresponding option:
 
 * `-DUSE_DC1394=true`
 * `-DUSE_SPINNAKER=true`
 * `-DUSE_mvIMPACT=true`
 * `-DUSE_PYLON=true`
 * `-DUSE_FLYCAP=true`
 * `-DUSE_V4L=true`
 
Example for a release build: `cmake -B build -DUSE_SPINNAKER=true`.
 As these are cached cmake options, you only need to run this once and can build with `make` afterwards.

### Virtual Splitter cameras

In addition to the physical cameras, you can activate virtual cameras with `-DUSE_SPLITTER=true`. 
There will be an additional 'Distributor Thread' that captures from a single physical camera (or from file).
The capture mode of the normal camera threads can be set to 'splitter'. That way, a part of the original image from the
distributor thread is used as input.
This may speedup processing time for cameras with large resolutions, but at the trait-of of multiple cameras with the
same camera center, which may not work well with some consumers.


### Matrix-Vision cameras

USB 2.0 [BlueFox MLC](https://www.matrix-vision.com/USB2.0-single-board-camera-mvbluefox-mlc.html) and 
USB 3.0 [BlueFox3-2](https://www.matrix-vision.com/USB3-vision-camera-mvbluefox3-2.html) cameras are supported.
Please note, that they require different SDKs. The SDK look very similar, but are not compatible. They get installed into the same directory by default.
 
Tested cameras: 
 * mvBlueFOX-MLC200wC
 * mvBlueFOX3-2089 
 
The SDK can be downloaded from the [driver page](https://www.matrix-vision.com/software-drivers-en.html). Go to Linux => mvBlueFOX (USB2.0) or mvBlueFOX3 (USB3.0). 
Download the `install_mvBlueFOX.sh` script and the correct `.tgz` file for your machine. 
Open a terminal and navigate to your download folder. For a quick installation run:
```
sh install_mvBlueFOX.sh -u
```

### Basler cameras

Basler cameras are supported via the [Pylon Software Suite](https://www.baslerweb.com/en/products/software/basler-pylon-camera-software-suite/). 
[This link](https://www.baslerweb.com/fp-1668420813/media/downloads/software/pylon_software/pylon_7.2.1.25747_x86_64_debs.tar.gz) should directly download the current supported version, 7.2.1.
Installation instructions are contained in the download. 

Tested with [Basler ace acA1300-75gc](https://www.baslerweb.com/en/products/cameras/area-scan-cameras/ace/aca1300-75gc/). 


### FLIR cameras

USB 3.0 cameras are currently supported.

Tested cameras:
 * Blackfly S (BFS-U3-51S5C-C) - [Documentation](https://www.flir.de/support-center/iis/machine-vision/knowledge-base/technical-documentation-blackfly-s-usb3/)
 
Download and install the [SDK](https://www.flir.com/products/spinnaker-sdk) and build ssl-vision with `-DUSE_SPINNAKER=true`.

## Compilation

Build the code by running:
```bash
make
```
If you need to pass extra parameters to cmake, you need to run `cmake` directly:
```bash
cmake -B build -DUSE_WHAT_SO_EVER=true
make
```
The `USE_*` parameters are cached, so they do not have to be passed in each time.

## Running

Depending on your OS, you might need to ensure that you have full access to the firewire devices /dev/fw*.
This *might* require logging in as root or adding your user to a certain group.

Run the software using the following command:
```bash
./bin/vision
```

You can automatically start capturing with the `-s` option.

If all `.` turn into `,` in robocup-ssl-teams.xml, you can change this by running
```shell
export LC_NUMERIC=en_US.UTF-8
```
before running `vision`. This is not required, though.

### Starting to Capture and Setting Parameters

Once the software is running, you should see some empty capture frames
on the right, and a data-tree structure on the left.  In this 
data-structure you can setup your camera parameters, 
such as resolution, capture mode, etc.

A quick hint: The text-field below the data-tree allows for
fast search through the data-tree.

See the section of DC1394 parameters below to get an idea of what the
parameters do.

Once you have them set up, you can start capturing by clicking
*"Image Capture/Capture Control/Start"* in the data-tree.

#### DC1394 Parameters

* If you expand the tree then the capture parameters are in
"Image Capture/DC1394/Capture Settings"

* Furthermore, conversion settings are in
"Image Capture/DC1394/Conversion Settings"

* "convert to mode" should currently be "yuv422" 
for best performance, "capture mode" should also be "yuv422",
but can also be a different format such as "yuv411" or "rgb"

* Alternatively, if you desire, you can do de-bayering in software,
but this will take extra CPU cycles. To do so, you would set the
capture mode to e.g. "raw8" and the convert mode to "rgb". Finally,
you will need to set "de-bayer" to true, and select the correct
de-bayer pattern and desired method.

* Capturing supports both DCAM native modes and Format7 modes.
This is selected in the "capture format" field. Leaving it on
"auto" will attempt native mode first, then `format7_0`.

* By default, ISO800 support is disabled. To enable it, mark the
field "use ISO800" as true.

##### ONLINE PARAMETERS

* Once you start capturing, you should see the realtime video image
on the right.  Furthermore, in the data tree, you should be able to go to the
"Camera Parameters" node which will then be expandable and show
all of your DCAM parameters.

* You can adjust all of these parameters in real-time.

* Note that the read-out of these parameters from the camera
only happens automatically if "auto refresh params" in the
Capture Control is set to true. Otherwise, you can use
the "re-read params" "Refresh" button to refresh them manually.
For performance reasons, it might make sense to set auto refresh
to false, so the bus is not being flooded with too much control
data and has full bandwidth available for the video streaming.

#### Storage of Settings and Parameters 

* When you quit the application normally *parameters will be 
saved automatically*, but on a system crash, they won't. 
Make sure to regularly save the settings manually by click on the `Save Settings`
button immediately under the main `Vision System` node.

* All settings will be automatically restored during the next
program start.

* In case the files should ever become corrupted, or the
program refuses to start completely when parsing the XML files
(this should normally never occur) then simply delete all
XML files and restart. The program will restore its default
settings.
