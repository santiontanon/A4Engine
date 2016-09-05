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

#include "A4Character.h"
#include "A4AICharacter.h"


void A4Game::drawAIDebugger(int SCREEN_X,int SCREEN_Y, class A4AICharacter *focus)
{
    // draw it on the left of the screeen:
    Ontology *o = getOntology();
//    Sort *showOntologyBelow = 0;
    Sort *showOntologyBelow = o->getSort("Character");
    float transparency = 0.66;
    int x = 8;
    int y = 48;
    int width = 464 + (showOntologyBelow==0 ? 0:160);
    int height = 360;
    int spacing = 8;

    A4AI *AI = focus->getAI();
    AIMemory *memory = AI->getMemory();

    drawQuad(x,y,width,height,0,0,0,transparency);
    char buffer[256];
    y+=spacing;
    sprintf(buffer,"Focus (%i): '%s': %s", focus->getID(), focus->getName()->get(), focus->getSort()->getName()->get());
    BInterface::print_left(buffer,m_font4,x+4,y+4); y+=spacing;
    sprintf(buffer,"  (x,y): %i,%i  Sight: %i", focus->getX(), focus->getY(),AI->getSightRadius());
    BInterface::print_left(buffer,m_font4,x+4,y+4); y+=spacing;

    int shortx = x;
    int shorty = y;
    BInterface::print_left("- SHORT TERM MEMORY -------",m_font4,shortx+4,shorty+4); shorty+=spacing;
    for(WME *wme:*memory->getShortTermWMEs()) {
        wme->toString(buffer);
        BInterface::print_left(buffer,m_font4,shortx+4,shorty+4); shorty+=spacing;
    }

    int longx = x + 232;
    int longy = y;
    BInterface::print_left("- LONG TERM MEMORY -------",m_font4,longx+4,longy+4); longy+=spacing;
    for(WME *wme:*memory->getLongTermWMEs()) {
        wme->toString(buffer);
        BInterface::print_left(buffer,m_font4,longx+4,longy+4); longy+=spacing;
    }

    if (showOntologyBelow!=0) {
        int ox = x + 464;
        int oy = y;
        BInterface::print_left("- ONTOLOGY -------",m_font4,ox+4,oy+4); oy+=spacing;
        std::vector<Sort *> *allSorts = o->getAllSorts();
        for(Sort *s:*allSorts) {
            if (s->is_a(showOntologyBelow)) {
                char buffer[256];
                sprintf(buffer, "%s: [ ",s->getName()->get());
                for(Sort *s:*(s->getParents())) sprintf(buffer+strlen(buffer),"%s ",s->getName()->get());
                sprintf(buffer+strlen(buffer), "]");
                BInterface::print_left(buffer,m_font4,ox+4,oy+4); oy+=spacing;
            }
        }
        delete allSorts;
    }

    // show navigation buffer:
    A4Map *map = focus->getMap();
    AI->updateAllObjectsCache();
    AI->updateNavigationPerceptionBuffer(true);
    int nbx = x;
    int nby = y+180;
    int grid = 8;
//    int grid = 16;
    BInterface::print_left("- NAVIGATION -------",m_font4,nbx+4,nby+4); nby+=spacing;
    sprintf(buffer,"Buffer(x,y): %i,%i",AI->m_navigationBuffer_x,AI->m_navigationBuffer_y);
    BInterface::print_left(buffer,m_font4,nbx+4,nby+4); nby+=spacing;
    sprintf(buffer,"Targets: ");
    for(int i = 0;i<AI->m_pathfinding_n_targets_old;i++) {
        A4Object *target = AI->m_pathfinding_targets[i].m_target;
        if (!contains(target)) target = 0;
        sprintf(buffer + strlen(buffer),"%s(%i) ",(AI->m_pathfinding_targets[i].m_flee ? "flee":"goto"),
                                                   (target==0 ? -1:target->getID()));
    }
    BInterface::print_left(buffer,m_font4,nbx+4,nby+4); nby+=spacing;    
    sprintf(buffer,"Current Target: %i,%i (%i), pixel diff: %i,%i",AI->m_pathfinding_result_x, AI->m_pathfinding_result_y, AI->m_pathfinding_result_priority,
                                                                   AI->m_pathfinding_result_offset_x, AI->m_pathfinding_result_offset_y);
    BInterface::print_left(buffer,m_font4,nbx+4,nby+4); nby+=spacing;
    sprintf(buffer,"Cycles since pathfinding: %i (took %i)",(AI->getCycle() - AI->m_pathFinding_lastUpdated), AI->m_pathFinding_iterations);
    BInterface::print_left(buffer,m_font4,nbx+4,nby+4); nby+=spacing;
    nbx+=grid;
    nby+=grid;
    for(int i = 0;i<AI->m_navigationBuffer_size;i++) {
        for(int j = 0;j<AI->m_navigationBuffer_size;j++) {
            int type = AI->m_navigationBuffer[j + i*AI->m_navigationBuffer_size];
            if ( type == NAVIGATION_BUFFER_WALKABLE) {
                drawQuad(nbx+j*grid,nby+i*grid,grid,grid,0.25,0.25,0.25,1);
            } else if (type == NAVIGATION_BUFFER_NOT_WALKABLE) {
                drawQuad(nbx+j*grid,nby+i*grid,grid,grid,1,1,1,1);
            } else if (type == NAVIGATION_BUFFER_BRIDGE) {
                drawQuad(nbx+j*grid,nby+i*grid,grid,grid,1,1,0.25,1);
            }
        }
    }
    for(int i = 0;i<AI->m_pathfinding_n_targets_old;i++){
        int t_x0 = AI->m_pathfinding_targets[i].m_x0 - AI->m_navigationBuffer_x*map->getTileWidth();
        int t_y0 = AI->m_pathfinding_targets[i].m_y0 - AI->m_navigationBuffer_y*map->getTileHeight();
        int t_x1 = AI->m_pathfinding_targets[i].m_x1 - AI->m_navigationBuffer_x*map->getTileWidth();
        int t_y1 = AI->m_pathfinding_targets[i].m_y1 - AI->m_navigationBuffer_y*map->getTileHeight();
        t_x0 = (t_x0*grid)/map->getTileWidth();
        t_y0 = (t_y0*grid)/map->getTileHeight();
        t_x1 = (t_x1*grid)/map->getTileWidth();
        t_y1 = (t_y1*grid)/map->getTileHeight();
        if (t_x0<-1) t_x0 = -1;
        if (t_x1<-1) t_x1 = -1;
        if (t_y0<-1) t_y0 = -1;
        if (t_y1<-1) t_y1 = -1;
        if (t_x0>AI->m_navigationBuffer_size*grid) t_x0 = AI->m_navigationBuffer_size*grid;
        if (t_y0>AI->m_navigationBuffer_size*grid) t_y0 = AI->m_navigationBuffer_size*grid;
        if (t_x1>AI->m_navigationBuffer_size*grid) t_x1 = AI->m_navigationBuffer_size*grid;
        if (t_y1>AI->m_navigationBuffer_size*grid) t_y1 = AI->m_navigationBuffer_size*grid;
        if (t_x1<=t_x0) t_x1 = t_x0+1;
        if (t_y1<=t_y0) t_y1 = t_y0+1;
        if (AI->m_pathfinding_targets[i].m_flee) {
            drawQuad(nbx+t_x0,nby+t_y0,t_x1-t_x0,t_y1-t_y0,1.00,0,0,0.5);
        } else {
            drawQuad(nbx+t_x0,nby+t_y0,t_x1-t_x0,t_y1-t_y0,0,1.0,0,0.5);
        }
    }
    // self:
    {
        int self_x = ((focus->getX() + focus->getPixelWidth()/2)/map->getTileWidth()) - AI->m_navigationBuffer_x;
        int self_y = ((focus->getY() + focus->getPixelHeight()/2)/map->getTileHeight()) - AI->m_navigationBuffer_y;
        drawQuad(nbx+self_x*grid+2,nby+self_y*grid+2,grid-4,grid-4,0,0,1,1);
    }
    if (grid>=16) {
        for(int i = 0;i<AI->m_navigationBuffer_size;i++) {
            for(int j = 0;j<AI->m_navigationBuffer_size;j++) {
                int score = 0, priority = 0;
                AI->pathFindingScore(j+AI->m_navigationBuffer_x,i+AI->m_navigationBuffer_y,score,priority, focus);
                sprintf(buffer,"%i",score);
                BInterface::print_left(buffer,m_font4,nbx+j*grid,nby+i*grid+8,1,0,0,0.5);
            }
        }
    }
    
    
    int cx = x + 232;
    int cy = y + 180;
    BInterface::print_left("- AI CACHE -------",m_font4,cx+4,cy+4); cy+=spacing;
    sprintf(buffer,"Friendly: ");
    BInterface::print_left(buffer,m_font4,cx+4,cy+4); cy+=spacing;
    for(int i = 0;i<AI->m_n_friendly;i++) {
        AI->m_friendly_cache[i]->toString(buffer);
        BInterface::print_left(buffer,m_font4,cx+12,cy+4); cy+=spacing;
    }
    sprintf(buffer,"Unfriendly: ");
    BInterface::print_left(buffer,m_font4,cx+4,cy+4); cy+=spacing;
    for(int i = 0;i<AI->m_n_unfriendly;i++) {
        AI->m_unfriendly_cache[i]->toString(buffer);
        BInterface::print_left(buffer,m_font4,cx+12,cy+4); cy+=spacing;
    }
    
}
