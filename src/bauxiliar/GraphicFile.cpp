//
//  GraphicFile.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 6/8/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include "math.h"

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

#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
#include "GraphicFile.h"


GraphicFile::GraphicFile(char *name, int tile_dx, int tile_dy, const char *path, GLTManager *GLTM) {
    m_tile_dx = tile_dx;
    m_tile_dy = tile_dy;
    m_name = new char[strlen(name)+1];
    strcpy(m_name,name);
    m_path = path;
    m_full_name = new char[strlen(m_name) + strlen(m_path)+2];
    sprintf(m_full_name,"%s/%s",m_path,m_name);
    
    m_GLTM = GLTM;
    SDL_Surface *img = m_GLTM->getImage(m_full_name);
    if (img==0) {
        output_debug_message("Graphic file '%s' cannot be found!",m_full_name);
        exit(1);
    }
    m_tiles_per_row = img->w/m_tile_dx;
    m_n_tiles = m_tiles_per_row*(img->h/m_tile_dy);
    
    m_tiles = new GLTile *[m_n_tiles];
    m_tileTypes = new int[m_n_tiles];
    m_tileSeeThrough = new int[m_n_tiles];
    m_tileCanDig = new int[m_n_tiles];
    for(int i = 0;i<m_n_tiles;i++) {
        m_tiles[i] = 0;
        m_tileTypes[i] = 0;
        m_tileSeeThrough[i] = 0;
        m_tileCanDig[i] = 0;
    }
    output_debug_message("GraphicFile created %i tiles (%i x %i)\n", m_n_tiles,m_tiles_per_row,img->h/m_tile_dy);
}


GraphicFile::~GraphicFile() {
    delete m_name;
    m_name = 0;
    delete m_tiles;
    m_tiles = 0;
    delete m_tileTypes;
    m_tileTypes = 0;
    delete m_tileSeeThrough;
    m_tileSeeThrough = 0;
    delete m_tileCanDig;
    m_tileCanDig = 0;
    m_GLTM = 0;
    m_path = 0;
    if (m_full_name!=0) delete m_full_name;
    m_full_name = 0;
}

int GraphicFile::getNTiles() {
    return m_n_tiles;
}


int GraphicFile::getTilesPerRow()
{
    return m_tiles_per_row;
}



GLTile *GraphicFile::getTile(int n)
{
    if (n<0 || n>=m_n_tiles) {
        output_debug_message("Requesting tile outside of range: %i\n",n);
        exit(1);
    }
    if (m_tiles[n]!=0) return m_tiles[n];
    m_tiles_per_row = getTilesPerRow();
    //	SDL_Surface *img = m_GLTM->getImage(m_full_name);
    int x = (n%m_tiles_per_row)*m_tile_dx;
    int y = (n/m_tiles_per_row)*m_tile_dy;
    //	output_debug_message("%i -> %i,%i - %i,%i\n",n,x,y,m_tile_dx,m_tile_dy);
    m_tiles[n] = m_GLTM->get(m_full_name,x,y,m_tile_dx,m_tile_dy);
    return m_tiles[n];
}


void GraphicFile::clearOpenGLState()
{
    output_debug_message("GraphicFile::clearOpenGLState...\n");
    for(int i = 0;i<m_n_tiles;i++) m_tiles[i] = 0;
    output_debug_message("GraphicFile::clearOpenGLState... OK\n");
}

