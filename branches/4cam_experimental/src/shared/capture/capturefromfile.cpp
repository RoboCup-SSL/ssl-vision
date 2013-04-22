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


#ifndef VDATA_NO_QT
CaptureFromFile::CaptureFromFile(VarList * _settings, QObject * parent) : QObject(parent), CaptureInterface(_settings)
#else
CaptureFromFile::CaptureFromFile(VarList * _settings) : CaptureInterface(_settings)
#endif
{
  currentImageIndex = 0;
  is_capturing=false;

  settings->addChild(conversion_settings = new VarList("Conversion Settings"));
  settings->addChild(capture_settings = new VarList("Capture Settings"));
    
  //=======================CONVERSION SETTINGS=======================
  conversion_settings->addChild(v_colorout=new VarStringEnum("convert to mode",Colors::colorFormatToString(COLOR_YUV422_UYVY)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
    
  //=======================CAPTURE SETTINGS==========================
  capture_settings->addChild(v_cap_dir = new VarString("directory", ""));
    
  // Valid file endings
  validImageFileEndings.push_back("PNG");
  validImageFileEndings.push_back("BMP");
  validImageFileEndings.push_back("JPG");
  validImageFileEndings.push_back("JPEG");
}

CaptureFromFile::~CaptureFromFile()
{
  for(unsigned int i=0; i<images.size(); ++i)
  {
    delete images[i];
  }
}

bool CaptureFromFile::stopCapture() 
{
  cleanup();
  return true;
}

void CaptureFromFile::cleanup()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  is_capturing=false;
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

bool CaptureFromFile::startCapture()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  if(images.size() == 0)
  {
    // Acquire a list of file names
    DIR *dp;
    struct dirent *dirp;
    if((v_cap_dir->getString() == "") || ((dp  = opendir(v_cap_dir->getString().c_str())) == 0)) 
    {
      fprintf(stderr,"Failed to open directory %s \n", v_cap_dir->getString().c_str());
#ifndef VDATA_NO_QT
      mutex.unlock();
#endif      
      is_capturing=false;
      return false;
    }  
    while ((dirp = readdir(dp))) 
    {
      if (strcmp(dirp->d_name,".") != 0 && strcmp(dirp->d_name,"..") != 0) 
      {
        if(isImageFileName(std::string(dirp->d_name)))
          imgs_to_load.push_back(v_cap_dir->getString() + std::string(dirp->d_name));
        else
          fprintf(stderr,"Not a valid image file: %s \n", dirp->d_name);
      }
    }
    closedir(dp);
    if(imgs_to_load.size() == 0)
    {
#ifndef VDATA_NO_QT
      mutex.unlock();
#endif      
      is_capturing=false;
      return false;
    }
  
    // Read images to buffer in memory:
    imgs_to_load.sort();
    imgs_it = imgs_to_load.begin();
    std::list<std::string>::iterator currentImage = imgs_it;
    while(currentImage != imgs_to_load.end())
    {
      int width(-1);
      int height(-1);
      rgba* rgba_img = ImageIO::readRGBA(width, height, currentImage->c_str());
      fprintf (stderr, "Loaded %s \n", currentImage->c_str());
      images.push_back(rgba_img);
      heights.push_back(height);
      widths.push_back(width);
      ++currentImage;
    }
    currentImageIndex = 0;
  }
  is_capturing=true;  
  
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return true;
}

bool CaptureFromFile::isImageFileName(const std::string& fileName)
{
  // Get ending and turn it to uppercase:
  string::size_type pointPos = fileName.find_last_of(".");
  if(pointPos == string::npos)
    return false;
  string ending = fileName.substr(pointPos+1);
  for(unsigned int i=0; i<ending.size(); ++i)
    ending[i] = toupper(ending[i]);
  // Compare against list of valid endings
  for(unsigned int i=0; i<validImageFileEndings.size();++i)
    if(ending == validImageFileEndings[i])
      return true;
  return false;
}

bool CaptureFromFile::copyAndConvertFrame(const RawImage & src, RawImage & target)
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  ColorFormat output_fmt = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
  ColorFormat src_fmt=src.getColorFormat();
    
  if (target.getData()==0)
    target.allocate(output_fmt, src.getWidth(), src.getHeight());
  else
    target.ensure_allocation(output_fmt, src.getWidth(), src.getHeight());
     
  target.setTime(src.getTime());
     
  if (output_fmt == src_fmt)
  {
    if (src.getData() != 0)
      memcpy(target.getData(),src.getData(),src.getNumBytes());
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
  else 
  {
    fprintf(stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",
            Colors::colorFormatToString(src_fmt).c_str(),
            Colors::colorFormatToString(output_fmt).c_str());
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return false;
  } 
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return true;
}

RawImage CaptureFromFile::getFrame()
{
#ifndef VDATA_NO_QT
   mutex.lock();
#endif

  RawImage result;
  result.setColorFormat(COLOR_RGB8); 
  result.setTime(0.0);
  rgba* rgba_img = 0;
  int width;
  int height;
  if(images.size())
  {
    rgba_img = images[currentImageIndex];
    width = widths[currentImageIndex];
    height = heights[currentImageIndex];
    currentImageIndex = (currentImageIndex + 1) % images.size();
  }
  if (rgba_img == 0)
  {
    fprintf (stderr, "CaptureFromFile Error, no images available");
    is_capturing=false;
    result.setData(0);
    result.setWidth(640);
    result.setHeight(480);
    frame = 0;
  }
  else
  {
    frame = new unsigned char[width*height*3];
    unsigned char* p = &frame[0];
    for (int i=0; i < width * height; i++)
    {
      *p = rgba_img[i].r;
      p++;
      *p = rgba_img[i].g;
      p++;
      *p = rgba_img[i].b;
      p++;
    }
    result.setWidth(width);
    result.setHeight(height);
    result.setData(frame);
    timeval tv;    
    gettimeofday(&tv,0);
    result.setTime((double)tv.tv_sec + tv.tv_usec*(1.0E-6));
  }
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif 
  return result;
}

void CaptureFromFile::releaseFrame() 
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  delete[] frame;
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

string CaptureFromFile::getCaptureMethodName() const 
{
  return "FromFile";
}
