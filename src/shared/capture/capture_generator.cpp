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
  \file    capture_generator.cpp
  \brief   C++ Implementation: CaptureGenerator
  \author  Stefan Zickler, (C) 2009
*/
// 2/4/16 - added ability to load team patterns from file (Zavesky)
//========================================================================

#include "capture_generator.h"
#include "conversions.h"

#include "cmpattern_team.h"         //added to load team files directly
#include "cmpattern_teamdetector.h"

#define TEAM_IMAGE_PLACEHOLDER      "(team images - reset bus to populate)"

#ifndef VDATA_NO_QT
CaptureGenerator::CaptureGenerator ( VarList * _settings, QObject * parent ) : QObject ( parent ), CaptureInterface ( _settings )
#else
CaptureGenerator::CaptureGenerator ( VarList * _settings ) : CaptureInterface ( _settings )
#endif
{
    is_capturing=false;
    
    settings->addChild ( conversion_settings = new VarList ( "Conversion Settings" ) );
    settings->addChild ( capture_settings = new VarList ( "Capture Settings" ) );
    
    //=======================CONVERSION SETTINGS=======================
    conversion_settings->addChild ( v_colorout=new VarStringEnum ( "convert to mode",Colors::colorFormatToString ( COLOR_YUV422_UYVY ) ) );
    v_colorout->addItem ( Colors::colorFormatToString ( COLOR_RGB8 ) );
    v_colorout->addItem ( Colors::colorFormatToString ( COLOR_YUV422_UYVY ) );
    
    //=======================CAPTURE SETTINGS==========================
    capture_settings->addChild ( v_framerate = new VarDouble ( "Framerate (FPS)", 60.0 ) );
    capture_settings->addChild ( v_width = new VarInt ( "Width (pixels)", 780 ) );
    capture_settings->addChild ( v_height = new VarInt ( "Height (pixels)", 580 ) );
    
    //modified 2/5/16 to allow generator to generate team patterns, E. Zavesky
    capture_settings->addChild( v_test_image = new VarStringEnum("Generate Mode", "none"));
    v_test_image->addItem("none");
    v_test_image->addItem("test image");
    v_test_image->addItem(TEAM_IMAGE_PLACEHOLDER);
}

CaptureGenerator::~CaptureGenerator()
{
}

bool CaptureGenerator::stopCapture()
{
  cleanup();
  return true;
}

void CaptureGenerator::cleanup()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  is_capturing=false;
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

bool CaptureGenerator::startCapture()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  limit.init ( v_framerate->getDouble() );
  is_capturing=true;


#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return true;
}

bool CaptureGenerator::resetBus()
{
    v_test_image->setSize(3);       //trim off prior settings
    v_test_image->selectIndex(0);   //force selection off of prior
    vectImages.clear();
    
    std::vector<VarType *> vectRelatives = settings->findRelatives("Teams", true);  //find Teams node
    if (vectRelatives.size() < 1) return false;

    VarType *pTeams = vectRelatives[0];    //jump to head
    std::vector<VarType *> vectTeamNodes = pTeams->getChildren();
    for (int iTeam=0; iTeam<vectTeamNodes.size(); iTeam++) {
        VarType *pTeam = vectTeamNodes[iTeam];
        std::vector<VarType *> vectImageNodes = pTeam->findRelatives("Marker Image File");  //find image node
        if (vectImageNodes.size()==1) {
            VarString *pString = reinterpret_cast<VarString*>(vectImageNodes[0]);
            vectImages.push_back(pString->getString());             // add filename
            v_test_image->addItem(pTeam->getName());                    // add team name
        }
    }
    return true;
}

bool CaptureGenerator::copyAndConvertFrame ( const RawImage & src, RawImage & target )
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  ColorFormat output_fmt = Colors::stringToColorFormat ( v_colorout->getSelection().c_str() );
  ColorFormat src_fmt=src.getColorFormat();

  if ( target.getData() ==0 ) {
    target.allocate ( output_fmt, src.getWidth(), src.getHeight() );
  } else {
    target.ensure_allocation ( output_fmt, src.getWidth(), src.getHeight() );
  }
  target.setTime ( src.getTime() );

  if ( output_fmt == src_fmt ) {
    if ( src.getData() != 0 ) memcpy ( target.getData(),src.getData(),src.getNumBytes() );
  } else if ( src_fmt == COLOR_RGB8 && output_fmt == COLOR_YUV422_UYVY ) {
    if ( src.getData() != 0 ) {
      dc1394_convert_to_YUV422 ( src.getData(), target.getData(), src.getWidth(), src.getHeight(),
                                 DC1394_BYTE_ORDER_UYVY, DC1394_COLOR_CODING_RGB8, 8 );
    }
  } else {
    fprintf ( stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",
              Colors::colorFormatToString ( src_fmt ).c_str(),
              Colors::colorFormatToString ( output_fmt ).c_str() );
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

RawImage CaptureGenerator::getFrame()
{
#ifndef VDATA_NO_QT
    mutex.lock();
#endif
    limit.waitForNextFrame();
    result.setColorFormat ( COLOR_RGB8 );
    result.setTime ( GetTimeSec() );
    result.allocate ( COLOR_RGB8,v_width->getInt(),v_height->getInt() );
    rgbImage img;
    img.fromRawImage(result);
    
    std::string sTestImage = v_test_image->getString();
    if (state_prior == sTestImage) {      //don't waste time reloading/drawing
        img.copy(img_prior);
    }
    else {
        if (sTestImage=="test image") {
            int w = result.getWidth();
            int h = result.getHeight();
            int n_colors = 8;
            int slice_width = w/n_colors;
            rgb color=RGB::Black;
            rgb color2;
            int gradient=h/256;
            for (int x = 0 ; x < w; x++) {
                int c_idx= x/slice_width;
                if (c_idx==0) {
                    color=RGB::White;
                } else if (c_idx==1) {
                    color=RGB::Red;
                } else if (c_idx==2) {
                    color=RGB::Green;
                } else if (c_idx==3) {
                    color=RGB::Blue;
                } else if (c_idx==4) {
                    color=RGB::Cyan;
                } else if (c_idx==5) {
                    color=RGB::Pink;
                } else if (c_idx==6) {
                    color=RGB::Yellow;
                } else if (c_idx==7) {
                    color=RGB::Black;
                }
                for (int y = 0 ; y < h; y++) {
                    color2.r= max(0,min(255,((int)color.r * (h-1-y))/h));
                    color2.g= max(0,min(255,((int)color.g * (h-1-y))/h));
                    color2.b= max(0,min(255,((int)color.b * (h-1-y))/h));
                    img.setPixel(x,y,color2);
                }
            }
        } else if (sTestImage=="none" || sTestImage==TEAM_IMAGE_PLACEHOLDER) {
            img.fillBlack();
        } else {                                    //otherwise, we have a team image...
            int iIdx = v_test_image->getIndex()-3;
            if (vectImages.size() < iIdx) {
                std::string sPath = vectImages[iIdx];    //copy filepath from second list
                fprintf ( stderr,"CaptureGenerator: Attempting to load image '%s' for team '%s'...\n",
                         sPath.c_str(), sTestImage.c_str());
                
                rgbImage img_load;
                if (!img_load.load(sPath))
                    img.fillBlack();
                else {
                    img.fillBlack();
                    int w = (result.getWidth() > img_load.getWidth()) ? img_load.getWidth() : result.getWidth();
                    int h = (result.getHeight() > img_load.getHeight()) ? img_load.getHeight() : result.getHeight();
                    img.copyFromRectArea(img_load, 0, 0, w, h, true);
                }
            }
        }
        img_prior.copy(img);                        //save for next round
        state_prior = v_test_image->getString();    //update new state
    }
    
    
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return result;
}

void CaptureGenerator::releaseFrame()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

string CaptureGenerator::getCaptureMethodName() const
{
  return "Image Generator";
}
