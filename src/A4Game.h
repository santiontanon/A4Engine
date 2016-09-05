#ifndef _A4ENGINE_GAME
#define _A4ENGINE_GAME

#include "string.h"
#include "Symbol.h"
#include "A4EventRule.h"
#include "GraphicFile.h"

#include <vector>

#define WALK_TILE_BY_TILE       false
#define A4_MOVEMENT_SLACK   0.25

#define A4_DIRECTION_NONE		-1
#define A4_DIRECTION_LEFT		0
#define A4_DIRECTION_UP			1
#define A4_DIRECTION_RIGHT		2
#define A4_DIRECTION_DOWN		3
#define A4_NDIRECTIONS          4

#define A4_SPELL_MAGIC_MISSILE	0
#define A4_SPELL_HEAL			1
#define A4_SPELL_SHIELD			2
#define A4_SPELL_INCREASE		3
#define A4_SPELL_DECREASE		4
#define A4_SPELL_FIREBALL		5
#define A4_SPELL_MAGIC_EYE		6
#define A4_SPELL_REGENERATE		7
#define A4_SPELL_INCINERATE		8
#define A4_N_SPELLS				9

#define A4_ANIMATION_IDLE		0
#define A4_ANIMATION_IDLE_LEFT	1
#define A4_ANIMATION_IDLE_UP	2
#define A4_ANIMATION_IDLE_RIGHT	3
#define A4_ANIMATION_IDLE_DOWN	4
#define A4_ANIMATION_MOVING			5
#define A4_ANIMATION_MOVING_LEFT	6
#define A4_ANIMATION_MOVING_UP		7
#define A4_ANIMATION_MOVING_RIGHT	8
#define A4_ANIMATION_MOVING_DOWN	9
#define A4_ANIMATION_ATTACKING			10
#define A4_ANIMATION_ATTACKING_LEFT		11
#define A4_ANIMATION_ATTACKING_UP		12
#define A4_ANIMATION_ATTACKING_RIGHT	13
#define A4_ANIMATION_ATTACKING_DOWN		14
#define A4_ANIMATION_INTERACTING		15
#define A4_ANIMATION_INTERACTING_LEFT	16
#define A4_ANIMATION_INTERACTING_UP		17
#define A4_ANIMATION_INTERACTING_RIGHT	18
#define A4_ANIMATION_INTERACTING_DOWN	19
#define A4_ANIMATION_CASTING		20
#define A4_ANIMATION_CASTING_LEFT	21
#define A4_ANIMATION_CASTING_UP		22
#define A4_ANIMATION_CASTING_RIGHT	23
#define A4_ANIMATION_CASTING_DOWN	24
#define A4_ANIMATION_TALKING		25
#define A4_ANIMATION_TALKING_LEFT	26
#define A4_ANIMATION_TALKING_UP		27
#define A4_ANIMATION_TALKING_RIGHT	28
#define A4_ANIMATION_TALKING_DOWN	29
#define A4_ANIMATION_DEATH			30
#define A4_ANIMATION_DEATH_LEFT		31
#define A4_ANIMATION_DEATH_UP		32
#define A4_ANIMATION_DEATH_RIGHT	33
#define A4_ANIMATION_DEATH_DOWN		34
#define A4_ANIMATION_OPEN		35
#define A4_ANIMATION_CLOSED		36
#define A4_N_ANIMATIONS			37

#define A4_EMOTION_DEFAULT		0
#define A4_EMOTION_HAPPY		1
#define A4_EMOTION_ANGRY		2
#define A4_EMOTION_SCARED		3
#define A4_EMOTION_CURIOUS		4
#define A4_EMOTION_TIRED		5
#define A4_N_EMOTIONS			6

#define A4_LAYER_BG				0
#define A4_LAYER_FG				1
#define A4_LAYER_CHARACTERS		2
#define A4_N_LAYERS				3

#define A4_EQUIPABLE_WEAPON		0
#define A4_EQUIPABLE_OFF_HAND	1
#define A4_EQUIPABLE_RING		2
#define A4_EQUIPABLE_N_SLOTS	3

#define A4_TILE_WALKABLE		0
#define A4_TILE_WALL			1
#define A4_TILE_TREE			2
#define A4_TILE_CHOPPABLE_TREE	3
#define A4_TILE_WATER			4

#define A4_N_MESSAGES_IN_HUD	4
#define A4_MAX_MESSAGE_LENGTH    68

#define A4_INVENTORY_SIZE		8

#define A4_TALK_PERFORMATIVE_NONE	(-1)
#define A4_TALK_PERFORMATIVE_HI		0
#define A4_TALK_PERFORMATIVE_BYE	1
#define A4_TALK_PERFORMATIVE_ASK	2
#define A4_TALK_PERFORMATIVE_INFORM	3
#define A4_TALK_PERFORMATIVE_TRADE	4
#define A4_TALK_PERFORMATIVE_END_TRADE	5
#define A4_TALK_PERFORMATIVE_TIMEOUT	6
#define A4_TALK_N_PERFORMATIVES     7

#define A4_RESPAWN_TIME         2000





class WarpRequest {
public:
	class A4Object *o;
	class A4Map *map;
	int x,y,layer;

	WarpRequest(A4Object *a_o, A4Map *a_map, int a_x, int a_y, int a_layer) {
		o = a_o;
		map = a_map;
		x = a_x;
		y = a_y;
		layer = a_layer;
	}

};


class SpeechAct {
public:
	int performative;
	char *keyword;
	char *text;

	SpeechAct(int p, char *k, char *t) {
		performative = p;
		keyword = k;
		text = t;
	}
    SpeechAct(SpeechAct *sa) {
        performative = sa->performative;
        if (sa->keyword==0) {
            keyword = 0;
        } else {
            keyword = new char[strlen(sa->keyword)+1];
            strcpy(keyword, sa->keyword);
        }
        if (sa->text==0) {
            text = 0;
        } else {
            text = new char[strlen(sa->text)+1];
            strcpy(text, sa->text);
        }
    }
	~SpeechAct() {
		if (keyword!=0) delete keyword;
		keyword = 0;
		if (text!=0) delete text;
		text = 0;
	}
};


class A4Game {
public:
	A4Game(XMLNode *xml, const char *game_path, GLTManager *GLTM, class SFXManager *SFXM, int a_sfx_volume);
    A4Game(const char *game_file, const char *game_path, const char *backup_game_path, GLTManager *GLTM, class SFXManager *SFXM, int a_sfx_volume);
	~A4Game();

    void loadContentFromXML(XMLNode *xml, const char *game_path, const char *backup_game_path, GLTManager *GLTM, class SFXManager *SFXM);
    
    void saveGame(const char *path, const char *saveName);
    void saveToXML(class XMLwriter *w);
    static bool checkSaveGame(const char *gamePath, const char *save, char *info); // checks if a save game exists, and if it does, it does, it writes a small description in "info"
    
    bool getAllowSaveGames() {return m_allowSaveGames;}
    bool getAllowTalking() {return m_allowTalking;}
    bool getAllowMagic() {return m_allowMagic;}
    bool getAllowInventory() {return m_allowInventory;}
    bool getAllowStats() {return m_allowStats;}

    char *getGameName() {return m_gameName;}
    char *getGameStory() {return m_storyText;}
    int getNEndings() {return m_n_endings;}
    char *getGameEnding(const char *ID);
    char *getGameTitle() {return m_gameTitle;}
    char *getGameTitleImage() {return m_gameTitleImage;}
    char *getGameSubtitle() {return m_gameSubtitle;}
    
    void addEnding(char *ID, std::vector<char *> *raw_ending, char *endingText);
    
	bool cycle(class KEYBOARDSTATE *k);
	void draw(int SCREEN_X,int SCREEN_Y);
	void drawWorld(int SCREEN_X,int SCREEN_Y);
	void drawHUD(int SCREEN_X,int SCREEN_Y);

    void drawAIDebugger(int SCREEN_X,int SCREEN_Y, class A4AICharacter *focus);

    bool getGameComplete() {return m_gameComplete;}
    char *getGameCompleteEndingID() {return m_gameComplete_ending_ID;}
    void setGameComplete(bool gc, char *ID) {m_gameComplete = gc; m_gameComplete_ending_ID = ID;}

    void executeScriptQueues();
    void addScriptQueue(class A4ScriptExecutionQueue *seq) {
        m_script_queues.push_back(seq);
    }

	// if an object is removed from a map, this needs to be called, to notify
	// the game that this happened.
	void objectRemoved(class A4Object *o);
    bool contains(A4Object *o);

	// Teleports an object to a requested map and position. This queues up the request,
	// but it is not executed until at the end of a game cycle, to prevent this from 
	// happening while we are still looping through lists of objects (concurrent modification)
	// if "map" is 0, then the request is to remove the object from the maps (e.g., when an object is taken)
	void requestWarp(A4Object *o, A4Map *map, int x, int y, int layer);
	// waits until the end of a cycle, and then deletes o
	void requestDeletion(A4Object *o);
    
    void setStoryStateVariable(const char *variable, const char *value);
    char *getStoryStateVariable(const char *variable);

    void playSound(const char *sound);

	int getTileDx() {return m_tile_dx;}
	int getTileDy() {return m_tile_dy;}
	GraphicFile *getGraphicFile(const char *file);
    std::vector<GraphicFile *> *getGraphicFiles() {return &m_graphicFiles;}
	class A4ObjectFactory *getObjectFactory() {return m_objectFactory;}
	class A4Map *getMap(int idx);
    class A4Map *getMap(Symbol *name);
	int getCameraX(class A4Object *focus, int map_width, int screen_width);
	int getCameraY(A4Object *focus, int map_height, int screen_height);
    int getCycle() {return m_cycle;}
    std::list<A4Character *> *getPlayers() {return &m_players;}
    A4Character *getCurrentPlayer() {return m_current_player;}
    const char *get_game_path() {return m_game_path;}
    A4Character *tradeRequested() {return m_trade_requested;}
    void setTradeRequested(A4Character *v) {m_trade_requested = v;}
    A4Object *getObject(int ID);
    class RespawnRecord *getRespawnRecord(int ID);
    
    void setDoorGroupState(Symbol *doorGroup, bool state, A4Character *character);
    bool checkIfDoorGroupStateCanBeChanged(Symbol *doorGroup, bool state, A4Character *character);
    
	class Animation *getCoinPurseAnimation() {return m_coinpurse_animation;}
    class Animation *getSpellAnimation(int spell, int direction) {return m_spell_animation[spell][direction];}
    class Animation *getEmotionAnimation(int emotion) {return m_emotion_animation[emotion];}

	void addMessage(const char *fmt, ...);
    void addMessage(A4Object *originator, const char *fmt, ...);    // message added only if the "originator" is in the same map as the "m_current_player"

	// getting input form the player:
	void playerInput_ToogleStats() {m_HUD_show_stats = (m_HUD_show_stats ? false:true);}
	void playerInput_ToogleInventory() {m_HUD_show_inventory = (m_HUD_show_inventory ? false:true);}
	void playerInput_ToogleSpells() {m_HUD_show_spells = (m_HUD_show_spells ? false:true);}
	void playerInput_ToogleZoom();
	void playerInput_SwitchPlayers();
    void messageConsoleUp();
    void messageConsoleDown();
	void playerInput_issueCommand(int cmd, SpeechAct *sa, int direction);
    int playerInput_issueCommand(int cmd, int arg, int direction);  // this checks if "cmd" should be changed to something else
                                                                    // e.g., walking into an enemy will change it to attack, and returns the
                                                                    // actual command to issue.
    void playerInput_issueCommand(int cmd, int arg, A4Object *target);
    
	void addSpeechAct(int performative, const char *keyword, const char *text);
	std::vector<SpeechAct *> *getKnownSpeechActs() {return &m_known_speech_acts;}

    class BitmapFont *getFont() {return m_font8;}
    class Ontology *getOntology() {return m_ontology;}

	// these will be called when toggling windowed/fullscreen (right before, and right after)
	void clearOpenGLState();

	static const char *animationNames[];
	static const char *spellNames[];
	static const char *spellAlternativeNames[];
    static const int spellCost[];
    static Symbol *emotionNames[];
	static const int direction_x_inc[];
	static const int direction_y_inc[];
    static const char *performativeNames[];

protected:
	int sfx_volume;

    bool m_allowSaveGames;
    bool m_allowTalking;
    bool m_allowMagic;
    bool m_allowInventory;
    bool m_allowStats;
    
    char *m_gameName;
    char *m_gameTitle;
    char *m_gameSubtitle;
    char *m_gameTitleImage;
    char *m_storyText;
    int m_n_endings;
    char **m_endingIDs;
    char **m_endingTexts;
    std::vector<char *> m_storyText_raw;
    std::vector<char *> **m_endingText_raw;
    std::vector<char *> m_characterDefinitionFiles;
    std::vector<char *> m_objectDefinitionFiles;
    
	int m_cycle;
    bool m_gameComplete;
    char *m_gameComplete_ending_ID;

	const char *m_game_path;
	GLTManager *m_GLTM;
	class SFXManager *m_SFXM;
	std::vector<GraphicFile *> m_graphicFiles;
	class A4ObjectFactory *m_objectFactory;
	int m_tile_dx, m_tile_dy;
	class BitmapFont *m_font8, *m_font4;

    // AI:
    class Ontology *m_ontology;

	// HUD:
    bool m_HUD_show_stats;
    bool m_HUD_show_inventory;
    bool m_HUD_show_spells;
    std::vector<char *> m_messages;
    A4Character *m_trade_requested;
    int m_console_first_message;

	// default animations:
	class Animation *m_coinpurse_animation;
	Animation **m_emotion_animation;
	Animation ***m_spell_animation;

	std::vector<class A4Map *> m_maps;
	std::list<class A4Character *> m_players;
	A4Character *m_current_player;
	std::list<class A4Character *>::iterator m_current_player_position;
	std::vector<WarpRequest *> warpRequests;
	std::vector<A4Object *> deletionRequests;

	// camera:
	float m_zoom, m_target_zoom, m_default_zoom;

	// scripts:
	std::vector<class A4EventRule *> m_event_scripts[A4_NEVENTS];

    // script excution queues (these contain scripts that are pending execution, will be executed in the next "cycle"):
    std::list<class A4ScriptExecutionQueue *> m_script_queues;

    // story state:
    std::vector<char *> m_storyStateVariables;
    std::vector<char *> m_storyStateValues;
	std::vector<SpeechAct *> m_known_speech_acts;

    // AI debugger:
    A4AICharacter *AI_debugger_focus;
};

#endif

