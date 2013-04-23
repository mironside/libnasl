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
  
  stage  vector       PASSED -> shader
  chunk  vector       DESTROYED (in conversion to stage)
  
  chunks:
  textureName      PASSED -> shader
  blendFunc        DESTROYED (in conversion to stage)
  
  
  if this crashes it'll leak like a mofo!

  shader options
          cullFace
          depthTest
*/


%{

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>

#include "../shader.h"
#include "shaderParse.h"
//#include "render/render.h"

#include <GL/gl.h>
#include <vector>

extern int shaderlex();
void shadererror(const char *format, ...);
int shlineno;

extern char* shadertext;

std::vector<Shader*> shaderList;

%}



%union
{
  float floatConst;
  int   intConst;
  
  char* string;
  int	  blendParm;

  int op;

  Shader* shader;
  std::vector<Shader*>* shaders;

  struct stage_s* stage;
  std::vector<stage_t*>* stages;

  struct chunk_s* chunk;
  std::vector<chunk_t*>* chunks;

  struct shaderOp_s* shaderOp;
  std::vector<shaderOp_t*>* shaderOps;

  waveType wavetype;

  int combiner;
  int wrapMode;

  // CHUNK TYPES
  struct blendfunc_s* blendFunc;
  struct alphafunc_s* alphaFunc;
  struct rgbgen_s* rgbgen;
  struct tcmod_s* tcmod;
  struct wave_s* wave;
};





%start shaderFile

%token tLBRACKET
%token tRBRACKET

%token tTEXMAP
%token tCUBEMAP
%token tNORMALMAP

%token tVERTEXPROGRAM

%token tWRAPMODE
%token tREPEAT
%token tCLAMP

%token tCOMBINER
%token tREPLACE
%token tMODULATE
%token tADD
%token tSUBTRACT
%token tINTERPOLATE
%token tDOT3


// Blend Function
%token tBLENDFUNC
%token tGL_ZERO
%token tGL_ONE
%token tGL_SRC_ALPHA
%token tGL_ONE_MINUS_SRC_ALPHA
%token tGL_DST_ALPHA
%token tGL_ONE_MINUS_DST_ALPHA
%token tGL_SRC_COLOR
%token tGL_ONE_MINUS_SRC_COLOR
%token tGL_DST_COLOR
%token tGL_ONE_MINUS_DST_COLOR


// GL Tests
%token tALPHATEST

// GL Comparators
%token tGL_ALWAYS
%token tGL_NEVER
%token tGL_NOTEQUAL
%token tGL_EQUAL
%token tGL_LESS
%token tGL_GREATER
%token tGL_LEQUAL
%token tGL_GEQUAL


%token tDISABLE
%token tENABLE

%token tDEPTHTEST

%token tCULLFACE
%token tFRONT
%token tBACK


%token tTCMOD

%token tRGBGEN
%token tDIFFUSELIGHTING

%token tDEPTHWRITE
%token tTRANSLUCENT

// tcmod
%token tTCMOD
%token tSCALE
%token tSCROLL
%token tROTATE

%token tWAVE
%token tTRIANGLE
%token tSAW
%token tINVSAW
%token tSQUARE
%token tSIN

%token <string>     tNAME
%token <intConst>   tINT
%token <floatConst> tFLOAT



%type <blendParm>   blendFuncParm

%type <shader>      shader
%type <shaders>     anyshader shaders

%type <stage>       stage
%type <stages>      anystage stages

%type <chunk>       chunk
%type <chunks>      anychunk chunks

%type <shaderOp>    shaderOp
%type <shaderOps>   anyshaderOp shaderOps

%type <tcmod>       tcmod
%type <wave>        waveFunc
%type <op>          glOp
%type <op>          enableDisable
%type <op>          cullFace
%type <wavetype>    waveType

%type <rgbgen>      rgbgen
%type <combiner>    combinerParm
%type <wrapMode>    wrapMode;

%%





shaderFile:
		shaders
		{
      /*
      for(unsigned int i=0; i < $1->size(); i++)
      {
        // put shaders into shadermanager
        //ShaderManager()->AddShader((*$1)[i]);
        //(*$1)[i]->Print();
      }

      // delete $1 vector (not the shaders)
      delete $1;
      */
		}
		;



///////////////////
// S H A D E R S
///////////////////
shaders:
		/* empty */
		{
      //      $$ = 0;
		}

		| anyshader
		{
      /*
      std::vector<Shader*>* sh = new std::vector<Shader*>();
      while($1->size())
      {
        sh->push_back($1->back());
        $1->pop_back();
      }

      // release reversed vector
      delete $1;
      $$ = sh;
      */
		}
		;



anyshader:
		shader
		{
      //      $$ = new std::vector<Shader*>();
      //      $$->push_back($1);
		}

		| shader anyshader
		{
      //      $2->push_back($1);
      //      $$ = $2;
		}
		;



shader:
		tNAME tLBRACKET shaderOps stages tRBRACKET
		{
      // the stages vector is used in the new shader
      // so DON'T DELETE IT!
      //			$$ = new Shader($1, $3, $4);
      shaderList.push_back(new Shader($1, $3, $4));
		}
		;



////////////////////////////
// S H A D E R   O P S
////////////////////////////
shaderOps:
    /* empty */
    {
      $$ = 0;
    }
    | anyshaderOp
    {
      $$ = $1;
    }
    ;



anyshaderOp:
    shaderOp
    {
      $$ = new std::vector<shaderOp_t*>();
      $$->push_back($1);
    }
    | shaderOp anyshaderOp
    {
      $2->push_back($1);
      $$ = $2;
    }
    ;



shaderOp:
    tVERTEXPROGRAM tNAME
    {
      shaderOp_t* op = new shaderOp_t;
      memset(op, 0, sizeof(shaderOp_t));

      op->kind = VERTEX_PROGRAM;
      op->vertexProgram = $2;
      
      $$ = op;
    }

    | tDEPTHTEST enableDisable
    {
      shaderOp_t* op = new shaderOp_t;
      memset(op, 0, sizeof(shaderOp_t));

      op->kind = DEPTH_TEST;
      op->depthTest = $2;
      
      $$ = op;
    }

    | tCULLFACE cullFace
    {
      shaderOp_t* op = new shaderOp_t;
      memset(op, 0, sizeof(shaderOp_t));

      op->kind = CULL_FACE;
      op->cullFace = $2;

      $$ = op;
    }

    | tDEPTHWRITE enableDisable
    {
      shaderOp_t* op = new shaderOp_t;
      memset(op, 0, sizeof(shaderOp_t));

      op->kind = DEPTH_WRITE;
      op->depthWrite = $2;

      $$ = op;
    }

    | tTRANSLUCENT
    {
      shaderOp_t* op = new shaderOp_t;
      memset(op, 0, sizeof(shaderOp_t));

      op->kind = TRANSLUCENT;
      op->translucent = true;

      $$ = op;
    }
    ;




//////////////////
// S T A G E S
//////////////////
stages:
		anystage
		{
      // stages come in reversed, correct that
      std::vector<stage_t*>* st = new std::vector<stage_t*>();
      while($1->size())
      {
        st->push_back($1->back());
        $1->pop_back();
      }

      // release reversed vector
      delete $1;
			$$ = st;
		}
		;



anystage:
    stage
    {
      // this is given to the shader to deal with
      // no need to release it from memory,
      // the Shader should do that
      $$ = new std::vector<stage_t*>();
      $$->push_back($1);
    }

		| stage anystage
		{
      $2->push_back($1);
      $$ = $2;
		}
		;



stage:
		tLBRACKET chunks tRBRACKET
		{
      if(!$2)
      {
        return 0;
      }


      $$ = new stage_t;
      memset($$, 0, sizeof(stage_t));

      // set stage defaults
      $$->srcBlend = GL_ONE;
      $$->dstBlend = GL_ZERO;
      $$->alphaTest = GL_ALWAYS;
      $$->alphaValue = 1.0;
      $$->color[0] = 1.0f;
      $$->color[1] = 1.0f;
      $$->color[2] = 1.0f;
      $$->color[3] = 1.0f;
      $$->combiner = GL_MODULATE;
      $$->wrapMode = GL_REPEAT;
      $$->rgbgen = 0;
      


      for(unsigned int i=0; i < $2->size(); i++)
      {
        switch((*$2)[i]->kind)
        {
        case TEX_MAP:
          $$->textureType = GL_TEXTURE_2D;
          $$->textureName = (*$2)[i]->textureName;
          break;


        case CUBE_MAP:
          $$->textureType = GL_TEXTURE_CUBE_MAP;
          $$->textureName = (*$2)[i]->textureName;
          break;


        case NORMAL_MAP:
          $$->textureType = GL_TEXTURE_2D;
          $$->textureName = (*$2)[i]->textureName;
          break;


        case COMBINER:
          $$->combiner = (*$2)[i]->combiner;
          break;


        case BLEND_FUNC:
          $$->srcBlend = (*$2)[i]->blendFunc->src;
          $$->dstBlend = (*$2)[i]->blendFunc->dst;

          // release blendFunc chunk
          delete (*$2)[i]->blendFunc;
          break;


        case ALPHA_TEST:
          $$->alphaTest  = (*$2)[i]->alphaTest->test;
          $$->alphaValue = (*$2)[i]->alphaTest->refValue;

          // release blendFunc chunk
          delete (*$2)[i]->alphaTest;
          break;

        case RGBGEN:
          {
            switch((*$2)[i]->rgbgen->kind)
            {
            case CONSTANT:
              $$->color[0] = (*$2)[i]->rgbgen->color[0];
              $$->color[1] = (*$2)[i]->rgbgen->color[1];
              $$->color[2] = (*$2)[i]->rgbgen->color[2];
              $$->color[3] = (*$2)[i]->rgbgen->color[3];
              $$->rgbgen   = (*$2)[i]->rgbgen;
              break;

            case DIFFUSE:
              break;

            default:
              printf("*** ERROR: Unknown rgbgen Type\n");
              break;
            }
          }
          break;

        case TCMOD:
          $$->tcmod = (*$2)[i]->tcmod;
          break;

        case WRAPMODE:
          $$->wrapMode = (*$2)[i]->wrapMode;
          printf("wrapmode %d\n", $$->wrapMode);
          break;

        default:
          printf("***ERROR: Unrecognized Chunk Type!\n");
          break;
        }

        // release the chunk
        delete (*$2)[i];
      }

      // release the chunk vector
      delete $2;
		}
    ;



/////////////////
// C H U N K S
/////////////////
chunks:
		/* empty */
		{
			$$ = 0;
		}
		
		| anychunk
		{
			$$ = $1;
		}
		;
		


anychunk:
		chunk
		{
      // this is a temporary array that is only needed
      // to create stages...  release it after the
      // shader has been created...
      $$ = new std::vector<chunk_t*>();
			$$->push_back($1);
		}

		| chunk anychunk
		{
			$2->push_back($1);
      $$ = $2;
		}
		;



chunk:
    // TEXTURE MAP
		tTEXMAP tNAME
		{
			chunk_t* ch = new chunk_t;
			memset(ch, 0, sizeof(chunk_t));

			ch->kind = TEX_MAP;
			ch->textureName = $2;

			$$ = ch;
		}

    // CUBE MAP
		| tCUBEMAP tNAME
		{
			chunk_t* ch = new chunk_t;
			memset(ch, 0, sizeof(chunk_t));

			ch->kind = CUBE_MAP;
			ch->textureName = $2;

			$$ = ch;
		}

    // NORMAL MAP
		| tNORMALMAP tNAME
		{
			chunk_t* ch = new chunk_t;
			memset(ch, 0, sizeof(chunk_t));

			ch->kind = NORMAL_MAP;
			ch->textureName = $2;

			$$ = ch;
		}

    // COMBINER
		| tCOMBINER combinerParm
		{
			chunk_t* ch = new chunk_t;
			memset(ch, 0, sizeof(chunk_t));

			ch->kind = COMBINER;
      ch->combiner = $2;

			$$ = ch;
		}

    | tWRAPMODE wrapMode
    {
			chunk_t* ch = new chunk_t;
			memset(ch, 0, sizeof(chunk_t));

			ch->kind = WRAPMODE;
      ch->wrapMode = $2;

			$$ = ch;
    }

    // BLEND FUNCTION
    | tBLENDFUNC blendFuncParm blendFuncParm
    {
      chunk_t* ch = new chunk_t;
      memset(ch, 0, sizeof(chunk_t));

      ch->kind = BLEND_FUNC;

      ch->blendFunc = new blendfunc_t;
      memset(ch->blendFunc, 0, sizeof(blendfunc_t));
      ch->blendFunc->src = $2;
      ch->blendFunc->dst = $3;

      $$ = ch;
    }

    // ALPHA TEST
    | tALPHATEST glOp tFLOAT
    {
      chunk_t* ch = new chunk_t;
      memset(ch, 0, sizeof(chunk_t));

      ch->kind = ALPHA_TEST;

      ch->alphaTest = new alphatest_t;
      memset(ch->alphaTest, 0, sizeof(alphatest_t));

      ch->alphaTest->test = $2;
      ch->alphaTest->refValue = $3;

      $$ = ch;
    }

    | rgbgen
    {
      chunk_t* ch = new chunk_t;
      memset(ch, 0, sizeof(chunk_t));

      ch->kind = RGBGEN;
      ch->rgbgen = $1;

      $$ = ch;
    }

    | tcmod
    {
      chunk_t* ch = new chunk_t;
      memset(ch, 0, sizeof(chunk_t));

      ch->kind = TCMOD;
      ch->tcmod = $1;

      $$ = ch;
    }
		;



wrapMode:
    tREPEAT
    {
      $$ = GL_REPEAT;
    }

    | tCLAMP
    {
      $$ = GL_CLAMP;
    }
    ;


combinerParm:
    tDOT3
    {
      $$ = GL_DOT3_RGB;
    }

    | tMODULATE
    {
      $$ = GL_MODULATE;
    }

    | tREPLACE
    {
      $$ = GL_REPLACE;
    }

    | tADD
    {
      $$ = GL_ADD;
    }

    | tSUBTRACT
    {
      $$ = GL_SUBTRACT;
    }

    | tINTERPOLATE
    {
      $$ = GL_INTERPOLATE;
    }
    ;



////////////////////////////////
// R G B G E N
////////////////////////////////
rgbgen:
    tRGBGEN tFLOAT tFLOAT tFLOAT
    {
      $$ = new rgbgen_t;
      memset($$, 0, sizeof(rgbgen_t));
      $$->kind = CONSTANT;
      $$->color[0] = $2;
      $$->color[1] = $3;
      $$->color[2] = $4;
      $$->color[3] = 1.0f;
      $$->waveFunc = 0;
    }

    | tRGBGEN tFLOAT tFLOAT tFLOAT tFLOAT
    {      
      $$ = new rgbgen_t;
      memset($$, 0, sizeof(rgbgen_t));
      $$->kind = CONSTANT;
      $$->color[0] = $2;
      $$->color[1] = $3;
      $$->color[2] = $4;
      $$->color[3] = $5;
      $$->waveFunc = 0;
    }

    | tRGBGEN tFLOAT tFLOAT tFLOAT waveFunc
    {
      $$ = new rgbgen_t;
      memset($$, 0, sizeof(rgbgen_t));
      $$->kind = CONSTANT;
      $$->color[0] = $2;
      $$->color[1] = $3;
      $$->color[2] = $4;
      $$->color[3] = 1.0f;
      $$->waveFunc = $5;
    }

    | tRGBGEN tFLOAT tFLOAT tFLOAT tFLOAT waveFunc
    {      
      $$ = new rgbgen_t;
      memset($$, 0, sizeof(rgbgen_t));
      $$->kind = CONSTANT;
      $$->color[0] = $2;
      $$->color[1] = $3;
      $$->color[2] = $4;
      $$->color[3] = $5;
      $$->waveFunc = $6;
    }

    | tRGBGEN tDIFFUSELIGHTING
    {
      $$ = new rgbgen_t;
      memset($$, 0, sizeof(rgbgen_t));
      $$->kind = DIFFUSE;
    }
    ;


/////////////////////////////////
// T C M O D
/////////////////////////////////
tcmod:
    tTCMOD tSCROLL tFLOAT tFLOAT
    {
      tcmod_t* tcmod = new tcmod_t;
      memset(tcmod, 0, sizeof(tcmod_t));
      tcmod->type = TCMOD_SCROLL;
      tcmod->param1 = $3;
      tcmod->param2 = $4;

      $$ = tcmod;
    }

    | tTCMOD tSCROLL tFLOAT tFLOAT waveFunc
    {
      tcmod_t* tcmod = new tcmod_t;
      memset(tcmod, 0, sizeof(tcmod_t));
      tcmod->type = TCMOD_SCROLL;
      tcmod->param1 = $3;
      tcmod->param2 = $4;
      tcmod->waveFunc = $5;

      $$ = tcmod;
    }

    | tTCMOD tSCALE tFLOAT tFLOAT
    {
      tcmod_t* tcmod = new tcmod_t;
      memset(tcmod, 0, sizeof(tcmod_t));
      tcmod->type = TCMOD_SCALE;
      tcmod->param1 = $3;
      tcmod->param2 = $4;

      $$ = tcmod;
    }

    | tTCMOD tSCROLL tFLOAT tFLOAT waveFunc
    {
      tcmod_t* tcmod = new tcmod_t;
      memset(tcmod, 0, sizeof(tcmod_t));
      tcmod->type = TCMOD_SCALE;
      tcmod->param1 = $3;
      tcmod->param2 = $4;
      tcmod->waveFunc = $5;

      $$ = tcmod;
    }

    | tTCMOD tSCROLL tFLOAT
    {
      tcmod_t* tcmod = new tcmod_t;
      memset(tcmod, 0, sizeof(tcmod_t));
      tcmod->type = TCMOD_SCROLL;
      tcmod->param1 = $3;

      $$ = tcmod;
    }

    | tTCMOD tSCROLL tFLOAT waveFunc
    {
      tcmod_t* tcmod = new tcmod_t;
      memset(tcmod, 0, sizeof(tcmod_t));
      tcmod->type = TCMOD_ROTATE;
      tcmod->param1 = $3;
      tcmod->waveFunc = $4;

      $$ = tcmod;
    }
    ;



waveFunc:
    tWAVE waveType tFLOAT tFLOAT tFLOAT tFLOAT
    {
      wave_t* wave = new wave_t;
      memset(wave, 0, sizeof(wave_t));

      wave->type      = $2;
      wave->base      = $3;
      wave->amplitude = $4;
      wave->phase     = $5;
      wave->frequency = $6;

      $$ = wave;
    }
    ;


waveType:
    tSIN
    {
      $$ = WAVE_SIN;
    }

    | tSAW
    {
      $$ = WAVE_SAW;
    }

    | tINVSAW
    {
      $$ = WAVE_INVSAW;
    }

    | tSQUARE
    {
      $$ = WAVE_SQUARE;
    }

    | tTRIANGLE
    {
      $$ = WAVE_TRIANGLE;
    }
    ;

enableDisable:
    tENABLE
    {
      $$ = true;
    }
    | tDISABLE
    {
      $$ = false;
    }
    ;



cullFace:
    tDISABLE
    {
      $$ = GL_NONE;
    }

    | tFRONT
    {
      $$ = GL_FRONT;
    }

    | tBACK
    {
      $$ = GL_BACK;
    }
    ;



blendFuncParm:
		tGL_ONE
		{
			$$ = GL_ONE;
		}
		
		| tGL_ZERO
		{
			$$ = GL_ZERO;
		}
		
		| tGL_SRC_ALPHA
		{
			$$ = GL_SRC_ALPHA;
		}
		
		| tGL_ONE_MINUS_SRC_ALPHA
		{
			$$ = GL_ONE_MINUS_SRC_ALPHA;
		}

		| tGL_DST_ALPHA
		{
			$$ = GL_DST_ALPHA;
		}

		| tGL_ONE_MINUS_DST_ALPHA
		{
			$$ = GL_ONE_MINUS_DST_ALPHA;
		}

		| tGL_SRC_COLOR
		{
			$$ = GL_SRC_COLOR;
		}

		| tGL_ONE_MINUS_SRC_COLOR
		{
			$$ = GL_ONE_MINUS_SRC_COLOR;
		}

		| tGL_DST_COLOR
		{
			$$ = GL_DST_COLOR;
		}

		| tGL_ONE_MINUS_DST_COLOR
		{
			$$ = GL_ONE_MINUS_DST_COLOR;
		}
    ;



glOp:
    tGL_GREATER
    {
      $$ = GL_GREATER;
    }
    | tGL_LESS
    {
      $$ = GL_LESS;
    }
    | tGL_EQUAL
    {
      $$ = GL_EQUAL;
    }
    | tGL_NOTEQUAL
    {
      $$ = GL_NOTEQUAL;
    }
    | tGL_LEQUAL
    {
      $$ = GL_LEQUAL;
    }
    | tGL_GEQUAL
    {
      $$ = GL_GEQUAL;
    }
    | tGL_NEVER
    {
      $$ = GL_NEVER;
    }
    ;      
%%



void shadererror(const char *format, ...)
{
	va_list argList;

	va_start(argList, format);
	vfprintf(stderr, format, argList);
	va_end(argList);

	printf("*** Parse Error on line %d:\n\'%s\'\n\n", shlineno+1, shadertext);

	exit(1);
}


