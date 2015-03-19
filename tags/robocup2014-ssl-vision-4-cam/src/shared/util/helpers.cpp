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
 \file    helpers.cpp
 \brief   Helper utility functions that generally don't fit anywhere else.
 \author  Joydeep Biswas, (C) 2013
 */
//========================================================================

#include "helpers.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

std::string StringPrintf(const char* format, ...) {
  va_list al;
  int string_length = 0;
  char* buffer = NULL;

  va_start(al,format);
  string_length = vsnprintf(buffer,string_length,format,al);
  va_end(al);
  if (string_length == 0) return (std::string());
  buffer = reinterpret_cast<char*>(malloc((string_length + 1) * sizeof(char)));
  if (buffer == NULL) return (std::string());

  va_start(al,format);
  string_length = vsnprintf(buffer,string_length + 1,format,al);
  va_end(al);
  return (std::string(buffer));
}
