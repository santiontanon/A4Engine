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
#include "A4Item.h"
#include "A4CoinPurse.h"
#include "A4Character.h"

#include "A4Behavior.h"
#include "BLeftRight.h"
#include "BUpDown.h"
#include "BCurious.h"
#include "BTired.h"
#include "BFleeUnfriendly.h"
#include "BAttackUnfriendly.h"
#include "BYellToUnfriendly.h"
#include "BWander.h"
#include "BWanderInCircles.h"
#include "BGoto.h"
#include "BFollow.h"
#include "BFollowFromWME.h"
#include "BFollowAngryFriendly.h"


A4Behavior::A4Behavior(int priority)
{
    m_priority = priority;
    m_ID = 0;
}

A4Behavior::A4Behavior(int priority, char *ID)
{
    m_priority = priority;
    m_ID = ID;
}


A4Behavior::~A4Behavior()
{
    if (m_ID!=0) delete m_ID;
    m_ID = 0;
}


A4CharacterCommand *A4Behavior::execute(class A4AICharacter *a_character, class A4Game *a_game)
{
    return 0;
}


void A4Behavior::toJSString(char *buffer)
{
    sprintf(buffer,"Behavior()");
}


A4Behavior *A4Behavior::createBehaviorFromXML(class XMLNode *xml, Ontology *o)
{
    char *name = xml->get_type()->get();
    
    if (strcmp(name,"BLeftRight")==0) {
        return new BLeftRight(0);
    } else if (strcmp(name,"BUpDown")==0) {
        return new BUpDown(0);
    } else if (strcmp(name,"BCurious")==0) {
        bool pickup = strcmp(xml->get_attribute("pickup"), "true")==0;
        Sort *sort = o->getSort(xml->get_attribute("sort"));
        int activation = atoi(xml->get_attribute("time"));
        return new BCurious(pickup, sort, activation, 0);
    } else if (strcmp(name,"BTired")==0) {
        int wt = atoi(xml->get_attribute("timeWalking"));
        int rt = atoi(xml->get_attribute("restTime"));
        return new BTired(wt, rt, 0);
    } else if (strcmp(name,"BFleeUnfriendly")==0) {
        return new BFleeUnfriendly(0);
    } else if (strcmp(name,"BAttackUnfriendly")==0) {
        return new BAttackUnfriendly(0);
    } else if (strcmp(name,"BYellToUnfriendly")==0) {
        char *message = xml->get_attribute("message");
        return new BYellToUnfriendly(message, 0);
    } else if (strcmp(name,"BWander")==0) {
        double forwardFactor = atof(xml->get_attribute("forwardFactor"));
        double pStop = atof(xml->get_attribute("pStop"));
        int stopTime = atoi(xml->get_attribute("stopTime"));
        return new BWander(forwardFactor, pStop, stopTime, 0);
    } else if (strcmp(name,"BWanderInCircles")==0) {
        int diameter = atoi(xml->get_attribute("diameter"));
        return new BWanderInCircles(diameter, 0);
    } else if (strcmp(name,"BGoto")==0) {
        char *location_str = xml->get_attribute("location");
        if (location_str == 0) {
            output_debug_message("BGoto behavior does not have the `location' attribute set!\n");
            exit(1);
        }
        Symbol *location = new Symbol(location_str);
        int radius = atoi(xml->get_attribute("radius"));
        return new BGoto(location, radius, 0);
    } else if (strcmp(name,"BFollow")==0) {
        char *target_str = xml->get_attribute("target");
        if (target_str == 0) {
            output_debug_message("BFollow behavior does not have the `target' attribute set!\n");
            exit(1);
        }
        Symbol *target = new Symbol(target_str);
        int radius = atoi(xml->get_attribute("radius"));
        return new BFollow(target, radius, 0);
    } else if (strcmp(name,"BFollowFromWME")==0) {
        char *radius_str = xml->get_attribute("radius");
        if (radius_str == 0) {
            output_debug_message("BFollowFromWME behavior does not have the `radius' attribute set!\n");
            exit(1);
        }
        int radius = atoi(radius_str);
        return new BFollowFromWME(radius, 0);
    } else if (strcmp(name,"BFollowAngryFriendly")==0) {
        char *radius_str = xml->get_attribute("radius");
        if (radius_str == 0) {
            output_debug_message("BFollowAngryFriendly behavior does not have the `radius' attribute set!\n");
            exit(1);
        }
        int radius = atoi(radius_str);
        return new BFollowAngryFriendly(radius, 0);
    } else {
        output_debug_message("createBehaviorFromXML, unknown type %s\n", name);
        exit(1);
    }
    
    return 0;
}