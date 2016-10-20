```
========================================================================
  This software is free: you can redistribute it and/or modify
  it under the terms of the GNU General Public License Version 3,
  as published by the Free Software Foundation.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  Version 3 in the file COPYING that came with this distribution.
  If not, see <http://www.gnu.org/licenses/>.
========================================================================
 ssl-vision
 RoboCup Small Size League Shared Vision System 
 https://github.com/RoboCup-SSL/ssl-vision
========================================================================
```

## Online Documentation

  To find more in-depth and up-to-date information about SSL-Vision
  (including installation and configuration), please visit the Wiki
  Documentation Page:

  https://github.com/RoboCup-SSL/ssl-vision/wiki
 
## Software Requirements
 * g++
 * QT >= 4.3 with opengl and networking support
 * cmake
 * Eigen3
 * Google protocol buffers (protoc)
 * OpenGL
 * GLU
 * libdc1394 Version >= 2.0
 * libjpeg
 * libpng
 * video for linux 2 (v4l)

To get all of these packages in (k)ubuntu, run:
```
    sudo apt-get install g++ libqt4-dev libeigen3-dev protobuf-compiler libprotobuf-dev libdc1394-22 libdc1394-22-dev cmake libv4l-0
```

## Hardware Requirements
 * The system supports 1394B / Firewire 800, but it's also backward compatible with 1394A.
 * The system also supports basic usb cameras via the [http://linuxtv.org/downloads/v4l-dvb-apis/](Video for Linux (V4L)) drivers. This implementation has only been tested on linux.

### (Optional) Matrix-Vision [BlueFox2](http://www.matrix-vision.com/USB2.0-single-board-camera-mvbluefox-mlc.html)
This camera type is supported via the mvIMPACT_acquire library. Tested with mvBlueFOX-MLC200wC. If the library is not found on your computer support for this camera will be disabled silently. To enable the BlueFox2 capture module please go to the Matrix-Vision [driver page](http://www.matrix-vision.com/software-drivers-en.html). Go to Linux => mvBlueFOX (USB2.0). Download the install_mvBlueFOX script and the correct .tgz file for your machine. Open a terminal and navigate to your download folder. For a quick installation run:
```
chmod +x install_mvBlueFOX.sh
./install_mvBlueFOX.sh -u
```

## Compilation
 build the code by running:
```
    make
```
 The project *should* build without errors or warnings.

## Running
  1. depending on your OS, you might need to ensure that you
     have full access to the firewire devices /dev/fw*
     This *might* require logging in as root.
  2. alternatively, the V4L drivers will attempt to open a 
     device enumerated as /dev/videoN where N is a zero-based index
  3. run the software using the following command:
```
    ./bin/vision
```

### Starting to Capture and Setting Parameters
   Once the software is running, you should see two empty capture frames
   on the right, and a data-tree structure on the left.  In this 
   data-structure you can setup your camera parameters, 
   such as resolution, capture mode, etc.

   *A quick hint: the text-field below the data-tree allows for
   fast search through the data-tree.*

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

   * When you quit the application normally *parameters will not be 
   automatically saved*.  Insetad, you need to click on the `Save Settings`
   button immediately under the main `Vision System` node.

   * All settings should be automatically restored during the next
   program start.

   * In case the files should ever become corrupted, or the
   program refuses to start completely when parsing the XML files
   (this should normally never occur) then simply delete all
   XML files and restart. The program should restore its default
   settings.
