#ifndef _A4ENGINE_APP
#define _A4ENGINE_APP

#include <list>

#define A4ENGINE_STATE_INTRO 0
#define A4ENGINE_STATE_TITLESCREEN 1
#define A4ENGINE_STATE_QUIT 2
#define A4ENGINE_STATE_GAME 3
#define A4ENGINE_STATE_GAMECOMPLETE 4

class A4EngineApp {
public:
	A4EngineApp(int a_dx, int a_dy, const char *configFilePath);
	~A4EngineApp();

	bool cycle(class KEYBOARDSTATE *k);
	void draw(int SCREEN_X,int SCREEN_Y);

	void MouseClick(SDL_MouseButtonEvent click);

	// these will be called when toggling windowed/fullscreen (right before, and right after)
	void clearOpenGLState();	
	void resetOpenGLState();
    
    char *getInstructionsString();

protected:
    
    void loadConfiguration();
    bool saveConfiguration();
    void setDefaultConfiguration();
    
	int intro_cycle(KEYBOARDSTATE *k);
	void intro_draw(void);

	int titlescreen_cycle(KEYBOARDSTATE *k);
	void titlescreen_draw(void);

	int game_cycle(KEYBOARDSTATE *k);
	void game_draw(void);

    int gamecomplete_cycle(KEYBOARDSTATE *k);
    void gamecomplete_draw(void);
    
	int screen_x(int x);	/* given a coordinate in 640x480, returns the proper coordinate at the current resolution */ 
	int screen_y(int y);    /* given a coordinate in 640x480, returns the proper coordinate at the current resolution */ 
    
    // talk dialog:
    void createTalkDialog(class A4Character *target);
    
    // trade dialog:
    void createTradeDialog(class A4Character *player, A4Character *other);
    bool updateTradeDialog();
    A4Character *m_trade_dialog_player;
    A4Character *m_trade_dialog_other;
    bool m_trade_needs_update;
    
	// Fade in/out effects:
	void fade_in_alpha(float f);
	void fade_in_squares(float f,float size);
	void fade_in_triangles(float f,float size);

	GLTManager *m_GLTM;
	class SFXManager *m_SFXM;

	// game configuration:
    int m_SFX_volume;
	int m_key_zoom, m_key_switch_players;
    int m_key_messageconsole_up, m_key_messageconsole_down;
	int m_key_stats_toggle;
	int m_key_inventory_toggle;
	int m_key_spells_toggle;
	int m_key_left,m_key_right,m_key_up,m_key_down;
	int m_key_take;
	int m_key_drop_gold;
	int m_key_talk;
    int m_key_fast_forward;
    int m_key_force_attack;
    int m_key_force_interaction;

	// game:
    char *m_configFilePath;
	char *m_game_path, *m_game_filename;
	class XMLNode *m_gameDefinition;
	class A4Game *m_game;

	BitmapFont *m_font32,*m_font16,*m_font8;

	int m_mouse_x,m_mouse_y,m_mouse_button;
	std::list<SDL_MouseButtonEvent> m_mouse_clicks;

	int m_screen_dx,m_screen_dy;
	int m_state,m_previous_state;
	int m_state_cycle;

	// intro:
    int m_redefining_key;

	// titlescreen:
	int m_titlescreen_state;	// 0: appearing, 1: normal, 2: disappearing
	int m_titlescreen_nextstate;
	int m_titlescreen_timer;
	float m_titlescreen_angle1,m_titlescreen_angle2,m_titlescreen_angle3;
	float m_titlescreen_zoom1,m_titlescreen_zoom2,m_titlescreen_zoom3;
	float m_titlescreen_x1,m_titlescreen_x2,m_titlescreen_x3;
	float m_titlescreen_y1,m_titlescreen_y2,m_titlescreen_y3;

	// state game:
	int m_ingame_menu;
	int m_cycles_per_frame;
};

#endif

