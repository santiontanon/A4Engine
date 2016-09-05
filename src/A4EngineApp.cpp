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
#include <map>
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

#include "A4EventRule.h"
#include "A4Game.h"
#include "A4EngineApp.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

// GAME TO LOAD (in case no configuration file is found):
const char *DEFAULT_game_path = "games/aventura4";
const char *DEFAULT_game_file = "aventura4.xml";

#ifdef __EMSCRIPTEN__
extern const char *persistent_data_folder;
#endif

/* Frames per second counter: */ 
extern int frames_per_sec;
extern int frames_per_sec_tmp;
extern int update_frames_per_sec;
extern int update_frames_per_sec_tmp;
extern int init_time;
extern bool show_fps;
extern SDL_Window *appwindow;

//double camera_distance_constant = 6.397f;
//double camera_distance_constant = 6.4164f;

A4EngineApp::A4EngineApp(int a_dx, int a_dy, const char *configFilePath)
//A4EngineApp::A4EngineApp(int a_dx, int a_dy, const char *path, const char *gamefile, bool allowSaveGames)
{
    m_configFilePath = new char[strlen(configFilePath)+1];
    strcpy(m_configFilePath, configFilePath);
    loadConfiguration();
    
	m_game = 0;
	m_state=A4ENGINE_STATE_INTRO;
	m_previous_state = m_state;
	m_state_cycle=0;

	m_screen_dx=a_dx;
	m_screen_dy=a_dy;
//	fullscreen=false;
	
	m_GLTM=new GLTManager();
	m_SFXM=new SFXManager();
//	m_SFXM->cache("sfx");

	output_debug_message("loading fonts...\n");	

	m_font8=new BitmapFont("fonts/emulogic8.png");
	m_font16=new BitmapFont("fonts/emulogic16.png");
	m_font32=new BitmapFont("fonts/emulogic32.png");

	output_debug_message("creating OpenGL shaders...\n");	

	resetOpenGLState();

	output_debug_message("done.\n");	

    if (m_game_path!=0 && m_game_filename!=0) {
        char *fullpath = new char[strlen(m_game_path) + strlen(m_game_filename) + 2];
        sprintf(fullpath,"%s/%s",m_game_path,m_game_filename);
        m_gameDefinition = XMLNode::from_file(fullpath);
        delete []fullpath;
    } else {
        m_gameDefinition = 0;
    }

	m_ingame_menu = 0;
	m_cycles_per_frame = 1;
    
    m_redefining_key = -1;
    
    m_trade_dialog_player = 0;
    m_trade_dialog_other = 0;
    
    m_trade_needs_update = false;

	output_debug_message("A4EngineApp created.\n");

    if (m_gameDefinition!=0) {
        m_game = new A4Game(m_gameDefinition, m_game_path, m_GLTM, m_SFXM, m_SFX_volume);
    }
    
} // A4EngineApp::A4EngineApp 


A4EngineApp::~A4EngineApp()
{
    if (m_game_path!=0) delete m_game_path;
    m_game_path = 0;
	if (m_game_filename!=0) delete m_game_filename;
    m_game_filename = 0;

	if (m_game!=0) delete m_game;
	m_game = 0;

	if (m_gameDefinition!=0) delete m_gameDefinition;
	m_gameDefinition = 0;

	BInterface::reset();

	m_GLTM->clear();
	delete m_GLTM;
	delete m_SFXM;

	delete m_font8;
	delete m_font16;
	delete m_font32;
} /* A4EngineApp::A4EngineApp */


void A4EngineApp::loadConfiguration()
{    
    // load config file:
#ifdef __EMSCRIPTEN__
    char tmp[2048];
    sprintf(tmp, "/%s/%s",persistent_data_folder,m_configFilePath);
    XMLNode *config_xml = XMLNode::from_file(tmp);
    if (config_xml==0) {
        output_debug_message("Persistent configuration file not found at '%s'...\n", tmp);
        config_xml = XMLNode::from_file(m_configFilePath);
    }
#else
    XMLNode *config_xml = XMLNode::from_file(m_configFilePath);
#endif
    
    if (config_xml!=0) {
        XMLNode *defaultGame = config_xml->get_child("defaultGame");
        if (defaultGame!=0) {
            char *path = defaultGame->get_attribute("path");
            if (path!=0) {
                m_game_path = new char[strlen(path)+1];
                strcpy(m_game_path, path);
            } else {
                m_game_path = 0;
            }
            
            char *gamefile = defaultGame->get_attribute("gamefile");
            if (gamefile!=0) {
                m_game_filename = new char[strlen(gamefile)+1];
                strcpy(m_game_filename, gamefile);
            } else {
                m_game_filename = 0;
            }
        } else {
            m_game_path = 0;
            m_game_filename = 0;
        }
        
        XMLNode *volume_xml = config_xml->get_child("volume");
        if (volume_xml!=0) {
            m_SFX_volume = atoi(volume_xml->get_attribute("sfx"));
        }

        XMLNode *controls_xml = config_xml->get_child("controls");
        if (controls_xml!=0) {
            XMLNode *key_xml = controls_xml->get_child("zoom"); m_key_zoom = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("switch_players"); m_key_switch_players = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("stats_toggle"); m_key_stats_toggle = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("inventory_toggle"); m_key_inventory_toggle = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("spells_toggle"); m_key_spells_toggle = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("messageconsole_up"); m_key_messageconsole_up = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("messageconsole_down"); m_key_messageconsole_down = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("left"); m_key_left = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("right"); m_key_right = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("up"); m_key_up = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("down"); m_key_down = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("take"); m_key_take = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("drop_gold"); m_key_drop_gold = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("talk"); m_key_talk = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("fast_forward"); m_key_fast_forward = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("force_attack"); m_key_force_attack = atoi(key_xml->get_attribute("key"));
            key_xml = controls_xml->get_child("force_interaction"); m_key_force_interaction = atoi(key_xml->get_attribute("key"));
        } else {
            setDefaultConfiguration();
        }
        
        delete config_xml;
        config_xml = 0;
    } else {
        // default configuration:
        m_game_path = new char[strlen(DEFAULT_game_path)+1];
        strcpy(m_game_path, DEFAULT_game_path);
        
        m_game_filename = new char[strlen(DEFAULT_game_file)+1];
        strcpy(m_game_filename, DEFAULT_game_file);
        
        setDefaultConfiguration();
    }
}


void A4EngineApp::setDefaultConfiguration()
{
    m_SFX_volume = MIX_MAX_VOLUME;
    
    m_key_zoom = SDL_SCANCODE_Z;
    m_key_switch_players = SDL_SCANCODE_N;
    m_key_stats_toggle = SDL_SCANCODE_S;
    m_key_inventory_toggle = SDL_SCANCODE_V;
    m_key_spells_toggle = SDL_SCANCODE_P;
    m_key_messageconsole_up = SDL_SCANCODE_PAGEUP;
    m_key_messageconsole_down = SDL_SCANCODE_PAGEDOWN;
    m_key_left = SDL_SCANCODE_LEFT;
    m_key_right = SDL_SCANCODE_RIGHT;
    m_key_up = SDL_SCANCODE_UP;
    m_key_down = SDL_SCANCODE_DOWN;
    m_key_take = SDL_SCANCODE_SPACE;
    m_key_drop_gold = SDL_SCANCODE_R;
    m_key_talk = SDL_SCANCODE_T;
    m_key_fast_forward = SDL_SCANCODE_TAB;
    m_key_force_attack = SDL_SCANCODE_LSHIFT;
    m_key_force_interaction = SDL_SCANCODE_LALT;
}


bool A4EngineApp::saveConfiguration()
{
#ifdef __EMSCRIPTEN__
    char tmp[2048];
    sprintf(tmp, "/%s/%s",persistent_data_folder,m_configFilePath);
    XMLfileWriter *w = new XMLfileWriter(tmp, 2);
#else
    XMLfileWriter *w = new XMLfileWriter(m_configFilePath, 2);
#endif
    w->openTag("Aventura4ConfigurationFile");
    
    w->openTag("defaultGame");
    if (m_game_path!=0) w->setAttribute("path", m_game_path);
    if (m_game_filename!=0) w->setAttribute("gamefile", m_game_filename);
    w->closeTag("defaultGame");
    
    w->openTag("volume"); w->setAttribute("sfx", m_SFX_volume); w->closeTag("volume");
    w->openTag("controls");
    w->openTag("zoom"); w->setAttribute("key", m_key_zoom); w->closeTag("zoom");
    w->openTag("switch_players"); w->setAttribute("key", m_key_switch_players); w->closeTag("switch_players");
    w->openTag("messageconsole_up"); w->setAttribute("key", m_key_messageconsole_up); w->closeTag("messageconsole_up");
    w->openTag("messageconsole_down"); w->setAttribute("key", m_key_messageconsole_down); w->closeTag("messageconsole_down");
    w->openTag("stats_toggle"); w->setAttribute("key", m_key_stats_toggle); w->closeTag("stats_toggle");
    w->openTag("inventory_toggle"); w->setAttribute("key", m_key_inventory_toggle); w->closeTag("inventory_toggle");
    w->openTag("spells_toggle"); w->setAttribute("key", m_key_spells_toggle); w->closeTag("spells_toggle");
    w->openTag("left"); w->setAttribute("key", m_key_left); w->closeTag("left");
    w->openTag("right"); w->setAttribute("key", m_key_right); w->closeTag("right");
    w->openTag("up"); w->setAttribute("key", m_key_up); w->closeTag("up");
    w->openTag("down"); w->setAttribute("key", m_key_down); w->closeTag("down");
    w->openTag("take"); w->setAttribute("key", m_key_take); w->closeTag("take");
    w->openTag("drop_gold"); w->setAttribute("key", m_key_drop_gold); w->closeTag("drop_gold");
    w->openTag("talk"); w->setAttribute("key", m_key_talk); w->closeTag("talk");
    w->openTag("fast_forward"); w->setAttribute("key", m_key_fast_forward); w->closeTag("fast_forward");
    w->openTag("force_attack"); w->setAttribute("key", m_key_force_attack); w->closeTag("force_attack");
    w->openTag("force_interaction"); w->setAttribute("key", m_key_force_interaction); w->closeTag("force_interaction");
    w->closeTag("controls");
    
    w->closeTag("Aventura4ConfigurationFile");
    delete w;
    
#ifdef __EMSCRIPTEN__
    EM_ASM(
           Module.print("Start File sync..");
           Module.syncdone = 0;
           FS.syncfs(false, function(err) {
        assert(!err);
        Module.print("End File sync..");
        Module.syncdone = 1;
    });
           );
#endif
    
    return false;
}


bool A4EngineApp::cycle(KEYBOARDSTATE *k)
{
	int old_state=m_state;
  
	if (m_state_cycle==0) output_debug_message("First Cycle started for state %i...\n",m_state);

	switch(m_state) {
	case A4ENGINE_STATE_INTRO:	m_state = intro_cycle(k);
								break;
	case A4ENGINE_STATE_TITLESCREEN:	m_state = titlescreen_cycle(k);
										break;
	case A4ENGINE_STATE_GAME:	m_state = game_cycle(k);
                                break;
    case A4ENGINE_STATE_GAMECOMPLETE:	m_state = gamecomplete_cycle(k);
                                        break;
	default:// A4ENGINE_STATE_QUIT:
			return false;
	} /* switch */ 

	if (old_state==m_state) {
		m_state_cycle++;
	} else {
		m_state_cycle=0;

		output_debug_message("State change: %i -> %i\n",old_state,m_state);
	} /* if */ 

	m_SFXM->next_cycle();

	m_previous_state = old_state;

	return true;
} /* A4EngineApp::cycle */ 


void A4EngineApp::draw(int SCREEN_X,int SCREEN_Y)
{	
	m_screen_dx=SCREEN_X;
	m_screen_dy=SCREEN_Y;

    // If no CYCLE has been executed for this state, do not redraw:
	if (m_state_cycle==0) return;

    glViewport(0,0,SCREEN_X,SCREEN_Y);
    glClearColor(0,0,0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);

#ifndef __EMSCRIPTEN__
    glEnable( GL_TEXTURE_2D ); 
#endif
    glDisable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    // set up model, view and projection matrices:
    glm::mat4x4 Projection = glm::ortho(0.f, (float)SCREEN_X, (float)SCREEN_Y, 0.f, -1.0f, 1.0f);
    glm::mat4 PV = Projection;
    glUniformMatrix4fv(PVMatrixID, 1, GL_FALSE, &PV[0][0]);
    glUniform4f(inColorID, 1.0,1.0,1.0,1.0);
    glUniform1i(useTextureID, 0);

	switch(m_state) {
	case A4ENGINE_STATE_INTRO:	intro_draw();
								break;
	case A4ENGINE_STATE_TITLESCREEN:	titlescreen_draw();
										break;
	case A4ENGINE_STATE_GAME:	game_draw();
								break;
    case A4ENGINE_STATE_GAMECOMPLETE:	gamecomplete_draw();
                                        break;
	} // switch

	if (show_fps) {
		char tmp[80];
//		sprintf(tmp,"video mem: %.4gmb - fps: %i",float(GLTile::get_memory_used())/float(1024*1024),frames_per_sec);
		sprintf(tmp,"fps: %i / %i",frames_per_sec, update_frames_per_sec);
		BInterface::print_center(tmp,m_font8,SCREEN_X/2,SCREEN_Y-10);
	} // if

    SDL_GL_SwapWindow(appwindow);
} /* A4EngineApp::draw */



int A4EngineApp::screen_x(int x)
{
	return ((x*m_screen_dx)/640);
} /* A4EngineApp::screen_x */ 


int A4EngineApp::screen_y(int y)
{
	return ((y*m_screen_dy)/480);
} /* A4EngineApp::screen_y */ 

void A4EngineApp::clearOpenGLState()
{
	m_GLTM->clearOpenGLState();
	BInterface::clear_print_cache();
	if (m_game!=0) m_game->clearOpenGLState();
}

void A4EngineApp::resetOpenGLState()
{
	programID = LoadShaders();

	glUseProgram(programID);
    PVMatrixID = glGetUniformLocation(programID, "PV");
    MMatrixID = glGetUniformLocation(programID, "M");
    inColorID = glGetUniformLocation(programID, "inColor");
    useTextureID = glGetUniformLocation(programID, "useTexture");
    output_debug_message("PVMatrixID: %i\n",PVMatrixID);
    output_debug_message("MMatrixID: %i\n",MMatrixID);
    output_debug_message("inColorID: %i\n",inColorID);
    output_debug_message("useTextureID: %i\n",useTextureID);

	BInterface::createOpenGLBuffers();  
	
	GLenum err = glGetError();
	if (err!=GL_NO_ERROR) output_debug_message("glError() = %i\n", err);
	output_debug_message("resetOpenGLState complete\n");  
}

void A4EngineApp::MouseClick(SDL_MouseButtonEvent event) 
{
	m_mouse_clicks.push_back(event);
} /* A4EngineApp::MouseClick */ 


