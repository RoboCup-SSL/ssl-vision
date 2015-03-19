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
  \file    texture.h
  \brief   C++ Interface: glTexture
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#ifdef GL_TEXTURE_RECTANGLE_EXT
  #define texTarget (nonPowerOfTwo ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D)
#else
  #ifdef GL_TEXTURE_RECTANGLE_NV
    #define texTarget (nonPowerOfTwo ? GL_TEXTURE_RECTANGLE_NV : GL_TEXTURE_2D)
  #else
    #ifdef GL_TEXTURE_RECTANGLE_ARB
      #define texTarget (nonPowerOfTwo ? GL_TEXTURE_RECTANGLE_ARB : GL_TEXTURE_2D)
    #endif
  #endif
#endif
#ifndef texTarget
  #define texTarget GL_TEXTURE_2D
#endif

#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <string>
#include "image.h"
using namespace std;

/*!
  \class  glTexture
  \brief  A template-based open-gl texture class
  \author Stefan Zickler, (C) 2008

  Usage: 1. construct it with first argument being the filename
          (or use setSourceFile(...) if constructed with empty filename)
          alternatively, filename can be left empty and surface can be
          loaded manually
       2. run load(int wrap_s, int wrap_t)
          e.g. load(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
          This will load the texture into GL
       3. you can use isReady() to confirm that everything
          loaded correctly
       4. run bind() to bind it to the current GL-context

  Optional. call update(...) to resync changes in the surface to GL
          this is useful for e.g. live-video streaming to GL
          as this doesn't require a full regeneration of the texture

  "Help I don't see textures!" Most common causes:
          -- You called load() or update() before a GL context existed
          -- You forgot to set glEnable(texTarget), e.g. GL_TEXTURE_2D
             before loading
          -- You forgot to call bind() on your texture
          -- You forgot to call unbind() after your texture
          -- Your alpha-channel is broken or inverted
*/
template <class image>
class glTexture
{
protected:
  GLuint texture;
  bool nonPowerOfTwo;
  bool loaded;
  bool buffer;
  string _filename;
public:
  image surface;

  /// Binds the texture to the current GL context
  bool bind()
  {
    if (isReady()) {
      glBindTexture( texTarget, texture );
      return true;
    } else {
      return false;
    }
  }

  string getFilename() {
    return _filename; 
  }

  /// Returns true if the texture is in video memory and ready to use
  bool isReady()
  {
    return loaded;
  }

  /// This will unload the texture from RAM only
  bool unloadSurface()
  {
    surface.clear();
    return true;
  }

  /// This will unload the texture from the graphics card only
  bool unload()
  {
    if(isReady()==true) {
      //printf("unload %d (%s) !\n",texture,_filename.c_str());
      glDeleteTextures(1,&texture);
    }
    return true;
  }

  /*!
  The keep_texture_in_ram flag describes whether or not to
  automatically unallocate the image-surface after the texture has
  been transfered from RAM to video memory.
  Sometimes it makes sense to keep it in RAM if the texture gets
  loaded and unloaded from the video card frequently.
  Under normal use however you might want to stick with the false value
  which will free up the memory after the texture has been loaded
  Note that the image-surface will ALWAYS be unloaded when this class is
  destructed, so you don't need to worry about that.
  */
  glTexture(string filename="", bool keep_texture_in_ram=false, bool use_non_power_of_two_extension=true)
  {
    nonPowerOfTwo=use_non_power_of_two_extension;
    buffer=keep_texture_in_ram;
    texture=0;
    loaded=false;
    setSourceFile(filename);
  };

  ~glTexture()
  {
    unload(); //unload the texture from the video card
    unloadSurface(); //unload any data in RAM.
  }

  bool update(GLint wrap_s=GL_CLAMP, GLint wrap_t=GL_CLAMP, GLint minfilter=GL_LINEAR, GLint magfilter=GL_LINEAR) {
    if (loaded==false) return false;
    //bind context of current texture
    glBindTexture( texTarget, texture );
    //printf("glupdatebind err: %s\n",gluErrorString(glGetError()));
    if (surface.getColorFormat()==COLOR_RGB8) {
      // generate texture from bitmap
      //printf("loading texture: %d %d\n",surface.getWidth(), surface.getHeight());
      glTexImage2D( texTarget, 0, 3, surface.getWidth(),
                  surface.getHeight(), 0, GL_RGB,
                  GL_UNSIGNED_BYTE, surface.data );

    } else if (surface.getColorFormat()==COLOR_RGBA8) {
      // generate texture from bitmap
      glTexImage2D( texTarget, 0, 4, surface.getWidth(),
                  surface.getHeight(), 0, GL_RGBA,
                  GL_UNSIGNED_BYTE, surface.data );
      //printf("data loaded: %d\n",surface.data);
      //printf("glteximage2d rgba err: %s\n",gluErrorString(glGetError()));
    } else {
      fprintf(stderr,"Error: unknown image format to convert to OpenGL\n");
    }
    // Linear Filtering
    glTexParameteri( texTarget, GL_TEXTURE_MIN_FILTER, minfilter );
    glTexParameteri( texTarget, GL_TEXTURE_MAG_FILTER, magfilter );
    glTexParameteri( texTarget, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri( texTarget, GL_TEXTURE_WRAP_T, wrap_t);
    return true;
  }

  //This loads the texture from the file into RAM (into an image surface)
  //You should not need to call this manually, as load() will do it for you
  bool loadFileToSurface()
  {
    if (_filename == "")
      return false;
    if(!surface.load(_filename)) {
      return false;
    }
    return true;
  }

  void unbind() {
    glBindTexture(texTarget,0);
  }

  /// This function loads the texture to video memory
  /// If the texture is already in RAM, it will use it,
  /// otherwise it will reload it into RAM
  bool load(GLint wrap_s=GL_CLAMP, GLint wrap_t=GL_CLAMP, GLint minfilter=GL_LINEAR, GLint magfilter=GL_LINEAR)
  {
    glEnable(texTarget);
    if (_filename != "" && (buffer==false || surface.getData()==0)) {
      //reload texture to RAM
      if (loadFileToSurface()==false)
        return false;
    }
    //unload previous instance from video card:
    unload();

    // create gl-texture
    glGenTextures( 1, &texture );

    loaded=true;
    //now sync data to video card
    update(wrap_s,wrap_t,minfilter,magfilter);
    if (buffer==false) {
      // Free up any memory we may have used
      surface.clear();
    }
    return true;
  }

  /*!
  allows to change the filename to be loaded
  this function does not do any loading...it only specifies the filename
  to be loaded on any succeeding load() load call.
  This does not need to be called when a texture has already been specified
  in the constructor
  */
  void setSourceFile(string filename)
  {
    _filename=filename;
  }

  /// return width of texture
  int getWidth()
  {
    return surface.getWidth();
  }

  /// return height of texture
  int getHeight()
  {
    return surface.getHeight();
  }

};

/*!
  \class rgbTexture
  \brief an instantiation  of the glTexture class using rgbImage
*/
typedef glTexture<rgbImage> rgbTexture;

/*!
  \class rgbaTexture
  \brief an instantiation  of the glTexture class using rgbaImage
*/
typedef glTexture<rgbaImage> rgbaTexture;

#endif

