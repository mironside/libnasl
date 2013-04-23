/**
  @file shader.cpp
  @author Christopher Olsen
  @date Mon Apr 26 22:42:54 2004

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
#include "shader.h"
#include "parser/shaderParse.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "textureManager.h"


#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOGDI
	#include <windows.h>

	#ifdef _DEBUG
		#include <crtdbg.h>
		#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#endif
#endif

// replace with render.h
#include <GL/gl.h>


float WaveValue(wave_t* w)
{
	float val = 0;

	switch(w->type)
	{
	case WAVE_SIN:
		val = (float)sin(w->phase * 2.0 * 3.14 / w->frequency) * w->amplitude + w->base;
		break;

	case WAVE_SQUARE:
		val = w->phase / w->frequency;
		val -= (int)(val);

		if(val > 0.5)
			val = w->amplitude;
		else
			val = -w->amplitude;
		break;

	case WAVE_TRIANGLE:
		val = w->phase / w->frequency;
		val -= (int)(val);

		if(val > 0.5)
			val = w->amplitude * (2.0f - 2.0f * val);
		else
			val = w->amplitude * val * 2.0f;
		break;

	case WAVE_SAW:
		val = w->phase / w->frequency;
		val -= (int)(val);

		val = w->amplitude * val;
		break;

	case WAVE_INVSAW:
		val = w->phase / w->frequency;
		val -= (int)(val);

		val = w->amplitude * (1.0f - val);
		break;

	default:
		break;
	}

	return val;
}




Shader::Shader(char* name, std::vector<shaderOp_t*>* shaderOps, std::vector<stage_t*>* stages) :
  name(0), stages(0), width(0), height(0), shaderOps(0), depthTest(true), cullFace(GL_BACK),
  depthWrite(true), flags(0)
{
  this->name = new char[strlen(name)+1];
  memset(this->name, 0, strlen(name)+1);
  strcpy(this->name, name);

  this->stages    = stages;
  this->shaderOps = shaderOps;
}



Shader::~Shader()
{
  delete name;

  if(stages)
  {
    for(int i = 0; i < stages->size(); i++)
    {
      delete (*stages)[i];
    }
    delete stages;
  }

  if(shaderOps)
  {
    for(int i = 0; i < shaderOps->size(); i++)
    {
      delete (*shaderOps)[i];
    }
    delete shaderOps;
  }
}



/* gets textures and opengl shit */
void Shader::Initialize()
{
  // initialize shaderOps (if any)
  if(shaderOps)
  {
    for(unsigned int i = 0; i < shaderOps->size(); i++)
    {
      switch((*shaderOps)[i]->kind)
      {
      case DEPTH_TEST:
        depthTest = (*shaderOps)[i]->depthTest;
        break;

      case CULL_FACE:
        cullFace = (*shaderOps)[i]->depthTest;
        break;

      case DEPTH_WRITE:
        depthWrite = (*shaderOps)[i]->depthWrite;
        break;

      case TRANSLUCENT:
        flags = SF_TRANSLUCENT;
        break;

        /*
        case VERTEX_PROGRAM:
        // load vertex program here
        printf("vertex program: %s\n", (*shaderOps)[i]->vertexProgram);
        printf("cgContext: %d\n", cgContext);

        vertexProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, (*shaderOps)[i]->vertexProgram, cgVertexProfile, "main", 0);
        printf("*** ERROR: %s\n", cgGetErrorString(cgGetError()));

        cgGLLoadProgram(vertexProgram);
        printf("*** ERROR: %s\n", cgGetErrorString(cgGetError()));
        printf("Compile Error: %s\n", cgGetLastListing(cgContext));

        modelViewMatrix = cgGetNamedParameter(vertexProgram, "modelViewProjMatrix");
        tangent         = cgGetNamedParameter(vertexProgram, "tangent");
        binormal        = cgGetNamedParameter(vertexProgram, "binormal");
        light           = cgGetNamedParameter(vertexProgram, "light");
        break;
        */

      default:
        printf("*** ERROR: Unknown Shader Operation\n");
        break;
      }
      delete (*shaderOps)[i];
    }

    delete shaderOps;
  }

  // initialize the stages
  for(unsigned int i = 0; i < stages->size(); i++)
  {
    printf("%s combiner %d\n", (*stages)[i]->textureName, (*stages)[i]->combiner);
    texture_t* texture = 0;
    if((*stages)[i]->textureType == GL_TEXTURE_CUBE_MAP)
      texture = texmanLoadCubeMap((*stages)[i]->textureName);
    else
      texture = texmanLoad2DMap((*stages)[i]->textureName);

    // use stage zero as the size of this shader
    if(i == 0)
    {
      width = texture->width;
      height = texture->height;
    }

    (*stages)[i]->textureId = texture->glID;
  }
}



void Shader::Print()
{
	printf("%s\n", name);
  printf("%d %d\n", width, height);
	printf("{\n");

  for(unsigned int i=0; i < stages->size(); i++)
  {
    printf("  {\n");
    printf("     texmap %s\n", (*stages)[i]->textureName);
    printf("     blendFunc %d %d\n", (*stages)[i]->srcBlend, (*stages)[i]->dstBlend);
    printf("     alphaTest %d %f\n", (*stages)[i]->alphaTest, (*stages)[i]->alphaValue);
    printf("  }\n");
  }
  printf("}\n");
}



void Shader::Update(float timeChange)
{
  for(unsigned int i = 0; i < stages->size(); i++)
  {
    //  T C M O D
    tcmod_t* tc = (*stages)[i]->tcmod;
    if(tc)
    {
      switch(tc->type)
      {
      case TCMOD_SCROLL:
        if(tc->waveFunc)
        {
          float val = WaveValue(tc->waveFunc);
          (*stages)[i]->u = tc->waveFunc->base + tc->param1 * val;
          (*stages)[i]->v = tc->waveFunc->base + tc->param2 * val;
          tc->waveFunc->phase += timeChange;
        }
        else
        {
          (*stages)[i]->u += tc->param1 * timeChange;
          (*stages)[i]->v += tc->param2 * timeChange;
        }
        break;

      case TCMOD_ROTATE:
        if(tc->waveFunc)
        {
          float val = WaveValue(tc->waveFunc);
          (*stages)[i]->rotation = tc->waveFunc->base + tc->param1 * val;
          tc->waveFunc->phase += timeChange;
        }
        else
        {
          (*stages)[i]->rotation = tc->param1 * timeChange;
        }
        break;

      default:
        break;
      }
    }

    rgbgen_t* rgbgen = (*stages)[i]->rgbgen;
    if(rgbgen)
    {
      if(rgbgen->waveFunc)
      {
        float val = WaveValue(rgbgen->waveFunc);
        (*stages)[i]->color[0] = rgbgen->color[0] * val;
        (*stages)[i]->color[1] = rgbgen->color[1] * val;
        (*stages)[i]->color[2] = rgbgen->color[2] * val;
        rgbgen->waveFunc->phase += timeChange;
      }
    }
  }
}



void Shader::Bind()
{
  if(depthTest)
  {
    glEnable(GL_DEPTH_TEST);
  }
  else
  {
    glDisable(GL_DEPTH_TEST);
  }

  glDepthMask(depthWrite);

  if(cullFace == GL_NONE)
  {
    glDisable(GL_CULL_FACE);
  }
  else
  {
    glCullFace(cullFace);
  }

  glEnable(GL_BLEND);

  for(int i = 0; i < 8; i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_CUBE_MAP);
  }



  for(unsigned int i = 0; i < stages->size(); i++)
  {
    //glColor4fv((*stages)[i]->color);
    glBlendFunc((*stages)[i]->srcBlend, (*stages)[i]->dstBlend);
    glActiveTexture(GL_TEXTURE0 + i);
    glEnable((*stages)[i]->textureType);
    glBindTexture((*stages)[i]->textureType, (*stages)[i]->textureId);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc((*stages)[i]->alphaTest, (*stages)[i]->alphaValue);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (*stages)[i]->wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (*stages)[i]->wrapMode);

    glClientActiveTexture(GL_TEXTURE0 + i);

    // normalmap
    if((*stages)[i]->combiner == GL_DOT3_RGB)
    {
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
      glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (*stages)[i]->color);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
    }
    else
    {
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
      glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (*stages)[i]->color);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, (*stages)[i]->combiner);
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef((*stages)[i]->u, (*stages)[i]->v, 0);
    glRotatef((*stages)[i]->rotation, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
  }

  /*
  if(vertexProgram)
  {
    cgGLSetStateMatrixParameter(modelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
    //    cgGLSetParameter4f(light, lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]);
    cgGLEnableProfile(cgVertexProfile);
    cgGLBindProgram(vertexProgram);
  }
  else
  {
    cgGLDisableProfile(cgVertexProfile);
  }
  */
}



int Shader::GetWidth()
{
  return width;
}



int Shader::GetHeight()
{
  return height;
}



char* Shader::GetName()
{
  return name;
}



int Shader::GetFlags()
{
  return flags;
}


