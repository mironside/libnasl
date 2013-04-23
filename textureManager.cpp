/**
  @file    textureManager.cpp
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
#include "textureManager.h"
//#include "sys/assetManager.h"
//#include "render/render.h"
#include <GL/gl.h>
#include <GL/glu.h>




typedef struct image_s
{
  int bpp;
  int width;
  int height;
  unsigned char *pData;
} image_t;

typedef struct
{
  unsigned char  header[12];
  unsigned short width;
  unsigned short height;
  unsigned char  bpp;
} TGAHeader;

typedef std::map<std::string, texture_t*> TextureMap;
TextureMap textureMap;

static int LoadTGA(char* filename, image_t* image);


texture_t* notex = 0;

int texmanInitialize()
{
  printf("texmanInitialize\n");
  notex = texmanLoad2DMap("radiant/notex.tga");

  if(!notex)
  {
    return 1;
  }
  return 0;
}


  
/** loads a tga texture.  Note that the filename must include the .tga */
texture_t* texmanLoad2DMap(char *filename)
{
  // return the texture if it has already been loaded
  TextureMap::iterator itor;
  if((itor = textureMap.find(filename)) != textureMap.end())
  {
    printf("found %s\n", filename);
    return itor->second;
  }



  image_t texImg;
  texture_t* texture = new texture_t;
  memset(texture, 0, sizeof(texture_t));

  char texname[256];
  sprintf(texname, "textures/%s", filename);

  if(!LoadTGA(texname, &texImg))
  {
    delete texture;
    return notex;
  }

  texture->width  = texImg.width;
  texture->height = texImg.height;
  texture->bpp    = texImg.bpp;

  glGenTextures(1, (GLuint*)&texture->glID);
  glBindTexture(GL_TEXTURE_2D, texture->glID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  if(texture->bpp == 32)
  {
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, texture->width, texture->height, GL_BGRA, GL_UNSIGNED_BYTE, texImg.pData);
  }
  else if(texture->bpp == 24)
  {
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture->width, texture->height, GL_BGR, GL_UNSIGNED_BYTE, texImg.pData);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  delete texImg.pData;
    
  textureMap.insert(TextureMap::value_type(filename, texture));
  printf("texman loaded %s\n", filename);

  return texture;
}



/** loads a tga texture.  Note that the filename must include the .tga */
texture_t* texmanLoadCubeMap(char *filename)
{
  printf("LoadCubeMap %s\n", filename);
  
  // return the texture if it has already been loaded
  TextureMap::iterator itor;
  if((itor = textureMap.find(filename)) != textureMap.end())
  {
    return itor->second;
  }
  // load the texture
  else
  {
    image_t texImg;
    texture_t* texture = new texture_t;
    memset(texture, 0, sizeof(texture_t));
    glGenTextures(1, (GLuint*)&texture->glID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture->glID);


    char* names[6] = {"_posx.tga", "_posy.tga", "_posz.tga",
                      "_negx.tga", "_negy.tga", "_negz.tga"};
    int sides[6] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

    char name[256];
    for(int i = 0; i < 6; i++)
    {
      sprintf(name, "textures/%s%s", filename, names[i]);
      printf("  %s\n", name);
      
      if(!LoadTGA(name, &texImg))
      {
        printf("not found\n");
        return texmanLoad2DMap("radiant/notex.tga");
      }

      texture->width  = texImg.width;
      texture->height = texImg.height;
      texture->bpp    = texImg.bpp;

      gluBuild2DMipmaps(sides[i], 3, texture->width, texture->height, GL_BGR, GL_UNSIGNED_BYTE, texImg.pData);
      
      delete texImg.pData;
    }

    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    textureMap.insert(TextureMap::value_type(filename, texture));

    printf("LoadCubeMap Done\n");
    return texture;
  }
}



/**
   Prints the list of currently loaded textures
*/
void texmanListTextures()
{
  printf("\n-- Texture Manager -----------------------------\n");
  int n = 0;
  for(TextureMap::iterator itor = textureMap.begin(); itor != textureMap.end(); ++itor)
  {
    n++;
    printf(" %-32s %dx%d %d-bit\n", (itor->first.c_str()),
                                    (itor->second)->width,
                                    (itor->second)->height,
                                    (itor->second)->bpp);
  }
  printf("-- %3d textures --------------------------------\n\n\n", n);
}










/**
   Opens a TGA file and loads it into the pointer given.
   Only supports 24 or 32 bit TGAs in BGR(A) format.
*/
static int LoadTGA(char* filename, image_t* image)
{
  image->width	= 0;
  image->height	= 0;
  image->bpp		= 0;
  image->pData	= NULL;
  
  unsigned char* data = 0;
  FILE* file = fopen(filename, "rb");
  if(!file)
  {
    printf("LoadTGA not found!\n");
    return 0;
  }

  fseek(file, 0, SEEK_END);
  int length = ftell(file);
  fseek(file, 0, SEEK_SET);
  data = new unsigned char[length];
  fread(data, 1, length, file);

  //  if(!AssetManager()->Open(filename, data))
  //  {
  //    printf("LoadTGA not found\n");
  //    return 0;
  //  }

  
  if(!data)
  {
    return 0;
  }

  unsigned char* ptr = data;
  
  TGAHeader tgaHeader;
  memcpy(&tgaHeader, ptr, sizeof(TGAHeader));
  ptr += sizeof(TGAHeader);
  
  image->width	  = tgaHeader.width;
  image->height   = tgaHeader.height;
  image->bpp		  = tgaHeader.bpp;

  /*
  printf("Loading TGA:\n");
  printf("------------\n");
  printf(" %s\n", filename);
  printf(" width: %d\n height: %d\n bpp: %d\n\n", image->width, image->height, image->bpp);
  */

  int dataSize = tgaHeader.width * tgaHeader.height * (tgaHeader.bpp / 8);
  image->pData	= new unsigned char[dataSize];
  memcpy(image->pData, ptr, dataSize);
  delete data;

  return 1;
}



