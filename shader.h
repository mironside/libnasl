/**
  @file    shader.h
  @author  Chris Olsen -- Vaporware Games
  @date    08-18-03

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
#ifndef __SHADER__
#define __SHADER__

#include "parser/shaderParse.h"
#include <vector>



// shader flags
const int SF_NONE            = 0;
const int SF_TRANSLUCENT     = 1;

class Shader
{
private:
	char* name;			// name of this shader
  std::vector<stage_t*>* stages;
  std::vector<shaderOp_t*>* shaderOps;

  int flags;
  int width, height;
  int depthTest;
  int depthWrite;
  int cullFace;


public:
	Shader(char* name, std::vector<shaderOp_t*>* ops, std::vector<stage_t*>* stages);
	virtual ~Shader();

	void Print();
  char* GetName();
  void Bind();
  void Initialize();
  void Update(float timeChange);

  int GetWidth();
  int GetHeight();
  int GetFlags();
};

#endif


