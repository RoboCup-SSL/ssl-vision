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

## Supported cameras

Multiple cameras are supported:

 * 1394B / Firewire 800 (backward compatible with 1394A)
 * Basic usb camera support via the [Video for Linux (V4L)](http://linuxtv.org/downloads/v4l-dvb-apis/) drivers
 * Matrix-Vision BlueFox (USB 2.0) and BlueFox3 (USB 3.0) cameras via [mvIMPACT Acquire SDK](http://www.matrix-vision.com/software-drivers-en.html)
 * Basler cameras via the [Pylon Software Suite](https://www.baslerweb.com/en/products/software/basler-pylon-camera-software-suite/)
 * FLIR cameras via the [SPINNAKER and FLYCAP SDK](https://www.flir.com/support-center/iis/machine-vision/downloads/spinnaker-sdk-flycapture-and-firmware-download/)
 
To enable support for one or more of those cameras, install the corresponding SDK (linked above and described in more details below) first.
Then build with the corresponding option:
 
 * `-DUSE_DC1394=true`
 * `-DUSE_SPINNAKER=true`
 * `-DUSE_mvIMPACT=true`
 * `-DUSE_PYLON=true`
 * `-DUSE_FLYCAP=true`
 * `-DUSE_V4L=true`
 
Example: `cd build; cmake -DUSE_SPINNAKER=true ..`. As these are cached cmake option, you only need to run this once and can build with `make` afterwards.

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
Installation instructions are contained in the download. 

Tested with [Basler ace acA1300-75gc](https://www.baslerweb.com/en/products/cameras/area-scan-cameras/ace/aca1300-75gc/). 


### FLIR cameras

USB 3.0 cameras are currently supported.

Tested cameras:
 * Blackfly S (BFS-U3-51S5C-C)
 
Download and install the [SDK](https://www.flir.com/products/spinnaker-sdk) and build ssl-vision with `-DUSE_SPINNAKER=true`.

## Compilation

Build the code by running:
```bash
make
```
If you need to pass extra parameters to cmake, you need to run `cmake` directly:
```bash
cd build
cmake -DUSE_WHAT_SO_EVER=true ..
cd ..
make
```
The `USE_*` parameters are cached, so they do not have to be passed in
each time.

### Apriltags Support

There is experimental support for detecting and tracking robots via
[Apriltags](https://april.eecs.umich.edu/software/apriltag). The
standard tag sets in the [Apriltags Imgs
Repo](https://github.com/AprilRobotics/apriltag-imgs) are all
supported along with two customized tag sets:

- Custom20h7: 26 unique tags

To generate the custom tag images you must use the
[Apriltag-Generation
Repo](https://github.com/AprilRobotics/apriltag-generation). Follow
the instructions listed in the repo. The tag layouts are as follows:

- Custom20h7

  custom_xxddddxxxwwwwwwxdwbbbbwddwbddbwddwbddbwddwbbbbwdxwwwwwwxxxddddxx
  
The hamming distance used during generation are as follows:

- Custom20h7: 7

To enable you must set the `USE_APRILTAG` option to `ON`. e.g.:

``` bash
cd build
cmake -DUSE_APRILTAG=ON ..
cd ..
make
```

This will add a new plugin to each camera that runs on every frame. At
the moment the old robot detection patterns are detected in addition
to the apriltag detection.

## Running

Depending on your OS, you might need to ensure that you have full access to the firewire devices /dev/fw*.
This *might* require logging in as root or adding your user to a certain group.

Run the software using the following command:
```bash
./bin/vision
```

You can automatically start capturing with the `-s` option.

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

#### Apriltags

Unlike the butterfly pattern, there are no colors in the apriltags. So
you must manually assign tag ids to the different team colors. Under
Global settings there are two settings list "Blue April Tags" and
"Yellow April Tags". Expand each list and click "Add Tag" to create a
new tag entry for the associated team color. Expand the new entry and
edit the value to a valid tag id in order to begin detection.

You must also configure the apriltag detection settings. The settings
are per camera under the "AprilTag" settings in each thread. Select
the tag family and configure the size in mm of the side of the
Apriltag quad. See the apriltags repository for explanation of the
other options and how they affect detection accuracy and speed.

Apriltag detection takes place on a greyscale version of the image. To
see this image enable "greyscale" un the Visualization plugin for the
appropriate camera. If a tag is not detecting check the greyscale
image contrast. Verify that the tag doesn't have a glare or a large
contrast due to shadows.

Under the Visualization plugin you can also enable the "detected
AprilTags" option. This will draw a cyan border around the quad of the
tag (i.e. the part you should measure and enter as quad size in the
detector settings). It will also draw the tag id in magenta over the
detected tag.

##### Large Distortions

Currently, the tag detection runs on a raw image provided by the
camera to maximize performance. Fixes for distortion happen *AFTER*
detection. However, the apriltag detector is designed to work on
undistorted (aka rectified) images. If your camera lens has a large
distortion (you can see this by the field lines and other image lines
being highly curved), then the detection accuracy may be lower or even
fail.
