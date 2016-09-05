#ifndef __A4_SCRIPT
#define __A4_SCRIPT

#include <list>
#include <vector>
#include "A4Agenda.h"

// game logic scripts:
#define A4_SCRIPT_GAMECOMPLETE      0
#define A4_SCRIPT_MESSAGE           8
#define A4_SCRIPT_DELAY             23
#define A4_SCRIPT_PLAYSOUND         24
#define A4_SCRIPT_STOPSOUND         25
#define A4_SCRIPT_IF                26

// character scripts:
#define A4_SCRIPT_TELEPORT          3
#define A4_SCRIPT_GOTO              4
#define A4_SCRIPT_USE               6
#define A4_SCRIPT_OPENDOORS         7
#define A4_SCRIPT_TALK              9
#define A4_SCRIPT_TALKOTHER         28
#define A4_SCRIPT_STEAL             16
#define A4_SCRIPT_GIVE              17
#define A4_SCRIPT_SELL              18
#define A4_SCRIPT_DROP              19
#define A4_SCRIPT_DROPGOLD          36
#define A4_SCRIPT_TAKE              37
#define A4_SCRIPT_EQUIP             38
#define A4_SCRIPT_UNEQUIP           39
#define A4_SCRIPT_INTERACT          40
#define A4_SCRIPT_EMBARK            41
#define A4_SCRIPT_DISEMBARK         42
#define A4_SCRIPT_ATTACK            43
#define A4_SCRIPT_SPELL             44
#define A4_SCRIPT_BUY               45
#define A4_SCRIPT_CHOP              46


// miscellanea scripts:
#define A4_SCRIPT_DIE               5
#define A4_SCRIPT_ADDBEHAVIOR       1
#define A4_SCRIPT_REMOVEBEHAVIOR    2
#define A4_SCRIPT_PENDINGTALK       10
#define A4_SCRIPT_ADDTOPIC          11
#define A4_SCRIPT_EVENTRULE         34
#define A4_SCRIPT_UPDATECONVERSATIONGRAPHTRANSITION       12
#define A4_SCRIPT_STORYSTATE        13
#define A4_SCRIPT_ADDWME            14
#define A4_SCRIPT_ADDWMETOOTHERS    15
#define A4_SCRIPT_ADDCURRENTPOSITIONWME     29
#define A4_SCRIPT_FAMILIARWITHMAP   31
#define A4_SCRIPT_LOSEITEM          20
#define A4_SCRIPT_GAINITEM          21
#define A4_SCRIPT_EXPERIENCEGAIN    22
#define A4_SCRIPT_GAINGOLD          35
#define A4_SCRIPT_GAINGOLDOTHER     27
#define A4_SCRIPT_STARTTRADING      30
#define A4_SCRIPT_ADDAGENDA         32
#define A4_SCRIPT_REMOVEAGENDA      33

#define SCRIPT_FINISHED         0
#define SCRIPT_NOT_FINISHED     1
#define SCRIPT_FAILED           2

typedef int (*scriptFunctionType) (class A4Script *script, class A4Object *character, class A4Map *map, class A4Game *game, class A4Character *otherCharacter);

class A4Script {
public:
	A4Script(class XMLNode *xml);
    A4Script(A4Script *s);
	A4Script(int type, const char *ID);
    A4Script(int type, const char *ID, const char *text);
    A4Script(int type, const char *ID, const char *text, int value, bool angry, bool wait);
	A4Script(int type, int v);
	~A4Script();
    
    void saveToXML(class XMLwriter *w);

//    class A4Behavior *createBehaviorFromJSString(char *exp, class Ontology *o);
//    A4Behavior *createBehaviorFromJSString(class Expression *exp, Ontology *o);

	int execute(class A4Object *character, class A4Map *map, class A4Game *game, A4Character *otherCharacter);
    void reset();

	static const char *scriptNames[];
	static const scriptFunctionType scriptFunctions[];


	int m_type;

	// script parameters:
	int value;
    int x,y;
	char *ID;
	char *text;
    char *text2;
    bool angry, wait, consume;

    int state;
    int if_state;
    std::vector<A4Script *> subScripts;
    std::vector<A4Script *> thenSubScripts;
    std::vector<A4Script *> elseSubScripts;
    XMLNode *objectDefinition;
    Agenda *agenda;
    class A4EventRule *rule;
};


class A4ScriptExecutionQueue {
public:
    A4ScriptExecutionQueue(A4Object *a_object, A4Map *a_map, A4Game *a_game, A4Character *a_otherCharacter) {
        object = a_object;
        map = a_map;
        game = a_game;
        otherCharacter = a_otherCharacter;
    }
    ~A4ScriptExecutionQueue() {
        for(A4Script *s:scripts) delete s;
        scripts.clear();
    }

    std::list<A4Script *> scripts;
    A4Object *object;
    A4Map *map;
    A4Game *game;
    A4Character *otherCharacter;
};

#endif
