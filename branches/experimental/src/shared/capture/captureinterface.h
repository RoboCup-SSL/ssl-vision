//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    captureinterface.h
  \brief   C++ Interface: CaptureInterface
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef CAPTUREINTERFACE_H
#define CAPTUREINTERFACE_H
#include <string>
#include <stdio.h>

#include "rawimage.h"
#include "colors.h"
#include "VarTypes.h"
using namespace VarTypes;
/*!
  \class   CaptureInterface
  \brief   The interface to be used by all video capture methods
  \author  Stefan Zickler, (C) 2008

  The CaptureInterface is the base class that should be inherited for
  any particular capturing method implementation.

  It relies on the VarTypes system (see \see VarTypes.h) for the
  management an introspection-abilities of settings.

*/
class CaptureInterface
{
public:
    CaptureInterface(VarList * _settings=0);

    virtual ~CaptureInterface();

    /// Any settings that your video capture method provides should
    /// be added as VarTypes-nodes in the VarList tree below:
    /// Please see VarTypes.h for more information on how to use
    /// VarTypes, or take a look at an existing capture method
    /// which implements this interface.
    VarList * settings;

    /// This returns a raw-image with a pointer directly to the video-buffer
    /// Note, that this pointer is only guaranteed to point to a valid
    /// memory location until releaseFrame() is called.
    virtual RawImage getFrame()     = 0;

    /// This function should return true, if your method is currently
    /// actively capturing data.
    virtual bool     isCapturing() = 0;

    /// This releases the pointer of a previous \c getFrame() call.
    virtual void     releaseFrame() = 0;

    /// This will make your method start capturing data
    /// Note, that upon construction, your class should NOT be starting
    /// to capture data automatically.
    /// Instead a call to startCapture() is required.
    virtual bool     startCapture() = 0;

    /// This will make your method stop capturing data
    virtual bool     stopCapture() = 0;

    /// If applicable, this will enforce a reinitialization / reset
    /// on your capturing hardware, in case something went wrong.
    ///
    /// This is applicable, for example for DC1394 based systems
    /// where a firewire bus is able to 'freeze up' and can be
    /// reinitialized by a bus reset.
    virtual bool     resetBus();

    /// If your capture device provides some kind of parameter readout
    /// that isn't automatically read-out at every frame, then
    /// this function should force a readout of such parameters.
    virtual void     readAllParameterValues();

    /// This function will allow the copying of a captured frame
    /// to another RawImage data-structure.
    /// Overloading this function is recommended to provide more advanced
    /// functionality, such as converting the frame from one format to another.
    /// which format to convert to, should be a VarTypes option, added
    /// to the settings above.
    /// For an example implementation, please see how this function
    /// is overloaded in the CaptureDC1394v2 class.
    ///
    /// In its current implementation in this base-interface,
    /// all this function will do is allocate the target image (if not
    /// already allocated, and then memcpy the data as-is.
    virtual bool     copyAndConvertFrame(const RawImage & src, RawImage & target);

    /// Return a string describing your capture method
    /// e.g. DC1394B, or GigEVision, or V4LCapture, or USBCam,...
    virtual string   getCaptureMethodName() const = 0;
};

#endif
