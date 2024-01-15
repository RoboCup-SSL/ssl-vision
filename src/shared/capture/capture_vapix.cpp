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
  \file    capture_vapix.cpp
  \brief   C++ Implementation: CaptureVapix
  \author  Sebastian Olsson, (C) 2023
*/
//========================================================================

#include "capture_vapix.h"
#include <memory>


int CaptureVapix::num_cams = 0;

CaptureVapix::CaptureVapix(VarList *_settings, int default_camera_id, QObject *parent) : QObject(parent), CaptureInterface(_settings) {
  is_capturing = false;


  mutex.lock();

  num_cams++;
  settings->addChild(capture_settings = new VarList("Capture Settings"));
  settings->addChild(dcam_parameters = new VarList("Camera Parameters"));

  //=======================CAPTURE SETTINGS==========================
  //TODO: Keep?: v_cam_network_prefix = new VarString("cam network prefix", default_network_prefix);

  v_cam_bus = new VarInt("cam idx", default_camera_id);
  v_cam_schema = new VarString("cam url schema", "http://");
  v_cam_username = new VarString("cam username", "");
  v_cam_password = new VarString("cam password", "");
  v_cam_ip = new VarString("cam ip", "");
  v_cam_port = new VarString("cam port", "");
  v_cam_video_stream_route = new VarString("cam video stream route", "");

  v_convert_to_mode = new VarStringEnum("convert to mode", Colors::colorFormatToString(COLOR_RGB8));
  v_convert_to_mode->addItem(Colors::colorFormatToString(COLOR_RGB8));
  v_convert_to_mode->addItem(Colors::colorFormatToString(COLOR_RAW8));

  capture_settings->addChild(v_cam_bus);
  capture_settings->addChild(v_cam_schema);
  capture_settings->addChild(v_cam_ip);
  capture_settings->addChild(v_cam_port);
  capture_settings->addChild(v_cam_video_stream_route);
  capture_settings->addChild(v_cam_username);
  capture_settings->addChild(v_cam_password);
  capture_settings->addChild(v_convert_to_mode);

  // //=======================CAMERA SETTINGS==========================
  // v_acquisition = new VarBool("Acquisition", true);

  v_capture_mode = new VarStringEnum("capture mode", Colors::colorFormatToString(COLOR_RGB8));
  v_capture_mode->addItem(Colors::colorFormatToString(COLOR_RGB8));

  dcam_parameters->addChild(v_capture_mode);

  mutex.unlock();
}

CaptureVapix::~CaptureVapix() {
  capture_settings->deleteAllChildren();
  num_cams--;
}

void CaptureVapix::slotResetTriggered() {
  mutex.lock();

  reset_parameters = true;

  mutex.unlock();
}

bool CaptureVapix::stopCapture() {

  if (isCapturing()) {
    is_capturing = false;
    fprintf(stderr, "VAPIX: Stopping capture on cam %d - %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
  }

  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i: tmp) {
    i->removeFlags(VARTYPE_FLAG_READONLY);
  }

  return true;
}

bool CaptureVapix::startCapture() {
  mutex.lock();

  fprintf(stderr, "VAPIX: Starting capture on camera %d - %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
  
  //pSystem = Spinnaker::System::GetInstance();

  //Spinnaker::CameraList camList = pSystem->GetCameras();
  fprintf(stderr, "VAPIX: Number of cams: %d\n", num_cams);
  

  if (v_cam_bus->getInt() >= num_cams) {
    fprintf(stderr, "VAPIX: Invalid cam_id: %u\n", v_cam_bus->getInt());
    //pSystem->ReleaseInstance();
    mutex.unlock();
    return false;
  }

  // is_capturing must be set before reloadParameters()!
  is_capturing = true;

  fprintf(stderr, "VAPIX: Camera %d connected to IP %s successfully!\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());

  //reloadParameters();

  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i: tmp) {
    i->addFlags(VARTYPE_FLAG_READONLY);
  }

  mutex.unlock();

  return true;

}

bool CaptureVapix::copyAndConvertFrame(const RawImage &src, RawImage &target) {
  mutex.lock();
  if (src.getData() == nullptr) {
    mutex.unlock();
    return false;
  }
  target.setTime(src.getTime());
  target.setTimeCam ( src.getTimeCam() );

  ColorFormat src_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
  ColorFormat out_color = Colors::stringToColorFormat(v_convert_to_mode->getSelection().c_str());
  if (src_color == out_color) {
    // We have to copy the data here to avoid a potential double free
    // If we do 'target = src', both RawImages own the same data, and if both try to free/delete it,
    // i.e. due to an ensure_allocation(), a double free error will occur
    target.deepCopyFromRawImage(src, false);
  } else if (src_color == COLOR_RGB8 && out_color == COLOR_RGB8) {
    target.ensure_allocation(out_color, src.getWidth(), src.getHeight());
    cv::Mat srcMat(src.getHeight(), src.getWidth(), CV_8UC1, src.getData());
    cv::Mat dstMat(target.getHeight(), target.getWidth(), CV_8UC3, target.getData());
    cvtColor(srcMat, dstMat, cv::COLOR_BayerBG2RGB);
  } else {
    fprintf(stderr, "Invalid conversion from %s to %s\n",
            v_capture_mode->getSelection().c_str(), v_convert_to_mode->getSelection().c_str());
    mutex.unlock();
    return false;
  }

  mutex.unlock();
  return true;
}

RawImage CaptureVapix::getFrame() {
  mutex.lock();

  ColorFormat out_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
  RawImage result;
  result.setColorFormat(out_color);
  
  int camera_number;
  if (v_cam_bus->getInt() == 0) camera_number = 1;
  else camera_number = 3;

  string img_file_name = img_file_prefix + to_string(camera_number) + ".txt";

  img_file = std::ifstream(img_file_name);

  // Create the image buffer
  std::stringstream buffer;
  buffer << img_file.rdbuf();
  std::string encoded_image = buffer.str();

  // Decode base64
  std::string decoded_image = base64_decode(encoded_image);

  // Reconstruct the cv::Mat object from the binary data
  std::vector<uchar> data(decoded_image.begin(), decoded_image.end());
  if (!data.size() == 0) {
    p_image = cv::imdecode(data, cv::IMREAD_UNCHANGED);

    // Display the image
    if (!p_image.empty()) {
      result.setData(p_image.data);
      result.setWidth(p_image.cols);
      result.setHeight(p_image.rows);
    } else {
      fprintf(stderr, 
        "VAPIX: Failed to decode image for camera %d - %s\n",
        v_cam_bus->getInt(),
        v_cam_ip->getString().c_str()
      );
    }
  }
  else {
    fprintf(stderr, 
      "VAPIX: Failed to get image data for camera %d - %s\n",
      v_cam_bus->getInt(),
      v_cam_ip->getString().c_str()
    );
  }


  mutex.unlock();
  return result;
}

void CaptureVapix::releaseFrame() {
  mutex.lock();
  mutex.unlock();
}


void CaptureVapix::changed(__attribute__((unused)) VarType *group) {
  mutex.lock();
  // TODO: Do something...
  mutex.unlock();
}


string CaptureVapix::base64_encode(const string &in) {

    std::string out;

    int val = 0, valb = -6;
    for (uchar c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }
    if (valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

string CaptureVapix::base64_decode(const string &in) {

    std::string out;

    std::vector<int> T(256,-1);
    for (int i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

    int val=0, valb=-8;
    for (uchar c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val>>valb)&0xFF));
            valb -= 8;
        }
    }
    return out;
}
