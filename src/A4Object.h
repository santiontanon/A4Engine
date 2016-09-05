#ifndef _A4ENGINE_OBJECT
#define _A4ENGINE_OBJECT

#include <list>

#include "A4Game.h"
#include "Sort.h"
#include "A4Agenda.h"

class A4Object {
	friend class A4ObjectFactory;
public:
	A4Object(Symbol *name, Sort *sort);
	virtual ~A4Object();
    
    virtual void loadObjectAdditionalContent(XMLNode *xml, A4Game *game, class A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit);
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void revisitObject(XMLNode *xml, A4Game *game);
    
    void saveToXML(class XMLwriter *w, A4Game *game, int type, bool saveLocation);
    void saveToXMLForMainFile(class XMLwriter *w, A4Game *game, int mapNumber);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);
    void saveObjectAttributeToXML(class XMLwriter *w, const char *property, const char *value);
    void saveObjectAttributeToXML(class XMLwriter *w, const char *property, bool value);
    void saveObjectAttributeToXML(class XMLwriter *w, const char *property, int value);
    void saveObjectAttributeToXML(class XMLwriter *w, const char *property, float value);

	virtual bool cycle(A4Game *game);
	virtual void draw(int offsetx, int offsety, float zoom, class A4Game *game);

	// this executes all the A4EventRules with the given event:
	virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
	virtual void eventWithID(int event, char *ID, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
    virtual void eventWithObject(int event, class A4Character *otherCharacter, A4Object *object, class A4Map *map, class A4Game *game);
    virtual void eventWithInteger(int event, int value, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
    virtual void eventWithSpeechAct(int event, class SpeechAct *speechAct, bool angry, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);

    void executeScriptQueues(A4Game *game);
    void addScriptQueue(class A4ScriptExecutionQueue *seq) {
        m_script_queues.push_back(seq);
    }

    void addEventRule(int event, A4EventRule *r) {m_event_scripts[event].push_back(r);}

    void setStoryStateVariable(const char *variable, const char *value);
    char *getStoryStateVariable(const char *variable);

	// these will be called when toggling windowed/fullscreen (right before)
	void clearOpenGLState();

	void warp(int x, int y, class A4Map *map, int layer);

    int getID() {return m_ID;}
	Symbol *getName() {return m_name;}
    void setName(Symbol *name) {if (m_name!=0) delete m_name;
                                m_name = name;}

	void setX(int x) {m_x = x;}
	int getX() {return m_x;}
	void setY(int y) {m_y = y;}
	int getY() {return m_y;}
	void setMap(A4Map *map) {m_map = map;}
	A4Map *getMap() {return m_map;}

	virtual int getPixelWidth();
	virtual int getPixelHeight();

	int getLayer() {return m_layer;}
	void setLayer(int l) {m_layer = l;}

	void setAnimation(int idx, Animation *a);
	Animation *getAnimation(int idx) {return m_animations[idx];}
	Animation *getAnimation() {return m_animations[m_currentAnimation];}

	void setGold(int g) {m_gold = g;}
	int getGold() {return m_gold;}

	void setTakeable(bool t) {m_takeable = t;}
	bool getTakeable() {return m_takeable;}

	void setUsable(bool u) {m_usable = u;}
	bool getUsable() {return m_usable;}
    
    void setBurrowed(bool b) {m_burrowed = b;}
    bool getBurrowed() {return m_burrowed;}

    void setCanSwim(bool cs) {m_canSwim = cs;}
    bool getCanSwim() {return m_canSwim;}
    bool getCanWalk() {return m_canWalk;}
    void setCanWalk(bool cs) {m_canWalk = cs;}
    
    void die() {m_deadRequest = true;}

	virtual bool isWalkable() {return true;}
	virtual bool isEquipable() {return false;}
	virtual bool isHeavy() {return false;}		// this is used by pressure plates
	virtual bool isPlayer() {return false;}
	virtual bool isInteracteable() {return false;}
	virtual bool isKey() {return false;}
    virtual bool isCharacter() {return false;}
    virtual bool isDoor() {return false;}
    virtual bool isVehicle() {return false;}
    virtual bool isAICharacter() {return false;}
    virtual bool isTrigger() {return false;}
    virtual bool isWand() {return false;}
    
    virtual bool respawns() {return false;}
	bool seeThrough();

	bool collision(int x, int y, int dx, int dy);
	bool collision(A4Object *o2);
    bool collision(int xoffs, int yoffs, A4Object *o2);
    int canMove(int direction, int slack, bool treatBridgesAsWalls);
    int canMove(int direction, int slack); // returns how much do we have to move in the other dimension to move (INT_MAX if we can't move)
    bool canMoveWithoutGoingThroughABridge(int direction);
    int pixelDistance(A4Object *o2);
    
    void addAgenda(Agenda *a);
    void removeAgenda(char *agenda);
    
    virtual void objectRemoved(A4Object *o);

    // sorts:
    bool is_a(Sort *s) {return m_sort->is_a(s);}
    bool is_a(char *s) {return m_sort->is_a(s);}
    Sort *getSort() {return m_sort;}

    static int s_nextID;

protected:
    int m_ID;

	Symbol *m_name;
    Sort *m_sort;
	int m_x, m_y;
	int m_layer;
	class A4Map *m_map;
	Animation **m_animations;
	int m_currentAnimation;
    int m_cycle;
    
    bool m_deadRequest; // this is set to true, when the script "die" is executed

    // pixel width/height cache:
    int m_pixel_width_cache_cycle;
    int m_pixel_width_cache;
    int m_pixel_height_cache;
    
	// attributes:
	int m_gold;
	bool m_takeable;
	bool m_usable;
    bool m_burrowed;
    bool m_canSwim, m_canWalk;

	// scripts:
	std::vector<class A4EventRule *> m_event_scripts[A4_NEVENTS];

    // script excution queues (these contain scripts that are pending execution, will be executed in the next "cycle"):
    std::list<A4ScriptExecutionQueue *> m_script_queues;

    // story state:
    std::vector<char *> m_storyStateVariables;
    std::vector<char *> m_storyStateValues;
    
    // agendas:
    std::list<Agenda *> m_agendas;
};

#endif

