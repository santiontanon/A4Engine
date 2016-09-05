//
//  A4TextBubble.h
//  A4Engine
//
//  Created by Santiago Ontanon on 3/13/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__A4TextBubble__
#define __A4Engine__A4TextBubble__

class A4TextBubble {
public:
    A4TextBubble(const char *text, int maxWidth, BitmapFont *font);
    ~A4TextBubble();
    
    void draw(int x, int y,int pointx, int pointy, bool angry, float alpha);
    int getWidth() {return dx;}
    int getHeight() {return dy;}
    
    GLfloat *getBubbleVertices(int x, int y, int width, int height, int pointx, int pointy, int &npoints);
    bool lineRectangleCollision(int lp1x, int lp1y, int lp2x, int lp2y,
                                int rp1x, int rp1y, int rp2x, int ry2y,
                                int &cx, int &cy);
    
    
private:
    std::vector<char *> m_lines;
    BitmapFont *m_font;
    
    int dx,dy;
    int nVertices;
    int old_diffx,old_diffy;
    GLuint VertexArrayID;
    GLuint vertexbuffer;
    int textureSamplerLocation, vPositionLocation, UVLocation;
};

#endif /* defined(__A4Engine__A4TextBubble__) */
