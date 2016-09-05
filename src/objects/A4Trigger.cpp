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

#include "A4Trigger.h"

A4Trigger::A4Trigger(Sort *sort, int w, int h) : A4Object(0, sort)
{
    m_name = new Symbol("trigger");
    m_triggerState = false;
    m_w = w;
    m_h = h;
    if (m_triggerState) m_currentAnimation = A4_ANIMATION_CLOSED;
                   else m_currentAnimation = A4_ANIMATION_OPEN;
}


A4Trigger::~A4Trigger()
{
}


bool A4Trigger::cycle(A4Game *game)
{
    A4Object *l[16];
    A4Object::cycle(game);
    
    int n = m_map->getAllObjectCollisions(this, l, 16);
    A4Object *triggered_by = 0;
    bool playerOver = false;
    
    for(int i = 0;i<n;i++) {
        A4Object *o = l[i];
        if (o->isPlayer()) {
            playerOver = true;
            triggered_by = o;
        }
    }
    
    if (m_triggerState) {
        if (playerOver) {
            // nothing to do, keep pressed
        } else {
            // release
            m_triggerState = false;
            event(A4_EVENT_DEACTIVATE,0, m_map, game);
            event(A4_EVENT_USE,0, m_map, game);
        }
    } else {
        if (triggered_by!=0 && playerOver) {
            // press!
            m_triggerState = true;
            event(A4_EVENT_ACTIVATE,(A4Character *)triggered_by, m_map, game);
            event(A4_EVENT_USE,(A4Character *)triggered_by, m_map, game);
        } else {
            // nothing to do, keep released
        }
    }
    
    return true;
}


bool A4Trigger::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Object::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    if (strcmp(a_name,"triggerState")==0) {
        m_triggerState = false;
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) m_triggerState = true;
        return true;
    }
    return false;
}


void A4Trigger::loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit)
{
    A4Object::loadObjectAdditionalContent(xml, game, of, objectsToRevisit);
    
    m_w = atoi(xml->get_attribute("width"));
    m_h = atoi(xml->get_attribute("height"));
}


void A4Trigger::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w,game);

    saveObjectAttributeToXML(w,"triggerState",m_triggerState);
}


