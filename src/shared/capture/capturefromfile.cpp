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
  \file    capturefromfile.cpp
  \brief   C++ Implementation: CaptureFromFile
  \author  OB, (C) 2008
  \author  TL, (C) 2009
*/
//========================================================================

#include <sys/time.h>
#include <cctype>
#include "capturefromfile.h"
#include "image_io.h"
#include "conversions.h"
#include <sstream>
#include <fstream>
#include <opencv2/opencv.hpp>


CaptureFromFile::CaptureFromFile(VarList * _settings, int default_camera_id, QObject * parent) : QObject(parent), CaptureInterface(_settings)
{
  currentImageIndex = 0;
  is_capturing=false;

  settings->addChild(conversion_settings = new VarList("Conversion Settings"));
  settings->addChild(capture_settings = new VarList("Capture Settings"));

  //=======================CONVERSION SETTINGS=======================
  conversion_settings->addChild(v_colorout=new VarStringEnum("convert to mode",Colors::colorFormatToString(COLOR_YUV422_UYVY)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RAW8));

  conversion_settings-> addChild(v_raw_width=new VarInt("raw width", 2448));
  conversion_settings-> addChild(v_raw_height=new VarInt("raw height", 2048));

  //=======================CAPTURE SETTINGS==========================
  ostringstream convert;
  convert << "test-data/rc2022/bots-center-ball-" << default_camera_id << "-2";
  capture_settings->addChild(v_cap_dir = new VarString("directory", convert.str()));

  // Valid file endings
  validImageFileEndings.push_back("PNG");
  validImageFileEndings.push_back("BMP");
  validImageFileEndings.push_back("JPG");
  validImageFileEndings.push_back("JPEG");
  validImageFileEndings.push_back("RAW");
}

CaptureFromFile::~CaptureFromFile()
{
}

bool CaptureFromFile::stopCapture()
{
  cleanup();
  return true;
}

void CaptureFromFile::cleanup()
{
  mutex.lock();
  is_capturing=false;
  mutex.unlock();
}

bool CaptureFromFile::startCapture()
{
  mutex.lock();
  if(images.size() == 0)
  {
    // Acquire a list of file names
    DIR *dp;
    struct dirent *dirp;
    if((v_cap_dir->getString() == "") || ((dp  = opendir(v_cap_dir->getString().c_str())) == 0))
    {
      fprintf(stderr,"Failed to open directory %s \n", v_cap_dir->getString().c_str());
      mutex.unlock();
      is_capturing=false;
      return false;
    }
    while ((dirp = readdir(dp)))
    {
      if (strcmp(dirp->d_name,".") != 0 && strcmp(dirp->d_name,"..") != 0)
      {
        if(isImageFileName(std::string(dirp->d_name)))
          imgs_to_load.push_back(v_cap_dir->getString() + "/" + std::string(dirp->d_name));
        else
          fprintf(stderr,"Not a valid image file: %s \n", dirp->d_name);
      }
    }
    closedir(dp);
    if(imgs_to_load.size() == 0)
    {
      mutex.unlock();
      is_capturing=false;
      return false;
    }

    // Read images to buffer in memory:
    imgs_to_load.sort();
    for (const auto& currentImage : imgs_to_load) {
      int width(v_raw_width->get());
      int height(v_raw_height->get());
      if(getFileExtension(currentImage) == "RAW")
      {
        if(width <= 0 || height <= 0)
        {
          std::cout << "Could not read image. Dimensions must be positive." << std::endl;
          continue;
        }
        std::ifstream file(currentImage, std::ios::binary );
        if(!file)
        {
          std::cout << "Could not read file: " << currentImage << std::endl;
          continue;
        }
        vector<char> buffer((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));

        if((int) buffer.size() < width*height)
        {
          std::cerr << "Image " << currentImage << " is too small!" << std::endl;
          continue;
        }

        RawImage raw_img;
        raw_img.allocate(ColorFormat::COLOR_RAW8, width, height);
        memcpy(raw_img.getData(), buffer.data(), static_cast<size_t>(raw_img.getNumBytes()));

        images.push_back(raw_img);
      }
      else
      {
        // read image to default OpenCV image format (BGR8)
        cv::Mat srcImg = imread(currentImage, cv::IMREAD_COLOR);
        RawImage img;
        img.allocate(ColorFormat::COLOR_RGB8, srcImg.cols, srcImg.rows);
        cv::Mat dstImg(img.getHeight(), img.getWidth(), CV_8UC3, img.getData());
        // convert to default ssl-vision format (RGB8)
        cvtColor(srcImg, dstImg, cv::COLOR_BGR2RGB);
        images.push_back(img);
      }
      fprintf (stderr, "Loaded %s \n", currentImage.c_str());
    }
    currentImageIndex = 0;
  }
  is_capturing=true;

  mutex.unlock();
  return true;
}

std::string CaptureFromFile::getFileExtension(const std::string &fileName)
{
  // Get ending and turn it to uppercase:
  string::size_type pointPos = fileName.find_last_of('.');
  if(pointPos == string::npos)
    return "";
  string ending = fileName.substr(pointPos+1);
  for (char &i : ending) {
    i = toupper(i);
  }
  return ending;
}

bool CaptureFromFile::isImageFileName(const std::string& fileName)
{
  auto ending = getFileExtension(fileName);
  if(ending.empty()) {
    return false;
  }
  // Compare against list of valid endings
  for (const auto &validImageFileEnding : validImageFileEndings) {
    if (ending == validImageFileEnding) {
      return true;
    }
  }
  return false;
}

bool CaptureFromFile::copyAndConvertFrame(const RawImage & src, RawImage & target)
{
  mutex.lock();

  ColorFormat output_fmt = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
  ColorFormat src_fmt = src.getColorFormat();

  target.ensure_allocation(output_fmt, src.getWidth(), src.getHeight());
  target.setTime(src.getTime());
  target.setTimeCam ( src.getTimeCam() );
  if (output_fmt == src_fmt)
  {
    memcpy(target.getData(), src.getData(), static_cast<size_t>(src.getNumBytes()));
  }
  else if(src_fmt == COLOR_RAW8 && output_fmt == COLOR_RGB8)
  {
      cv::Mat srcMat(src.getHeight(), src.getWidth(), CV_8UC1, src.getData());
      cv::Mat dstMat(target.getHeight(), target.getWidth(), CV_8UC3, target.getData());
      cvtColor(srcMat, dstMat, cv::COLOR_BayerRG2BGR);
  }
#ifndef NO_DC1394_CONVERSIONS
  else if(src_fmt == COLOR_RAW8 && output_fmt == COLOR_YUV422_UYVY)
  {
    // note: this an inefficient double conversion and should only be used for testing!
    cv::Mat srcMat(src.getHeight(), src.getWidth(), CV_8UC1, src.getData());
    cv::Mat dstMat(target.getHeight(), target.getWidth(), CV_8UC3);
    cvtColor(srcMat, dstMat, cv::COLOR_BayerRG2BGR);
    dc1394_convert_to_YUV422(dstMat.data, target.getData(), src.getWidth(), src.getHeight(),
                             DC1394_BYTE_ORDER_UYVY, DC1394_COLOR_CODING_RGB8, 8);
  }
  else if (src_fmt == COLOR_RGB8 && output_fmt == COLOR_YUV422_UYVY)
  {
    if (src.getData() != 0)
      dc1394_convert_to_YUV422(src.getData(), target.getData(), src.getWidth(), src.getHeight(),
                               DC1394_BYTE_ORDER_UYVY, DC1394_COLOR_CODING_RGB8, 8);
  }
  else if (src_fmt == COLOR_YUV422_UYVY && output_fmt == COLOR_RGB8)
  {
    if (src.getData() != 0)
      dc1394_convert_to_RGB8(src.getData(),target.getData(), src.getWidth(), src.getHeight(),
                             DC1394_BYTE_ORDER_UYVY, DC1394_COLOR_CODING_YUV422, 8);
  }
#endif
  else
  {
    fprintf(stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",
            Colors::colorFormatToString(src_fmt).c_str(),
            Colors::colorFormatToString(output_fmt).c_str());
    mutex.unlock();
    return false;
  }
  mutex.unlock();
  return true;
}

RawImage CaptureFromFile::getFrame()
{
   mutex.lock();

  RawImage result;
  if(images.empty())
  {
    fprintf (stderr, "CaptureFromFile Error, no images available");
    is_capturing=false;
    result.setWidth(640);
    result.setHeight(480);
  } else {
    result = images[currentImageIndex];
    currentImageIndex = static_cast<unsigned int>((currentImageIndex + 1) % images.size());
  }

  mutex.unlock();
  return result;
}

void CaptureFromFile::releaseFrame()
{
  mutex.lock();
  mutex.unlock();
}

string CaptureFromFile::getCaptureMethodName() const
{
  return "FromFile";
}
