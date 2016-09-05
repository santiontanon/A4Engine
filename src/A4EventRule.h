#ifndef __A4_EVENT_RULE
#define __A4_EVENT_RULE

#define A4_EVENT_USE			0
#define A4_EVENT_PICKUP			1
#define A4_EVENT_ACTIVATE		2
#define A4_EVENT_DEACTIVATE		3
#define A4_EVENT_DROP			4
#define A4_EVENT_EQUIP			5
#define A4_EVENT_UNEQUIP		6
#define A4_EVENT_OPEN			7
#define A4_EVENT_CLOSE			8
#define A4_EVENT_PUSH			9
#define A4_EVENT_TIMER			10
#define A4_EVENT_RECEIVE        11
#define A4_EVENT_INTERACT		12
#define A4_EVENT_START			13
#define A4_EVENT_END			14
#define A4_EVENT_STORYSTATE     15

#define A4_EVENT_ACTION_TAKE    16
#define A4_EVENT_ACTION_DROP    17
#define A4_EVENT_ACTION_DROP_GOLD   18
#define A4_EVENT_ACTION_USE     19
#define A4_EVENT_ACTION_EQUIP   20
#define A4_EVENT_ACTION_UNEQUIP 21
#define A4_EVENT_ACTION_INTERACT    22
#define A4_EVENT_ACTION_TALK    23
#define A4_EVENT_ACTION_ATTACK  24
#define A4_EVENT_ACTION_SPELL   25
#define A4_EVENT_ACTION_GIVE    26
#define A4_EVENT_ACTION_SELL    27
#define A4_EVENT_ACTION_BUY     28
#define A4_EVENT_ACTION_CHOP    29
#define	A4_NEVENTS				30

#define A4_STORYSTATE_GAME   0
#define A4_STORYSTATE_MAP    1
#define A4_STORYSTATE_OBJECT 2

class A4EventRule {
public:
	A4EventRule(class XMLNode *xml);
    A4EventRule(A4EventRule *rule);
    A4EventRule(int event);
	A4EventRule(int event, class A4Script *script, bool once);
	A4EventRule(int event, int a_time, int period,A4Script *script, bool once);
	~A4EventRule();
    
    void saveToXML(class XMLwriter *w);
    
    void addScript(class A4Script *s);

	int getEvent() {return m_event;}
	int executeEffects(class A4Object *o, class A4Map *map, class A4Game *game, class A4Character *otherCharacter);
	int execute(class A4Object *o, class A4Map *map, class A4Game *game, class A4Character *otherCharacter);

    char *getItem() {return m_item;}
    int getSpell() {return m_spell;}
    char *getCharacter() {return m_character;}
    int getAngry() {return m_angry;}
    int getHit() {return m_hit;}
    int getPerformative() {return m_performative;}
    
private:
	int m_event;
	int m_time, m_period;	// for the A4_EVENT_TIMER event
    int m_start_time;

    int m_ss_scope;         // for A4_EVENT_STORYSTATE
    char *m_variable, *m_value;
    
    char *m_item;           // for A4_EVENT_RECEIVE, etc.
    char *m_character;      // for A4_EVENT_ACTION_ATTACK, etc.
    int m_performative, m_spell;
    int m_hit, m_angry;     // -1: not set, 0: false, 1: true

    bool m_once;
	std::vector<class A4Script *> m_effects;
    
    bool m_executed;
};

#endif
