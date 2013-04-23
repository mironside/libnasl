/**

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
#ifndef __SHADER_PARSE__
#define __SHADER_PARSE__

enum chunkKinds   {TEX_MAP, CUBE_MAP, NORMAL_MAP, ALPHA_TEST, BLEND_FUNC, RGBGEN, TCMOD, COMBINER, WRAPMODE};
enum shaderOpKind {CULL_FACE, DEPTH_TEST, DEPTH_WRITE, TRANSLUCENT, VERTEX_PROGRAM};
enum rgbgenKind   {CONSTANT, DIFFUSE};
enum waveType     {WAVE_TRIANGLE, WAVE_SAW, WAVE_INVSAW, WAVE_SQUARE, WAVE_SIN};
enum tcmodType    {TCMOD_SCROLL, TCMOD_SCALE, TCMOD_ROTATE};

typedef struct wave_s
{
  waveType type;
  float base;
  float amplitude;
  float phase;
  float frequency;
} wave_t;


/* one line of info  chunks */
typedef struct chunk_s
{
	chunkKinds kind;

	union
	{
		char* textureName;
		struct blendfunc_s* blendFunc;
    struct alphatest_s* alphaTest;
    int depthTest;
    struct rgbgen_s* rgbgen;
    struct tcmod_s* tcmod;
    int combiner;
    int wrapMode;
	};
} chunk_t;


typedef struct rgbgen_s
{
  int kind;
  float color[3];
  wave_t* waveFunc;
} rgbgen_t;

typedef struct shaderOp_s
{
  shaderOpKind kind;

  union
  {
    int cullFace;
    int depthTest;
    int depthWrite;
    int translucent;
    char* vertexProgram;
  };
} shaderOp_t;



/* blend function chunk */
typedef struct blendfunc_s
{
	int src;
	int dst;
} blendfunc_t;



/* alpha function chunk */
typedef struct alphatest_s
{
  int test;
  float refValue;
} alphatest_t;





typedef struct tcmod_s
{
  tcmodType type;
  float param1;
  float param2;
  wave_t* waveFunc;
} tcmod_t;






/* complete stage */
typedef struct stage_s
{
  // stuff that gets filled by parsing
  int textureType;
	char* textureName;

	int   srcBlend;
	int   dstBlend;

  int   combiner;

  // optional stage info
  int   alphaTest;
  float alphaValue;

  int   wrapMode;

  float color[4];

  tcmod_t* tcmod;
  rgbgen_t* rgbgen;

  // other stuff
  int textureId;  // OpenGL texture id

  float u, v, rotation;
} stage_t;


extern int shaderparse();

#endif

