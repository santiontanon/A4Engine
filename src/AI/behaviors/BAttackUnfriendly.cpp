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

#include <vector>
#include "sound.h"
#include "keyboardstate.h"
#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
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
#include "A4AICharacter.h"
#include "AIMemory.h"

#include "A4Behavior.h"
#include "BAttackUnfriendly.h"

BAttackUnfriendly::BAttackUnfriendly(int priority) : A4Behavior(priority)
{
    m_angryActivation = 250;
}

A4CharacterCommand *BAttackUnfriendly::execute(A4AICharacter *a_character, class A4Game *a_game)
{
    A4AI *ai = a_character->getAI();
    if (ai->m_n_unfriendly>0) {
        ai->addShortTermWME(new WME(A4Game::emotionNames[A4_EMOTION_ANGRY],
									WMEParameter(a_character->getID()), WME_PARAMETER_INTEGER,
                                    m_angryActivation));        
    }
    for(int i = 0;i<ai->m_n_unfriendly;i++) {
        WME *w = ai->m_unfriendly_cache[i];
        
        // check if a magic spell can be used:
        int spell = -1;
        if (a_character->knowsSpell(A4_SPELL_MAGIC_MISSILE) && a_character->getMp()>=A4Game::spellCost[A4_SPELL_MAGIC_MISSILE]) spell = A4_SPELL_MAGIC_MISSILE;
        if (a_character->knowsSpell(A4_SPELL_FIREBALL) && a_character->getMp()>=A4Game::spellCost[A4_SPELL_FIREBALL]) spell = A4_SPELL_FIREBALL;
        if (a_character->knowsSpell(A4_SPELL_INCINERATE) && a_character->getMp()>=A4Game::spellCost[A4_SPELL_INCINERATE]) spell = A4_SPELL_INCINERATE;
        if (spell!=-1) {
            A4Map *map = a_character->getMap();
            A4Object *target = w->getSource();
            if (target==0) continue;    // if we cannot see the object, then do not try to attack it
            if (map == target->getMap()) {
                int tx0 = target->getX();
                int ty0 = target->getY();
                int tw = target->getPixelWidth();
                int th = target->getPixelHeight();
                for(int direction = 0;direction<4;direction++) {
                    Animation *a = a_game->getSpellAnimation(spell, A4_ANIMATION_MOVING_LEFT+direction);
                    if (a==0) a = a_game->getSpellAnimation(spell, A4_ANIMATION_MOVING);
                    if (a==0) a = a_game->getSpellAnimation(spell, A4_ANIMATION_IDLE_LEFT+direction);
                    if (a==0) a = a_game->getSpellAnimation(spell, A4_ANIMATION_IDLE);

                    // 0) Get the position where the spell would start
                    int spell_width = a->getPixelWidth();
                    int spell_height = a->getPixelHeight();
                    int sp_x0 = a_character->getX() + (a_character->getPixelWidth()-spell_width)/2;
                    int sp_y0 = a_character->getY() + (a_character->getPixelHeight()-spell_height)/2;
                    
                    // 1) Check if the target is aligned in any of the 4 directions
                    switch(direction) {
                        case A4_DIRECTION_LEFT:
                            if (tx0+tw>sp_x0 ||
                                sp_y0+spell_height<=ty0 ||
                                sp_y0>= ty0+th) continue;
                            // 2) check if there is an obstable in between:
                            if (map->spellCollision(tx0+tw, sp_y0, sp_x0 - (tx0+tw), spell_height, a_character)) continue;
                            break;
                        case A4_DIRECTION_RIGHT:
                            if (tx0<sp_x0+spell_width ||
                                sp_y0+spell_height<=ty0 ||
                                sp_y0>=ty0+th) continue;
                            // 2) check if there is an obstable in between:
                            if (map->spellCollision(sp_x0+spell_width, sp_y0, tx0 - (sp_x0+spell_width), spell_height, a_character)) continue;
                            break;
                        case A4_DIRECTION_UP:
                            if (ty0+th>sp_y0||
                                sp_x0+spell_width<=tx0 ||
                                sp_x0>=tx0+tw) continue;
                            // 2) check if there is an obstable in between:
                            if (map->spellCollision(sp_x0, ty0+th, spell_width, sp_y0 - (ty0+th), a_character)) continue;
                            break;
                        case A4_DIRECTION_DOWN:
                            if (ty0<sp_y0+spell_height ||
                                sp_x0+spell_width<=tx0 ||
                                sp_x0>=tx0+tw) continue;
                            // 2) check if there is an obstable in between:
                            if (map->spellCollision(sp_x0, sp_y0+spell_height, spell_width, ty0 - (sp_y0+spell_height), a_character)) continue;
                            break;
                    }
                    
                    // 3) Fire!
                    return new A4CharacterCommand(A4CHARACTER_COMMAND_SPELL, spell, direction, target, 0, m_priority);
                }
            }
        }
        
        ai->addPFTargetWME(w, a_game, A4CHARACTER_COMMAND_ATTACK, m_priority, false);
    }
    return 0;
}


void BAttackUnfriendly::toJSString(char *buffer)
{
    sprintf(buffer,"BAttackUnfriendly()");
}


void BAttackUnfriendly::saveToXML(XMLwriter *w)
{
    w->openTag("BAttackUnfriendly");
    w->closeTag("BAttackUnfriendly");
}
