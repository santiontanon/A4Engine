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
#include "XMLwriter.h"
#include "Animation.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4Object.h"
#include "A4Game.h"

#include "A4Item.h"
#include "A4Container.h"
#include "A4Character.h"

#include "A4ObjectFactory.h"

A4Container::A4Container(Symbol *name, Sort *sort, Animation *a) : A4Item(name, sort)
{
	m_animations[A4_ANIMATION_IDLE] = a;
    setUsable(true);
}


A4Container::A4Container(Symbol *name, Sort *sort, A4Object *content, Animation *a) : A4Item(name, sort)
{
	m_animations[A4_ANIMATION_IDLE] = a;
	m_content.push_back(content);
    setUsable(true);
}


A4Container::~A4Container()
{
	for(A4Object *o:m_content) delete o;
	m_content.clear();
}

void A4Container::event(int a_event, A4Character *otherCharacter, A4Map *map, A4Game *game)
{
    A4Item::event(a_event, otherCharacter, map, game);
    
    if (a_event == A4_EVENT_USE) {
        this->event(A4_EVENT_OPEN, otherCharacter, map, game);
        otherCharacter->removeFromInventory(this);
        for(A4Object *o:m_content) {
            otherCharacter->addObjectToInventory(o, game);
        }
        m_content.clear();
        game->requestDeletion(this);
    }
}


void A4Container::loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit)
{
    A4Object::loadObjectAdditionalContent(xml,game,of, objectsToRevisit);
    
    XMLNode *items_xml = xml->get_child("items");
    if (items_xml!=0) {
        for(XMLNode *item_xml:*(items_xml->get_children())) {
            char *tmp = item_xml->get_attribute("probability");
            if (tmp!=0) {
                float p = atof(tmp);
                if (((float)rand() / RAND_MAX)>=p) continue;
            }
            bool completeRedefinition = false;
            tmp = item_xml->get_attribute("completeRedefinition");
            if (tmp!=0 && strcmp(tmp,"true")==0) completeRedefinition = true;
            A4Object *item = of->createObject(item_xml->get_attribute("class"), game, false, completeRedefinition);
            item->loadObjectAdditionalContent(item_xml, game, of, objectsToRevisit);
            addContent(item);
        }
    }
    
}


void A4Container::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Object::savePropertiesToXML(w,game);
    if (!m_content.empty()) {
        w->openTag("items");
        for(A4Object *o:m_content) {
            o->saveToXML(w, game, 0, false);
        }
        w->closeTag("items");
    }
}
