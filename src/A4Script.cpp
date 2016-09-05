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
#include "SDL/SDL_image.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
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
#include "BB2D.h"
#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
#include "XMLparser.h"
#include "XMLwriter.h"
#include "Animation.h"
#include "ExpressionParser.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4Game.h"
#include "A4Object.h"
#include "A4CoinPurse.h"
#include "A4Character.h"
#include "A4AICharacter.h"
#include "A4PlayerCharacter.h"
#include "A4EquipableItem.h"
#include "A4Vehicle.h"

#include "ConversationGraph.h"

#include "A4ObjectFactory.h"

#include "Ontology.h"
#include "WME.h"

#include "A4Behavior.h"


int execute_gameComplete(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_addBehavior(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_removeBehavior(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_teleport(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_goto(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_die(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_use(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_openDoors(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_message(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_talk(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_pendingTalk(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_addTopic(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_updateConversationGraphTransition(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_storyState(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_addWME(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_addWMEToOthers(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_steal(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_give(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_sell(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_drop(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_loseItem(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_gainItem(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_experienceGain(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_delay(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_playSound(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_stopSound(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_if(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_gainGoldOther(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_talkOther(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_addCurrentPositionWME(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_startTrading(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_familiarWithMap(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_addAgenda(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_removeAgenda(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_eventRule(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_gainGold(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_dropGold(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_take(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_equip(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_unequip(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_interact(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_embark(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_disembark(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_attack(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_spell(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_buy(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
int execute_chop(A4Script *script, A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);


const char *A4Script::scriptNames[] = {
    "gameComplete", // A4_SCRIPT_GAMECOMPLETE      0
    "addBehavior", // A4_SCRIPT_ADDBEHAVIOR       1
    "removeBehavior", // A4_SCRIPT_REMOVEBEHAVIOR    2
    "teleport", // A4_SCRIPT_TELEPORT          3
    "goto", // A4_SCRIPT_GOTO              4
    "die", // A4_SCRIPT_DIE               5
    "use", // A4_SCRIPT_USE     6
    "openDoors", // A4_SCRIPT_OPENDOORS          7
    "message", // A4_SCRIPT_MESSAGE           8
    "talk", // A4_SCRIPT_TALK              9
    "pendingTalk", // A4_SCRIPT_PENDINGTALK       10
    "addTopic", // A4_SCRIPT_ADDTOPIC          11
    "updateConversationGraphTransition", // A4_SCRIPT_UPDATECONVERSATIONGRAPHTRANSITION       12
    "storyState", // A4_SCRIPT_STORYSTATE        13
    "addWME", // A4_SCRIPT_ADDWME    14
    "addWMEToOthers", // A4_SCRIPT_ADDWMETOOTHERS       15
    "steal", // A4_SCRIPT_STEAL             16
    "give", // A4_SCRIPT_GIVE              17
    "sell", // A4_SCRIPT_SELL              18
    "drop", // A4_SCRIPT_DROP              19
    "loseItem", // A4_SCRIPT_LOSEITEM          20
    "gainItem", // A4_SCRIPT_GAINITEM          21
    "experienceGain", // A4_SCRIPT_EXPERIENCEGAIN    22
    "delay", // A4_SCRIPT_DELAY             23
    "playSound", // A4_SCRIPT_PLAYSOUND         24
    "stopSound", // A4_SCRIPT_STOPSOUND         25
    "if", // A4_SCRIPT_IF                26
    // new scripts (not in v1.2.2):
    "gainGoldOther", // A4_SCRIPT_GAINGOLDOTHER          27
    "talkOther", // A4_SCRIPT_TALKOTHER         28
    "addCurrentPositionWME",    // A4_SCRIPT_ADDCURRENTPOSITIONWME  29
    "startTrading", // A4_SCRIPT_STARTTRADING   30
    "familiarWithMap", // A4_SCRIPT_FAMILIARWITHMAP 31
    "addAgenda", // A4_SCRIPT_ADDAGENDA 32
    "removeAgenda", // A4_SCRIPT_REMOVEAGENDA 33
    "eventRule", // A4_SCRIPT_EVENTRULE 34
    "gainGold", // A4_SCRIPT_GAINGOLD   35
    "dropGold", // A4_SCRIPT_DROPGOLD   36
    "take", // A4_SCRIPT_TAKE    37
    "equip", // A4_SCRIPT_EQUIP    38
    "unequip", // A4_SCRIPT_UNEQUIP    39
    "interact", // A4_SCRIPT_INTERACT          40
    "embark", // A4_SCRIPT_EMBARK              41
    "disembark", // A4_SCRIPT_DISEMBARK            42
    "attack", // A4_SCRIPT_ATTACK            43
    "spell", // A4_SCRIPT_SPELL             44
    "buy", // A4_SCRIPT_BUY               45
    "chop", // A4_SCRIPT_CHOP              46
    
	0,	// marker to indicate the end of the array
};

const scriptFunctionType A4Script::scriptFunctions[] = {
    &execute_gameComplete,
    &execute_addBehavior,
    &execute_removeBehavior,
    &execute_teleport,
    &execute_goto,
    &execute_die,
    &execute_use,
    &execute_openDoors,
    &execute_message,
    &execute_talk,
    &execute_pendingTalk,
    &execute_addTopic,
    &execute_updateConversationGraphTransition,
    &execute_storyState,
    &execute_addWME,
    &execute_addWMEToOthers,
    &execute_steal,
    &execute_give,
    &execute_sell,
    &execute_drop,
    &execute_loseItem,
    &execute_gainItem,
    &execute_experienceGain,
    &execute_delay,
    &execute_playSound,
    &execute_stopSound,
    &execute_if,
    &execute_gainGoldOther,
    &execute_talkOther,
    &execute_addCurrentPositionWME,
    &execute_startTrading,
    &execute_familiarWithMap,
    &execute_addAgenda,
    &execute_removeAgenda,
    &execute_eventRule,
    &execute_gainGold,
    &execute_dropGold,
    &execute_take,
    &execute_equip,
    &execute_unequip,
    &execute_interact,
    &execute_embark,
    &execute_disembark,
    &execute_attack,
    &execute_spell,
    &execute_buy,
    &execute_chop,
};


A4Script::A4Script(XMLNode *xml)
{
	// initialize all the parameters to 0:
	value = 0;
	ID = 0;
	text = text2 = 0;
    state = 0;
    if_state = 0;
    angry = false;
    wait = false;
    consume = false;
    agenda = 0;
    rule = 0;
    objectDefinition = 0;

	for(int i = 0;scriptNames[i]!=0;i++) {
		if (strcmp(xml->get_type()->get(),scriptNames[i])==0) {
			// found:
			m_type = i;
			switch(m_type) {
				case A4_SCRIPT_GAMECOMPLETE:
                    {
                        char *tmp = xml->get_attribute("id");
                        if (tmp!=0) {
                            ID = new char[strlen(tmp)+1];
                            strcpy(ID,tmp);
                        } else {
                            ID = 0;
                        }
                        break;
                    }
                    break;
				case A4_SCRIPT_ADDBEHAVIOR:
                {
                    char *tmp = xml->get_attribute("priority");
                    value = atoi(tmp);
                    tmp = xml->get_attribute("id");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    
                    objectDefinition = new XMLNode(xml->get_children()->at(0));
                    break;
                }
				case A4_SCRIPT_REMOVEBEHAVIOR:
                {
                    char *tmp = xml->get_attribute("id");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    break;
                }
				case A4_SCRIPT_TELEPORT:
                    {
                        x = atoi(xml->get_attribute("x"));
                        y = atoi(xml->get_attribute("y"));
                        char *tmp = xml->get_attribute("map");
                        if (tmp==0) {
                            ID = 0;
                        } else {
                            ID = new char[strlen(tmp)+1];
                            strcpy(ID,tmp);
                        }
                        
                        break;
                    }
				case A4_SCRIPT_GOTO:
                {
                    x = atoi(xml->get_attribute("x"));
                    y = atoi(xml->get_attribute("y"));
                    char *tmp = xml->get_attribute("map");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    
                    break;
                }
				case A4_SCRIPT_DIE:
                    break;
				case A4_SCRIPT_USE:
                {
                    char *tmp = xml->get_attribute("x");
                    if (tmp!=0) {
                        x = atoi(tmp);
                        y = atoi(xml->get_attribute("y"));
                    } else {
                        x = -1;
                        y = -1;
                    }
                    tmp = xml->get_attribute("map");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    char *tmp2 = xml->get_attribute("inventory");
                    if (tmp2!=0) {
                        ID = new char[strlen(tmp2)+1];
                        strcpy(ID,tmp2);
                        x = -1;
                        y = -1;
                        if (tmp!=0) {
                            output_debug_message("'map' and 'inventory' cannot be both specified in the same 'use' script!\n");
                            exit(1);
                        }
                        if (x!=-1 || y!=-1) {
                            output_debug_message("'x/y' and 'inventory' cannot be both specified in the same 'use' script!\n");
                            exit(1);
                        }
                    }
                    break;
                }
				case A4_SCRIPT_OPENDOORS:
                {
                    char *tmp = xml->get_attribute("door");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    break;
                }
				case A4_SCRIPT_MESSAGE:
                {
                    char *tmp = xml->get_attribute("text");
                    text = new char[strlen(tmp)+1];
                    strcpy(text,tmp);
                    break;
                }
				case A4_SCRIPT_TALK:
                {
                    char *tmp = xml->get_attribute("text");
                    text = new char[strlen(tmp)+1];
                    strcpy(text,tmp);
                    tmp = xml->get_attribute("topic");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    tmp = xml->get_attribute("performative");
                    value = A4_TALK_PERFORMATIVE_NONE;
                    if (tmp!=0) {
                        if (strcmp(tmp,"hi")==0) value = A4_TALK_PERFORMATIVE_HI;
                        if (strcmp(tmp,"bye")==0) value = A4_TALK_PERFORMATIVE_BYE;
                        if (strcmp(tmp,"ask")==0) value = A4_TALK_PERFORMATIVE_ASK;
                        if (strcmp(tmp,"inform")==0) value = A4_TALK_PERFORMATIVE_INFORM;
                        if (strcmp(tmp,"trade")==0) value = A4_TALK_PERFORMATIVE_TRADE;
                        if (strcmp(tmp,"endtrade")==0) value = A4_TALK_PERFORMATIVE_END_TRADE;
                    }
                    tmp = xml->get_attribute("angry");
                    if (tmp==0) {
                        angry = false;
                    } else if (strcmp(tmp,"true")==0) {
                        angry = true;
                    }
                    tmp = xml->get_attribute("wait");
                    if (tmp==0) {
                        wait = false;
                    } else if (strcmp(tmp,"true")==0) {
                        wait = true;
                    }
                    break;
                }
				case A4_SCRIPT_PENDINGTALK:
                {
                    char *tmp = xml->get_attribute("character");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    tmp = xml->get_attribute("text");
                    text = new char[strlen(tmp)+1];
                    strcpy(text,tmp);
                    // subscripts:
                    for(XMLNode *sub_xml:*(xml->get_children())) {
                        A4Script *sub = new A4Script(sub_xml);
                        subScripts.push_back(sub);
                    }
                    break;
                }
				case A4_SCRIPT_ADDTOPIC:
                {
                    char *tmp = xml->get_attribute("topic");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    tmp = xml->get_attribute("text");
                    text = new char[strlen(tmp)+1];
                    strcpy(text,tmp);
                    break;
                }
				case A4_SCRIPT_UPDATECONVERSATIONGRAPHTRANSITION:
                {
                    value = CGT_ACTOR_OTHER;
                    if (xml->get_attribute("actor")!=0 &&
                        strcmp(xml->get_attribute("actor"),"self")==0) value = CGT_ACTOR_SELF;
                    if (xml->get_attribute("performative")!=0) {
                    if (strcmp(xml->get_attribute("performative"),"hi")==0) x = A4_TALK_PERFORMATIVE_HI;
                    else if (strcmp(xml->get_attribute("performative"),"bye")==0) x = A4_TALK_PERFORMATIVE_BYE;
                    else if (strcmp(xml->get_attribute("performative"),"trade")==0) x = A4_TALK_PERFORMATIVE_TRADE;
                    else if (strcmp(xml->get_attribute("performative"),"endtrade")==0) x = A4_TALK_PERFORMATIVE_END_TRADE;
                    else if (strcmp(xml->get_attribute("performative"),"ask")==0) x = A4_TALK_PERFORMATIVE_ASK;
                    else if (strcmp(xml->get_attribute("performative"),"inform")==0) x = A4_TALK_PERFORMATIVE_INFORM;
                    else if (strcmp(xml->get_attribute("performative"),"timeout")==0) x = A4_TALK_PERFORMATIVE_TIMEOUT;
                    }
                    // both "topic" or "keyword" are the same:
                    if (xml->get_attribute("keyword")!=0) {
                        char *tmp = xml->get_attribute("keyword");
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    } else if (xml->get_attribute("topic")!=0) {
                        char *tmp = xml->get_attribute("topic");
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    if (xml->get_attribute("state")!=0) {
                        char *tmp = xml->get_attribute("state");
                        text = new char[strlen(tmp)+1];
                        strcpy(text,tmp);
                    }
                    consume = true;
                    if (xml->get_attribute("consume")!=0 &&
                        strcmp(xml->get_attribute("consume"),"false")==0) consume = false;
                    char *tmp = xml->get_attribute("from");
                    text2 = new char[strlen(tmp)+1];
                    strcpy(text2,tmp);
                    
                    for(XMLNode *script_xml:*(xml->get_children())) {
                        subScripts.push_back(new A4Script(script_xml));
                    }
                    break;
                }
				case A4_SCRIPT_STORYSTATE:
                {
                    char *tmp = xml->get_attribute("scope");
                    if (strcmp(tmp,"game")==0) value = A4_STORYSTATE_GAME;
                    else if (strcmp(tmp,"map")==0) value = A4_STORYSTATE_MAP;
                    else if (strcmp(tmp,"object")==0) value = A4_STORYSTATE_OBJECT;
                    else if (strcmp(tmp,"character")==0) value = A4_STORYSTATE_OBJECT;
                    else {
                        output_debug_message("unrecognized scope in storyState %s\n",tmp);
                    }
                    tmp = xml->get_attribute("variable");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    tmp = xml->get_attribute("value");
                    text = new char[strlen(tmp)+1];
                    strcpy(text,tmp);
                    break;
                }
				case A4_SCRIPT_ADDWME:
                {
                    char *tmp = xml->get_attribute("wme");
                    text = new char[strlen(tmp)+1];
                    strcpy(text,tmp);
                    tmp = xml->get_attribute("activation");
                    value = atoi(tmp);
                    break;
                }
				case A4_SCRIPT_ADDWMETOOTHERS:
                {
                    char *tmp = xml->get_attribute("wme");
                    text = new char[strlen(tmp)+1];
                    strcpy(text,tmp);
                    tmp = xml->get_attribute("activation");
                    value = atoi(tmp);
                    
                    // characterClass
                    tmp = xml->get_attribute("characterClass");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    
                    // select
                    if (strcmp(xml->get_attribute("select"),"all")==0) {
                        x = 0;
                    } else {
                        x = 1;
                    }
                    break;
                }
				case A4_SCRIPT_STEAL:
                    {
                        char *tmp = xml->get_attribute("name");
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    break;
				case A4_SCRIPT_GIVE:
                    {
                        char *tmp = xml->get_attribute("inventory");
                        if (tmp==0) {
                            ID = 0;
                        } else {
                            ID = new char[strlen(tmp)+1];
                            strcpy(ID,tmp);
                        }
                        XMLNode *object = xml->get_child("object");
                        if (object!=0) objectDefinition = new XMLNode(object);
                        break;
                    }
				case A4_SCRIPT_SELL:
                {
                    char *tmp = xml->get_attribute("inventory");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    XMLNode *object = xml->get_child("object");
                    if (object!=0) objectDefinition = new XMLNode(object);
                    break;
                }
				case A4_SCRIPT_DROP:
                {
                    char *tmp = xml->get_attribute("inventory");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    XMLNode *object = xml->get_child("object");
                    if (object!=0) objectDefinition = new XMLNode(object);
                    break;
                }
				case A4_SCRIPT_LOSEITEM:
                {
                    char *tmp = xml->get_attribute("inventory");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    break;
                }
				case A4_SCRIPT_GAINITEM:
                {
                    XMLNode *object = xml->get_child("object");
                    objectDefinition = new XMLNode(object);
                    break;
                }
				case A4_SCRIPT_EXPERIENCEGAIN:
                {
                    char *tmp = xml->get_attribute("xp");
                    value = atoi(tmp);
                    break;
                }
				case A4_SCRIPT_DELAY:
                {
                    char *tmp = xml->get_attribute("cycles");
                    value = atoi(tmp);
                    break;
                }
				case A4_SCRIPT_PLAYSOUND:
                {
                    char *tmp = xml->get_attribute("sound");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    break;					
                }
				case A4_SCRIPT_STOPSOUND:
                {
                    char *tmp = xml->get_attribute("sound");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    break;
                }
				case A4_SCRIPT_IF:
                {
                    XMLNode *condition_node = xml->get_child("condition");
                    XMLNode *then_node = xml->get_child("then");
                    XMLNode *else_node = xml->get_child("else");
                    for(XMLNode *sub_xml:*(condition_node->get_children())) {
                        A4Script *sub = new A4Script(sub_xml);
                        subScripts.push_back(sub);
                    }
                    for(XMLNode *sub_xml:*(then_node->get_children())) {
                        A4Script *sub = new A4Script(sub_xml);
                        thenSubScripts.push_back(sub);
                    }
                    for(XMLNode *sub_xml:*(else_node->get_children())) {
                        A4Script *sub = new A4Script(sub_xml);
                        elseSubScripts.push_back(sub);
                    }
                    break;
                }
				case A4_SCRIPT_GAINGOLDOTHER:
                {
                    char *tmp = xml->get_attribute("gold");
                    value = atoi(tmp);
                    break;
                }
				case A4_SCRIPT_TALKOTHER:
                {
                    char *tmp = xml->get_attribute("text");
                    text = new char[strlen(tmp)+1];
                    strcpy(text,tmp);
                    tmp = xml->get_attribute("topic");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    tmp = xml->get_attribute("performative");
                    value = A4_TALK_PERFORMATIVE_INFORM;
                    if (tmp!=0) {
                        if (strcmp(tmp,"hi")==0) value = A4_TALK_PERFORMATIVE_HI;
                        if (strcmp(tmp,"bye")==0) value = A4_TALK_PERFORMATIVE_BYE;
                        if (strcmp(tmp,"ask")==0) value = A4_TALK_PERFORMATIVE_ASK;
                        if (strcmp(tmp,"inform")==0) value = A4_TALK_PERFORMATIVE_INFORM;
                        if (strcmp(tmp,"trade")==0) value = A4_TALK_PERFORMATIVE_TRADE;
                        if (strcmp(tmp,"endtrade")==0) value = A4_TALK_PERFORMATIVE_END_TRADE;
                    }
                    tmp = xml->get_attribute("angry");
                    if (tmp==0) {
                        angry = false;
                    } else if (strcmp(tmp,"true")==0) {
                        angry = true;
                    }
                    tmp = xml->get_attribute("wait");
                    if (tmp==0) {
                        wait = false;
                    } else if (strcmp(tmp,"true")==0) {
                        wait = true;
                    }
                    break;
                }
                case A4_SCRIPT_ADDCURRENTPOSITIONWME:
                {
                    char *tmp = xml->get_attribute("name");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    tmp = xml->get_attribute("activation");
                    value = atoi(tmp);
                    break;
                }
                case A4_SCRIPT_STARTTRADING:
                    break;
                case A4_SCRIPT_FAMILIARWITHMAP:
                {
                    char *tmp = xml->get_attribute("map");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    break;
                }
                case A4_SCRIPT_ADDAGENDA:
                {
                    agenda = new Agenda(xml);
                    break;
                }
                case A4_SCRIPT_REMOVEAGENDA:
                {
                    char *tmp = xml->get_attribute("agenda");
                    ID = new char[strlen(tmp)+1];
                    strcpy(ID,tmp);
                    break;
                }
                case A4_SCRIPT_EVENTRULE:
                {
                    rule = new A4EventRule(xml);
                    break;
                }
                case A4_SCRIPT_GAINGOLD:
                {
                    char *tmp = xml->get_attribute("gold");
                    value = atoi(tmp);
                    break;
                }
                case A4_SCRIPT_DROPGOLD:
                {
                    char *tmp = xml->get_attribute("gold");
                    value = atoi(tmp);
                    break;
                }
                case A4_SCRIPT_TAKE:
                {
                    char *tmp = xml->get_attribute("x");
                    if (tmp!=0) {
                        x = atoi(tmp);
                        y = atoi(xml->get_attribute("y"));
                    } else {
                        x = -1;
                        y = -1;
                    }
                    tmp = xml->get_attribute("map");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    break;
                }
                    
                case A4_SCRIPT_EQUIP:
                {
                    char *tmp = xml->get_attribute("inventory");
                    if (tmp!=0) {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    break;
                }
                    
                case A4_SCRIPT_UNEQUIP:
                {
                    char *tmp = xml->get_attribute("inventory");
                    if (tmp!=0) {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    break;
                }

                case A4_SCRIPT_INTERACT:
                {
                    char *tmp = xml->get_attribute("x");
                    if (tmp!=0) {
                        x = atoi(tmp);
                        y = atoi(xml->get_attribute("y"));
                    } else {
                        x = -1;
                        y = -1;
                    }
                    tmp = xml->get_attribute("map");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    break;
                }

                case A4_SCRIPT_EMBARK:
                {
                    char *tmp = xml->get_attribute("x");
                    if (tmp!=0) {
                        x = atoi(tmp);
                        y = atoi(xml->get_attribute("y"));
                    } else {
                        x = -1;
                        y = -1;
                    }
                    tmp = xml->get_attribute("map");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    break;
                }

                case A4_SCRIPT_DISEMBARK:
                {
                    char *tmp = xml->get_attribute("x");
                    if (tmp!=0) {
                        x = atoi(tmp);
                        y = atoi(xml->get_attribute("y"));
                    } else {
                        x = -1;
                        y = -1;
                    }
                    tmp = xml->get_attribute("map");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    break;
                }

                case A4_SCRIPT_ATTACK:
                {
                    char *tmp = xml->get_attribute("target");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    tmp = xml->get_attribute("untilDead");
                    if (tmp==0) {
                        wait = false;
                    } else if (strcmp(tmp,"true")==0) {
                        wait = true;
                    }
                    break;
                }

                case A4_SCRIPT_SPELL:
                {
                    char *tmp = xml->get_attribute("spell");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    tmp = xml->get_attribute("direction");
                    if (tmp==0) {
                        value = A4_DIRECTION_NONE;
                    } else {
                        if (strcmp(tmp,"east")==0) value = A4_DIRECTION_RIGHT;
                        if (strcmp(tmp,"west")==0) value = A4_DIRECTION_LEFT;
                        if (strcmp(tmp,"north")==0) value = A4_DIRECTION_UP;
                        if (strcmp(tmp,"south")==0) value = A4_DIRECTION_DOWN;
                        if (strcmp(tmp,"right")==0) value = A4_DIRECTION_RIGHT;
                        if (strcmp(tmp,"left")==0) value = A4_DIRECTION_LEFT;
                        if (strcmp(tmp,"up")==0) value = A4_DIRECTION_UP;
                        if (strcmp(tmp,"down")==0) value = A4_DIRECTION_DOWN;
                        if (strcmp(tmp,"self")==0) value = A4_DIRECTION_NONE;
                    }
                    break;
                }

                case A4_SCRIPT_BUY:
                {
                    char *tmp = xml->get_attribute("seller");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    tmp = xml->get_attribute("object");
                    if (tmp==0) {
                        text = 0;
                    } else {
                        text = new char[strlen(tmp)+1];
                        strcpy(text,tmp);
                    }
                    break;
                }

                case A4_SCRIPT_CHOP:
                {
                    char *tmp = xml->get_attribute("x");
                    if (tmp!=0) {
                        x = atoi(tmp);
                        y = atoi(xml->get_attribute("y"));
                    } else {
                        x = -1;
                        y = -1;
                    }
                    tmp = xml->get_attribute("map");
                    if (tmp==0) {
                        ID = 0;
                    } else {
                        ID = new char[strlen(tmp)+1];
                        strcpy(ID,tmp);
                    }
                    break;
                }
            }
			return;
		}
	}
	output_debug_message("Unknown script type: %s\n",xml->get_type()->get());
	exit(1);
}


A4Script::A4Script(A4Script *s)
{
    m_type = s->m_type;
    value = s->value;
    x = s->x;
    y = s->y;
    state = s->state;
    if_state = s->if_state;
    angry = s->angry;
    wait = s->wait;
    consume = s->consume;
    if (s->ID==0) {
        ID = 0;
    } else {
        ID = new char[strlen(s->ID)+1];
        strcpy(ID,s->ID);
    }
    if (s->text==0) {
        text = 0;
    } else {
        text = new char[strlen(s->text)+1];
        strcpy(text,s->text);
    }
    if (s->text2==0) {
        text2 = 0;
    } else {
        text2 = new char[strlen(s->text2)+1];
        strcpy(text2,s->text2);
    }
    for(A4Script *ss:s->subScripts) {
        subScripts.push_back(new A4Script(ss));
    }
    for(A4Script *ss:s->thenSubScripts) {
        thenSubScripts.push_back(new A4Script(ss));
    }
    for(A4Script *ss:s->elseSubScripts) {
        elseSubScripts.push_back(new A4Script(ss));
    }
    if (s->agenda==0) {
        agenda = 0;
    } else {
        agenda = new Agenda(s->agenda);
    }
    if (s->rule==0) {
        rule = 0;
    } else {
        rule = new A4EventRule(s->rule);
    }
    if (s->objectDefinition!=0) {
        objectDefinition = new XMLNode(s->objectDefinition);
    } else {
        objectDefinition = 0;
    }
}


A4Script::A4Script(int type, const char *a_ID)
{
	value = 0;
    if (a_ID==0) {
        ID = 0;
    } else {
        ID = new char[strlen(a_ID)+1];
        strcpy(ID, a_ID);
    }
	text = text2 = 0;
	m_type = type;
    state = 0;
    if_state = 0;
    angry = false;
    wait = false;
    consume = false;
    agenda = 0;
    objectDefinition = 0;
}


A4Script::A4Script(int type, const char *a_ID, const char *a_text)
{
    value = 0;
    if (a_ID==0) {
        ID = 0;
    } else {
        ID = new char[strlen(a_ID)+1];
        strcpy(ID, a_ID);
    }
    if (a_text==0) {
        text = 0;
    } else {
        text = new char[strlen(a_text)+1];
        strcpy(text,a_text);
    }
    text2 = 0;
    m_type = type;
    state = 0;
    if_state = 0;
    angry = false;
    wait = false;
    consume = false;
    agenda = 0;
    objectDefinition = 0;
}


A4Script::A4Script(int type, const char *a_ID, const char *a_text, int a_value, bool a_angry, bool a_wait)
{
    value = a_value;
    if (a_ID==0) {
        ID = 0;
    } else {
        ID = new char[strlen(a_ID)+1];
        strcpy(ID, a_ID);
    }
    if (a_text==0) {
        text = 0;
    } else {
        text = new char[strlen(a_text)+1];
        strcpy(text,a_text);
    }
    text2 = 0;
    m_type = type;
    state = 0;
    if_state = 0;
    angry = a_angry;
    wait = a_wait;
    consume = false;
    agenda = 0;
    objectDefinition = 0;
}


A4Script::A4Script(int type, int v)
{
	value = v;
	ID = 0;
	text = text2 = 0;
	m_type = type;
    state = 0;
    if_state = 0;
    angry = false;
    wait = false;
    consume = false;
    agenda = 0;
    objectDefinition = 0;
}


A4Script::~A4Script()
{
	if (ID!=0) delete ID;
	ID = 0;
	if (text!=0) delete text;
	text = 0;
    if (text2!=0) delete text2;
    text2 = 0;
    for(A4Script *s:subScripts) delete s;
    subScripts.clear();
    for(A4Script *s:thenSubScripts) delete s;
    thenSubScripts.clear();
    for(A4Script *s:elseSubScripts) delete s;
    elseSubScripts.clear();
    if (agenda!=0) delete agenda;
    agenda = 0;
    if (objectDefinition!=0) delete objectDefinition;
    objectDefinition = 0;
}


void A4Script::saveToXML(XMLwriter *w)
{
    w->openTag(scriptNames[m_type]);
    switch(m_type) {
        case A4_SCRIPT_GAMECOMPLETE:
        {
            if (ID!=0) w->setAttribute("id", ID);
            break;
        }
        case A4_SCRIPT_ADDBEHAVIOR:
        {
            w->setAttribute("priority", value);
            if (ID!=0) w->setAttribute("id", ID);
            objectDefinition->saveToXML(w);
            break;
        }
        case A4_SCRIPT_REMOVEBEHAVIOR:
        {
            w->setAttribute("id", ID);
            break;
        }
        case A4_SCRIPT_TELEPORT:
        {
            w->setAttribute("x", x);
            w->setAttribute("y", y);
            if (ID!=0) w->setAttribute("map", ID);
            break;
        }
        case A4_SCRIPT_GOTO:
        {
            w->setAttribute("x", x);
            w->setAttribute("y", y);
            if (ID!=0) w->setAttribute("map", ID);
            break;
        }
        case A4_SCRIPT_DIE:
            break;
        case A4_SCRIPT_USE:
        {
            if (x>=0) {
                w->setAttribute("x", x);
                w->setAttribute("y", y);
                if (ID!=0) w->setAttribute("map", ID);
            } else {
                if (ID!=0) w->setAttribute("inventory", ID);
            }
            break;
        }
        case A4_SCRIPT_OPENDOORS:
        {
            w->setAttribute("door", ID);
            break;
        }
        case A4_SCRIPT_MESSAGE:
        {
            w->setAttribute("text", text);
            break;
        }
        case A4_SCRIPT_TALK:
        {
            w->setAttribute("text", text);
            if (ID!=0) w->setAttribute("topic", ID);
            if (value!=A4_TALK_PERFORMATIVE_NONE) w->setAttribute("performative", A4Game::performativeNames[value]);
            if (angry) w->setAttribute("angry", "true");
            if (angry) w->setAttribute("wait", "true");
            break;
        }
        case A4_SCRIPT_PENDINGTALK:
        {
            w->setAttribute("character", ID);
            w->setAttribute("text", text);
            // subscripts:
            for(A4Script *s:subScripts) {
                s->saveToXML(w);
            }
            break;
        }
        case A4_SCRIPT_ADDTOPIC:
        {
            w->setAttribute("topic", ID);
            w->setAttribute("text", text);
            break;
        }
        case A4_SCRIPT_UPDATECONVERSATIONGRAPHTRANSITION:
        {
            const char *actorNames[]={"self","other"};
            w->setAttribute("actor", actorNames[value]);
            if (x!=-1) w->setAttribute("performative", A4Game::performativeNames[x]);
            if (ID!=0) w->setAttribute("keyword", ID);
            if (text!=0) w->setAttribute("state", text);
            w->setAttribute("consume", consume ? "true":"false");
            if (text2!=0) w->setAttribute("from", text2);
            for(A4Script *s:subScripts) s->saveToXML(w);
            break;
        }
        case A4_SCRIPT_STORYSTATE:
        {
            if (value==A4_STORYSTATE_GAME) w->setAttribute("scope", "game");
            if (value==A4_STORYSTATE_MAP) w->setAttribute("scope", "map");
            if (value==A4_STORYSTATE_OBJECT) w->setAttribute("scope", "object");
            w->setAttribute("variable", ID);
            w->setAttribute("value", text);
            break;
        }
        case A4_SCRIPT_ADDWME:
        {
            w->setAttribute("wme", text);
            w->setAttribute("activation", value);
            break;
        }
        case A4_SCRIPT_ADDWMETOOTHERS:
        {
            w->setAttribute("wme", text);
            w->setAttribute("activation", value);
            w->setAttribute("characterClass", ID);
            if (x==0) w->setAttribute("select", "all");
                 else w->setAttribute("select", "first");
            break;
        }
        case A4_SCRIPT_STEAL:
        {
            w->setAttribute("name", ID);
            break;
        }
        case A4_SCRIPT_GIVE:
        {
            if (ID!=0) w->setAttribute("inventory", ID);
            if (objectDefinition!=0) objectDefinition->saveToXML(w);
            break;
        }
        case A4_SCRIPT_SELL:
        {
            if (ID!=0) w->setAttribute("inventory", ID);
            if (objectDefinition!=0) objectDefinition->saveToXML(w);
            break;
        }
        case A4_SCRIPT_DROP:
        {
            if (ID!=0) w->setAttribute("inventory", ID);
            if (objectDefinition!=0) objectDefinition->saveToXML(w);
            break;
        }
        case A4_SCRIPT_LOSEITEM:
        {
            if (ID!=0) w->setAttribute("inventory", ID);
            break;
        }
        case A4_SCRIPT_GAINITEM:
        {
            if (objectDefinition!=0) objectDefinition->saveToXML(w);
            break;
        }
        case A4_SCRIPT_EXPERIENCEGAIN:
        {
            w->setAttribute("xp", value);
            break;
        }
        case A4_SCRIPT_DELAY:
        {
            w->setAttribute("delay", value);
            break;
        }
        case A4_SCRIPT_PLAYSOUND:
        {
            w->setAttribute("sound", ID);
            break;
        }
        case A4_SCRIPT_STOPSOUND:
        {
            w->setAttribute("sound", ID);
            break;
        }
        case A4_SCRIPT_IF:
        {
            w->openTag("condition");
            for(A4Script *s:subScripts) s->saveToXML(w);
            w->closeTag("condition");
            if (!thenSubScripts.empty()) {
                w->openTag("then");
                for(A4Script *s:thenSubScripts) s->saveToXML(w);
                w->closeTag("then");
            }
            if (!elseSubScripts.empty()) {
                w->openTag("else");
                for(A4Script *s:elseSubScripts) s->saveToXML(w);
                w->closeTag("else");
            }
            break;
        }
        case A4_SCRIPT_GAINGOLDOTHER:
        {
            w->setAttribute("gold", value);
            break;
        }
        case A4_SCRIPT_TALKOTHER:
        {
            w->setAttribute("text", text);
            if (ID!=0) w->setAttribute("topic", ID);
            w->setAttribute("performative", A4Game::performativeNames[value]);
            if (angry) w->setAttribute("angry", "true");
            if (wait) w->setAttribute("wait", "true");
            break;
        }
        case A4_SCRIPT_ADDCURRENTPOSITIONWME:
        {
            w->setAttribute("name", ID);
            w->setAttribute("activation", value);
            break;
        }
        case A4_SCRIPT_STARTTRADING:
            break;
        case A4_SCRIPT_FAMILIARWITHMAP:
        {
            w->setAttribute("map", ID);
            break;
        }
        case A4_SCRIPT_ADDAGENDA:
        {
            w->setAttribute("agenda", agenda->name);
            w->setAttribute("duration", agenda->duration);
            w->setAttribute("loop", agenda->loop ? "true":"false");
            w->setAttribute("absoluteTime", agenda->absoluteTime ? "true":"false");
            w->setAttribute("cycle", agenda->cycle);
            for(AgendaEntry *ae:agenda->m_entries) {
                w->openTag("entry");
                w->setAttribute("time", ae->time);
                for(A4Script *s:ae->m_scripts) {
                    s->saveToXML(w);
                }
                w->closeTag("entry");
            }
            break;
        }
        case A4_SCRIPT_REMOVEAGENDA:
        {
            w->setAttribute("agenda", ID);
            break;
        }
        case A4_SCRIPT_GAINGOLD:
        {
            w->setAttribute("gold", value);
            break;
        }
        case A4_SCRIPT_DROPGOLD:
        {
            w->setAttribute("gold", value);
            break;
        }
        case A4_SCRIPT_TAKE:
        {
            if (x>=0) {
                w->setAttribute("x", x);
                w->setAttribute("y", y);
            }
            if (ID!=0) w->setAttribute("map", ID);
            break;
        }
        case A4_SCRIPT_EQUIP:
        {
            if (ID!=0) w->setAttribute("inventory", ID);
            break;
        }
        case A4_SCRIPT_UNEQUIP:
        {
            if (ID!=0) w->setAttribute("inventory", ID);
            break;
        }
        case A4_SCRIPT_INTERACT:
        {
            if (x>=0) {
                w->setAttribute("x", x);
                w->setAttribute("y", y);
            }
            if (ID!=0) w->setAttribute("map", ID);
            break;
        }
            
        case A4_SCRIPT_EMBARK:
        {
            if (x>=0) {
                w->setAttribute("x", x);
                w->setAttribute("y", y);
            }
            if (ID!=0) w->setAttribute("map", ID);
            break;
        }
            
        case A4_SCRIPT_DISEMBARK:
        {
            if (x>=0) {
                w->setAttribute("x", x);
                w->setAttribute("y", y);
            }
            if (ID!=0) w->setAttribute("map", ID);
            break;
        }
            
        case A4_SCRIPT_ATTACK:
        {
            w->setAttribute("target", ID);
            if (wait) w->setAttribute("untilDead", "true");
            break;
        }
            
        case A4_SCRIPT_SPELL:
        {
            w->setAttribute("spell", ID);
            if (text!=0) w->setAttribute("direction", text);
            break;
        }
            
        case A4_SCRIPT_BUY:
        {
            w->setAttribute("seller", ID);
            w->setAttribute("object", text);
            break;
        }
            
        case A4_SCRIPT_CHOP:
        {
            if (x>=0) {
                w->setAttribute("x", x);
                w->setAttribute("y", y);
            }
            if (ID!=0) w->setAttribute("map", ID);
            break;
        }
    }
    w->closeTag(scriptNames[m_type]);
}


int A4Script::execute(A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
	return (*(A4Script::scriptFunctions[m_type]))(this, o, map, game, otherCharacter);
}


void A4Script::reset() {
    state = 0;
    if_state = 0;
}

/* ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
                        SCRIPT FUNCTIONS
   ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----*/

int execute_gameComplete(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    game->setGameComplete(true, script->ID);
	return SCRIPT_FINISHED;
}

int execute_addBehavior(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    A4Behavior *b = A4Behavior::createBehaviorFromXML(script->objectDefinition, game->getOntology());
//    A4Behavior *b = script->createBehaviorFromJSString(script->text, game->getOntology());
    if (b==0) return SCRIPT_FAILED;
    b->setPriority(script->value);
    if (script->ID!=0) {
        char *tmp = new char[strlen(script->ID)+1];
        strcpy(tmp,script->ID);
        b->setID(tmp);
    }
    if (o->isAICharacter()) {
        ((A4AICharacter *)o)->addBehavior(b);
    }
    return SCRIPT_FINISHED;
}

int execute_removeBehavior(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isAICharacter()) {
        bool match = false;
        A4AI *ai = ((A4AICharacter *)o)->getAI();
        std::list<A4Behavior *> todelete;
        for(A4Behavior *b:*(ai->getBehaviors())) {
            if (b->getID()!=0 && strcmp(b->getID(),script->ID)==0) {
                // match!
                todelete.push_back(b);
                match = true;
            }
        }
        for(A4Behavior *b:todelete) {
            ai->getBehaviors()->remove(b);
        }
        todelete.clear();
        if (match) return SCRIPT_FINISHED;
    }
    return SCRIPT_FAILED;
}

int execute_teleport(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (script->ID!=0) {
        Symbol *map_s = new Symbol(script->ID);
        map = game->getMap(map_s);
        delete map_s;
    }
    game->requestWarp(o, map, script->x, script->y, o->getLayer());
    return SCRIPT_FINISHED;
}

int execute_goto(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isAICharacter()) {
        int priority = 10;
        A4AICharacter *aic = (A4AICharacter *)o;
        A4AI *ai = aic->getAI();
        if (script->ID!=0) {
            Symbol *map_s = new Symbol(script->ID);
            map = game->getMap(map_s);
            delete map_s;
        }
        if (o->getX()==script->x && o->getY()==script->y && o->getMap()==map) {
            return SCRIPT_FINISHED;
        } else {
            WME *wme = new WME(A4AI::s_object_symbol,
                               WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                               WMEParameter(script->x), WME_PARAMETER_INTEGER,
                               WMEParameter(script->y), WME_PARAMETER_INTEGER,
                               WMEParameter(script->x + o->getPixelWidth()), WME_PARAMETER_INTEGER,
                               WMEParameter(script->y + o->getPixelHeight()), WME_PARAMETER_INTEGER,
                               WMEParameter(new Symbol(map->getNameSymbol())),WME_PARAMETER_SYMBOL,
                               0);
            ai->addPFTargetWME(wme, game, A4CHARACTER_COMMAND_IDLE, priority, false);
            delete wme;
            return SCRIPT_NOT_FINISHED;
        }
    } else {
        return SCRIPT_FAILED;
    }
}

int execute_die(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    o->die();
    return SCRIPT_FINISHED;
}

int execute_use(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        A4Character *c = (A4Character *)o;
        int priority = 10;
        
        if (script->x==-1 && script->y==-1 && script->ID==0) {
            // use an object in the current position:
            if (c->isIdle()) {
                // activte lever:
                if (!c->use(game)) return SCRIPT_FAILED;
                return SCRIPT_FINISHED;
            } else {
                return SCRIPT_NOT_FINISHED;
            }
        } else if (script->x>=0 && script->y>=0) {
            // x,y,map version:
            if (o->isAICharacter()) {
                A4AICharacter *aic = (A4AICharacter *)o;
                A4AI *ai = aic->getAI();
                if (script->ID!=0) {
                    Symbol *map_s = new Symbol(script->ID);
                    map = game->getMap(map_s);
                    delete map_s;
                }
                if (o->getX()==script->x && o->getY()==script->y && o->getMap()==map) {
                    if (aic->isIdle()) {
                        // activte lever:
                        if (!aic->use(game)) return SCRIPT_FAILED;
                        return SCRIPT_FINISHED;
                    } else {
                        return SCRIPT_NOT_FINISHED;
                    }
                } else {
                    WME *wme = new WME(A4AI::s_object_symbol,
                                       WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x + o->getPixelWidth()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y + o->getPixelHeight()), WME_PARAMETER_INTEGER,
                                       WMEParameter(new Symbol(map->getNameSymbol())),WME_PARAMETER_SYMBOL,
                                       0);
                    ai->addPFTargetWME(wme, game, A4CHARACTER_COMMAND_IDLE, priority, false);
                    delete wme;
                    return SCRIPT_NOT_FINISHED;
                }
            } else {
                return SCRIPT_FAILED;
            }
        } else {
            // use an item in the inventory:
            if (c->isIdle()) {
                for(A4Object *o2:*(((A4Character *)o)->getInventory())) {
                    if (strstr(o2->getName()->get(),script->ID)!=0) {
                        // match!
                        if (o2->getUsable()) {
                            o2->event(A4_EVENT_USE, c, map, game);
                            c->eventWithObject(A4_EVENT_ACTION_USE, 0, o, map, game);
                        } else {
                            return SCRIPT_FAILED;
                        }
                        return SCRIPT_FINISHED;
                    }
                }
                return SCRIPT_FAILED;
            }
        }
        
    } else {
        return SCRIPT_FAILED;
    }
    return SCRIPT_FINISHED;
}

int execute_openDoors(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    map->triggerObjectsEventWithID(A4_EVENT_OPEN, script->ID, otherCharacter, map, game);
    return SCRIPT_FINISHED;
}

int execute_message(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    game->addMessage(script->text);
    return SCRIPT_FINISHED;
}

int execute_talk(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (((A4Character *)o)->getTalkingState()==A4CHARACTER_STATE_IDLE) {
        if (script->state==0) {
            char *tmp_ID = 0;
            if (script->ID!=0) {
                char *tmp_ID = new char[strlen(script->ID)+1];
                strcpy(tmp_ID,script->ID);
            }
            char *tmp = new char[strlen(script->text)+1];
            strcpy(tmp,script->text);
            SpeechAct *sa = new SpeechAct(script->value, tmp_ID, tmp);
            if (script->angry) {
                ((A4Character *)o)->issueCommand(A4CHARACTER_COMMAND_TALK_ANGRY, sa, A4_DIRECTION_NONE, otherCharacter, game);
            } else {
                ((A4Character *)o)->issueCommand(A4CHARACTER_COMMAND_TALK, sa, A4_DIRECTION_NONE, otherCharacter, game);
            }
            delete sa;
            script->state = 1;
            if (!script->wait) return SCRIPT_FINISHED;
            return SCRIPT_NOT_FINISHED;
        } else {
            return SCRIPT_FINISHED;
        }
    } else {
        return SCRIPT_NOT_FINISHED;
    }
}

int execute_pendingTalk(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isAICharacter()) {
        A4AI *ai = ((A4AICharacter *)o)->getAI();
        ai->addPendingTalk(new A4Script(script));
    }
    return SCRIPT_FINISHED;
}

int execute_addTopic(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    output_debug_message("execute_addTopic: '%s' '%s'\n",script->ID, script->text);
    game->addSpeechAct(A4_TALK_PERFORMATIVE_ASK, script->ID, script->text);
    return SCRIPT_FINISHED;
}


int execute_updateConversationGraphTransition(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isAICharacter()) {
        ConversationGraphTransition *t = new ConversationGraphTransition();
        t->m_trigger_actor = script->value;
        t->m_trigger_performative = script->x;
        if (script->ID!=0) t->m_trigger_keyword = new Symbol(script->ID);
        t->m_consume = script->consume;
        if (script->text!=0) t->m_state = new Symbol(script->text);
        Symbol *from = new Symbol(script->text2);
        A4AI *ai = ((A4AICharacter *)o)->getAI();
        for(A4Script *s:script->subScripts) {
            t->m_effects.push_back(new A4Script(s));
        }
        ConversationGraph *cg = ai->getConversationGraph();
        if (cg==0) {
            cg = new ConversationGraph();
            ai->setConversationGraph(cg);
        }
        cg->addConversationGraphTransition(from, t);
        delete from;
        return SCRIPT_FINISHED;
    } else {
        return SCRIPT_FAILED;
    }
}

int execute_storyState(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    switch(script->value) {
        case A4_STORYSTATE_GAME:
            game->setStoryStateVariable(script->ID,script->text);
            break;
        case A4_STORYSTATE_MAP:
            map->setStoryStateVariable(script->ID,script->text);
            break;
        case A4_STORYSTATE_OBJECT:
            o->setStoryStateVariable(script->ID,script->text);
            break;
    }
    return SCRIPT_FINISHED;
}

int execute_addWME(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    Expression *exp = Expression::from_string(script->text);
    if (exp==0) return SCRIPT_FINISHED;
    WME *wme = new WME(exp, game->getOntology(), script->value);
    delete exp;
    ((A4AICharacter *)o)->getAI()->addShortTermWME(wme);
	return SCRIPT_FINISHED;
}

int execute_addWMEToOthers(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    Expression *exp = Expression::from_string(script->text);
    if (exp==0) return SCRIPT_FINISHED;
    
    A4AI *ai = ((A4AICharacter *)o)->getAI();
    Sort *sort = game->getOntology()->getSort(script->ID);
    
    for(int i = 0;i<ai->getObjectPerceptionCacheSize();i++) {
        A4Object *o2 = ai->getObjectPerceptionCache()[i];
        if (o2->is_a(sort)) {
            WME *wme = new WME(exp, game->getOntology(), script->value);
            ((A4AICharacter *)o2)->getAI()->addShortTermWME(wme);
            if (script->x!=0) { // if "select = first":
                delete exp;
                return SCRIPT_FINISHED;
            }
        }
    }
    
    delete exp;
    return SCRIPT_FINISHED;
}

int execute_steal(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (script->ID!=0 && otherCharacter!=0) {
        // it's from the inventory:
        for(A4Object *o2:*(otherCharacter->getInventory())) {
            if (strstr(o2->getName()->get(),script->ID)!=0) {
                // match!
                otherCharacter->removeFromInventory(o2);
                ((A4Character *)o)->addObjectToInventory(o2, game);
                return SCRIPT_FINISHED;
            }
        }
    }
    return SCRIPT_FAILED;
}

int execute_give(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (otherCharacter==0) return SCRIPT_FAILED;
    A4Object *item = 0;
    if (script->ID!=0) {
        // it's from the inventory:
        for(A4Object *o2:*(((A4Character *)o)->getInventory())) {
            if (strstr(o2->getName()->get(),script->ID)!=0) {
                // match!
                item = o2;
                break;
            }
        }
    } else if (script->objectDefinition!=0) {
        // it's a new item:
        A4Object *item = game->getObjectFactory()->createObject(script->objectDefinition->get_attribute("class"), game, false, false);
        item->loadObjectAdditionalContent(script->objectDefinition, game, game->getObjectFactory(), 0);
//    } else if (script->text!=0) {
        // it's a new item:
//        item = game->getObjectFactory()->createObjectFromJSString(script->text, game);
    }
    if (item==0) {
        return SCRIPT_FAILED;
    } else {
        ((A4Character *)o)->removeFromInventory(item);
        otherCharacter->addObjectToInventory(item, game);
        return SCRIPT_FINISHED;
    }
}

int execute_sell(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (otherCharacter==0) return SCRIPT_FAILED;
    A4Object *item = 0;
    if (script->ID!=0) {
        // it's from the inventory:
        for(A4Object *o2:*(((A4Character *)o)->getInventory())) {
            if (strstr(o2->getName()->get(),script->ID)!=0) {
                // match!
                item = o2;
                break;
            }
        }
    } else if (script->objectDefinition!=0) {
        // it's a new item:
        A4Object *item = game->getObjectFactory()->createObject(script->objectDefinition->get_attribute("class"), game, false, false);
        item->loadObjectAdditionalContent(script->objectDefinition, game, game->getObjectFactory(), 0);
//    } else if (script->text!=0) {
        // it's a new item:
//        item = game->getObjectFactory()->createObjectFromJSString(script->text, game);
    }
    if (item==0) {
        return SCRIPT_FAILED;
    } else {
        if (otherCharacter->getGold()>=item->getGold()) {
            otherCharacter->setGold(otherCharacter->getGold() - item->getGold());
            o->setGold(o->getGold() + item->getGold());
            ((A4Character *)o)->removeFromInventory(item);
            otherCharacter->addObjectToInventory(item, game);
            return SCRIPT_FINISHED;
        } else {
            return SCRIPT_FAILED;
        }
    }
    return SCRIPT_FINISHED;
}

int execute_drop(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    A4Object *item = 0;
    if (script->ID!=0) {
        // it's from the inventory:
        for(A4Object *o2:*(((A4Character *)o)->getInventory())) {
            if (strstr(o2->getName()->get(),script->ID)!=0) {
                // match!
                item = o2;
                break;
            }
        }
    } else if (script->objectDefinition!=0) {
        // it's a new item:
        item = game->getObjectFactory()->createObject(script->objectDefinition->get_attribute("class"), game, false, false);
        item->loadObjectAdditionalContent(script->objectDefinition, game, game->getObjectFactory(), 0);
    }
    if (item==0) {
        return SCRIPT_FAILED;
    } else {
        ((A4Character *)o)->removeFromInventory(item);
        game->requestWarp(item, map, o->getX(), o->getY(), A4_LAYER_FG);
        return SCRIPT_FINISHED;
    }
    return SCRIPT_FINISHED;
}

int execute_loseItem(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        for(A4Object *o2:*(((A4Character *)o)->getInventory())) {
            if (strstr(o2->getName()->get(),script->ID)!=0) {
                // match!
                ((A4Character *)o)->removeFromInventory(o2);
                game->requestDeletion(o2);
                return SCRIPT_FINISHED;
            }
        }
    } else if (otherCharacter!=0 && otherCharacter->isCharacter()) {
        for(A4Object *o2:*(((A4Character *)otherCharacter)->getInventory())) {
            if (strstr(o2->getName()->get(),script->ID)!=0) {
                // match!
                ((A4Character *)otherCharacter)->removeFromInventory(o2);
                game->requestDeletion(o2);
                return SCRIPT_FINISHED;
            }
        }
    }
    return SCRIPT_FAILED;
}

int execute_gainItem(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    A4Object *o2 = game->getObjectFactory()->createObject(script->objectDefinition->get_attribute("class"), game, false, false);
    o2->loadObjectAdditionalContent(script->objectDefinition, game, game->getObjectFactory(), 0);
    
//    A4Object *o2 = game->getObjectFactory()->createObjectFromJSString(script->ID, game);
    if (o2==0) output_debug_message("Cannot generate object '%s' in execute_gainItem\n", script->ID);
    if (otherCharacter->getInventory()->size()<A4_INVENTORY_SIZE) {
        otherCharacter->addObjectToInventory(o2, game);
    } else {
        game->requestWarp(o2, otherCharacter->getMap(), otherCharacter->getX(), otherCharacter->getY(), A4_LAYER_FG);
    }
    return SCRIPT_FINISHED;
}

int execute_experienceGain(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isPlayer()) {
        ((A4PlayerCharacter *)o)->gainXp(script->value);
    }
    return SCRIPT_FINISHED;
}

int execute_delay(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    script->state++;
    if (script->state >= script->value) {
        script->state = 0;
        return SCRIPT_FINISHED;
    } else {
        return SCRIPT_NOT_FINISHED;
    }
}

int execute_playSound(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    // only play if it's in the same map as the current player:
    if (map==game->getCurrentPlayer()->getMap()) game->playSound(script->ID);
    return SCRIPT_FINISHED;
}

int execute_stopSound(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    output_debug_message("stopSound not supported in the C++ version yet!\n");
    return SCRIPT_FINISHED;
}

int execute_if(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    switch(script->if_state) {
        case 0: // condition:
            {
                std::vector<A4Script *>::iterator it = script->subScripts.begin();
                for(int i = 0;i<script->state;i++) it++;
                if (it == script->subScripts.end()) {
                    // reaching the end, assume everything went fine, swith to "then":
                    script->if_state = 1;
                    script->state = 0;
                    return SCRIPT_NOT_FINISHED;
                } else {
                    switch((*it)->execute(o, map, game, otherCharacter)) {
                    case SCRIPT_FAILED:
                        script->if_state = 2;
                        script->state = 0;
                        return SCRIPT_NOT_FINISHED;
                    case SCRIPT_NOT_FINISHED:
                        return SCRIPT_NOT_FINISHED;
                    case SCRIPT_FINISHED:
                        script->state++;
                        return SCRIPT_NOT_FINISHED;
                    }
                }
            }
            break;
        case 1: // then:
            {
                std::vector<A4Script *>::iterator it = script->thenSubScripts.begin();
                for(int i = 0;i<script->state;i++) it++;
                if (it == script->thenSubScripts.end()) return SCRIPT_FINISHED;
                else {
                    switch((*it)->execute(o, map, game, otherCharacter)) {
                        case SCRIPT_FAILED:
                            return SCRIPT_FAILED;
                        case SCRIPT_NOT_FINISHED:
                            return SCRIPT_NOT_FINISHED;
                        case SCRIPT_FINISHED:
                            script->state++;
                            return SCRIPT_NOT_FINISHED;
                    }
                }
            }
            break;
        case 2: // else:
            {
                std::vector<A4Script *>::iterator it = script->elseSubScripts.begin();
                for(int i = 0;i<script->state;i++) it++;
                if (it == script->elseSubScripts.end()) return SCRIPT_FINISHED;
                else {
                    switch((*it)->execute(o, map, game, otherCharacter)) {
                        case SCRIPT_FAILED:
                            return SCRIPT_FAILED;
                        case SCRIPT_NOT_FINISHED:
                            return SCRIPT_NOT_FINISHED;
                        case SCRIPT_FINISHED:
                            script->state++;
                            return SCRIPT_NOT_FINISHED;
                    }
                }
            }
            break;
    }
    return SCRIPT_FAILED;
}

int execute_gainGoldOther(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
	otherCharacter->setGold(otherCharacter->getGold() + script->value);
	return SCRIPT_FINISHED;
}

int execute_talkOther(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (otherCharacter->getTalkingState()==A4CHARACTER_STATE_IDLE) {
        if (script->state==0) {
            char *tmp_ID = 0;
            if (script->ID!=0) {
                char *tmp_ID = new char[strlen(script->ID)+1];
                strcpy(tmp_ID,script->ID);
            }
            char *tmp = new char[strlen(script->text)+1];
            strcpy(tmp,script->text);
            SpeechAct *sa = new SpeechAct(script->value, tmp_ID, tmp);
            if (script->angry) {
                otherCharacter->issueCommand(A4CHARACTER_COMMAND_TALK_ANGRY, sa, A4_DIRECTION_NONE, 0, game);
            } else {
                otherCharacter->issueCommand(A4CHARACTER_COMMAND_TALK, sa, A4_DIRECTION_NONE, 0, game);
            }
            delete sa;
            script->state = 1;
            if (!script->wait) return SCRIPT_FINISHED;
            return SCRIPT_NOT_FINISHED;
        } else {
            return SCRIPT_FINISHED;
        }
    } else {
        return SCRIPT_NOT_FINISHED;
    }
}

int execute_addCurrentPositionWME(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    WME *wme = new WME(A4AI::s_location_symbol,
                       WMEParameter(new Symbol(script->ID)), WME_PARAMETER_SYMBOL,
                       WMEParameter(o->getX()), WME_PARAMETER_INTEGER,
                       WMEParameter(o->getY()), WME_PARAMETER_INTEGER,
                       WMEParameter(o->getX() + o->getPixelWidth()), WME_PARAMETER_INTEGER,
                       WMEParameter(o->getY() + o->getPixelHeight()), WME_PARAMETER_INTEGER,
                       WMEParameter(new Symbol(o->getMap()->getNameSymbol())), WME_PARAMETER_SYMBOL,
                       script->value);
    ((A4AICharacter *)o)->getAI()->addShortTermWME(wme);
    return SCRIPT_FINISHED;
}

int execute_startTrading(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    // if any of the players involved in the trade is the player, then pull-up the trade dialog:
    if (game->getCurrentPlayer() == o) {
        game->setTradeRequested(otherCharacter);
    } else if (game->getCurrentPlayer() == otherCharacter) {
        game->setTradeRequested((A4Character *)o);
    }
    return SCRIPT_FINISHED;
}

int execute_familiarWithMap(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (!o->isAICharacter()) return SCRIPT_FAILED;
    AIMemory *m = ((A4AICharacter *)o)->getAI()->getMemory();
    
    Symbol *map_tf_s = new Symbol(script->ID);
    A4Map *map_tf = game->getMap(map_tf_s);
    delete map_tf_s;
    
    if (map_tf==0) {
        return SCRIPT_FAILED;
    } else {
        for(A4MapBridge *b:*map_tf->getBridges()) {
            // perceived a bridge:
            WME *wme = new WME(A4AI::s_bridge_symbol,
                               WMEParameter(new Symbol(b->m_linkedTo->getMap()->getNameSymbol())), WME_PARAMETER_SYMBOL,
                               WMEParameter(b->getX()), WME_PARAMETER_INTEGER,
                               WMEParameter(b->getY()), WME_PARAMETER_INTEGER,
                               WMEParameter(b->getX() + b->m_dx), WME_PARAMETER_INTEGER,
                               WMEParameter(b->getY() + b->m_dy), WME_PARAMETER_INTEGER,
                               WMEParameter(new Symbol(map_tf->getName())), WME_PARAMETER_SYMBOL,
                               m->getFreezeThreshold());
            wme->setSource(b);
            m->addLongTermWME(wme);
        }
        return SCRIPT_FINISHED;
    }
}

int execute_addAgenda(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    o->addAgenda(new Agenda(script->agenda));
    return SCRIPT_FINISHED;
}

int execute_removeAgenda(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    o->removeAgenda(script->ID);
    return SCRIPT_FINISHED;
}

int execute_eventRule(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    o->addEventRule(script->rule->getEvent(), new A4EventRule(script->rule));
    return SCRIPT_FINISHED;
}

int execute_gainGold(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    o->setGold(o->getGold() + script->value);
    return SCRIPT_FINISHED;
}

int execute_dropGold(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->getGold()>=script->value) {
        o->setGold(o->getGold() - script->value);
        A4Object *cp = new A4CoinPurse(game->getOntology()->getSort("CoinPurse"), script->value, new Animation(game->getCoinPurseAnimation()), true);
        game->requestWarp(cp, map, o->getX(), o->getY(), A4_LAYER_FG);
        map->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_drop, cp->getID(), cp->getSort(), cp->getID(), cp->getSort(), cp->getX(), cp->getY(), cp->getX()+cp->getPixelWidth(), cp->getY()+cp->getPixelHeight()));
        o->event(A4_EVENT_ACTION_DROP_GOLD, 0, map, game);
        return SCRIPT_FINISHED;
    } else {
        return SCRIPT_FAILED;
    }
}


int execute_take(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isAICharacter()) {
        int priority = 10;
        A4AICharacter *aic = (A4AICharacter *)o;
        A4AI *ai = aic->getAI();
        if (script->ID!=0) {
            Symbol *map_s = new Symbol(script->ID);
            map = game->getMap(map_s);
            delete map_s;
        }
        if (o->getX()==script->x && o->getY()==script->y && o->getMap()==map) {
            if (aic->isIdle()) {
                // take:
                if (!aic->take(game)) return SCRIPT_FAILED;
                return SCRIPT_FINISHED;
            } else {
                return SCRIPT_NOT_FINISHED;
            }
        } else {
            WME *wme = new WME(A4AI::s_object_symbol,
                               WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                               WMEParameter(script->x), WME_PARAMETER_INTEGER,
                               WMEParameter(script->y), WME_PARAMETER_INTEGER,
                               WMEParameter(script->x + o->getPixelWidth()), WME_PARAMETER_INTEGER,
                               WMEParameter(script->y + o->getPixelHeight()), WME_PARAMETER_INTEGER,
                               WMEParameter(new Symbol(map->getNameSymbol())),WME_PARAMETER_SYMBOL,
                               0);
            ai->addPFTargetWME(wme, game, A4CHARACTER_COMMAND_IDLE, priority, false);
            delete wme;
            return SCRIPT_NOT_FINISHED;
        }
    } else {
        return SCRIPT_FAILED;
    }
    return SCRIPT_FINISHED;
}


int execute_equip(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        A4Character *c = (A4Character *)o;
        if (c->isIdle()) {
            for(A4Object *o2:*(((A4Character *)o)->getInventory())) {
                if (strstr(o2->getName()->get(),script->ID)!=0) {
                    // match!
                    
                    if (o2->isEquipable()) {
                        A4EquipableItem *ei = (A4EquipableItem *)o2;
                        int slot = ei->getEquipableSlot();
                        if (c->getEquipment(slot)!=0) {
                            // unequip first:
                            A4Object *tmp = c->getEquipment(slot);
                            c->setEquipment(slot, 0);
                            c->addObjectToInventory(tmp, game);
                            tmp->event(A4_EVENT_UNEQUIP, c, map, game);
                            c->eventWithObject(A4_EVENT_ACTION_UNEQUIP, 0, tmp, c->getMap(), game);
                        }
                        if (c->getEquipment(slot)==0) {
                            // equip:
                            c->removeFromInventory(o2);	// make space to unequip
                            c->setEquipment(slot, o2);
                            o2->event(A4_EVENT_EQUIP, c, map, game);
                            c->eventWithObject(A4_EVENT_ACTION_EQUIP, 0, o, c->getMap(), game);
                        }
                        return SCRIPT_FINISHED;
                    } else {
                        return SCRIPT_FAILED;
                    }
                }
            }
            return SCRIPT_FAILED;
        } else {
            return SCRIPT_NOT_FINISHED;
        }
    }
    return SCRIPT_FAILED;
}


int execute_unequip(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        A4Character *c = (A4Character *)o;
        for(int i = 0;i<A4_EQUIPABLE_N_SLOTS;i++) {
            A4Object *tmp = c->getEquipment(i);
            if (strstr(tmp->getName()->get(),script->ID)!=0) {
                // match!
                c->setEquipment(i, 0);
                c->addObjectToInventory(tmp, game);
                tmp->event(A4_EVENT_UNEQUIP, c, map, game);
                c->eventWithObject(A4_EVENT_ACTION_UNEQUIP, 0, tmp, c->getMap(), game);
            }
        }
    }
    return SCRIPT_FINISHED;
}


int execute_interact(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        A4Character *c = (A4Character *)o;
        int priority = 10;
        
        if (script->x==-1 && script->y==-1 && script->ID==0) {
            // you need to specify a target position to interact, since interacting requires a direction
            return SCRIPT_FAILED;
        } else if (script->x>=0 && script->y>=0) {
            // x,y,map version:
            if (o->isAICharacter()) {
                A4AICharacter *aic = (A4AICharacter *)o;
                A4AI *ai = aic->getAI();
                if (script->ID!=0) {
                    Symbol *map_s = new Symbol(script->ID);
                    map = game->getMap(map_s);
                    delete map_s;
                }
                
                int interactDirection = A4_DIRECTION_NONE;
                if (o->getMap()==map) {
                    if (o->getX()+o->getPixelWidth() == script->x && o->getY() == script->y) interactDirection = A4_DIRECTION_RIGHT;
                    if (o->getX()-o->getPixelWidth() == script->x && o->getY() == script->y) interactDirection = A4_DIRECTION_LEFT;
                    if (o->getX() == script->x && o->getY()+o->getPixelHeight() == script->y) interactDirection = A4_DIRECTION_DOWN;
                    if (o->getX() == script->x && o->getY()-o->getPixelHeight() == script->y) interactDirection = A4_DIRECTION_UP;
                }
                
                if (interactDirection != A4_DIRECTION_NONE) {
                    if (aic->isIdle()) {
                        // character is on position:
                        c->issueCommand(A4CHARACTER_COMMAND_INTERACT,0,interactDirection, 0, game);
                        return SCRIPT_FINISHED;
                    } else {
                        return SCRIPT_NOT_FINISHED;
                    }
                } else {
                    WME *wme = new WME(A4AI::s_object_symbol,
                                       WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x + o->getPixelWidth()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y + o->getPixelHeight()), WME_PARAMETER_INTEGER,
                                       WMEParameter(new Symbol(map->getNameSymbol())),WME_PARAMETER_SYMBOL,
                                       0);
                    ai->addPFTargetWME(wme, game, A4CHARACTER_COMMAND_IDLE, priority, false);
                    delete wme;
                    return SCRIPT_NOT_FINISHED;
                }
            } else {
                return SCRIPT_FAILED;
            }
        }
    } else {
        return SCRIPT_FAILED;
    }
    return SCRIPT_FINISHED;
}


int execute_embark(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        A4Character *c = (A4Character *)o;
        int priority = 10;
        
        if (script->x==-1 && script->y==-1 && script->ID==0) {
            // embark on the transport in the current position
            A4Object *v = c->getMap()->getVehicleObject(c->getX() + c->getPixelWidth()/2 - 1, c->getY() + c->getPixelHeight()/2 - 1, 2, 2);
            if (v!=0) {
                c->embark((A4Vehicle *)v);
                c->getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_embark, c->getID(), c->getSort(), v->getID(), v->getSort(), c->getX(), c->getY(), c->getX()+c->getPixelWidth(), c->getY()+c->getPixelHeight()));
                return SCRIPT_FINISHED;
            }
            return SCRIPT_FAILED;
        } else if (script->x>=0 && script->y>=0) {
            // x,y,map version:
            if (o->isAICharacter()) {
                A4AICharacter *aic = (A4AICharacter *)o;
                A4AI *ai = aic->getAI();
                if (script->ID!=0) {
                    Symbol *map_s = new Symbol(script->ID);
                    map = game->getMap(map_s);
                    delete map_s;
                }
                if (o->getX()==script->x && o->getY()==script->y && o->getMap()==map) {
                    if (aic->isIdle()) {
                        // character is on position:
                        A4Object *v = c->getMap()->getVehicleObject(c->getX() + c->getPixelWidth()/2 - 1, c->getY() + c->getPixelHeight()/2 - 1, 2, 2);
                        if (v!=0) {
                            c->embark((A4Vehicle *)v);
                            c->getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_embark, c->getID(), c->getSort(), v->getID(), v->getSort(), c->getX(), c->getY(), c->getX()+c->getPixelWidth(), c->getY()+c->getPixelHeight()));
                            return SCRIPT_FINISHED;
                        }
                        return SCRIPT_FAILED;
                    } else {
                        return SCRIPT_NOT_FINISHED;
                    }
                } else {
                    WME *wme = new WME(A4AI::s_object_symbol,
                                       WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x + o->getPixelWidth()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y + o->getPixelHeight()), WME_PARAMETER_INTEGER,
                                       WMEParameter(new Symbol(map->getNameSymbol())),WME_PARAMETER_SYMBOL,
                                       0);
                    ai->addPFTargetWME(wme, game, A4CHARACTER_COMMAND_IDLE, priority, false);
                    delete wme;
                    return SCRIPT_NOT_FINISHED;
                }
            } else {
                return SCRIPT_FAILED;
            }
        }
    } else {
        return SCRIPT_FAILED;
    }
    return SCRIPT_FINISHED;
}


int execute_disembark(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        A4Character *c = (A4Character *)o;
        int priority = 10;
        
        if (!c->isInVehicle()) return SCRIPT_FAILED;
        
        if (script->x==-1 && script->y==-1 && script->ID==0) {
            // disembark on the transport in the current position
            c->disembark();
            c->getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_disembark, c->getID(), c->getSort(), c->getVehicle()->getID(), c->getVehicle()->getSort(), c->getX(), c->getY(), c->getX()+c->getPixelWidth(), c->getY()+c->getPixelHeight()));
            return SCRIPT_FINISHED;
        } else if (script->x>=0 && script->y>=0) {
            // x,y,map version:
            if (o->isAICharacter()) {
                A4AICharacter *aic = (A4AICharacter *)o;
                A4AI *ai = aic->getAI();
                if (script->ID!=0) {
                    Symbol *map_s = new Symbol(script->ID);
                    map = game->getMap(map_s);
                    delete map_s;
                }
                if (o->getX()==script->x && o->getY()==script->y && o->getMap()==map) {
                    // character is on position:
                    c->getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_disembark, c->getID(), c->getSort(), c->getVehicle()->getID(), c->getVehicle()->getSort(), c->getX(), c->getY(), c->getX()+c->getPixelWidth(), c->getY()+c->getPixelHeight()));
                    c->disembark();
                    return SCRIPT_FINISHED;
                } else {
                    WME *wme = new WME(A4AI::s_object_symbol,
                                       WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x + o->getPixelWidth()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y + o->getPixelHeight()), WME_PARAMETER_INTEGER,
                                       WMEParameter(new Symbol(map->getNameSymbol())),WME_PARAMETER_SYMBOL,
                                       0);
                    ai->addPFTargetWME(wme, game, A4CHARACTER_COMMAND_IDLE, priority, false);
                    delete wme;
                    return SCRIPT_NOT_FINISHED;
                }
            } else {
                return SCRIPT_FAILED;
            }
        }
    } else {
        return SCRIPT_FAILED;
    }
    return SCRIPT_FINISHED;
}


int execute_attack(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isAICharacter()) {
        A4AICharacter *aic = (A4AICharacter *)o;
        A4AI *ai = aic->getAI();
        int priority = 10;
        
        WME *pattern = new WME(A4AI::s_object_symbol,
                              WMEParameter(game->getOntology()->getSort(script->ID)), WME_PARAMETER_SORT,
                              WMEParameter(0), WME_PARAMETER_WILDCARD,
                              WMEParameter(0), WME_PARAMETER_WILDCARD,
                              WMEParameter(0), WME_PARAMETER_WILDCARD,
                              WMEParameter(0), WME_PARAMETER_WILDCARD,
                              WMEParameter(0), WME_PARAMETER_WILDCARD,
                              0);
        
        WME *wme = ai->getMemory()->retrieveFirstByRelativeSubsumption(pattern);
        delete pattern;
        
        if (wme==0) {
            if (script->wait) return SCRIPT_FINISHED;   // when we don't see the target anymore, we are done
            return SCRIPT_FAILED;
        } else {
            ai->addPFTargetWME(wme, game, A4CHARACTER_COMMAND_ATTACK, priority, false);

            if (!script->wait) {
                pattern = new WME(A4AI::s_action_attack,
                                   WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                                   WMEParameter(wme->getParameter(0).m_integer), WME_PARAMETER_INTEGER,
                                   0);
                wme = ai->getMemory()->retrieveFirstBySubsumption(pattern);
                delete pattern;
                if (wme!=0) return SCRIPT_FINISHED;
            }
            return SCRIPT_NOT_FINISHED;
        }
    }
    
    return SCRIPT_FAILED;
}


int execute_spell(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        A4Character *c = (A4Character *)o;
        for(int i = 0;i<A4_N_SPELLS;i++) {
            if (strcmp(script->ID,A4Game::spellNames[i])==0) {
                if (c->castSpell(i, script->value, game, false)) {
                    return SCRIPT_FINISHED;
                } else {
                    return SCRIPT_FAILED;
                }
            }
        }
        return false;
    }
    return SCRIPT_FAILED;
}


int execute_buy(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        A4Character *c = (A4Character *)o;
        if (script->ID!=0 && otherCharacter!=0) {
            for(A4Object *o2:*(otherCharacter->getInventory())) {
                if (strstr(o2->getName()->get(),script->ID)!=0) {
                    // match!
                    if (c->getGold()>=o2->getGold()) {
                        otherCharacter->removeFromInventory(o2);
                        c->addObjectToInventory(o2, game);
                        c->setGold(c->getGold()-o2->getGold());
                        otherCharacter->setGold(otherCharacter->getGold()+o2->getGold());
                        return SCRIPT_FINISHED;
                    } else {
                        return SCRIPT_FAILED;
                    }
                }
            }
        }
    }
    return SCRIPT_FAILED;
}


int execute_chop(A4Script *script, A4Object *o, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    if (o->isCharacter()) {
        A4Character *c = (A4Character *)o;
        int priority = 10;
        
        if (script->x==-1 && script->y==-1 && script->ID==0) {
            // you need to specify a target position to interact, since interacting requires a direction
            return SCRIPT_FAILED;
        } else if (script->x>=0 && script->y>=0) {
            // x,y,map version:
            if (o->isAICharacter()) {
                A4AICharacter *aic = (A4AICharacter *)o;
                A4AI *ai = aic->getAI();
                if (script->ID!=0) {
                    Symbol *map_s = new Symbol(script->ID);
                    map = game->getMap(map_s);
                    delete map_s;
                }
                
                int interactDirection = A4_DIRECTION_NONE;
                if (o->getMap()==map) {
                    if (o->getX()+o->getPixelWidth() == script->x && o->getY() == script->y) interactDirection = A4_DIRECTION_RIGHT;
                    if (o->getX()-o->getPixelWidth() == script->x && o->getY() == script->y) interactDirection = A4_DIRECTION_LEFT;
                    if (o->getX() == script->x && o->getY()+o->getPixelHeight() == script->y) interactDirection = A4_DIRECTION_DOWN;
                    if (o->getX() == script->x && o->getY()-o->getPixelHeight() == script->y) interactDirection = A4_DIRECTION_UP;
                }
                
                if (interactDirection != A4_DIRECTION_NONE) {
                    if (aic->isIdle()) {
                        // character is on position:
                        c->issueCommand(A4CHARACTER_COMMAND_INTERACT,0,interactDirection, 0, game);
                        return SCRIPT_FINISHED;
                    } else {
                        return SCRIPT_NOT_FINISHED;
                    }
                } else {
                    WME *wme = new WME(A4AI::s_object_symbol,
                                       WMEParameter(o->getID()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->x + o->getPixelWidth()), WME_PARAMETER_INTEGER,
                                       WMEParameter(script->y + o->getPixelHeight()), WME_PARAMETER_INTEGER,
                                       WMEParameter(new Symbol(map->getNameSymbol())),WME_PARAMETER_SYMBOL,
                                       0);
                    ai->addPFTargetWME(wme, game, A4CHARACTER_COMMAND_IDLE, priority, false);
                    delete wme;
                    return SCRIPT_NOT_FINISHED;
                }
            } else {
                return SCRIPT_FAILED;
            }
        }
    } else {
        return SCRIPT_FAILED;
    }
    return SCRIPT_FINISHED;
}

