#ifdef _WIN32
#include <windows.h>
#else
#include "unistd.h"
#include "sys/stat.h"
#include "sys/types.h"
#endif

#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include <string>
#include <vector>
#include <algorithm>

#include "SDL.h"
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#ifdef __APPLE__
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#else
#include "GL/glew.h"
#include "GL/gl.h"
#include "GL/glu.h"
#endif
#endif
#include "SDLauxiliar.h"
#include "GLauxiliar.h"

#include "SDLauxiliar.h"
#include "GLauxiliar.h"


// these global variables can be used to avoid having to pass programIDs all over the code
GLuint programID = 0;

GLuint PVMatrixID = 0;
GLuint MMatrixID = 0;
GLuint inColorID = 0;
GLuint useTextureID = 0;

GLuint lastTexture = 0;

GLuint LoadShaders()
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode =
    "attribute vec3 vPosition;\n"
    "attribute vec2 UV;\n"
    "varying vec2 texture_coordinate;\n"
    "uniform mat4 PV;\n"
    "uniform mat4 M;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = PV * M * vec4(vPosition, 1.0);\n"
    "  texture_coordinate = UV;\n"
    "}";
    
    std::string FragmentShaderCode =
#ifdef __EMSCRIPTEN__
    "precision mediump float;\n"
#endif
    "varying vec2 texture_coordinate;\n"
    "uniform sampler2D textureSampler;\n"
    "uniform vec4 inColor;\n"
    "uniform int useTexture;\n"
    "void main()\n"
    "{\n"
    "  if (useTexture==1) {\n"
    "    vec4 c = texture2D(textureSampler, texture_coordinate);\n"
    "    gl_FragColor = vec4(inColor.r*c.r, inColor.g*c.g, inColor.b*c.b, inColor.a*c.a);\n"
    "  } else {\n"
    "    gl_FragColor = inColor;\n"
    "  }\n"
    "}                                         \n";
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    // Compile Vertex Shader
    output_debug_message("Compiling vertex shader\n");
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(std::max(InfoLogLength, int(1)));
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	output_debug_message("%s\n", &VertexShaderErrorMessage[0]);
    
    // Compile Fragment Shader
    output_debug_message("Compiling fragment shader\n");
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(std::max(InfoLogLength, int(1)));
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	output_debug_message("%s\n", &FragmentShaderErrorMessage[0]);
    
    // Link the program
    output_debug_message("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    
    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( std::max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    output_debug_message("%s\n", &ProgramErrorMessage[0]);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    
    return ProgramID;
}



void setTexture(GLuint tex)
{
    if (tex!=lastTexture) {
        // if lastTexture = 0 it means it's the first time we use a texture:
        glBindTexture(GL_TEXTURE_2D, tex);
        lastTexture = tex;
    }
}



int nearest_2pow(int n)
{
    int res=2;
    
    for(res=2;res<2048;res*=2) {
        if (res>=n) return res;
    } /* for */
    
    return res;
} /* nearest_2pow */


GLuint  createTexture(SDL_Surface *sfc,float *tx,float *ty)
{
    unsigned int tname=0;
    int szx,szy;
    
    if (sfc!=0) {
        
        SDL_Surface *sfc2=0;
        
        szx=nearest_2pow(sfc->w);
        szy=nearest_2pow(sfc->h);
        *tx=(sfc->w)/float(szx);
        *ty=(sfc->h)/float(szy);
        sfc2=SDL_CreateRGBSurface(SDL_SWSURFACE,szx,szy,32,RMASK,GMASK,BMASK,AMASK);
        //        output_debug_message("createTexture: sfc: %p (%i,%i), sfc2: %p (%i,%i) [%g,%g]\n",sfc,sfc->w,sfc->h,sfc2,sfc2->w,sfc2->h,*tx,*ty);
        //        SDL_SetSurfaceAlphaMod(sfc, 0);
        SDL_BlitSurface(sfc,0,sfc2,0);
        //        output_debug_message("Surface copied\n");
        
        setTexture(0);
        glGenTextures(1,&tname);
        //glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        setTexture(tname);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        //      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
        //      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        //      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        //      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,szx,szy,0,GL_RGBA,GL_UNSIGNED_BYTE,sfc2->pixels);
        SDL_FreeSurface(sfc2);
        setTexture(0);
        
        if (!glIsTexture(tname)) {
            output_debug_message("createTexture: %i is not a texture!! Error %i\n", tname, glGetError());
            exit(1);
        }
        
        //        output_debug_message("createTexture: %i [%i,%i] -> [%i,%i]\n",tname,sfc->w,sfc->h,szx,szy);
    } else {
        return 0;
    } // if
    
    return tname;
} // createTexture



bool drawQuad_called = false;
GLuint drawQuad_VertexArrayID = 0;
GLuint drawQuad_Vertexbuffer = 0;
GLfloat drawQuad_buffer_data[] = {
    0,0,0, 0,0,
    0,0,0, 1,0,
    0,0,0, 1,1,
    0,0,0, 0,1,
};

void clearDrawQuadData()
{
    if (drawQuad_called) {
        glDeleteBuffers(1,&drawQuad_Vertexbuffer);
#ifdef __APPLE__
        glDeleteVertexArraysAPPLE(1,&drawQuad_VertexArrayID);
#else
        glDeleteVertexArrays(1,&drawQuad_VertexArrayID);
#endif
        drawQuad_called = false;
    }
}


void drawQuad(float x, float y, float w, float h, float r, float g, float b, float a)
{
    glUniform4f(inColorID, r, g, b, a);
    drawQuad_buffer_data[0] = x;
    drawQuad_buffer_data[1] = y;
    
    drawQuad_buffer_data[5] = x+w;
    drawQuad_buffer_data[6] = y;
    
    drawQuad_buffer_data[10] = x+w;
    drawQuad_buffer_data[11] = y+h;
    
    drawQuad_buffer_data[15] = x;
    drawQuad_buffer_data[16] = y+h;
    if (!drawQuad_called) {
        glGenVertexArrays(1, &drawQuad_VertexArrayID);
        glBindVertexArray(drawQuad_VertexArrayID);
        glGenBuffers(1, &drawQuad_Vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, drawQuad_Vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*5*4, drawQuad_buffer_data, GL_DYNAMIC_DRAW);
        drawQuad_called = true;
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, drawQuad_Vertexbuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat)*5*4, drawQuad_buffer_data);
    }
    
    glm::mat4 tmp = glm::mat4(1.0f);
    glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &tmp[0][0]);
    int loc1 = glGetAttribLocation(programID, "vPosition");
    int loc2 = glGetAttribLocation(programID, "UV");
    glUniform1i(useTextureID, 0);
    glBindVertexArray(drawQuad_VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, drawQuad_Vertexbuffer);
    glEnableVertexAttribArray(loc1);
    glEnableVertexAttribArray(loc2);
    glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)0);
    glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (GLvoid*)(sizeof(float)*3));
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(loc1);
    glDisableVertexAttribArray(loc2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);    
}

