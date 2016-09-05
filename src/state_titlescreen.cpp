#ifdef _WIN32
#include <windows.h>
#else
#include "unistd.h"
#include "sys/stat.h"
#include "sys/types.h"
#endif

#include "stdio.h"
#include "math.h"
#include "stdlib.h"

#include "debug.h"

#include "SDL.h"
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
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
#include "auxiliar.h"

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

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Game.h"
#include "A4EngineApp.h"


#ifdef __EMSCRIPTEN__
extern const char *persistent_data_folder;
#endif


int A4EngineApp::titlescreen_cycle(KEYBOARDSTATE *k)
{
	if (SDL_ShowCursor(SDL_QUERY)!=SDL_ENABLE) SDL_ShowCursor(SDL_ENABLE);
	if (m_state_cycle==0) {
		// create the menus:
		m_titlescreen_state = 0;
		m_titlescreen_timer = 0;
		BInterface::reset();
        if (m_game->getAllowSaveGames()) {
#ifdef __EMSCRIPTEN__
            BInterface::createMenu("play\nload\nstory\ninstructions\nconfiguration",m_font16,200,270,240,171,1);
#else
            BInterface::createMenu("play\nload\nstory\ninstructions\nconfiguration\nquit",m_font16,200,270,240,171,1);
#endif
        } else {
#ifdef __EMSCRIPTEN__
            BInterface::createMenu("play\ncontinue\nstory\ninstructions",m_font16,200,270,240,171,1);
#else
            BInterface::createMenu("play\ncontinue\nstory\ninstructions\nconfiguration\nquit",m_font16,200,270,240,171,1);
#endif
            char info[256];
            if (!A4Game::checkSaveGame(m_game_path, "slot0", info)) BInterface::disable(2);
        }

		m_titlescreen_angle1 = 0;
		m_titlescreen_angle2 = 0;
		m_titlescreen_angle3 = 0;
		m_titlescreen_zoom1 = 1;
		m_titlescreen_zoom2 = 1;
		m_titlescreen_zoom3 = 1;
		m_titlescreen_x1 = 0;
		m_titlescreen_x2 = 0;
		m_titlescreen_x3 = 0;
		m_titlescreen_y1 = 0;
		m_titlescreen_y2 = 0;
		m_titlescreen_y3 = 0;
	}

	if (m_titlescreen_state==0) {
		m_titlescreen_timer++;
		if (m_titlescreen_timer>=50) m_titlescreen_state = 1;
	} else if (m_titlescreen_state==2) {
		m_titlescreen_timer++;
		if (m_titlescreen_timer>=50) return m_titlescreen_nextstate;
	}

	int ID = BInterface::update_state(&m_mouse_clicks, k);

    if (m_redefining_key!=-1) {
        for(int i = 0;i<k->k_size;i++) {
            if (k->key_press(i)) {
                switch(m_redefining_key) {
                    case 0: m_key_up = i; break;
                    case 1: m_key_down = i; break;
                    case 2: m_key_left = i; break;
                    case 3: m_key_right = i; break;
                    case 4: m_key_force_attack = i; break;
                    case 5: m_key_force_interaction = i; break;
                    case 6: m_key_take = i; break;
                    case 7: m_key_talk = i; break;
                    case 8: m_key_drop_gold = i; break;
                    case 9: m_key_fast_forward = i; break;
                    case 10: m_key_zoom = i; break;
                    case 11: m_key_switch_players = i; break;
                }
                BInterface::pop();
                saveConfiguration();
                ID = 5;
                break;
            }
        }
    }

	switch(ID) {
	case 1:	BInterface::reset();
            if (m_game!=0) delete m_game;
            m_game = new A4Game(m_gameDefinition, m_game_path, m_GLTM, m_SFXM, m_SFX_volume);
			return A4ENGINE_STATE_GAME;
			break;

    case 2: // load
            if (m_game->getAllowSaveGames()) {
                const char *savenames[]={"slot1","slot2","slot3","slot4"};
                bool active[4] = {false,false,false,false};
                char tmp[1024];
                char info[256];
                tmp[0] = 0;
                for(int i = 0;i<4;i++) {
                    if (A4Game::checkSaveGame(m_game_path, savenames[i], info)) {
                        sprintf(tmp+strlen(tmp),"slot %i: %s\n",i+1,info);
                        active[i] = true;
                    } else {
                        sprintf(tmp+strlen(tmp),"slot %i\n",i+1);
                    }
                }
                sprintf(tmp+strlen(tmp),"back");
                BInterface::push();
                BInterface::createMenu(tmp,m_font16,140,290,360,141,21);
                for(int i = 0;i<4;i++) {
                    if (!active[i]) BInterface::disable(21+i);
                }
            } else {
                char path[1024];
                char fileName[1024];
#ifdef __EMSCRIPTEN__
                sprintf(path, "/%s/slot0",persistent_data_folder);
#else
                sprintf(path, "%s/slot0",m_game_path);
#endif
                sprintf(fileName, "slot0.xml");
                if (m_game!=0) delete m_game;
                m_game = new A4Game(fileName, path, m_game_path, m_GLTM, m_SFXM, m_SFX_volume);

                BInterface::reset();
                return A4ENGINE_STATE_GAME;

            }
            break;
    case 21:
    case 22:
    case 23:
    case 24:
            {
                const char *savenames[]={"slot1","slot2","slot3","slot4"};
                int slot = ID-21;
                char path[1024];
                char fileName[1024];
#ifdef __EMSCRIPTEN__
                sprintf(path, "/%s/%s",persistent_data_folder,savenames[slot]);
#else
                sprintf(path, "%s/%s",m_game_path,savenames[slot]);
#endif
                sprintf(fileName, "%s.xml",savenames[slot]);
                if (m_game!=0) delete m_game;
                m_game = new A4Game(fileName, path, m_game_path, m_GLTM, m_SFXM, m_SFX_volume);

                BInterface::reset();
                return A4ENGINE_STATE_GAME;
            }
            break;
    case 25:
            BInterface::pop();
            break;

	case 3:	// story:
            if (m_game!=0) {
                BInterface::push();
                BInterface::add_element(new BTextFrame(m_game->getGameStory(), false, m_font8, 70, 30, 500, 390));
                BInterface::add_element(new BButton("Back", m_font16, 270, 430, 100, 30, 31));
            }
			break;
	case 31:// back from story
			BInterface::pop();
			break;

	case 4:	// instructions:
            {
                BInterface::push();
                char *instructions = getInstructionsString();
                BInterface::add_element(new BTextFrame(instructions, false, m_font8, 70, 30, 500, 390));
                delete []instructions;
                BInterface::add_element(new BButton("Back", m_font16, 270, 430, 100, 30, 31));
            }
			break;
	case 41:// back from instructions
			BInterface::pop();
			break;

    case 5:	// configuration:
            {
                char tmp[4096] = "";
                const char *keynames[]={"up","down","left","right","force attack","force interact","take/use","talk","drop gold",
                                        "fast forward","zoom","switch player"};
                int keyValues[] = {m_key_up, m_key_down, m_key_left, m_key_right,
                                   m_key_force_attack, m_key_force_interaction, m_key_take, m_key_talk, m_key_drop_gold,
                                   m_key_fast_forward, m_key_zoom, m_key_switch_players};
                bool allowed[12];
                allowed[0] = true;
                allowed[1] = true;
                allowed[2] = true;
                allowed[3] = true;
                
                allowed[4] = m_game->getAllowInventory();
                allowed[5] = true;
                allowed[6] = true;
                allowed[7] = m_game->getAllowTalking();
                allowed[8] = m_game->getAllowStats();
                
                allowed[9] = true;
                allowed[10] = true;
                allowed[11] = true;

                sprintf(tmp,"SFX volume - %i\n", m_SFX_volume);
                for(int i = 0;i<12;i++) {
                    sprintf(tmp+strlen(tmp),"%s - %s\n", keynames[i], SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)keyValues[i])));
                }
                sprintf(tmp+strlen(tmp),"back\n");
                BInterface::push();
                
                BInterface::add_element(new BText("Reserved keys: A, B, C, D, E, F, G, H, I, J, K, L", m_font8, m_screen_dx/2, 180, true));
                BInterface::add_element(new BText("               S, V, P, 1, 2, 3, 4, 5, 6, 7, 8   ", m_font8, m_screen_dx/2, 190, true));
                
                BInterface::createMenu(tmp,m_font8,140,220,360,230,501);
                m_redefining_key = -1;

                for(int i = 0;i<12;i++) {
                    if (!allowed[i]) BInterface::disable(502+i);
                }
            }
            break;
    case 501:
        // volume change
        {
            m_SFX_volume += 16;
            if (m_SFX_volume>MIX_MAX_VOLUME) m_SFX_volume = 0;
            BButtonTransparent *b = (BButtonTransparent *)BInterface::get(501);
            char tmp[1024];
            sprintf(tmp,"SFX volume - %i\n", m_SFX_volume);
            b->changeText(tmp);
            saveConfiguration();
        }
        break;
    case 502:
    case 503:
    case 504:
    case 505:
    case 506:
    case 507:
    case 508:
    case 509:
    case 510:
    case 511:
    case 512:
    case 513:
        {
            int keyToRedefine = ID - 502;
            BInterface::pop();
            {
                char tmp[8192] = "";
                const char *keynames[]={"up","down","left","right","force attack","force interact","take/use","talk","drop gold",
                                        "fast forward","zoom","switch player"};
                int keyValues[] = {m_key_up, m_key_down, m_key_left, m_key_right,
                                   m_key_force_attack, m_key_force_interaction, m_key_take, m_key_talk, m_key_drop_gold,
                                   m_key_fast_forward, m_key_zoom, m_key_switch_players};

                bool allowed[12];
                allowed[0] = true;
                allowed[1] = true;
                allowed[2] = true;
                allowed[3] = true;
                
                allowed[4] = m_game->getAllowInventory();
                allowed[5] = true;
                allowed[6] = true;
                allowed[7] = m_game->getAllowTalking();
                allowed[8] = m_game->getAllowStats();
                
                allowed[9] = true;
                allowed[10] = true;
                allowed[11] = true;
                
                sprintf(tmp,"SFX volume - %i\n", m_SFX_volume);
                
                for(int i = 0;i<12;i++) {
                    if (i==keyToRedefine) {
                        sprintf(tmp+strlen(tmp),"%s - [press a key]\n", keynames[i]);
                    } else {
                        sprintf(tmp+strlen(tmp),"%s - %s\n", keynames[i], SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)keyValues[i])));
                    }
                }
                sprintf(tmp+strlen(tmp),"back\n");
                BInterface::push();
                
                BInterface::add_element(new BText("Reserved keys: A, B, C, D, E, F, G, H, I, J, K, L", m_font8, m_screen_dx/2, 180, true));
                BInterface::add_element(new BText("               S, V, P, 1, 2, 3, 4, 5, 6, 7, 8   ", m_font8, m_screen_dx/2, 190, true));
                
                BInterface::createMenu(tmp,m_font8,140,220,360,230,501);
                m_redefining_key = keyToRedefine;
                
                for(int i = 0;i<12;i++) {
                    if (!allowed[i]) BInterface::disable(502+i);
                }
            }
        }
        break;
    case 514:
            m_redefining_key = -1;
            BInterface::pop();
            break;

    case 6:	// quit:
            BInterface::add_element(new BConfirmation("Are you sure you want to quit?", m_font8, 320, 240, 61, true));
            break;
	case 61:// quit confirmation:
			m_titlescreen_timer = 0;
			m_titlescreen_state = 2;
			m_titlescreen_nextstate = A4ENGINE_STATE_QUIT;
			break;
	}
	return A4ENGINE_STATE_TITLESCREEN;
} /* A4EngineApp::intro_cycle */


void A4EngineApp::titlescreen_draw(void)
{
    if (m_game->getGameTitleImage()!=0) {
        char path[1024];
        sprintf(path,"%s/%s",m_game_path,m_game->getGameTitleImage());
        GLTile *ti = m_GLTM->get(path);
        if (ti!=0) {
            ti->set_hotspot(ti->get_dx()/2,ti->get_dy()/2);
            ti->draw(1,1,1,1,320,240,0,0,1);
        }
    }
    
	m_titlescreen_angle1*=0.85f;
	m_titlescreen_angle2*=0.85f;
	m_titlescreen_angle3*=0.85f;
	m_titlescreen_zoom1 = 1 * 0.15f + m_titlescreen_zoom1 * 0.85f;
	m_titlescreen_zoom2 = 1 * 0.15f + m_titlescreen_zoom2 * 0.85f;
	m_titlescreen_zoom3 = 1 * 0.15f + m_titlescreen_zoom3 * 0.85f;
	m_titlescreen_x1*=0.85f;
	m_titlescreen_x2*=0.85f;
	m_titlescreen_x3*=0.85f;
	m_titlescreen_y1*=0.85f;
	m_titlescreen_y2*=0.85f;
	m_titlescreen_y3*=0.85f;

	if (rand()%100==0) {
		m_titlescreen_angle1 += (-5 + rand()%11) * 0.025f;
		m_titlescreen_angle2 += (-5 + rand()%11) * 0.025f;
		m_titlescreen_angle3 += (-5 + rand()%11) * 0.025f;
		m_titlescreen_zoom1 += (-5 + rand()%11) * 0.04f;
		m_titlescreen_zoom2 += (-5 + rand()%11) * 0.04f;
		m_titlescreen_zoom3 += (-5 + rand()%11) * 0.04f;
		m_titlescreen_x1 += (-8 + rand()%17);
		m_titlescreen_x2 += (-8 + rand()%17);
		m_titlescreen_x3 += (-8 + rand()%17);
		m_titlescreen_y1 += (-4 + rand()%9);
		m_titlescreen_y2 += (-4 + rand()%9);
		m_titlescreen_y3 += (-4 + rand()%9);
	}
    
    if (m_game!=0 && m_game->getGameTitle()!=0) {
        GLTile *t = BInterface::get_text_tile(m_game->getGameTitle(),m_font32);
        t->set_hotspot(t->get_dx()/2,t->get_dy()/2);
        t->draw(1,0.75,0.75,0.5,320+m_titlescreen_x1,120+m_titlescreen_y1,0,m_titlescreen_angle2,m_titlescreen_zoom1);
        t->draw(0.75,0.75,1,0.5,320+m_titlescreen_x2,120+m_titlescreen_y2,0,m_titlescreen_angle3,m_titlescreen_zoom2);
        t->draw(1,1,1,1,320+m_titlescreen_x3,120+m_titlescreen_y3,0,m_titlescreen_angle1,m_titlescreen_zoom3);
    }
    
    if (m_game!=0 && m_game->getGameSubtitle()!=0) {
        GLTile *t = BInterface::get_text_tile(m_game->getGameSubtitle(),m_font16);
        t->set_hotspot(t->get_dx()/2,t->get_dy()/2);
        t->draw(0.75,0.75,0.75,0.5,320,160,0,0,1);
    }

	float f = 1;
	if (m_titlescreen_state==0) {
		f = ((float)m_titlescreen_timer)/50.f;
		if (f<0) f = 0;
		if (f>1) f = 1;
	} else if (m_titlescreen_state==2) {
		f = 1 - ((float)m_titlescreen_timer)/50.f;
		if (f<0) f = 0;
		if (f>1) f = 1;
	}

    BInterface::draw(f);
} /* A4EngineApp::intro_draw */



char *A4EngineApp::getInstructionsString()
{
    char *tmp = new char[25*60];
    tmp[0] = 0;
    strcat(tmp,"Default Controls:\n");
    strcat(tmp,"- Move with the arrow keys.\n");
    if (m_game->getAllowStats()) {
        if (m_game->getAllowTalking()) {
            strcat(tmp,"- Walk into enemies to attack, and into NPCs to talk.\n");
            strcat(tmp,"- SHIFT + arrow keys to force attack\n");
        } else {
            strcat(tmp,"- Walk into enemies to attack.\n");
        }
    } else {
        if (m_game->getAllowTalking()) {
            strcat(tmp,"- Walk into NPCs to talk.\n");
        }
    }
    strcat(tmp,"- ALT + arrow keys to interact (e.g., push, open)\n");
    if (m_game->getAllowTalking()) {
        strcat(tmp,"- 'T' to talk to the closest character.\n");
    }
    strcat(tmp,"- SPACE to take/use/embark/disembark\n");
    strcat(tmp,"- Switch characters with 'N'.\n");

    if (m_game->getAllowInventory()) {
        strcat(tmp,"- Use/equip/unquip items by pressing their number.\n");
        strcat(tmp,"- SHIFT + number to drop an item.\n");
    }
    if (m_game->getAllowStats()) {
        strcat(tmp,"- 'R' to drop gold.\n");
    }
    if (m_game->getAllowMagic()) {
        strcat(tmp,"- letters to cast spells (+ arrow keys for direction\n");
        strcat(tmp,"  or 'enter' to cast on self)\n");
    }
    strcat(tmp,"- 'Z' to zoom in and out.\n");
    strcat(tmp,"- Hold 'TAB' to increase the game speed x4.\n");
    strcat(tmp,"- 'ESC' to pause/load/save/quit.\n");
    strcat(tmp," \n");
    strcat(tmp,"Hints:\n");
    strcat(tmp,"- Levers open/close doors or trigger other secrets.\n");
    if (m_game->getAllowStats()) {
        strcat(tmp,"- You might lose hit points by fighting.\n");
        strcat(tmp,"- Find potions to recover hit/magic points.\n");
    }
    if (m_game->getAllowInventory()) {
        strcat(tmp,"- Remember to equip the most powerful items.\n");
    }
    strcat(tmp,"- Explore methodically, you might find some clues...\n");
    if (m_game->getAllowInventory()) {
        strcat(tmp,"- To fight while on a vehicle, first disembark.");
    }
    
    return tmp;
}

