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

#include "A4Character.h"
#include "A4PlayerCharacter.h"

A4PlayerCharacter::A4PlayerCharacter(Symbol *name, Sort *sort) : A4Character(name, sort)
{
//    for(int i = 0;i<A4_N_SPELLS;i++) learnSpell(i);
}


A4PlayerCharacter::~A4PlayerCharacter()
{
}


bool A4PlayerCharacter::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Character::loadObjectAttribute(attribute_xml)) return true;
    
    // Thi functino is just so that we can reuse object class definitions between Players and AI characters.
    char *a_name = attribute_xml->get_attribute("name");
    if (strcmp(a_name,"sightRadius")==0) {
        return true;
    } else if (strcmp(a_name,"respawn")==0) {
        return true;
    } else if (strcmp(a_name,"AI.period")==0) {
        return true;
    } else if (strcmp(a_name,"AI.cycle")==0) {
        return true;
    } else if (strcmp(a_name,"respawnRecordID")==0) {
        return true;
    }
    return false;
}

