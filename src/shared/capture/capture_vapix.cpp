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

size_t CaptureVapix::camImageCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t total_size = size * nmemb;
    data->append((char*)contents, total_size);
    return total_size;
}

void CaptureVapix::setupCurl() {
  curl = curl_easy_init();

  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  complete_url = v_cam_schema->getString() + v_cam_ip->getString() + v_cam_port->getString() + v_cam_video_stream_route->getString();
  
  fprintf(stderr, "%s\n", complete_url.c_str());
  // Set auth credentials
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
  curl_easy_setopt(curl, CURLOPT_URL, complete_url.c_str());
  curl_easy_setopt(curl, CURLOPT_USERPWD, (v_cam_username->getString() + ":" + v_cam_password->getString()).c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
}

void CaptureVapix::slotResetTriggered() {
  mutex.lock();

  reset_parameters = true;

  mutex.unlock();
}

bool CaptureVapix::stopCapture() {

  if (isCapturing()) {
    is_capturing = false;
    curl_easy_cleanup(curl);
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

  setupCurl();

  fprintf(stderr, "VAPIX: Starting capture on camera %d - %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
  
  //pSystem = Spinnaker::System::GetInstance();

  //Spinnaker::CameraList camList = pSystem->GetCameras();
  fprintf(stderr, "VAPIX: Number of cams: %d\n", num_cams);
  
  
  if (!curl) {
    fprintf(stderr, "VAPIX: Could not initialize CURL on camera %d - %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
    mutex.unlock();
    return false;
  }

  if (v_cam_bus->getInt() >= num_cams) {
    fprintf(stderr, "VAPIX: Invalid cam_id: %u\n", v_cam_bus->getInt());
    //pSystem->ReleaseInstance();
    curl_easy_cleanup(curl);
    mutex.unlock();
    return false;
  }

  // Perform a GET on the IP to check if it exists
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  CURLcode curl_res_code = curl_easy_perform(curl);
  bool alive_check_success = false;

  if (curl_res_code == CURLE_OK) {
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code >= 200 && response_code < 300) {
      alive_check_success = true;
      fprintf(stderr, "VAPIX: Website alive check PASSED");
    }
  }

  if (!alive_check_success) {
    fprintf(stderr, "VAPIX: Could not connect camera %d to IP %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
    curl_easy_cleanup(curl);
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

  // Perform the HTTP request
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, camImageCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cam_response);
  CURLcode curl_res_code = curl_easy_perform(curl);

  if (curl_res_code == CURLE_OK) {

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    if (response_code >= 200 && response_code < 300) {
      vector<unsigned char> image_data(cam_response.begin(), cam_response.end());

      p_image = cv::Mat(cv::imdecode(image_data, out_color));

      if (!p_image.empty()) {
        result.setData(p_image.data);
        result.setWidth(p_image.cols);
        result.setHeight(p_image.rows);
        // Ignoring timestamp ATM since I don't know if we actually receive one...
        //result.setTimeCam

      }
      else {
        fprintf(stderr, "Failed to read image from camera %d - %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
      }
    }
    else {
      fprintf(stderr, "Failed HTTP GET request to camera %d - %s with status code: %ld\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str(), response_code);
    }
  }
  else {
    fprintf(stderr, "VAPIX: Failed to send GET request on camera %d - %s | HTTP ERROR: %s\n", 
      v_cam_bus->getInt(), 
      v_cam_ip->getString().c_str(), 
      curl_easy_strerror(curl_res_code));
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
