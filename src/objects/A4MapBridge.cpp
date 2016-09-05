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

#include "A4MapBridge.h"


A4MapBridge::A4MapBridge(XMLNode *bridge_node) : A4Object(0, 0)
{
    m_name = new Symbol(bridge_node->get_attribute("name"));
    m_x = atoi(bridge_node->get_attribute("x"));
    m_y = atoi(bridge_node->get_attribute("y"));
    m_dx = atoi(bridge_node->get_attribute("width"));
    m_dy = atoi(bridge_node->get_attribute("height"));
    m_appearDirection = A4_DIRECTION_NONE;
    m_appearWalking = false;
    m_linkedTo = 0;
    m_map = 0;
}


A4MapBridge::A4MapBridge(XMLNode *bridge_node, A4Map *map) :  A4Object(0, 0)
{
    m_name = new Symbol(bridge_node->get_attribute("name"));
    m_x = atoi(bridge_node->get_attribute("x"));
    m_y = atoi(bridge_node->get_attribute("y"));
    m_dx = atoi(bridge_node->get_attribute("width"));
    m_dy = atoi(bridge_node->get_attribute("height"));
    m_appearDirection = A4_DIRECTION_NONE;
    m_appearWalking = false;
    m_linkedTo = 0;
    m_map = map;
}


// for bridges
bool A4MapBridge::findAvailableTargetLocation(A4Object *o, int tile_dx, int tile_dy,int &x, int &y)
{
    for(int i = 0;i<m_dy;i+=tile_dy) {
        for(int j = 0;j<m_dx;j+=tile_dx) {
            if (m_map->walkable(m_x+j, m_y+i, o->getPixelWidth(), o->getPixelHeight(), o)) {
                x = m_x+j;
                y = m_y+i;
                return true;
            }
        }
    }
    return false;
}
