#ifndef _A4ENGINE_MAP
#define _A4ENGINE_MAP

#include "A4MapBridge.h"

#define CYCLES_IN_PERCEPTION_BUFFER   50

class DamageVisualization {
public:
    DamageVisualization(int d, int a_x, int a_y, int t) {
        damage = d;
        x = a_x;
        y = a_y;
        timer = t;
    }

    int damage;
    int x, y;
    int timer;
};




class RespawnRecord {
public:
    RespawnRecord(int a_ID, int a_x, int a_y, int a_l, XMLNode *a_xml, float a_rp) {
        ID = a_ID;
        x = a_x;
        y = a_y;
        layer = a_l;
        xml = new XMLNode(a_xml);
        respawn_probability = a_rp;
        time_to_respawn = -1;
    }
    ~RespawnRecord() {
        delete xml;
    }

    int ID;    // data to respawn the enemy:
    int x,y,layer;
    XMLNode *xml;

    float respawn_probability;
    int time_to_respawn;
};


class PerceptionBufferRecord {
public:
    PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, int x0, int y0, int x1, int y1);
    PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, int objectID, Sort *object, int x0, int y0, int x1, int y1);
    PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, int objectID, Sort *object, int indirectID, Sort *indirect, int x0, int y0, int x1, int y1);
    PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, Symbol *object, int x0, int y0, int x1, int y1);
    PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, Symbol *object, int indirectID, Sort *indirect, int x0, int y0, int x1, int y1);
    ~PerceptionBufferRecord();

    Symbol *m_action;
    int m_subject_ID, m_direct_object_ID, m_indirect_object_ID;
    class Sort *m_subject_sort, *m_direct_object_sort, *m_indirect_object_sort;
    Symbol *m_direct_object_symbol;
    int m_x0,m_y0,m_x1,m_y1;
    int m_time;
};


class PerceptionBufferObjectWarpedRecord {
public:
    PerceptionBufferObjectWarpedRecord(int ID, Sort *sort, Symbol *map, int x0, int y0, int x1, int y1) {
        m_ID = ID;
        m_sort = sort;
        m_target_map = map;
        m_x0 = x0;
        m_y0 = y0;
        m_x1 = x1;
        m_y1 = y1;
    }
    ~PerceptionBufferObjectWarpedRecord() {
        if (m_target_map!=0) delete m_target_map;
        m_target_map = 0;
    }
    int m_ID;
    Sort *m_sort;
    Symbol *m_target_map;
    int m_x0,m_y0,m_x1,m_y1;
    int m_time;
};


class A4MapLayer {
	friend class A4Map;
public:
	A4MapLayer(int dx, int dy, int n_gfs,class GraphicFile **gfs);
	~A4MapLayer();

	void cycle(class A4Game *game);
	void draw(int offsetx, int offsety, float zoom, int SCREENX, int SCREENY, A4Game *game);
    void draw(int offsetx, int offsety, float zoom, int SCREENX, int SCREENY, int *visibility, int visibilityRegion, A4Game *game);
    void drawSpeechBubbles(int offsetx, int offsety, float zoom, int SCREENX, int SCREENY, A4Game *game);
    void drawSpeechBubbles(int offsetx, int offsety, float zoom, int SCREENX, int SCREENY, int *visibility, int visibilityRegion, A4Game *game);
	void clearOpenGLState();

	void removeObject(class A4Object *o);
	void addObject(A4Object *o);
    bool contains(class A4Object *o);
    void objectRemoved(A4Object *o);

    bool walkableOnlyBackground(int x, int y, int dx, int dy, A4Object *subject);
    bool walkableOnlyObjects(int x, int y, int dx, int dy, A4Object *subject);
	bool walkable(int x, int y, int dx, int dy, A4Object *subject);
    A4Object *getTakeableObject(int x, int y, int dx, int dy);
	A4Object *getBurrowedObject(int x, int y, int dx, int dy);
	A4Object *getUsableObject(int x, int y, int dx, int dy);
    A4Object *getVehicleObject(int x, int y, int dx, int dy);
    void getAllObjects(int x, int y, int dx, int dy, std::vector<A4Object *> *l);
    void getAllObjectCollisions(A4Object *o, int xoffs, int yoffs, std::vector<A4Object *> *l);

    int getAllObjects(int x, int y, int dx, int dy, A4Object **l, int max_l);
    int getAllObjectsInRegion(int x, int y, int dx, int dy, A4Map *map, int region, A4Object **l, int max_l);
    int getAllObjectCollisions(A4Object *o, int xoffs, int yoffs, A4Object **l, int max_l);
	
    bool seeThrough(int x, int y);
    bool chopTree(int x, int y, int dx, int dy);
    bool spellCollision(A4Object *spell, A4Object *caster);
    bool spellCollision(int x, int y, int dx, int dy, A4Object *caster);
    A4Object *getObject(int ID);
    
    void setDoorGroupState(Symbol *doorGroup, bool state, A4Character *character, A4Map *map, A4Game *game);
    bool checkIfDoorGroupStateCanBeChanged(Symbol *doorGroup, bool state, A4Character *character, A4Map *map, A4Game *game);

	void triggerObjectsEvent(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
	void triggerObjectsEventWithID(int event, char *ID, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);

    int getTileWidth() {return m_tile_dx;}
    int getTileHeight() {return m_tile_dy;}

private:
	int m_dx, m_dy;
	int m_tile_dx, m_tile_dy;
	int m_n_graphicFiles;
	GraphicFile **m_gfs;
	int *m_gfs_startTile;
	int *m_tiles;
//	int *m_seeThrough;
	int *m_canDig;
	std::list<A4Object *> m_objects;

	GLTile **m_gl_tiles;	// this is a cache, that is cleared when the clearOpenGLState method is called;
};


class A4Map {
public:
	A4Map(XMLNode *xml, A4Game *game, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit, int &respawnID);
    A4Object *loadObjectFromXML(XMLNode *object_xml, A4Game *game, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit);
    ~A4Map();
    
    void saveToXML(class XMLwriter *w, A4Game *game);

	void cycle(A4Game *game);
	void draw(int offsetx, int offsety, float zoom, int SCREEN_X,int SCREEN_Y, A4Game *game);
    void draw(int offsetx, int offsety, float zoom, int SCREEN_X,int SCREEN_Y, int region, A4Game *game);
    void drawSpeechBubbles(int offsetx, int offsety, float zoom, int SCREEN_X,int SCREEN_Y, A4Game *game);
    void drawSpeechBubbles(int offsetx, int offsety, float zoom, int SCREEN_X,int SCREEN_Y, int region, A4Game *game);

    std::vector<A4Map *> *getNeighborMaps();

    void executeScriptQueues(A4Game *game);
    void addScriptQueue(class A4ScriptExecutionQueue *seq) {
        m_script_queues.push_back(seq);
    }

    void setStoryStateVariable(const char *variable, const char *value);
    char *getStoryStateVariable(const char *variable);

	// these will be called when toggling windowed/fullscreen (right before, and right after)
	void clearOpenGLState();

	void removeObject(A4Object *o);
	void removeObject(A4Object *o, int layer);
    void removePerceptionBuffersForObject(A4Object *o, bool actions, bool warps);
	void addObject(A4Object *o, int layer);
    bool contains(A4Object *o);
    void objectRemoved(A4Object *o);
    
    RespawnRecord *getRespawnRecord(int ID);
    
    void setDoorGroupState(Symbol *doorGroup, bool state, A4Character *character, A4Map *map, A4Game *game);
    bool checkIfDoorGroupStateCanBeChanged(Symbol *doorGroup, bool state, A4Character *character, A4Map *map, A4Game *game);

	char *getName() {return m_name;}
    Symbol *getNameSymbol() {return m_name_symbol;}
	int getDx() {return m_dx;}
	int getDy() {return m_dy;}
    int getTileWidth() {return m_layers[0]->getTileWidth();}
    int getTileHeight() {return m_layers[0]->getTileHeight();}
	std::vector<A4MapBridge *> *getBridges() {return &m_bridges;}
	std::vector<A4MapBridge *> *getBridgeDestinations() {return &m_bridgeDestinations;}
    std::list<PerceptionBufferRecord *> *getPerceptionBuffer() {return &m_perception_buffer;}
    std::list<PerceptionBufferObjectWarpedRecord *> *getWarpPerceptionBuffer() {return &m_warp_perception_buffer;}
    void addPerceptionBufferRecord(PerceptionBufferRecord *pbr) {pbr->m_time = m_cycle;m_perception_buffer.push_back(pbr);}
    void addPerceptionBufferRecord(PerceptionBufferObjectWarpedRecord *pbr) {pbr->m_time = m_cycle;m_warp_perception_buffer.push_back(pbr);}

    bool walkableOnlyBackground(int x, int y, int dx, int dy, A4Object *subject);
	bool walkable(int x, int y, int dx, int dy, A4Object *subject);
    bool walkableConsideringVehicles(int x, int y, int dx, int dy, A4Object *subject);
	A4MapBridge *getBridge(int x, int y);
	A4Object *getTakeableObject(int x, int y, int dx, int dy);
    A4Object *getBurrowedObject(int x, int y, int dx, int dy);
	A4Object *getUsableObject(int x, int y, int dx, int dy);
    A4Object *getVehicleObject(int x, int y, int dx, int dy);
    std::vector<A4Object *> *getAllObjects(int x, int y, int dx, int dy);
	std::vector<A4Object *> *getAllObjectCollisions(A4Object *o);
    std::vector<A4Object *> *getAllObjectCollisions(A4Object *o, int xoffs, int yoffs);
    int getAllObjects(int x, int y, int dx, int dy, A4Object **l, int max_l);
    int getAllObjectsInRegion(int x, int y, int dx, int dy, int region, A4Object **l, int max_l);
    int getAllObjectCollisions(A4Object *o, A4Object **l, int max_l);
    int getAllObjectCollisions(A4Object *o, int xoffs, int yoffs, A4Object **l, int max_l);
    bool chopTree(A4Character *o, A4Object *tool, A4Game *game, int direction);
    bool spellCollision(A4Object *spell, A4Object *caster);
    bool spellCollision(int x, int y, int dx, int dy, A4Object *caster);
    A4Object *getObject(int ID);

	void triggerObjectsEvent(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);
	void triggerObjectsEventWithID(int event, char *ID, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);

    void addDamageVisualization(DamageVisualization *dv) {m_damageVisualizations.push_back(dv);}

	// coordinates for these functions are in tiles:
	void reevaluateVisibility();
	bool seeThrough(int x, int y);
	bool visible(int tilex, int tiley, int region);
    int visiblilityRegion(int tilex, int tiley);

protected:
    XMLNode *m_xml; // we store this, since the RespawnRecords have pointers to internal nodes of this
	char *m_name;
    Symbol *m_name_symbol;
	int m_dx, m_dy;	// width and height in tiles
	unsigned int m_n_layers;
	A4MapLayer **m_layers;
	std::vector<A4MapBridge *> m_bridges;
	std::vector<A4MapBridge *> m_bridgeDestinations;
	int m_cycle;

    int *m_visibility_regions;

    // respawning enemies:
    std::vector<RespawnRecord *> m_respawnRecords;

    // drawing daken damage:
    std::list<DamageVisualization *> m_damageVisualizations;

    // scripts:
	std::vector<class A4EventRule *> m_event_scripts[A4_NEVENTS];

    // script excution queues (these contain scripts that are pending execution, will be executed in the next "cycle"):
    std::list<A4ScriptExecutionQueue *> m_script_queues;

    // story state:
    std::vector<char *> m_storyStateVariables;
    std::vector<char *> m_storyStateValues;

    // perception buffers:
    std::list<PerceptionBufferRecord *> m_perception_buffer;
    std::list<PerceptionBufferObjectWarpedRecord *> m_warp_perception_buffer;
};

#endif

