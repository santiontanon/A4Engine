//
//  A4TextBubble.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 3/13/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//
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

#include "SDL.h"
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
#include "SDL_mixer.h"
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

#include <vector>
#include "sound.h"
#include "keyboardstate.h"
#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
#include "SFXManager.h"
#include "BitmapFont.h"
#include "Binterface.h"
#include "XMLparser.h"
#include "Animation.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4Object.h"
#include "A4Game.h"

#include "A4TextBubble.h"

A4TextBubble::A4TextBubble(const char *text, int maxWidth, BitmapFont *font)
{
    char *buffer = new char[maxWidth+1];
    int j = 0, last_space = 0;
    int longestLine = 0;
    
    for(int i=0;text[i]!=0;i++) {
        buffer[j++] = text[i];
        if (text[i]==' ') last_space = i;
        if (j>=maxWidth) {
            if (last_space==0) {
                // a single word doesn't fit, just split it!
                buffer[j++] = 0;
                char *tmp = new char[j];
                strcpy(tmp,buffer);
                m_lines.push_back(tmp);
                if (strlen(tmp)>longestLine) longestLine = (int)strlen(tmp);
                j = 0;
            } else {
                int backspaces = i - last_space;
                j-=backspaces;
                i-=backspaces;
                buffer[j++] = 0;
                char *tmp = new char[j];
                strcpy(tmp,buffer);
                m_lines.push_back(tmp);
                if (strlen(tmp)>longestLine) longestLine = (int)strlen(tmp);
                j = 0;
            }
        }
    }
    if (j!=0) {
        buffer[j++] = 0;
        char *tmp = new char[j];
        strcpy(tmp,buffer);
        m_lines.push_back(tmp);
        if (strlen(tmp)>longestLine) longestLine = (int)strlen(tmp);
//        j = 0;
    }
    
    m_font = font;

    dx = 16 + longestLine * m_font->getWidth();
    dy = int(16 + m_lines.size() * (m_font->getHeight() + 2));
    textureSamplerLocation = vPositionLocation = UVLocation = -1;
    nVertices = 0;
    old_diffx = old_diffy = 0;
	delete []buffer;
}


A4TextBubble::~A4TextBubble()
{
    for(char *l:m_lines) delete l;
    m_lines.clear();
    m_font = 0;
    
    if (nVertices!=0) {
        glDeleteBuffers(1,&vertexbuffer);
#ifdef __APPLE__
        glDeleteVertexArraysAPPLE(1,&VertexArrayID);
#else
        glDeleteVertexArrays(1,&VertexArrayID);
#endif
    }
}


void A4TextBubble::draw(int x, int y, int pointx, int pointy, bool angry, float alpha)
{
    // create bubble OpenGL buffers (if necessary):
    if (nVertices==0 || old_diffx!=(pointx-x) || old_diffy!=(pointy-y)) {
        if (nVertices!=0) {
            glDeleteBuffers(1,&vertexbuffer);
#ifdef __APPLE__
            glDeleteVertexArraysAPPLE(1,&VertexArrayID);
#else
            glDeleteVertexArrays(1,&VertexArrayID);
#endif
        }
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
        GLfloat *g_vertex_buffer_data = A4TextBubble::getBubbleVertices(0,0,dx,dy,pointx-x,pointy-y,nVertices);
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*nVertices*3, g_vertex_buffer_data, GL_STATIC_DRAW);
        old_diffx = pointx-x;
        old_diffy = pointy-y;
        delete []g_vertex_buffer_data;
    }
    
    // draw bubble:
    {
        if (angry) {
            glUniform4f(inColorID, 0.66f, 0, 0, 0.66f*alpha);
        } else {
            glUniform4f(inColorID, 0, 0, 0, 0.66f*alpha);
        }
        glm::mat4 tmp = glm::mat4(1.0f);
        tmp = glm::translate(tmp, glm::vec3(x,y,0));
        glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &tmp[0][0]);
        if (textureSamplerLocation==-1) {
            textureSamplerLocation = glGetUniformLocation(programID,"textureSampler");
            vPositionLocation = glGetAttribLocation(programID, "vPosition");
            UVLocation = glGetAttribLocation(programID, "UV");
        }
        glUniform1i(useTextureID, 0);
        glBindVertexArray(VertexArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glEnableVertexAttribArray(vPositionLocation);
        glEnableVertexAttribArray(UVLocation);
        glVertexAttribPointer(vPositionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
        glVertexAttribPointer(UVLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);
        glDisableVertexAttribArray(vPositionLocation);
        glDisableVertexAttribArray(UVLocation);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    x+=8;
    y+=8 + m_font->getHeight();
    for(char *line:m_lines) {
        BInterface::print_left(line,m_font,x,y,1,1,1,alpha);
        y+= m_font->getHeight()+2;
    }
}


GLfloat *A4TextBubble::getBubbleVertices(int x, int y, int width, int height, int pointx, int pointy, int &npoints) {
    GLfloat *points = new GLfloat[9*3];   // at most there can be 9 points
    int pointsType[9];
    // make sure the point s not inside of the bubble:
    if (pointx>x && pointx<x+width &&
        pointy>y && pointy<y+height) {
        // just draw a rectangle:
        points[0] = x;          points[1] = y;          points[2] = 0;
        points[3] = x+width;    points[4] = y;          points[5] = 0;
        points[6] = x+width;    points[7] = y+height;   points[8] = 0;
        points[9] = x;          points[10] = y+height;  points[11] = 0;
        npoints = 4;
        return points;
    }

    double cx = x + width/2;
    double cy = y + height/2;
    double vx = cx - pointx;
    double vy = cy - pointy;
    double n = sqrt(vx*vx + vy*vy);
    vx /= n;
    vy /= n;
    double wx = -vy;
    double wy = vx;
    double triangleWidth = floor(width-50)/10;
    if (triangleWidth<0) triangleWidth = 0;
    if (triangleWidth>10) triangleWidth = 10;
    triangleWidth+=10;
    
    // 1) add the arrow tip to a list
    points[0] = pointx;
    points[1] = pointy;
    points[2] = 0;
    pointsType[0] = 0;
    npoints = 1;

    // 2) Compute the 2 collision points of the arrow with the rectangle, and add them to a list
    int tmpx, tmpy;
    if (lineRectangleCollision(pointx,pointy,floor(cx + wx*triangleWidth),floor(cy + wy*triangleWidth),
                               x,y,x+width,y+height, tmpx, tmpy)) {
        points[npoints*3] = tmpx;
        points[npoints*3+1] = tmpy;
        points[npoints*3+2] = 0;
        pointsType[npoints] = 0;
        npoints++;
    }
    
    if (lineRectangleCollision(pointx,pointy,floor(cx - wx*triangleWidth), floor(cy - wy*triangleWidth),
                               x,y,x+width,y+height, tmpx, tmpy)) {
        points[npoints*3] = tmpx;
        points[npoints*3+1] = tmpy;
        points[npoints*3+2] = 0;
        pointsType[npoints] = 0;
        npoints++;
    }
    
    // 3) Add all the 4 vertives of the rectangle:
    points[npoints*3+0] = x;          points[npoints*3+1] = y;          points[npoints*3+2] = 0;
    points[npoints*3+3] = x+width;    points[npoints*3+4] = y;          points[npoints*3+5] = 0;
    points[npoints*3+6] = x+width;    points[npoints*3+7] = y+height;   points[npoints*3+8] = 0;
    points[npoints*3+9] = x;          points[npoints*3+10] = y+height;  points[npoints*3+11] = 0;
    pointsType[npoints] = 1;
    pointsType[npoints+1] = 1;
    pointsType[npoints+2] = 1;
    pointsType[npoints+3] = 1;
    npoints+=4;
    
    // 4) sort those vertices clock-wise
    double angles[9];
    for(int idx = 0;idx<npoints;idx++) {
        angles[idx] = atan2(points[idx*3+1] - cy, points[idx*3] - cx);
    }
    {
        bool change = true;
        while(change) {
            change = false;
            for(int i = 0;i<npoints-1;i++) {
                if (angles[i]>angles[i+1]) {
                    double tmp = angles[i];
                    angles[i] = angles[i+1];
                    angles[i+1] = tmp;
                    
                    tmp = points[i*3];
                    points[i*3] = points[(i+1)*3];
                    points[(i+1)*3] = tmp;
                    
                    tmp = points[i*3+1];
                    points[i*3+1] = points[(i+1)*3+1];
                    points[(i+1)*3+1] = tmp;
                    
                    int itmp = pointsType[i];
                    pointsType[i] = pointsType[i+1];
                    pointsType[i+1] = itmp;
                    change = true;
                }
            }
        }
    }
//    output_debug_message("bubble points (After sort): %i\n", npoints);
    
    // 5) if after sorting, any point of type 1 is isolated in between points of type 0, it is removed (that means that
    //	  such point is inside of the pointing triangle):
    {
        int toDelete = -1;
        for(int i = 0;i<npoints;i++){
            int idec = i-1;
            if (idec<0) idec += npoints;
            int iinc = i+1;
            if (iinc>=npoints) iinc -= npoints;
            if (pointsType[i]==1 && pointsType[idec]==0 && pointsType[iinc]==0) toDelete = i;
        }
        if (toDelete!=-1) {
            for(int i = toDelete;i<npoints-1;i++) {
                points[i*3] = points[(i+1)*3];
                points[i*3+1] = points[(i+1)*3+1];
            }
            npoints--;
        }
    }
    
//    output_debug_message("bubble points (After deletion): %i\n", npoints);

    // add the center point, and duplicate the starting point:
    for(int i = npoints;i>0;i--) {
        points[i*3] = points[(i-1)*3];
        points[i*3+1] = points[(i-1)*3+1];
    }
    npoints++;
    points[0] = cx;
    points[1] = cy;
    points[2] = 0;
    points[npoints*3] = points[3];
    points[npoints*3+1] = points[4];
    points[npoints*3+2] = 0;
    npoints++;
/*
    output_debug_message("bubble points:\n");
    for(int i = 0;i<npoints;i++) {
        output_debug_message("  %g,%g,%g\n",points[i*3],points[i*3+1],points[i*3+2]);
    }
  */
    return points;
}


bool A4TextBubble::lineRectangleCollision(int lp1x, int lp1y, int lp2x, int lp2y,
                                          int rp1x, int rp1y, int rp2x, int rp2y,
                                          int &cx, int &cy) {
	double vx = lp2x-lp1x;
	double vy = lp2y-lp1y;
 
	// collision 1: left side
	if (lp1x<=rp1x && vx!=0) {
        double k = (rp1x - lp1x)/vx;
        int collission_y = lp1y + k*vy;
        if (collission_y>=rp1y && collission_y<rp2y) {
            cx = rp1x;
            cy = collission_y;
            return true;
        }
	}
    
	// collision 2: right side
	if (lp1x>=rp2x && vx!=0) {
        double k = (rp2x - lp1x)/vx;
        int collission_y = lp1y + k*vy;
        if (collission_y>=rp1y && collission_y<rp2y) {
            cx = rp2x;
            cy = collission_y;
            return true;
        }
    }
    
	// collision 3: top side
	if (lp1y<=rp1y && vy!=0) {
        double k = (rp1y - lp1y)/vy;
        int collission_x = lp1x + k*vx;
        if (collission_x>=rp1x && collission_x<rp2x) {
            cx = collission_x;
            cy = rp1y;
            return true;
        }
    }
    
	// collision 4: bottom side
    if (lp1y>=rp2y && vy!=0) {
        double k = (rp2y - lp1y)/vy;
        int collission_x = lp1x + k*vx;
        if (collission_x>=rp1x && collission_x<rp2x) {
            cx = collission_x;
            cy = rp2y;
            return true;
        }
	}
	return false;
 }
