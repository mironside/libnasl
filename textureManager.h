/**
  @file    textureManager.h
  @author  Chris Olsen
  @date    04-21-03

  Copyright (C) 2004 Christopher A Olsen
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifndef __TEXTURE_MANAGER__
#define __TEXTURE_MANAGER__


#include <map>
#include <string>

typedef struct
{
  int width;
  int height;
  int bpp;
  unsigned int glID;
} texture_t;

texture_t* texmanLoad2DMap(char* filename);
texture_t* texmanLoadCubeMap(char* filename);
int  texmanInitialize();
void texmanListTextures();
#endif
