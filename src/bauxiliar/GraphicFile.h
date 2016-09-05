//
//  GraphicFile.h
//  A4Engine
//
//  Created by Santiago Ontanon on 6/8/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__GraphicFile__
#define __A4Engine__GraphicFile__

class GraphicFile {
public:
    GraphicFile(char *name, int tile_dx, int tile_dy, const char *m_path, class GLTManager *GLTM);
    ~GraphicFile();
    class GLTile *getTile(int n);
    int getTilesPerRow();
    int getNTiles();
    
    void clearOpenGLState();
    
    char *m_name, *m_full_name;
    const char *m_path;
    int m_n_tiles, m_tiles_per_row;
    int m_tile_dx, m_tile_dy;
    GLTile **m_tiles;
    int *m_tileTypes;
    int *m_tileSeeThrough;
    int *m_tileCanDig;
    GLTManager *m_GLTM;
};

#endif /* defined(__A4Engine__GraphicFile__) */
