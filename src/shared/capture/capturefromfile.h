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
  \file    capturefromfile.h
  \brief   C++ Interface: CaptureFromFile
  \author  OB, (C) 2009
  \author  TL, (C) 2009
*/
//========================================================================

#ifndef CAPTUREFROMFILE_H
#define CAPTUREFROMFILE_H

#include "captureinterface.h"
#include <dirent.h>
#include <string>
#include <list>
#include <algorithm>
#include "VarTypes.h"

#ifndef VDATA_NO_QT
  #include <QMutex>
#else
  #include <pthread.h>
#endif


#ifndef VDATA_NO_QT
  #include <QMutex>
  //if using QT, inherit QObject as a base
class CaptureFromFile : public QObject, public CaptureInterface
#else
class CaptureFromFile : public CaptureInterface
#endif
{
#ifndef VDATA_NO_QT
  Q_OBJECT
/*   public slots: */
/*   void changed(VarType * group); */
  protected:
  QMutex mutex;
  public:
#endif

protected:
  bool is_capturing;

  //processing variables:
  VarStringEnum * v_colorout;

  //capture variables:
  VarString * v_cap_dir;
  VarList * capture_settings;
  VarList * conversion_settings;

  unsigned char* frame;
  std::list<std::string> imgs_to_load;
  std::list<std::string>::iterator imgs_it;
  std::vector<rgba*> images;
  std::vector<int> heights;
  std::vector<int> widths;
  unsigned int currentImageIndex;
  
  bool isImageFileName(const std::string& fileName);
  std::vector<std::string> validImageFileEndings;
  
public:
#ifndef VDATA_NO_QT
  CaptureFromFile(VarList * _settings, QObject * parent=0);
  void mvc_connect(VarList * group);
#else
  CaptureFromFile(VarList * _settings);
#endif
  ~CaptureFromFile();
    
  virtual bool startCapture();
  virtual bool stopCapture();
  virtual bool isCapturing() { return is_capturing; };
  
  virtual RawImage getFrame();
  virtual void releaseFrame();
   
  void cleanup();

  virtual bool copyAndConvertFrame(const RawImage & src, RawImage & target);
  virtual string getCaptureMethodName() const;
};

#endif
