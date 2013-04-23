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
%{
#include <vector>
#include "../shader.h"
#include "shaderParse.h"
#include "shaderParse.cpp.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <io.h>
#else
#include <unistd.h>
#endif

#define YY_SKIP_YYWRAP
int yywrap() { return 1; }
extern int shlineno;

%}


LETTER    [a-zA-Z_]
DIGIT     [0-9]
IDENT     {LETTER}({LETTER}|{DIGIT})*
STRING    \"[^\"]*\"
WSPACE    [ \t]+
SECTION   "["{IDENT}"]"
FLOAT     [\-]?{DIGIT}*["."]?{DIGIT}*
NAME      ({LETTER}|{DIGIT}|"/"|".")*



%%

"{"                         { return tLBRACKET; }
"}"                         { return tRBRACKET; }
"texmap"		    { return tTEXMAP; }
"cubemap"                   { return tCUBEMAP; }
"normalmap"                 { return tNORMALMAP; }

"vertexProgram"             { return tVERTEXPROGRAM; }

"wrapMode"                  { return tWRAPMODE; }
"repeat"                    { return tREPEAT; }
"clamp"                     { return tCLAMP; }

"combiner"                  { return tCOMBINER; }
"dot3"                      { return tDOT3; }
"replace"                   { return tREPLACE; }
"modulate"                  { return tMODULATE; }
"add"                       { return tADD; }
"subtract"                  { return tSUBTRACT; }
"interpolate"               { return tINTERPOLATE; }



"blendFunc"                 { return tBLENDFUNC; }
"GL_ZERO"                   { return tGL_ZERO; }
"GL_ONE"                    { return tGL_ONE; }
"GL_SRC_ALPHA"              { return tGL_SRC_ALPHA; }
"GL_ONE_MINUS_SRC_ALPHA"    { return tGL_ONE_MINUS_SRC_ALPHA; }
"GL_DST_ALPHA"              { return tGL_DST_ALPHA; }
"GL_ONE_MINUS_DST_ALPHA"    { return tGL_ONE_MINUS_SRC_ALPHA; }
"GL_SRC_COLOR"              { return tGL_SRC_COLOR; }
"GL_ONE_MINUS_SRC_COLOR"    { return tGL_ONE_MINUS_SRC_COLOR; }
"GL_DST_COLOR"              { return tGL_DST_COLOR; }
"GL_ONE_MINUS_DST_COLOR"    { return tGL_ONE_MINUS_DST_COLOR; }


"alphaTest"                 { return tALPHATEST; }

"GT"                        { return tGL_GREATER; }
"LT"                        { return tGL_LESS; }
"NE"                        { return tGL_EQUAL; }
"EQ"                        { return tGL_EQUAL; }
"GTE"                       { return tGL_GEQUAL; }
"LTE"                       { return tGL_LEQUAL; }
"NEVER"                     { return tGL_NEVER; }
"ALWAYS"                    { return tGL_NEVER; }


"disable"                   { return tDISABLE; }
"enable"                    { return tENABLE; }

"depthTest"                 { return tDEPTHTEST; }

"cullFace"                  { return tCULLFACE; }
"front"                     { return tFRONT; }
"back"                      { return tBACK; }


"rgbgen"                    { return tRGBGEN; }
"diffuseLighting"           { return tDIFFUSELIGHTING; }
"depthWrite"                { return tDEPTHWRITE; }
"translucent"               { return tTRANSLUCENT; }


"tcmod"                     { return tTCMOD; }
"scale"                     { return tSCALE; }
"scroll"                    { return tSCROLL; }
"rotate"                    { return tROTATE; }

"wave"                      { return tWAVE; }
"triangle"                  { return tTRIANGLE; }
"saw"                       { return tSAW; }
"invsaw"                    { return tINVSAW; }
"square"                    { return tSQUARE; }
"sin"                       { return tSIN; }


{FLOAT}     {
              shaderlval.floatConst = atof(yytext);
              return tFLOAT;
            }


{NAME}      {
              shaderlval.string = new char[strlen(shadertext)+1];
              memset(shaderlval.string, 0, strlen(shadertext));
              strcpy(shaderlval.string, shadertext);
              return tNAME;
            }


"#"         {
              while(yyinput() != '\n') {}
              shlineno++;
            }


\n          {
              shlineno++;
            }


{WSPACE}    {
            }


.           {
              printf("Lexical Error on line %d:\n\'%s'\n", shlineno+1, shadertext);
              exit(1);
            }

%%
