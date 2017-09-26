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
#include "A4Object.h"
#include "A4Item.h"
#include "A4ObjectFactory.h"
#include "A4Game.h"
#include "A4Character.h"
#include "A4Trigger.h"
#include "A4Map.h"
#include "A4Door.h"

#include "A4Vehicle.h"


A4MapLayer::A4MapLayer(int dx, int dy, int n_gfs, GraphicFile **gfs) {
    m_dx = dx;
    m_dy = dy;
    m_n_graphicFiles = n_gfs;
    m_gfs = new GraphicFile *[m_n_graphicFiles];
    m_gfs_startTile = new int[m_n_graphicFiles];
    int startTile = 0;
    for(int i = 0;i<m_n_graphicFiles;i++) {
        m_gfs[i] = gfs[i];
        m_gfs_startTile[i] = startTile;
        startTile += gfs[i]->getNTiles();
    }
    m_tiles = new int[m_dx*m_dy];
//    m_seeThrough = new int[m_dx*m_dy];
    m_canDig = new int[m_dx*m_dy];
    for(int i = 0;i<m_dx*m_dy;i++) {
        m_tiles[i] = -1;
//        m_seeThrough[i] = m_canDig[i] = 0;
    }
    m_gl_tiles = 0;

    m_tile_dx = m_gfs[0]->m_tile_dx;
    m_tile_dy = m_gfs[0]->m_tile_dy;
}


A4MapLayer::~A4MapLayer() {
    delete m_tiles;
//    delete m_seeThrough;
    delete m_canDig;
    if (m_gl_tiles!=0) delete m_gl_tiles;
    m_gl_tiles = 0;
    for(A4Object *o:m_objects) delete o;
    m_objects.clear();
    delete m_gfs;
    m_gfs = 0;
    delete m_gfs_startTile;
    m_gfs_startTile = 0;
}


void A4MapLayer::cycle(A4Game *game)
{
    std::vector<A4Object *> toDelete;

    for(A4Object *o:m_objects) {
        if (!o->cycle(game)) {
            game->requestDeletion(o);
            toDelete.push_back(o);
        }
    }

    for(A4Object *o:toDelete) m_objects.remove(o);
    toDelete.clear();
}


void A4MapLayer::draw(int offsetx, int offsety, float zoom, int SCREEN_X, int SCREEN_Y, A4Game *game)
{
    if (m_gl_tiles==0) {
        m_gl_tiles = new GLTile *[m_dx*m_dy];
        for(int i = 0;i<m_dx*m_dy;i++) {
            if (m_tiles[i]>=0) {
                for(int j = 0;j<m_n_graphicFiles;j++) {
                    if (j<m_n_graphicFiles-1 && m_tiles[i]>=m_gfs_startTile[j+1]) continue;
                    m_gl_tiles[i] = m_gfs[j]->getTile(m_tiles[i]-m_gfs_startTile[j]);
                    break;
                }
            } else {
                m_gl_tiles[i] = 0;
            }
        }
    }

    //	output_debug_message("A4MapLayer::draw\n");

    float y = -offsety;
    float ZSY = SCREEN_Y/zoom;
    float ZSX = SCREEN_X/zoom;
    for(int i = 0;i<m_dy && y<ZSY;i++, y+=m_tile_dy) {
        if (y+m_tile_dy<0) continue;
        int offset = i*m_dx;
        float x = -offsetx;
        for(int j = 0;j<m_dx && x<ZSX;j++,x+=m_tile_dx,offset++) {
            if (x+m_tile_dx<0) continue;
            if (m_gl_tiles[offset]!=0) {
                m_gl_tiles[offset]->draw(x*zoom,y*zoom,0,0,zoom);
            }
        }
    }

    for(A4Object *o:m_objects) {
        if (o->getBurrowed()) continue;
        int tx = o->getX()/m_tile_dx;
        int ty = o->getY()/m_tile_dy;
        bool draw = false;
        for(int i = 0;i<o->getPixelHeight()/m_tile_dy && !draw;i++) {
            for(int j = 0;j<o->getPixelWidth()/m_tile_dx && !draw;j++) {
                if (tx+j>=0 && tx+j<m_dx &&
                    ty+i>=0 && ty+i<m_dy) {
                    draw = true;
                }
            }
        }
        if (draw) o->draw(-offsetx,-offsety,zoom, game);
    }
}


void A4MapLayer::draw(int offsetx, int offsety, float zoom, int SCREEN_X, int SCREEN_Y, int *visibility, int visibilityRegion, A4Game *game)
{
    if (m_gl_tiles==0) {
        m_gl_tiles = new GLTile *[m_dx*m_dy];
        for(int i = 0;i<m_dx*m_dy;i++) {
            if (m_tiles[i]>=0) {
                for(int j = 0;j<m_n_graphicFiles;j++) {
                    if (j<m_n_graphicFiles-1 && m_tiles[i]>=m_gfs_startTile[j+1]) continue;
                    m_gl_tiles[i] = m_gfs[j]->getTile(m_tiles[i]-m_gfs_startTile[j]);
                    break;
                }
            } else {
                m_gl_tiles[i] = 0;
            }
        }
    }
    
    //	output_debug_message("A4MapLayer::draw\n");
    
    float y = -offsety;
    float ZSY = SCREEN_Y/zoom;
    float ZSX = SCREEN_X/zoom;
    for(int i = 0;i<m_dy && y<ZSY;i++, y+=m_tile_dy) {
        if (y+m_tile_dy<0) continue;
        int offset = i*m_dx;
        float x = -offsetx;
        for(int j = 0;j<m_dx && x<ZSX;j++,x+=m_tile_dx,offset++) {
            if (x+m_tile_dx<0) continue;
            if (m_gl_tiles[offset]!=0) {
                if (visibility[offset]==visibilityRegion) {
                    m_gl_tiles[offset]->draw(x*zoom,y*zoom,0,0,zoom);
                } else if (visibility[offset]==0 &&
                           ((j>0 && visibility[offset-1]==visibilityRegion) ||
                            (j<m_dx-1 && visibility[offset+1]==visibilityRegion) ||
                            (i>0 && visibility[offset-m_dx]==visibilityRegion) ||
                            (i<m_dy-1 && visibility[offset+m_dx]==visibilityRegion) ||

                            (j>0 && i>0 && visibility[offset-(1+m_dx)]==visibilityRegion) ||
                            (j>0 && i<m_dy-1 && visibility[offset+m_dx-1]==visibilityRegion) ||
                            (j<m_dx-1 && i>0 && visibility[offset+1-m_dx]==visibilityRegion) ||
                            (j<m_dx-1 && i<m_dy-1 && visibility[offset+1+m_dx]==visibilityRegion)
                           )) {
                    m_gl_tiles[offset]->draw(x*zoom,y*zoom,0,0,zoom);
                }
            }
        }
    }
    
    int xx,yy,offset;
    for(A4Object *o:m_objects) {
        if (o->getBurrowed()) continue;
        int tx = o->getX()/m_tile_dx;
        int ty = o->getY()/m_tile_dy;
        bool draw = false;
        for(int i = 0;i<o->getPixelHeight()/m_tile_dy && !draw;i++) {
            for(int j = 0;j<o->getPixelWidth()/m_tile_dx && !draw;j++) {
                xx = tx+j;
                yy = ty+i;
                offset = xx + yy*m_dx;
                if (xx>=0 && xx<m_dx &&
                    yy>=0 && yy<m_dy) {
                    if (visibility[offset]==visibilityRegion) {
                        draw = true;
                    } else if (visibility[offset]==0 &&
                               ((xx>0 && visibility[offset-1]==visibilityRegion) ||
                                (xx<m_dx-1 && visibility[offset+1]==visibilityRegion) ||
                                (yy>0 && visibility[offset-m_dx]==visibilityRegion) ||
                                (yy<m_dy-1 && visibility[offset+m_dx]==visibilityRegion) ||
                                
                                (xx>0 && yy>0 && visibility[offset-(1+m_dx)]==visibilityRegion) ||
                                (xx>0 && yy<m_dy-1 && visibility[offset+m_dx-1]==visibilityRegion) ||
                                (xx<m_dx-1 && yy>0 && visibility[offset+1-m_dx]==visibilityRegion) ||
                                (xx<m_dx-1 && yy<m_dy-1 && visibility[offset+1+m_dx]==visibilityRegion)
                               )) {
                        draw = true;
                    }
                }
            }
        }
        if (draw) o->draw(-offsetx,-offsety,zoom, game);
    }
}


void A4MapLayer::drawSpeechBubbles(int offsetx, int offsety, float zoom, int SCREEN_X, int SCREEN_Y, A4Game *game)
{
    for(A4Object *o:m_objects) {
        if (o->getBurrowed()) continue;
        if (!o->isCharacter()) continue;
        int tx = o->getX()/m_tile_dx;
        int ty = o->getY()/m_tile_dy;
        bool draw = false;
        for(int i = 0;i<o->getPixelHeight()/m_tile_dy && !draw;i++) {
            for(int j = 0;j<o->getPixelWidth()/m_tile_dx && !draw;j++) {
                if (tx+j>=0 && tx+j<m_dx &&
                    ty+i>=0 && ty+i<m_dy) {
                    draw = true;
                }
            }
        }
        if (draw) ((A4Character *)o)->drawSpeechBubbles(-offsetx,-offsety,zoom, game);
    }
}


void A4MapLayer::drawSpeechBubbles(int offsetx, int offsety, float zoom, int SCREEN_X, int SCREEN_Y, int *visibility, int visibilityRegion, A4Game *game)
{
    int xx,yy,offset;
    for(A4Object *o:m_objects) {
        if (o->getBurrowed()) continue;
        if (!o->isCharacter()) continue;
        int tx = o->getX()/m_tile_dx;
        int ty = o->getY()/m_tile_dy;
        bool draw = false;
        for(int i = 0;i<o->getPixelHeight()/m_tile_dy && !draw;i++) {
            for(int j = 0;j<o->getPixelWidth()/m_tile_dx && !draw;j++) {
                xx = tx+j;
                yy = ty+i;
                offset = xx + yy*m_dx;
                if (xx>=0 && xx<m_dx &&
                    yy>=0 && yy<m_dy) {
                    if (visibility[offset]==visibilityRegion) {
                        draw = true;
                    } else if (visibility[offset]==0 ||
                               (xx>0 && visibility[offset-1]==visibilityRegion) ||
                               (xx<m_dx-1 && visibility[offset+1]==visibilityRegion) ||
                               (yy>0 && visibility[offset-m_dx]==visibilityRegion) ||
                               (yy<m_dy-1 && visibility[offset+m_dx]==visibilityRegion)) {
                        draw = true;
                    }
                }
            }
        }
        if (draw) ((A4Character *)o)->drawSpeechBubbles(-offsetx,-offsety,zoom, game);
    }
}


void A4MapLayer::clearOpenGLState()
{
    if (m_gl_tiles!=0) delete m_gl_tiles;
    m_gl_tiles = 0;
    for(A4Object *o:m_objects) o->clearOpenGLState();
}


void A4MapLayer::removeObject(A4Object *o)
{
    m_objects.remove(o);
}


void A4MapLayer::addObject(A4Object *o)
{
    m_objects.push_back(o);
}


bool A4MapLayer::contains(A4Object *o)
{
    for(A4Object *o2:m_objects) if (o==o2) return true;
    return false;
}


void A4MapLayer::objectRemoved(A4Object *o)
{
    for(A4Object *o2:m_objects)
        o2->objectRemoved(o);
}



bool A4MapLayer::walkableOnlyBackground(int x, int y, int dx, int dy, A4Object *subject)
{
    if (x<0 || y<0) return false;
    int tile_x = x/m_tile_dx;
    int tile_y = y/m_tile_dy;
    int tile_x2 = (x+dx-1)/m_tile_dx;
    int tile_y2 = (y+dy-1)/m_tile_dy;
    int tile;
    int type;
    //	if (tile_x2==tile_x) tile_x2++;
    //	if (tile_y2==tile_y) tile_y2++;
    for(int i = tile_y;i<=tile_y2;i++) {
        if (i>=m_dy) return false;
        for(int j = tile_x;j<=tile_x2;j++) {
            if (j>=m_dx) return false;
            tile = m_tiles[j+i*m_dx];
            if (tile>=0) {
                for(int k = 0;k<m_n_graphicFiles;k++) {
                    if (k<m_n_graphicFiles-1 && tile>=m_gfs_startTile[k+1]) continue;
                    type = m_gfs[k]->m_tileTypes[tile-m_gfs_startTile[k]];
                    if (type==A4_TILE_WALL ||
                        type==A4_TILE_TREE ||
                        type==A4_TILE_CHOPPABLE_TREE) return false;
                    if (!subject->getCanWalk() && type == A4_TILE_WALKABLE) return false;
                    if (!subject->getCanSwim() && type == A4_TILE_WATER) return false;
                    break;
                }
            }
        }
    }
    
    return true;
}


bool A4MapLayer::walkableOnlyObjects(int x, int y, int dx, int dy, A4Object *subject)
{
    for(A4Object *o:m_objects) {
        if (o!=subject) {
            if (subject->isCharacter() && o->isVehicle()) {
                // characters can always walk onto empty vehicles:
                if (((A4Vehicle *)o)->isEmpty()) continue;
            }
            if (!o->isWalkable()) {
                if (o->collision(x,y,dx,dy)) return false;
            }
        }
    }
    
    return true;
}


bool A4MapLayer::walkable(int x, int y, int dx, int dy, A4Object *subject)
{
    if (!walkableOnlyBackground(x,y,dx,dy,subject)) return false;
    return walkableOnlyObjects(x,y, dx, dy, subject);
}


bool A4MapLayer::seeThrough(int tilex, int tiley)
{
    int tile = m_tiles[tilex+tiley*m_dx];
    if (tile>=0) {
        for(int k = 0;k<m_n_graphicFiles;k++) {
            if (k<m_n_graphicFiles-1 && tile>=m_gfs_startTile[k+1]) continue;
            if (m_gfs[k]->m_tileSeeThrough[tile-m_gfs_startTile[k]]!=0) return false;
            break;
        }
    }

    for(A4Object *o:m_objects) {
        if (o->getX()<=tilex*m_tile_dx && o->getX()+o->getPixelWidth()>=(tilex+1)*m_tile_dx &&
            o->getY()<=tiley*m_tile_dy && o->getY()+o->getPixelHeight()>=(tiley+1)*m_tile_dy) {
            if (!o->seeThrough()) return false;
        }
    }

    return true;
}


A4Object *A4MapLayer::getTakeableObject(int x, int y, int dx, int dy)
{
    for(A4Object *o:m_objects) {
        //		output_debug_message("object: %s - %i,%i (%s)\n", o->getName()->get(), o->getX(), o->getY(), (o->getTakeable() ? "true":"false"));
        if (o->getTakeable() && !o->getBurrowed() && o->collision(x,y,dx,dy)) return o;
    }
    return 0;
}


A4Object *A4MapLayer::getBurrowedObject(int x, int y, int dx, int dy)
{
    for(A4Object *o:m_objects) {
        if (o->getBurrowed() && o->collision(x,y,dx,dy)) return o;
    }
    return 0;
}


A4Object *A4MapLayer::getUsableObject(int x, int y, int dx, int dy)
{
    for(A4Object *o:m_objects) {
        if (o->getUsable() && !o->getBurrowed() && o->collision(x,y,dx,dy)) return o;
    }
    return 0;
}


A4Object *A4MapLayer::getVehicleObject(int x, int y, int dx, int dy)
{
    for(A4Object *o:m_objects) {
        if (o->isVehicle() && o->collision(x,y,dx,dy)) return o;
    }
    return 0;
}


void A4MapLayer::getAllObjects(int x, int y, int dx, int dy, std::vector<A4Object *> *l)
{
    for(A4Object *o:m_objects) {
        if (o->collision(x,y,dx,dy)) l->push_back(o);
    }
}


void A4MapLayer::getAllObjectCollisions(A4Object *o, int xoffs, int yoffs, std::vector<A4Object *> *l)
{
    for(A4Object *o2:m_objects) {
        //		output_debug_message("object: %s - %i,%i (%s)\n", o->getName()->get(), o->getX(), o->getY(), (o->getTakeable() ? "true":"false"));
        if (o2!=o && o->collision(xoffs, yoffs, o2)) l->push_back(o2);
    }
}


int A4MapLayer::getAllObjects(int x, int y, int dx, int dy, A4Object **l, int max_l)
{
    int n = 0;
    for(A4Object *o:m_objects) {
        if (n>=max_l) break;
        if (o->collision(x,y,dx,dy)) l[n++] = o;
    }
    return n;
}


int A4MapLayer::getAllObjectsInRegion(int x, int y, int dx, int dy, A4Map *map, int region, A4Object **l, int max_l)
{
    int n = 0;
    for(A4Object *o:m_objects) {
        if (n>=max_l) break;
        if (o->collision(x,y,dx,dy)) {
            int tx = o->getX()/m_tile_dx;
            int ty = o->getY()/m_tile_dy;
            int region2 = map->visiblilityRegion(tx,ty);
            if (region == region2) l[n++] = o;
        }
    }
    return n;
}


int A4MapLayer::getAllObjectCollisions(A4Object *o, int xoffs, int yoffs, A4Object **l, int max_l)
{
    int n = 0;
    for(A4Object *o2:m_objects) {
        if (n>=max_l) break;
        if (o2!=o && o->collision(xoffs, yoffs, o2)) l[n++] = o2;
    }
    return n;
}


A4Object *A4MapLayer::getObject(int ID)
{
    for(A4Object *o2:m_objects) {
        if (o2->getID()==ID) return o2;
    }
    return 0;
}


void A4MapLayer::triggerObjectsEvent(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game)
{
    for(A4Object *o:m_objects) {
        o->event(event,otherCharacter,map,game);
    }
}


void A4MapLayer::triggerObjectsEventWithID(int event, char *ID, class A4Character *otherCharacter, class A4Map *map, class A4Game *game)
{
    for(A4Object *o:m_objects) {
        o->eventWithID(event,ID,otherCharacter,map,game);
    }
}


bool A4MapLayer::chopTree(int x, int y, int dx, int dy)
{
    if (x<0 || y<0) return false;
    int tile_x = x/m_tile_dx;
    int tile_y = y/m_tile_dy;
    int tile_x2 = (x+dx-1)/m_tile_dx;
    int tile_y2 = (y+dy-1)/m_tile_dy;
    int tile;
    for(int i = tile_y;i<=tile_y2;i++) {
        if (i>=m_dy) return false;
        for(int j = tile_x;j<=tile_x2;j++) {
            if (j>=m_dx) return false;
            tile = m_tiles[j+i*m_dx];
            if (tile>=0) {
                for(int k = 0;k<m_n_graphicFiles;k++) {
                    if (k<m_n_graphicFiles-1 && tile>=m_gfs_startTile[k+1]) continue;
                    if (m_gfs[k]->m_tileTypes[tile-m_gfs_startTile[k]]==A4_TILE_CHOPPABLE_TREE) {
                        m_tiles[j+i*m_dx] = -1;
                        m_gl_tiles[j+i*m_dx] = 0;
                        return true;
                    }
                    break;
                }
            }
        }
    }
    return false;
}

bool A4MapLayer::spellCollision(A4Object *spell, A4Object *caster)
{
    int x = spell->getX();
    int y = spell->getY();
    int dx = spell->getPixelWidth();
    int dy = spell->getPixelHeight();
    return spellCollision(x,y,dx,dy,caster);
}


bool A4MapLayer::spellCollision(int x, int y, int dx, int dy, A4Object *caster)
{
    if (x<0 || y<0) return true;
    int tile_x = x/m_tile_dx;
    int tile_y = y/m_tile_dy;
    int tile_x2 = (x+dx-1)/m_tile_dx;
    int tile_y2 = (y+dy-1)/m_tile_dy;
    int tile;
    int type;
    for(int i = tile_y;i<=tile_y2;i++) {
        if (i>=m_dy) return true;
        for(int j = tile_x;j<=tile_x2;j++) {
            if (j>=m_dx) return true;
            tile = m_tiles[j+i*m_dx];
            if (tile>=0) {
                for(int k = 0;k<m_n_graphicFiles;k++) {
                    if (k<m_n_graphicFiles-1 && tile>=m_gfs_startTile[k+1]) continue;
                    type = m_gfs[k]->m_tileTypes[tile-m_gfs_startTile[k]];
                    if (type!=A4_TILE_WALKABLE &&
                        type!=A4_TILE_WATER) return true;
                    break;
                }
            }
        }
    }

    for(A4Object *o:m_objects) {
        if (o!=caster && !o->isWalkable()) {
            if (o->collision(x,y,dx,dy)) {
                return true;
            }
        }
    }

    return false;
}


bool A4MapLayer::checkIfDoorGroupStateCanBeChanged(Symbol *doorGroup, bool state, A4Character *character, A4Map *map, A4Game *game)
{
    for(A4Object *o:m_objects) {
        if (o->isDoor()) {
            A4Door *d = ((A4Door *)o);
            if (d->getDoorGroupID()!=0 && d->getDoorGroupID()->cmp(doorGroup)) {
                if (!d->checkForBlockages(state, character, map, game)) return false;
            }
        }
    }
    return true;
}


void A4MapLayer::setDoorGroupState(Symbol *doorGroup, bool state, A4Character *character, A4Map *map, A4Game *game)
{
    for(A4Object *o:m_objects) {
        if (o->isDoor()) {
            A4Door *d = ((A4Door *)o);
            if (d->getDoorGroupID()!=0 && d->getDoorGroupID()->cmp(doorGroup)) {
                d->changeStateRecursively(state, character, map, game);
            }
        }
    }
}

