#ifdef _WIN32
#include "windows.h"
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
#include "A4Map.h"
#include "A4Object.h"
#include "A4Character.h"
#include "A4AICharacter.h"
#include "Animation.h"
#include "A4EngineApp.h"


#ifdef __EMSCRIPTEN__
extern const char *persistent_data_folder;
#endif


#define INGAME_MENU             1
#define INGAME_DROPGOLD_MENU    2
#define INGAME_TALK_MENU        3
#define INGAME_TRADE_MENU       4
#define INGAME_SAVE_MENU        5
#define INGAME_LOAD_MENU        6

int A4EngineApp::game_cycle(KEYBOARDSTATE *k)
{
	if (m_state_cycle==0) {
		BInterface::reset();
		m_ingame_menu = 0;
        m_trade_needs_update = false;
	}

	if (m_ingame_menu == INGAME_MENU) {
		// pause:
		int ID = BInterface::update_state(&m_mouse_clicks, k);
        if (m_game->getAllowSaveGames()) {
            switch(ID) {
                case 1: BInterface::reset();
                        m_ingame_menu = 0;
                        break;
                case 2: // save
    //                    BInterface::push();
    //                    BInterface::createMenu("slot 1\nslot 2\nslot 3\nslot 4\ncancel",m_font16,210,250,220,141,11);
    //                    m_ingame_menu = INGAME_SAVE_MENU;
                        {
                            const char *savenames[]={"slot1","slot2","slot3","slot4"};
                            char tmp[1024];
                            char info[256];
                            tmp[0] = 0;
                            for(int i = 0;i<4;i++) {
                                if (A4Game::checkSaveGame(m_game_path, savenames[i], info)) {
                                    sprintf(tmp+strlen(tmp),"slot %i: %s\n",i+1,info);
                                } else {
                                    sprintf(tmp+strlen(tmp),"slot %i\n",i+1);
                                }
                            }
                            sprintf(tmp+strlen(tmp),"back");
                            BInterface::push();
                            BInterface::createMenu(tmp,m_font16,150,290,360,141,11);
                            m_ingame_menu = INGAME_SAVE_MENU;
                        }
                        break;
                case 3: // load
                        {
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
                            BInterface::createMenu(tmp,m_font16,150,290,360,141,31);
                            for(int i = 0;i<4;i++) {
                                if (!active[i]) BInterface::disable(31+i);
                            }
                            m_ingame_menu = INGAME_LOAD_MENU;
                        }
                        break;
                case 4:	// instructions:
                        {
                            BInterface::push();
                            char *instructions = getInstructionsString();
                            BInterface::add_element(new BTextFrame(instructions, false, m_font8, 70, 30, 500, 390));
                            delete []instructions;
                            BInterface::add_element(new BButton("Back", m_font16, 270, 430, 100, 30, 41));
                            break;
                        }
                case 41:// back from instructions
                        BInterface::pop();
                        break;
                case 5:	// quit:
                        BInterface::add_element(new BConfirmation("Are you sure you want to quit?", m_font8, 320, 240, 51, true));
                        break;
                case 51:// quit confirmation:
                        BInterface::reset();
//                        delete m_game;
//                        m_game = 0;
                        m_ingame_menu = 0;
                        return A4ENGINE_STATE_INTRO;
            }
        } else {
            switch(ID) {
                case 1: BInterface::reset();
                    m_ingame_menu = 0;
                    break;
                case 2:	// instructions:
                    {
                        BInterface::push();
                        char *instructions = getInstructionsString();
                        BInterface::add_element(new BTextFrame(instructions, false, m_font8, 70, 30, 500, 390));
                        delete []instructions;
                        BInterface::add_element(new BButton("Back", m_font16, 270, 430, 100, 30, 21));
                        break;
                    }
                case 21:// back from instructions
                    BInterface::pop();
                    break;
                case 3:	// quit:
                    BInterface::add_element(new BConfirmation("Are you sure you want to save & quit?", m_font8, 320, 240, 31, true));
                    break;
                case 31:// quit confirmation:
                    // save:
                    m_game->saveGame(m_game_path,"slot0");
                    BInterface::reset();
//                    delete m_game;
//                    m_game = 0;
                    m_ingame_menu = 0;
                    return A4ENGINE_STATE_INTRO;
            }
        }
		if (k->key_press(SDL_SCANCODE_ESCAPE)) {
            k->consume_key_press(SDL_SCANCODE_ESCAPE);
			BInterface::reset();
			m_ingame_menu = 0;
		}
	} else if (m_ingame_menu == INGAME_DROPGOLD_MENU) {
		// drop gold:
		int ID = BInterface::update_state(&m_mouse_clicks, k);
		if (ID == 1) {
			// drop gold:
			BTextInputFrame *tmp = (BTextInputFrame *)BInterface::get(1);
			int gold = atoi(tmp->m_editing);
			if (gold>0) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_DROP_GOLD,gold,0);
			BInterface::reset();
			m_ingame_menu = 0;
		}
		if (k->key_press(SDL_SCANCODE_ESCAPE)) {
            k->consume_key_press(SDL_SCANCODE_ESCAPE);
			BInterface::reset();
			m_ingame_menu = 0;
		}
	} else if (m_ingame_menu == INGAME_TALK_MENU) {
		// talk:
		int ID = BInterface::update_state(&m_mouse_clicks, k);
        if (ID>=1) {
            std::vector<SpeechAct *> *l = m_game->getKnownSpeechActs();
            for(SpeechAct *sa:*l) {
                if (ID==1) {
                    m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_TALK,sa, A4_DIRECTION_NONE);
                    BInterface::reset();
                    m_ingame_menu = 0;
                    break;
                }
                ID--;
            }
        } else if (k->key_press(SDL_SCANCODE_ESCAPE)) {
            k->consume_key_press(SDL_SCANCODE_ESCAPE);
			BInterface::reset();
			m_ingame_menu = 0;
		}
    } else if (m_ingame_menu == INGAME_TRADE_MENU) {
        // trade:
        if (m_trade_needs_update) {
            BInterface::reset();
            createTradeDialog(m_trade_dialog_player, m_trade_dialog_other);
            m_trade_needs_update = false;
        }
        if (updateTradeDialog()) {
            int ID = BInterface::update_state(&m_mouse_clicks, k);
            if (ID>=2) {
                if (ID>=100 && ID<200) {
                    // give:
                    m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_GIVE, ID-100, m_trade_dialog_other);
                    m_trade_needs_update = true;
                } else if (ID>=200 && ID<300) {
                    // Sell:
                    m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_SELL, ID-200, m_trade_dialog_other);
                    m_trade_needs_update = true;
                } else if (ID>=300) {
                    // buy:
                    m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_BUY, ID-300, m_trade_dialog_other);
                    m_trade_needs_update = true;
                }
            } else if (k->key_press(SDL_SCANCODE_ESCAPE) || ID==1) {
                k->consume_key_press(SDL_SCANCODE_ESCAPE);
                BInterface::reset();
                m_ingame_menu = 0;
                char *buffer = new char[9]; strcpy(buffer, "endtrade");
                char *buffer2 = new char[11]; strcpy(buffer2, "Thank you!");
                SpeechAct *sa = new SpeechAct(A4_TALK_PERFORMATIVE_END_TRADE, buffer, buffer2);
                m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_TALK,sa, A4_DIRECTION_NONE);
            }
        } else {
            BInterface::reset();
            m_ingame_menu = 0;
        }
    } else if (m_ingame_menu == INGAME_SAVE_MENU) {
        // save game:
        int ID = BInterface::update_state(&m_mouse_clicks, k);
        switch(ID) {
            case 11: // slot 1
                m_game->saveGame(m_game_path,"slot1");
                BInterface::pop();
                m_ingame_menu = INGAME_MENU;
                break;
            case 12: // slot 2
                m_game->saveGame(m_game_path,"slot2");
                BInterface::pop();
                m_ingame_menu = INGAME_MENU;
                break;
            case 13: // slot 3
                m_game->saveGame(m_game_path,"slot3");
                BInterface::pop();
                m_ingame_menu = INGAME_MENU;
                break;
            case 14: // slot 4
                m_game->saveGame(m_game_path,"slot4");
                BInterface::pop();
                m_ingame_menu = INGAME_MENU;
                break;
            case 15:	// cancel:
                BInterface::pop();
                m_ingame_menu = INGAME_MENU;
        }
        if (k->key_press(SDL_SCANCODE_ESCAPE)) {
            k->consume_key_press(SDL_SCANCODE_ESCAPE);
            BInterface::pop();
            m_ingame_menu = INGAME_MENU;
        }
    } else if (m_ingame_menu == INGAME_LOAD_MENU) {
        // load game:
        int ID = BInterface::update_state(&m_mouse_clicks, k);
        if (ID>=31 && ID<=34) {
            const char *savenames[]={"slot1","slot2","slot3","slot4"};
            int slot = ID-31;
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
            m_state_cycle = 0;
            m_ingame_menu = 0;
            return A4ENGINE_STATE_GAME;
        } else if (ID==35) {
            BInterface::pop();
            m_ingame_menu = INGAME_MENU;
        }
        if (k->key_press(SDL_SCANCODE_ESCAPE)) {
            k->consume_key_press(SDL_SCANCODE_ESCAPE);
            BInterface::pop();
            m_ingame_menu = INGAME_MENU;
        }
    } else {
        if (m_game->tradeRequested()!=0) {
            createTradeDialog(m_game->getCurrentPlayer(), m_game->tradeRequested());
            m_trade_needs_update = false;
            m_game->setTradeRequested(0);
            m_ingame_menu = INGAME_TRADE_MENU;
        }
    }

    if (k->keyboard[m_key_fast_forward]) {
        m_cycles_per_frame = 4;
    } else {
        m_cycles_per_frame = 1;
    }

    for(int i = 0;i<m_cycles_per_frame;i++) {
        if (m_ingame_menu == 0) {
            // playing:
            if (k->key_press(SDL_SCANCODE_ESCAPE)) {
                // in-game menu:
                if (m_game->getAllowSaveGames()) {
                    BInterface::createMenu("play\nsave\nload\ninstructions\nquit",m_font16,210,250,220,141,1);
                    m_ingame_menu = INGAME_MENU;
                } else {
                    BInterface::createMenu("play\ninstructions\nsave & quit",m_font16,210,250,220,101,1);
                    m_ingame_menu = INGAME_MENU;
                }
            }

            if (k->key_press(m_key_drop_gold)) {
                BTextInputFrame *tmp = new BTextInputFrame("Gold to drop:","0",10,m_font8,230,208,180,44,1);
                BInterface::add_element(tmp);
                BInterface::setFocus(1);
                tmp->setFocus();
                m_ingame_menu = INGAME_DROPGOLD_MENU;
            }

            // HUD:
            if (k->key_press(m_key_stats_toggle)) m_game->playerInput_ToogleStats();
            if (k->key_press(m_key_inventory_toggle)) m_game->playerInput_ToogleInventory();
            if (k->key_press(m_key_spells_toggle)) m_game->playerInput_ToogleSpells();

            // zoom:
            if (k->key_press(m_key_zoom)) m_game->playerInput_ToogleZoom();

            // player switch:
            if (k->key_press(m_key_switch_players)) m_game->playerInput_SwitchPlayers();

            // console control:
            if (k->key_press(m_key_messageconsole_up)) m_game->messageConsoleUp();
            if (k->key_press(m_key_messageconsole_down)) m_game->messageConsoleDown();

            // issuing player commands:
            if (k->keyboard[m_key_force_interaction]) {
                if (k->keyboard[m_key_left]) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_INTERACT,0,A4_DIRECTION_LEFT);
                if (k->keyboard[m_key_up]) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_INTERACT,0,A4_DIRECTION_UP);
                if (k->keyboard[m_key_right]) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_INTERACT,0,A4_DIRECTION_RIGHT);
                if (k->keyboard[m_key_down]) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_INTERACT,0,A4_DIRECTION_DOWN);
            } else if (k->keyboard[m_key_force_attack]) {
                if (k->keyboard[m_key_left]) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_ATTACK,0,A4_DIRECTION_LEFT);
                if (k->keyboard[m_key_up]) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_ATTACK,0,A4_DIRECTION_UP);
                if (k->keyboard[m_key_right]) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_ATTACK,0,A4_DIRECTION_RIGHT);
                if (k->keyboard[m_key_down]) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_ATTACK,0,A4_DIRECTION_DOWN);
            } else {
                bool anySpellPressed = false;
                if (m_game->getAllowMagic()) {
                    for(int i = 0;i<A4_N_SPELLS;i++) {
                        if (k->keyboard[SDL_SCANCODE_D+i]) {
                            if (k->key_press(SDL_SCANCODE_RETURN)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_SPELL,i,A4_DIRECTION_NONE);
                            if (k->key_press(m_key_left)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_SPELL,i,A4_DIRECTION_LEFT);
                            if (k->key_press(m_key_up)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_SPELL,i,A4_DIRECTION_UP);
                            if (k->key_press(m_key_right)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_SPELL,i,A4_DIRECTION_RIGHT);
                            if (k->key_press(m_key_down)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_SPELL,i,A4_DIRECTION_DOWN);
                            anySpellPressed = true;
                        }
                    }
                }
                if (!anySpellPressed) {
                    int command = -1;
                    int direction = A4_DIRECTION_NONE;
                    if (k->keyboard[m_key_left]) direction = A4_DIRECTION_LEFT;
                    if (k->keyboard[m_key_up]) direction = A4_DIRECTION_UP;
                    if (k->keyboard[m_key_right]) direction = A4_DIRECTION_RIGHT;
                    if (k->keyboard[m_key_down]) direction = A4_DIRECTION_DOWN;
                    if (direction != A4_DIRECTION_NONE) command = m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_WALK,0,direction);
                    if (m_game->getAllowTalking() && command == A4CHARACTER_COMMAND_TALK) {
                        // find who do we want to talk to:
                        A4Map *map = m_game->getCurrentPlayer()->getMap();
                        std::vector<A4Object *> *collisions = map->getAllObjectCollisions(m_game->getCurrentPlayer(), A4Game::direction_x_inc[direction], A4Game::direction_y_inc[direction]);
                        for(A4Object *o:*collisions) {
                            if (o->isCharacter()) {
                                createTalkDialog((A4Character *)o);
                                m_ingame_menu = INGAME_TALK_MENU;
                            }
                        }
                        delete collisions;
                    }
                }
            }
            
            if (k->key_press(m_key_take)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_TAKE,0,A4_DIRECTION_NONE);
            
            if (m_game->getAllowInventory()) {
                for(int i = 0;i<A4_INVENTORY_SIZE;i++) {
                    // check if it's a wand:
                    A4Object *o = m_game->getCurrentPlayer()->getInventory(i);
                    if (o!=0 && o->isWand()) {
                        if (k->key_press(SDL_SCANCODE_1+i)) {
                            if (k->keyboard[SDL_SCANCODE_LSHIFT] || k->keyboard[SDL_SCANCODE_RSHIFT]) {
                                m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_DROP,i,0);
                            }
                        }
                        if (k->keyboard[SDL_SCANCODE_1+i]) {
                            if (k->key_press(SDL_SCANCODE_RETURN)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_USE,i,A4_DIRECTION_NONE);
                            if (k->key_press(m_key_left)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_USE,i,A4_DIRECTION_LEFT);
                            if (k->key_press(m_key_up)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_USE,i,A4_DIRECTION_UP);
                            if (k->key_press(m_key_right)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_USE,i,A4_DIRECTION_RIGHT);
                            if (k->key_press(m_key_down)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_USE,i,A4_DIRECTION_DOWN);
                        }
                    } else {
                        if (k->key_press(SDL_SCANCODE_1+i)) {
                            if (k->keyboard[SDL_SCANCODE_LSHIFT] || k->keyboard[SDL_SCANCODE_RSHIFT]) {
                                m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_DROP,i,0);
                            } else {
                                m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_USE,i,0);
                            }
                        }
                    }
                }
                if (k->key_press(SDL_SCANCODE_A)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_UNEQUIP,0,0);
                if (k->key_press(SDL_SCANCODE_B)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_UNEQUIP,1,0);
                if (k->key_press(SDL_SCANCODE_C)) m_game->playerInput_issueCommand(A4CHARACTER_COMMAND_UNEQUIP,2,0);
            }
            
            if (m_game->getAllowTalking() &&  k->key_press(m_key_talk)) {
                createTalkDialog(0);
                m_ingame_menu = INGAME_TALK_MENU;
            }

        }

        if (m_ingame_menu != 1 && m_ingame_menu != 5 && m_ingame_menu != 6) {
			if (!m_game->cycle(k)) {
                if (!m_game->getAllowSaveGames()) {
                    // delete savegame if it exists:
                    char path[1024];
#ifdef __EMSCRIPTEN__
                    sprintf(path, "/%s/slot0",persistent_data_folder);
#else
                    sprintf(path, "%s/slot0",m_game_path);
#endif
                    remove_dir(path);
                }
				return A4ENGINE_STATE_INTRO;
			}
            if (m_game->getGameComplete()) {
                if (!m_game->getAllowSaveGames()) {
                    // delete savegame:
                    char path[1024];
#ifdef __EMSCRIPTEN__
                    sprintf(path, "/%s/slot0",persistent_data_folder);
#else
                    sprintf(path, "%s/slot0",m_game_path);
#endif
                    remove_dir(path);
                }
                return A4ENGINE_STATE_GAMECOMPLETE;
            }
		}
	}

	return A4ENGINE_STATE_GAME;
} /* A4EngineApp::intro_cycle */


void A4EngineApp::game_draw(void)
{
	m_game->draw(m_screen_dx,m_screen_dy);

	if (m_cycles_per_frame>1) {
		if ((m_state_cycle%16)<8) {
			char tmp[80];
			sprintf(tmp,"x%i",m_cycles_per_frame);
			BInterface::print_left(tmp,m_font16,8,m_screen_dy-(16+4*10));
		}
	}

    BInterface::draw();
} /* A4EngineApp::intro_draw */


void A4EngineApp::createTalkDialog(class A4Character *target)
{
    if (target==0) {
        // find closest target:
        A4Character *player = m_game->getCurrentPlayer();
        A4Map *map = player->getMap();
        std::vector<A4Object *> *candidates = map->getAllObjects(player->getX() - 5*map->getTileWidth(), player->getY() - 5*map->getTileWidth(),
                                                               player->getPixelWidth() + 10*map->getTileWidth(),
                                                               player->getPixelHeight() + 10*map->getTileHeight());
        A4Object *closest = 0;
        int closest_d = 0;
        for(A4Object *o:*candidates) {
            if (o!=player && o->isCharacter()) {
                int d = o->pixelDistance(player);
                if (closest==0 || d<closest_d) {
                    closest = o;
                    closest_d = d;
                }
            }
        }
        if (closest!=0) target = (A4Character *)closest;
    }

    std::vector<SpeechAct *> *l = m_game->getKnownSpeechActs();

    BInterface::reset();
    int n = (int)l->size();
    int x = 230;
    int y = 200;
    int dx = 180;
    int dy = 40+n*15;
    int by = y+14;
    int bdy = m_font8->getHeight();
    int starting_ID = 1;
    BFrame *frame = new BFrame(x,y,dx,dy);
    BInterface::add_element(frame);
    // add target name and a horizontal line
    char buffer[256];
    sprintf(buffer,"Talk to: %s",(target==0 ? "-":target->getName()->get()));
    int name_width = (int)strlen(buffer)*9;
    if (name_width+16 > dx) {
        dx = name_width+16;
        x = 320 - dx/2;
        frame->setX(x);
        frame->setDx(dx);
    }
    BInterface::add_element(new BText(buffer, m_font8,
                                      x+dx/2-((strlen(buffer)*9)/2),
                                      by,false));
    by+=bdy-4;
    // horizontal line
    BInterface::add_element(new BQuad(x,by,dx,2, 1, 1, 1, 1));
    by+=10;
    for(SpeechAct *sa:*l) {
        switch(sa->performative) {
            case A4_TALK_PERFORMATIVE_HI:
                sprintf(buffer,"Greet");
                break;
            case A4_TALK_PERFORMATIVE_BYE:
                sprintf(buffer,"Good bye");
                break;
            case A4_TALK_PERFORMATIVE_ASK:
                sprintf(buffer,"Ask: %s",sa->keyword);
                break;
            case A4_TALK_PERFORMATIVE_INFORM:
                sprintf(buffer,"Inform: %s",sa->keyword);
                break;
            case A4_TALK_PERFORMATIVE_TRADE:
                sprintf(buffer,"Buy/sell/give");
                break;
            default:
                sprintf(buffer,"???");
                break;
        }
        int text_width = (int)strlen(buffer)*9;
        if (text_width+16 > dx) {
            dx = text_width+16;
            x = 320 - dx/2;
            frame->setX(x);
            frame->setDx(dx);
        }

        // determine whether the speech act will make the other character angry:
        bool acceptable = false;
        if (target!=0 && target->isAICharacter()) {
            if (((A4AICharacter *)target)->getAI()->willAcceptSpeechAct(m_game->getCurrentPlayer(), target, sa)) acceptable = true;
        }
        if (acceptable) {
            BInterface::add_element(new BButtonTransparent((const char *)buffer,m_font8,x,by,dx,bdy,starting_ID++));
        } else {
            BInterface::add_element(new BButtonTransparent((const char *)buffer,m_font8,x,by,dx,bdy,starting_ID++, 1, 0.25, 0.25, 1));
        }
        by += bdy+4;
    }
}


void A4EngineApp::createTradeDialog(A4Character *player, A4Character *other)
{
    output_debug_message("createTradeDialog: %i %i %s\n", m_state_cycle, m_ingame_menu, m_trade_needs_update ? "true":"false");

    m_trade_dialog_player = player;
    m_trade_dialog_other = other;

    int x = 80;
    int y = 120;
    int dx = 480;
    int dy = 224;
    char buffer[32];
    BInterface::add_element(new BFrame(x,y,dx,dy));
    BInterface::add_element(new BInterfaceAnimation(new Animation(m_trade_dialog_player->getAnimation()), x+16, y+16));
    BInterface::add_element(new BInterfaceAnimation(new Animation(m_trade_dialog_other->getAnimation()), x+dx-(m_trade_dialog_other->getPixelWidth()+16), y+16));
    sprintf(buffer,"%s (%i)",m_trade_dialog_player->getName()->get(), m_trade_dialog_player->getGold());
    BInterface::add_element(new BText(buffer, m_font8, x+16,y+24+m_trade_dialog_player->getPixelHeight(),false));
    sprintf(buffer,"%s (%i)",m_trade_dialog_other->getName()->get(), m_trade_dialog_other->getGold());
    BInterface::add_element(new BText(buffer, m_font8,
                                      x+dx-(16+(strlen(buffer)*9)),
                                      y+24+m_trade_dialog_other->getPixelHeight(),false));

    int header_y = y+16+std::max(m_trade_dialog_player->getPixelHeight(), m_trade_dialog_other->getPixelHeight())+8+8+16;
    BInterface::add_element(new BText("GIVE", m_font8, x+16,header_y,false));
    BInterface::add_element(new BText("SELL", m_font8, x+16+20*8+16,header_y,false));
    BInterface::add_element(new BText("BUY",  m_font8, x+dx/2+16,header_y,false));
    BInterface::add_element(new BQuad(x+16,header_y+8+3,dx-32,2, 1, 1, 1, 1));
    BInterface::add_element(new BQuad(x+dx/2-1,header_y+8+3,2,9*10, 1, 1, 1, 1));

    // player items:
    int inv_y = header_y + 16;
    int ID = 0;
    for(A4Object *o:*(m_trade_dialog_player->getInventory())) {
        BInterface::add_element(new BButtonTransparent(o->getName()->get(), m_font8, x+16, inv_y, strlen(o->getName()->get())*9, 10, ID+100));
        sprintf(buffer,"%i",o->getGold());
        BInterface::add_element(new BButtonTransparent(buffer, m_font8, x+16+20*8+16, inv_y, strlen(buffer)*9, 10, ID+200));
        ID++;
        inv_y+=10;
    }

    inv_y = header_y + 16;
    ID = 0;
    for(A4Object *o:*(m_trade_dialog_other->getInventory())) {
        char buffer[64];
        sprintf(buffer,"%s",o->getName()->get());
        while(strlen(buffer)<18) sprintf(buffer+strlen(buffer)," ");
        sprintf(buffer+strlen(buffer),"%i",o->getGold());
        BInterface::add_element(new BButtonTransparent(buffer, m_font8, x+dx/2+16, inv_y, strlen(buffer)*9, 10, ID+300));
        if (o->getGold()==0) BInterface::disable(ID+300);
        ID++;
        inv_y+=10;
    }

    BInterface::add_element(new BButtonTransparent("done", m_font8, x+dx/2-32, header_y+112, 64, 10, 1));
}


bool A4EngineApp::updateTradeDialog()
{
    // if any of the characters dies, trading is over!
    if (!m_game->contains(m_trade_dialog_player) || !m_game->contains(m_trade_dialog_other)) return false;

    return true;
}


