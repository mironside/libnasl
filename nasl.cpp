/**
  @file nasl.cpp
  @author 
  @date Sun Apr 25 17:02:20 2004

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
#include "nasl.h"
#include <map>
#include <string>
#include <stdio.h>
#include "parser/shaderParse.h"
#include "shader.h"
#include "textureManager.h"
#include <GL/gl.h>



#define MAX_SHADERS 256

extern std::vector<Shader*> shaderList;
typedef std::map<std::string, int> ShaderMap;
static ShaderMap shaderMap;
static Shader* loadedShaders[MAX_SHADERS];




/**
   ShaderFromTGA

   Creates a new shader with a single opaque stage from a TGA

   @parm name The name of the shader
         NOTE: the name should not have a tga extension

   @return A new shader created from the TGA, or 0 if the TGA was not found
   
*/
static Shader* ShaderFromTGA(char* name)
{
  char texname[256];
  sprintf(texname, "%s.tga", name);

  char* textureName = new char[strlen(texname)+1];
  memset(textureName, 0, strlen(texname)+1);
  strcpy(textureName, texname);
  
  texture_t* texture = texmanLoad2DMap(texname);

  if(!texture)
    return 0;

  printf("shaderFromTGA %s: %dx%d %d-bit\n", name, texture->width, texture->height, texture->bpp);

  std::vector<stage_t*>* stages = new std::vector<stage_t*>();
  stage_t* stage = new stage_t;
  memset(stage, 0, sizeof(stage_t));

  stage->textureType = GL_TEXTURE_2D;
  stage->textureName = textureName;
  stage->srcBlend    = GL_ONE;
  stage->dstBlend    = GL_ZERO;
  stage->alphaTest   = GL_ALWAYS;
  stage->wrapMode    = GL_REPEAT;
  stage->color[0]    = 1.0f;
  stage->color[1]    = 1.0f;
  stage->color[2]    = 1.0f;
  stage->color[3]    = 1.0f;
  stage->combiner    = GL_MODULATE;

  stages->push_back(stage);

  return new Shader(name, 0, stages);
}




/**

   naslUpdateShaders

   Updates everything in all shaders that is dependent on time.  This
   should be called only once per frame.

   @paran timeChange The amount of time that has passed since the last update

*/  
void naslUpdateShaders(float timeChange)
{
  for(int i = 0; i < MAX_SHADERS; i++)
  {
    if(loadedShaders[i])
    {
      loadedShaders[i]->Update(timeChange);
    }
  }
}




/**

   naslInitialize

   Initializes the nasl system.

   @return 1 if there was an error, 0 otherwise

*/
int naslInitialize()
{
  texmanInitialize();

  texmanListTextures();
  
  // clear out data structures
  memset(&loadedShaders, 0, MAX_SHADERS * sizeof(loadedShaders[0]));
  shaderMap.clear();

  // load notex shader into loadedShaders[0], the default for all failed loads
  // if this doesn't exist, there is a huge problem
  loadedShaders[0] = ShaderFromTGA("radiant/notex");

  if(!loadedShaders[0])
  {
    printf("*** NASL ERROR: failed to load radiant/notex, oh snap!\n");
    return 1;
  }

  shaderMap.insert(ShaderMap::value_type("radiant/notex", 0));

  return 0;
}




/**

   naslDestroy

   Destroys all shaders and shutsdown NASL.

*/
void naslDestroy()
{
  naslFreeAllShaders();
  delete loadedShaders[0];
}




/**

   naslBindShader

   Binds the shader of the given id, if a shader of that id does not
   exist the notex shader is bound.  naslBindShader(0) also binds the
   notex shader.

   @param id The id of the shader that is to be bound.

*/
void naslBindShader(int id)
{
  if(loadedShaders[id])
  {
    loadedShaders[id]->Bind();
  }
  else
  {
    loadedShaders[0]->Bind();
  }
}




/**

   naslLoadShader

   Attempts to load a shader.  NASL tries to load the shader from the
   default shader file first.  If the shader is not found there a
   shader is made from a TGA of the same name as the shader.  If that
   fails, the notex shader id is returned.  This function also
   gauruntees that no shader is ever loaded more than once.

   @param name The name of the shader to load

   @return The id of the shader that was loaded

*/
int naslLoadShader(char* name)
{
  // search loaded shaders
  ShaderMap::iterator itor;
	if((itor = shaderMap.find(name)) != shaderMap.end())
  {
    printf("found %s %d\n", name, itor->second);
    return itor->second;
  }




  printf("%s not previously loaded\n", name);




  //////////
  //
  // scan the shaders in the default shader file
  //
  //////////

  FILE* file = freopen("shader.txt", "r", stdin);
  shaderparse();
  fclose(file);

  Shader* shader = 0;
  for(int i = 0; i < shaderList.size(); i++)
  {
    if(strcmp(name, shaderList[i]->GetName()) == 0)
    {
      printf("found %s in shader.txt\n", name);
      shader = shaderList[i];
    }
  }

  // delete the shaders
  //  NOTE: the selected shader's data must not be destroyed,
  // otherwise the new shader's data will be invalid (because of
  // heinous pointers
  for(int i = 0; i < shaderList.size(); i++)
  {
    if(shader != shaderList[i])
      delete shaderList[i];
  }
  shaderList.clear();



  if(shader)
  {
    // insert the shader into a free spot
    int freeShader = 0;
    for(int i = 1; i < MAX_SHADERS; i++)
    {
      if(loadedShaders[i] == 0)
      {
        freeShader = i;
        break;
      }
    }

    if(freeShader == 0)
    {
      printf("SHIT! no room for more shaders!\n");
      return 0;
    }

    // add the shader to the shader list and map
    shader->Initialize();
    loadedShaders[freeShader] = shader;
    shaderMap.insert(ShaderMap::value_type(name, freeShader));
    return freeShader;
  }




  //////////
  //
  // Try to make a shader from a tga
  //
  //////////

  shader = ShaderFromTGA(name);
  if(!shader)
  {
    printf("TGA not found for %s\n", name);
    return 0;
  }



  // insert the shader into a free spot
  int freeShader = 0;
  for(int i = 1; i < MAX_SHADERS; i++)
  {
    if(loadedShaders[i] == 0)
    {
      freeShader = i;
      break;
    }
  }

  if(freeShader == 0)
  {
    printf("SHIT! no room for more shaders!\n");
    return 0;
  }

  printf("%s made from texture\n", name);

  // add the shader to the shader list and map
  shader->Initialize();
  loadedShaders[freeShader] = shader;
  shaderMap.insert(ShaderMap::value_type(name, freeShader));
  return freeShader;
}



void naslFreeShader(int id)
{
  // do not allow shader 0 to be freed
  if(id <= 0)
    return;

  // remove shader from loadedShaders
  if(loadedShaders[id])
  {
    printf("removing shader %d\n", id);
    delete loadedShaders[id];
    loadedShaders[id] = 0;
  }


  // remove shader from the map
  ShaderMap::iterator itor;
  for(itor = shaderMap.begin(); itor != shaderMap.end(); ++itor)
  {
    //    printf("%d %s\n", itor->first.c_str(), itor->second);
    if(itor->second == id)
    {
      shaderMap.erase(itor);
      break;
    }
  }
}




/**

   naslFreeAllShaders

   Destroys all loaded shaders.

*/
void naslFreeAllShaders()
{
  printf("Freeing all shaders\n");

  for(int i = 1; i < MAX_SHADERS; i++)
  {
    if(loadedShaders[i])
    {
      printf("  %d: %s\n", i, loadedShaders[i]->GetName());
      delete loadedShaders[i];
      loadedShaders[i] = 0;
    }
  }
}




/**

   naslListShaders

   Prints a listing of all the shaders to stdout

*/
void naslListShaders()
{
  printf("\n\nLoaded Shaders\n");
  printf("------------------------------------------\n");
  int n = 0;
  for(int i = 0; i < MAX_SHADERS; i++)
  {
    if(loadedShaders[i])
    {
      n++;
      printf("  %03d: %s\n", i, loadedShaders[i]->GetName());
    }
  }
  printf("-- %3d shaders ---------------------------\n\n");

  texmanListTextures();
}
