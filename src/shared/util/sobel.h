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
  \file    sobel.h
  \brief   Some functions for edge detection
  \author  Tim Laue, (C) 2009
 */
//========================================================================

#ifndef SOBEL_H
#define SOBEL_H

#include <image.h>

/*!
  \class Sobel

  \brief Collection of functions for edge detection

  \author Tim Laue, (C) 2009
 **/
class Sobel
{
  //For explanation, refer to wikipedia's enlightening description of the Sobel Operator:
  // http://en.wikipedia.org/w/index.php?title=Sobel_operator&oldid=351797455
  public:
  
  //Gx convolution. see: http://en.wikipedia.org/w/index.php?title=Sobel_operator&oldid=351797455#Formulation
  static int horizontalBrighter(const greyImage& img, int x, int y, int threshold)
  {
    int sobel = (-1*img.getPixel(x-1,y-1).v) + 
                (-2*img.getPixel(x-1,y).v) +
                (-1*img.getPixel(x-1,y+1).v) +
                ( 1*img.getPixel(x+1,y-1).v) + 
                ( 2*img.getPixel(x+1,y).v) +
                ( 1*img.getPixel(x+1,y+1).v);
    return sobel > threshold ? sobel : 0;
  }
  
  //-Gx convolution. see: http://en.wikipedia.org/w/index.php?title=Sobel_operator&oldid=351797455#Formulation
  static int horizontalDarker(const greyImage& img, int x, int y, int threshold)
  {
    int sobel = ( 1*img.getPixel(x-1,y-1).v) + 
                ( 2*img.getPixel(x-1,y).v) +
                ( 1*img.getPixel(x-1,y+1).v) +
                (-1*img.getPixel(x+1,y-1).v) + 
                (-2*img.getPixel(x+1,y).v) +
                (-1*img.getPixel(x+1,y+1).v);
    return sobel > threshold ? sobel : 0;
  }
  
  //-Gy convolution. see: http://en.wikipedia.org/w/index.php?title=Sobel_operator&oldid=351797455#Formulation
  static int verticalBrighter(const greyImage& img, int x, int y, int threshold)
  {
    int sobel = (-1*img.getPixel(x-1,y-1).v) + 
                (-2*img.getPixel(x,y-1).v) +
                (-1*img.getPixel(x-1,y-1).v) +
                ( 1*img.getPixel(x-1,y+1).v) + 
                ( 2*img.getPixel(x,y+1).v) +
                ( 1*img.getPixel(x+1,y+1).v);
    return sobel > threshold ? sobel : 0;
  }
  
  //Gy convolution. see: http://en.wikipedia.org/w/index.php?title=Sobel_operator&oldid=351797455#Formulation
  static int verticalDarker(const greyImage& img, int x, int y, int threshold)
  {
    int sobel = ( 1*img.getPixel(x-1,y-1).v) + 
                ( 2*img.getPixel(x,y-1).v) +
                ( 1*img.getPixel(x-1,y-1).v) +
                (-1*img.getPixel(x-1,y+1).v) + 
                (-2*img.getPixel(x,y+1).v) +
                (-1*img.getPixel(x+1,y+1).v);
    return sobel > threshold ? sobel : 0;
  }
  
  //Scans img raster from pixel (xStart,y) to (xEnd,y) and returns the x coordinate which provided the maximum response to the (convolution) function f
  //If no pixel had a response>0, the function returns -1
  static int maximumHorizontalEdge(const greyImage& img, int y, int xStart, int xEnd,
                                   int threshold, int f(const greyImage&, int, int, int))
  {
    int resX = -1;
    int maxEdge(0);
    for(int x=xStart; x<=xEnd; ++x)
    {
      int currentEdge = f(img,x,y,threshold);
      if(currentEdge > maxEdge)
      {
        maxEdge = currentEdge;
        resX = x;
      }
    }
    return resX;
  }
  
  //Scans img vertical line from pixel (x,yStart) to (x,yEnd) and returns the y coordinate which provided the maximum response to the (convolution) function f
  //If no pixel had a response>0, the function returns -1
  static int maximumVerticalEdge(const greyImage& img, int x, int yStart, int yEnd,
                                   int threshold, int f(const greyImage&, int, int, int))
  {
    int resY = -1;
    int maxEdge(0);
    for(int y=yStart; y<=yEnd; ++y)
    {
      int currentEdge = f(img,x,y,threshold);
      if(currentEdge > maxEdge)
      {
        maxEdge = currentEdge;
        resY = y;
      }
    }
    return resY;
  }
  
  //Scans img vertical line from pixel (x,yStart) to (x,yEnd) and returns the y coordinate of the center of the horizontal line
  //The line is demarcated by the low-to-high transition maxBrightEdge, and the high-to-low transition maxDarkEdge
  //Note that this function does not care whether the line is light on dark or dark on light.
  static int centerOfHorizontalLine(const greyImage& img, int x, int yStart, int yEnd,
                                    int threshold)
  {
    int maxDarkEdge(0);
    int maxBrightEdge(0);
    int maxDarkY(0);
    int maxBrightY(0);
    for(int y=yStart; y<=yEnd; ++y)
    {
      int currentDarkEdge = Sobel::verticalDarker(img, x, y, threshold);
      int currentBrightEdge = Sobel::verticalBrighter(img, x, y, threshold);
      if(currentDarkEdge > maxDarkEdge)
      {
        maxDarkEdge = currentDarkEdge;
        maxDarkY = y;
      }
      if(currentBrightEdge > maxBrightEdge)
      {
        maxBrightEdge = currentBrightEdge;
        maxBrightY = y;
      }
    }
    return (maxBrightY + maxDarkY) / 2;
  }
  
  //Scans img raster from pixel (xStart,y) to (xEnd,y) and returns the x coordinate of the center of the vertical line
  static int centerOfVerticalLine(const greyImage& img, int y, int xStart, int xEnd,
                                    int threshold)
  {
    int maxDarkEdge(0);
    int maxBrightEdge(0);
    int maxDarkX(0);
    int maxBrightX(0);
    for(int x=xStart; x<=xEnd; ++x)
    {
      int currentDarkEdge = Sobel::horizontalDarker(img, x, y, threshold);
      int currentBrightEdge = Sobel::horizontalBrighter(img, x, y, threshold);
      if(currentDarkEdge > maxDarkEdge)
      {
        maxDarkEdge = currentDarkEdge;
        maxDarkX = x;
      }
      if(currentBrightEdge > maxBrightEdge)
      {
        maxBrightEdge = currentBrightEdge;
        maxBrightX = x;
      }
    }
    return (maxBrightX + maxDarkX) / 2;
  }
};


#endif // SOBEL_H
