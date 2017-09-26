#ifdef _WIN32
#include <windows.h>
#include "direct.h"
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
#include <vector>
#include <map>
#include <algorithm>

#include "SDLauxiliar.h"
#include "GLauxiliar.h"
#include "A4auxiliar.h"

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
#include "Animation.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4Object.h"
#include "A4ObjectFactory.h"
#include "A4Game.h"
#include "A4Character.h"
#include "A4AICharacter.h"
#include "A4Vehicle.h"

#include "Ontology.h"
#include "A4AI.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif


const char *A4Game::animationNames[] = {
	"idle",
	"idle-left",
	"idle-up",
	"idle-right",
	"idle-down",
	"moving",
	"moving-left",
	"moving-up",
	"moving-right",
	"moving-down",
	"attacking",
	"attacking-left",
	"attacking-up",
	"attacking-right",
	"attacking-down",
	"interacting",
	"interacting-left",
	"interacting-up",
	"interacting-right",
	"interacting-down",
	"casting",
	"casting-left",
	"casting-up",
	"casting-right",
	"casting-down",
	"talking",
	"talking-left",
	"talking-up",
	"talking-right",
	"talking-down",
	"death",
	"death-left",
	"death-up",
	"death-right",
	"death-down",
    "open",
    "closed"
};

const char *A4Game::spellNames[] = {
	"magic missile",
	"heal",
	"shield",
	"increase",
	"decrease",
	"fireball",
	"magic eye",
	"regenerate",
	"incinerate",
};

const char *A4Game::spellAlternativeNames[] = {
	"SPELL_MAGIC_MISSILE",
	"SPELL_HEAL",
	"SPELL_SHIELD",
	"SPELL_INCREASE",
	"SPELL_DECREASE",
	"SPELL_FIREBALL",
	"SPELL_MAGIC_EYE",
	"SPELL_REGENERATE",
	"SPELL_INCINERATE",
};

const int A4Game::spellCost[] = {1,1,2,2,2,4,4,8,8};

Symbol *A4Game::emotionNames[] = {0,0,0,0,0,0};

const int A4Game::direction_x_inc[] = {-1,0,1,0};
const int A4Game::direction_y_inc[] = {0,-1,0,1};

const char *A4Game::performativeNames[] = {
    "hi",
    "bye",
    "ask",
    "inform",
    "trade",
    "endtrade",
    "timeout"
};

#ifdef __EMSCRIPTEN__
extern const char *persistent_data_folder;
#endif

extern bool sound;

A4Game::A4Game(const char *game_file, const char *game_path, const char *backup_game_path, GLTManager *GLTM, class SFXManager *SFXM, int a_sfx_volume)
{
    char fullPath[2048];
    sprintf(fullPath,"%s/%s",game_path, game_file);
    XMLNode *xml = XMLNode::from_file(fullPath);
    loadContentFromXML(xml, game_path, backup_game_path, GLTM, SFXM);
    delete xml;

    sfx_volume = a_sfx_volume;
    m_trade_requested = 0;
}


A4Game::A4Game(XMLNode *xml, const char *game_path, GLTManager *GLTM, SFXManager *SFXM, int a_sfx_volume)
{
    loadContentFromXML(xml, game_path, game_path, GLTM, SFXM);

    sfx_volume = a_sfx_volume;
    m_trade_requested = 0;
}


A4Game::~A4Game()
{
    Sort::clearPrecomputedIsA();

    delete m_font8;
    m_font8 = 0;

    delete m_font4;
    m_font4 = 0;

    if (m_gameName!=0) delete m_gameName;
    m_gameName = 0;
    if (m_storyText!=0) delete m_storyText;
    m_storyText = 0;
    for(int i = 0;i<m_n_endings;i++) {
        if (m_endingTexts[i]!=0) delete m_endingTexts[i];
        m_endingTexts[i] = 0;
        if (m_endingIDs[i]!=0) delete m_endingIDs[i];
        m_endingIDs[i] = 0;
        for(char *l:*(m_endingText_raw[i])) delete l;
        m_endingText_raw[i]->clear();
        delete m_endingText_raw[i];
        m_endingText_raw[i] = 0;
    }
    if (m_endingTexts!=0) delete []m_endingTexts;
    m_endingTexts = 0;
    if (m_endingIDs!=0) delete []m_endingIDs;
    m_endingIDs = 0;
    if (m_endingText_raw!=0) delete []m_endingText_raw;
    m_endingText_raw = 0;
    
    if (m_gameTitle!=0) delete m_gameTitle;
    m_gameTitle = 0;
    if (m_gameSubtitle!=0) delete m_gameSubtitle;
    m_gameSubtitle = 0;
    if (m_gameTitleImage!=0) delete m_gameTitleImage;
    m_gameTitleImage = 0;
    for(char *l:m_storyText_raw) delete l;
    m_storyText_raw.clear();

    m_players.clear();
    for(char *l:m_characterDefinitionFiles) delete l;
    m_characterDefinitionFiles.clear();
    for(char *l:m_objectDefinitionFiles) delete l;
    m_objectDefinitionFiles.clear();

    if (m_coinpurse_animation!=0) delete m_coinpurse_animation;
    m_coinpurse_animation = 0;
    for(int i = 0;i<A4_N_EMOTIONS;i++) {
        if (m_emotion_animation[i] != 0) {
            delete m_emotion_animation[i];
            m_emotion_animation[i] = 0;
        }
    }
    delete m_emotion_animation;
    for(int i = 0;i<A4_N_SPELLS;i++) {
        for(int j = 0;j<A4_N_ANIMATIONS;j++) {
            if (m_spell_animation[i][j] != 0) {
                delete m_spell_animation[i][j];
                m_spell_animation[i][j] =0;
            }
        }
        delete m_spell_animation[i];
        m_spell_animation[i] = 0;
    }
    delete m_spell_animation;

    for(GraphicFile *gf:m_graphicFiles) delete gf;
    m_graphicFiles.clear();
    for(A4Map *m:m_maps) delete m;
    m_maps.clear();
    m_GLTM = 0;
    m_game_path = 0;

    if (m_objectFactory!=0) delete m_objectFactory;
    m_objectFactory = 0;

    for(char *m:m_messages) delete m;
    m_messages.clear();

    for(SpeechAct *s:m_known_speech_acts) delete s;
    m_known_speech_acts.clear();

    for(int i = 0;i<A4_NEVENTS;i++) {
        for(A4EventRule *s:m_event_scripts[i]) delete s;
        m_event_scripts[i].clear();
    }

    for(A4ScriptExecutionQueue *dv:m_script_queues) delete dv;
    m_script_queues.clear();

    for(char *a:m_storyStateVariables) delete a;
    m_storyStateVariables.clear();
    for(char *a:m_storyStateValues) delete a;
    m_storyStateValues.clear();
    for(SpeechAct *a:m_known_speech_acts) delete a;
    m_known_speech_acts.clear();

    delete m_ontology;
    m_ontology = 0;
}


/*

 - "game_path" is the place from where we are loading (i.e., could be a save file)
 - "real_game_path" is the actual game path of the original files

 */
void A4Game::loadContentFromXML(XMLNode *xml, const char *game_path, const char *real_game_path, GLTManager *GLTM, class SFXManager *SFXM)
{
    A4Object::s_nextID = 1;
    if (emotionNames[A4_EMOTION_HAPPY]==0) {
        emotionNames[A4_EMOTION_DEFAULT] =0;
        emotionNames[A4_EMOTION_HAPPY] = new Symbol("happy");
        emotionNames[A4_EMOTION_ANGRY] = new Symbol("angry");
        emotionNames[A4_EMOTION_SCARED] = new Symbol("scared");
        emotionNames[A4_EMOTION_CURIOUS] = new Symbol("curious");
        emotionNames[A4_EMOTION_TIRED] = new Symbol("tired");
    }

    m_HUD_show_stats = false;
    m_HUD_show_inventory = false;
    m_HUD_show_spells = false;
    m_console_first_message = -1;

	m_game_path = real_game_path;
	m_GLTM = GLTM;
	m_SFXM = SFXM;
	m_objectFactory = new A4ObjectFactory();

	m_font8 = new BitmapFont("fonts/emulogic8.png");
    m_font4 = new BitmapFont("fonts/novaterm-4x8.png");

	m_coinpurse_animation = 0;
	m_emotion_animation = new Animation *[A4_N_EMOTIONS];
	for(int i = 0;i<A4_N_EMOTIONS;i++) {
		m_emotion_animation[i] = 0;
	}
	m_spell_animation = new Animation **[A4_N_SPELLS];
	for(int i = 0;i<A4_N_SPELLS;i++) {
		m_spell_animation[i] = new Animation *[A4_N_ANIMATIONS];
		for(int j = 0;j<A4_N_ANIMATIONS;j++) {
			m_spell_animation[i][j] = 0;
		}
	}

    m_ontology = new Ontology();
    A4AI::initSorts(m_ontology);

	addSpeechAct(A4_TALK_PERFORMATIVE_HI,"hi","Good day!");
	addSpeechAct(A4_TALK_PERFORMATIVE_BYE,"bye","Farewell!");
	addSpeechAct(A4_TALK_PERFORMATIVE_TRADE,"trade","Would you like to trade?");

    char *tmp = xml->get_attribute("name");
    if (tmp==0) {
        m_gameName = 0;
    } else {
        m_gameName = new char[strlen(tmp)+1];
        strcpy(m_gameName,tmp);
    }
    tmp = xml->get_attribute("title");
    if (tmp==0) {
        m_gameTitle = 0;
    } else {
        m_gameTitle = new char[strlen(tmp)+1];
        strcpy(m_gameTitle,tmp);
    }
    tmp = xml->get_attribute("subtitle");
    if (tmp==0) {
        m_gameSubtitle = 0;
    } else {
        m_gameSubtitle = new char[strlen(tmp)+1];
        strcpy(m_gameSubtitle,tmp);
    }

    XMLNode *titleXML = xml->get_child("titleImage");
    if (titleXML==0) {
        m_gameTitleImage = 0;
    } else {
        tmp = titleXML->get_value();
        m_gameTitleImage = new char[strlen(tmp)+1];
        strcpy(m_gameTitleImage,tmp);
    }
    
    m_allowSaveGames = true;
    m_allowTalking = true;
    m_allowMagic = true;
    m_allowInventory = true;
    m_allowStats = true;

    tmp = xml->get_attribute("allowSaveGames");
    if (tmp!=0) m_allowSaveGames = strcmp(tmp,"true")==0;
    tmp = xml->get_attribute("allowTalking");
    if (tmp!=0) m_allowTalking = strcmp(tmp,"true")==0;
    tmp = xml->get_attribute("allowMagic");
    if (tmp!=0) m_allowMagic = strcmp(tmp,"true")==0;
    tmp = xml->get_attribute("allowInventory");
    if (tmp!=0) m_allowInventory = strcmp(tmp,"true")==0;
    tmp = xml->get_attribute("allowStats");
    if (tmp!=0) m_allowStats = strcmp(tmp,"true")==0;

    {
        XMLNode *story_xml = xml->get_child("story");
        std::vector<XMLNode *> *story_lines = story_xml->get_children("line");
        int len = 0;
        for(XMLNode *line:*story_lines) {
            len += strlen(line->get_value())+1;
            char *tmp = new char[strlen(line->get_value())+1];
            strcpy(tmp, line->get_value());
            m_storyText_raw.push_back(tmp);
        }
        m_storyText = new char[len+1];
        m_storyText[0]=0;
        for(XMLNode *line:*story_lines) {
            strcat(m_storyText,line->get_value());
            strcat(m_storyText,"\n");
        }
        delete story_lines;
    }
    {
        m_n_endings = 0;
        m_endingTexts = 0;
        m_endingIDs = 0;
        m_endingText_raw = 0;
        
        for(XMLNode *ending_xml:*(xml->get_children("ending"))) {
            if (ending_xml->get_attribute("id")==0) {
                output_debug_message("Ending in game definition file does not specify an ID!\n");
                exit(1);
            }
            char *ID = new char[strlen(ending_xml->get_attribute("id"))+1];
            strcpy(ID, ending_xml->get_attribute("id"));
            std::vector<XMLNode *> *ending_lines = ending_xml->get_children("line");
            int len = 0;
            std::vector<char *> *ending = new std::vector<char *>();
            for(XMLNode *line:*ending_lines) {
                len += strlen(line->get_value())+1;
                char *tmp = new char[strlen(line->get_value())+1];
                strcpy(tmp, line->get_value());
                ending->push_back(tmp);
            }
            char *endingText = new char[len+1];
            endingText[0]=0;
            for(XMLNode *line:*ending_lines) {
                strcat(endingText,line->get_value());
                strcat(endingText,"\n");
            }
            addEnding(ID, ending, endingText);
            
            delete ending_lines;
            ending_lines = 0;
        }
    }

	XMLNode *tiles_xml = xml->get_child("tiles");
	{
		std::vector<XMLNode *> *children = tiles_xml->get_children();
		int targetwidth = atoi(tiles_xml->get_attribute("targetwidth"));
		m_tile_dx = atoi(tiles_xml->get_attribute("sourcewidth"));
		m_tile_dy = atoi(tiles_xml->get_attribute("sourceheight"));

		m_default_zoom = float(targetwidth)/m_tile_dx;

		for(XMLNode *c:*children) {
//			output_debug_message("processing tiles child: %s\n",c->get_type()->get());
			char *file = c->get_attribute("file");
			char *value = c->get_value();
			GraphicFile *gf = getGraphicFile(file);
			if (gf==0) {
				output_debug_message("graphic file not found... %s\n",file);
                if (fileExists(game_path,file)) {
                    gf = new GraphicFile(file,m_tile_dx,m_tile_dy,game_path,m_GLTM);
                } else {
                    gf = new GraphicFile(file,m_tile_dx,m_tile_dy,real_game_path,m_GLTM);
                }
				m_graphicFiles.push_back(gf);
//				output_debug_message("Creating GraphicFile %s with %i tiles\n",file,ntiles);
			}
			if (c->get_type()->cmp("types")) {
				char buffer[256];
				int ntile = 0;
				int j = 0;
				for(int i = 0;value[i]!=0;i++) {
					if (value[i]==',') {
						if (ntile>=gf->m_n_tiles) {
							output_debug_message("more tiles than expected while reading types definition.");
							break;
						}
						buffer[j++] = 0;
						int v = 0;
						sscanf(buffer,"%i",&v);
						gf->m_tileTypes[ntile] = v;
						ntile++;
						j = 0;
					} else if (value[i]!=' ' && value[i]!='\t' && value[i]!='\n' && value[i]!='\r') {
						buffer[j++] = value[i];
					}
					if (j>=256) {
						output_debug_message("value longer than 256 characters while reading types definition.");
						break;
					}
				}
			} else if (c->get_type()->cmp("seeThrough")) {
				int ntile = 0;
				int j = 0;
				char buffer[256];
				for(int i = 0;value[i]!=0;i++) {
					if (value[i]==',') {
						buffer[j++] = 0;
						int v = 0;
						sscanf(buffer,"%i",&v);
						gf->m_tileSeeThrough[ntile] = v;
						ntile++;
						j=0;
					} else if (value[i]!=' ' && value[i]!='\t' && value[i]!='\n' && value[i]!='\r') {
						buffer[j++] = value[i];
					}
					if (j>=256) {
						output_debug_message("value longer than 256 characters while reading types definition.");
						break;
					}
				}
			} else if (c->get_type()->cmp("canDig")) {
				int ntile = 0;
				int j = 0;
				char buffer[256];
				for(int i = 0;value[i]!=0;i++) {
					if (value[i]==',') {
						buffer[j++] = 0;
						int v = 0;
						sscanf(buffer,"%i",&v);
						gf->m_tileCanDig[ntile] = v;
						ntile++;
						j=0;
					} else if (value[i]!=' ' && value[i]!='\t' && value[i]!='\n' && value[i]!='\r') {
						buffer[j++] = value[i];
					}
					if (j>=256) {
						output_debug_message("value longer than 256 characters while reading types definition.");
						break;
					}
				}
			} else if (c->get_type()->cmp("animation")) {
				Animation *a = new Animation(c, this);
				if (strcmp(c->get_attribute("name"),"coinpurse")==0) {
					m_coinpurse_animation = a;
				} else if (strcmp(c->get_attribute("name"),"happy")==0) {
					m_emotion_animation[A4_EMOTION_HAPPY] = a;
				} else if (strcmp(c->get_attribute("name"),"angry")==0) {
					m_emotion_animation[A4_EMOTION_ANGRY] = a;
				} else if (strcmp(c->get_attribute("name"),"scared")==0) {
					m_emotion_animation[A4_EMOTION_SCARED] = a;
				} else if (strcmp(c->get_attribute("name"),"curious")==0) {
					m_emotion_animation[A4_EMOTION_CURIOUS] = a;
				} else if (strcmp(c->get_attribute("name"),"tired")==0) {
					m_emotion_animation[A4_EMOTION_TIRED] = a;
				} else if (strcmp(c->get_attribute("name"),"magic missile")==0) {
					m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING] = a;
				} else if (strcmp(c->get_attribute("name"),"magic missile-left")==0) {
					m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_LEFT] = a;
				} else if (strcmp(c->get_attribute("name"),"magic missile-up")==0) {
					m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_UP] = a;
				} else if (strcmp(c->get_attribute("name"),"magic missile-right")==0) {
					m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_RIGHT] = a;
				} else if (strcmp(c->get_attribute("name"),"magic missile-down")==0) {
					m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_DOWN] = a;
				} else if (strcmp(c->get_attribute("name"),"fireball")==0) {
					m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING] = a;
				} else if (strcmp(c->get_attribute("name"),"fireball-left")==0) {
					m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_LEFT] = a;
				} else if (strcmp(c->get_attribute("name"),"fireball-up")==0) {
					m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_UP] = a;
				} else if (strcmp(c->get_attribute("name"),"fireball-right")==0) {
					m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_RIGHT] = a;
				} else if (strcmp(c->get_attribute("name"),"fireball-down")==0) {
					m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_DOWN] = a;
				} else if (strcmp(c->get_attribute("name"),"incinerate")==0) {
					m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING] = a;
				} else if (strcmp(c->get_attribute("name"),"incinerate-left")==0) {
					m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_LEFT] = a;
				} else if (strcmp(c->get_attribute("name"),"incinerate-up")==0) {
					m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_UP] = a;
				} else if (strcmp(c->get_attribute("name"),"incinerate-right")==0) {
					m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_RIGHT] = a;
				} else if (strcmp(c->get_attribute("name"),"incinerate-down")==0) {
					m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_DOWN] = a;
				} else {
					output_debug_message("ignoring animation %s...\n",c->get_attribute("name"));
					delete a;
				}
			}
		}
	}

	// loading object types:
	{
		std::vector<XMLNode *> *files_xml = xml->get_children("objectDefinition");
		for(XMLNode *file_xml:*files_xml) {
			char *objects_file = file_xml->get_attribute("file");
            char *tmp = new char[strlen(objects_file)+1];
            strcpy(tmp,objects_file);
            m_objectDefinitionFiles.push_back(tmp);
			char fullpath[2048];
            XMLNode *objects_xml = 0;
            if (fileExists(objects_file)) {
                sprintf(fullpath,"%s",objects_file);
                objects_xml = XMLNode::from_file(fullpath);
            } else if (fileExists(game_path,objects_file)) {
                sprintf(fullpath,"%s/%s",game_path,objects_file);
                objects_xml = XMLNode::from_file(fullpath);
            } else {
                sprintf(fullpath,"%s/%s",real_game_path,objects_file);
                objects_xml = XMLNode::from_file(fullpath);
            }
			m_objectFactory->addObjectDefinitions(objects_xml, this);
            delete objects_xml;
		}
		delete files_xml;
	}
	// loading character types:
	{
		std::vector<XMLNode *> *files_xml = xml->get_children("characterDefinition");
		for(XMLNode *file_xml:*files_xml) {
			char *objects_file = file_xml->get_attribute("file");
            char *tmp = new char[strlen(objects_file)+1];
            strcpy(tmp,objects_file);
            m_characterDefinitionFiles.push_back(tmp);
            char fullpath[2048];
            XMLNode *characters_xml = 0;
            if (fileExists(objects_file)) {
                sprintf(fullpath,"%s",objects_file);
                characters_xml = XMLNode::from_file(fullpath);
            } else if (fileExists(game_path,objects_file)) {
                sprintf(fullpath,"%s/%s",game_path,objects_file);
                characters_xml = XMLNode::from_file(fullpath);
            } else {
                sprintf(fullpath,"%s/%s",real_game_path,objects_file);
                characters_xml = XMLNode::from_file(fullpath);
            }
			m_objectFactory->addCharacterDefinitions(characters_xml, this);
            delete characters_xml;
		}
		delete files_xml;
	}
	// loading maps:
    std::vector<std::pair<XMLNode *, A4Object *>> objectsToRevisit;
    int respawnID = 1;
	{
		std::vector<XMLNode *> *maps_xml = xml->get_children("map");
		for(XMLNode *map_xml:*maps_xml) {
			char *tmx_file = map_xml->get_attribute("file");
            char fullpath[2048];
            XMLNode *tmx = 0;
            if (fileExists(tmx_file)) {
                sprintf(fullpath,"%s",tmx_file);
                tmx = XMLNode::from_file(fullpath);
            } else if (fileExists(game_path,tmx_file)) {
                sprintf(fullpath,"%s/%s",game_path,tmx_file);
                tmx = XMLNode::from_file(fullpath);
            } else {
                sprintf(fullpath,"%s/%s",real_game_path,tmx_file);
                tmx = XMLNode::from_file(fullpath);
            }
			output_debug_message("loading A4Map.. %s\n",fullpath);
			A4Map *map = new A4Map(tmx, this, &objectsToRevisit, respawnID);
			m_maps.push_back(map);
            // delete tmx;  <-- no need to delete this, the map stores it, and will delete it at the end
		}
		delete maps_xml;

		// link bridges/bridge destinations:
		for(A4Map *map1:m_maps) {
			for(A4Map *map2:m_maps) {
				if (map1!=map2) {
					for(A4MapBridge *b:*(map1->getBridges())) {
						for(A4MapBridge *bd:*(map2->getBridgeDestinations())) {
							if (b->getName()->cmp(bd->getName())) b->link(bd);
						}
					}
				}
			}
		}
	}
	// loading scripts:
	{
		// on start:
		std::vector<XMLNode *> *onstarts_xml = xml->get_children("onStart");
		for(XMLNode *onstart_xml:*onstarts_xml) {
            A4ScriptExecutionQueue *tmp = 0;
            for(XMLNode *script_xml:*(onstart_xml->get_children())) {
                A4Script *s = new A4Script(script_xml);
                if (tmp==0) tmp = new A4ScriptExecutionQueue(0, 0, this, 0);
                tmp->scripts.push_back(s);
            }
            if (tmp!=0) m_script_queues.push_back(tmp);
		}
		delete onstarts_xml;

		// event rules:
		std::vector<XMLNode *> *eventrules_xml = xml->get_children("eventRule");
		for(XMLNode *rule_xml:*eventrules_xml) {
			A4EventRule *r = new A4EventRule(rule_xml);
			m_event_scripts[r->getEvent()].push_back(r);
		}
		delete eventrules_xml;
	}

	m_current_player = 0;
	m_zoom = m_target_zoom = m_default_zoom;
	// spawning player characters:
	{
		std::vector<XMLNode *> *players_xml = xml->get_children("player");
		for(XMLNode *player_xml:*players_xml) {
			char *className = player_xml->get_attribute("class");
			int x = atoi(player_xml->get_attribute("x"));
			int y = atoi(player_xml->get_attribute("y"));
			int map = atoi(player_xml->get_attribute("map"));
			output_debug_message("Spawning player %s at %i,%i in map %i\n", className, x, y, map);

            bool completeRedefinition = false;
            tmp = player_xml->get_attribute("completeRedefinition");
            if (tmp!=0 && strcmp(tmp,"true")==0) completeRedefinition = true;
			A4Object *p = m_objectFactory->createObject(className, this, true, completeRedefinition);
            p->loadObjectAdditionalContent(player_xml, this, m_objectFactory, &objectsToRevisit);
			p->warp(x, y, getMap(map), A4_LAYER_CHARACTERS);
			m_players.push_back((A4Character *)p);
		}
		delete players_xml;
	}

    for(std::pair<XMLNode *, A4Object *> p:objectsToRevisit) {
        p.second->revisitObject(p.first, this);
        delete p.first;
        p.first = 0;
    }
    objectsToRevisit.clear();

	// set initial camera:
	if (m_players.size()>0) {
		m_current_player_position = m_players.begin();
		m_current_player = *m_current_player_position;
	}

	m_cycle = 0;
    m_gameComplete = false;
    m_gameComplete_ending_ID = 0;

    Sort::precomputeIsA();

	output_debug_message("A4Game created\n");
}


void A4Game::addEnding(char *ID, std::vector<char *> *raw_ending, char *endingText)
{
    char **endingIDs = new char *[m_n_endings+1];
    char **endingTexts = new char *[m_n_endings+1];
    std::vector<char *> **endingText_raw = new std::vector<char *> *[m_n_endings+1];

    for(int i = 0;i<m_n_endings;i++) {
        endingIDs[i] = m_endingIDs[i];
        m_endingIDs[i] = 0;
        endingTexts[i] = m_endingTexts[i];
        m_endingTexts[i] = 0;
        endingText_raw[i] = m_endingText_raw[i];
        m_endingText_raw[i] = 0;
    }

    endingIDs[m_n_endings] = ID;
    endingTexts[m_n_endings] = endingText;
    endingText_raw[m_n_endings] = raw_ending;
    
    if (m_n_endings>0) {
        delete m_endingIDs;
        delete m_endingTexts;
        delete m_endingText_raw;
    }
    
    m_endingIDs = endingIDs;
    m_endingTexts = endingTexts;
    m_endingText_raw = endingText_raw;
    
    m_n_endings++;
}


char *A4Game::getGameEnding(const char *ID) {
    if (ID==0) return m_endingTexts[0];
    for(int i = 0;i<m_n_endings;i++) {
        if (strcmp(ID,m_endingIDs[i])==0) return m_endingTexts[i];
    }
    return 0;
}


bool A4Game::cycle(class KEYBOARDSTATE *k)
{
    if (m_cycle==0) {
        for(A4EventRule *rule:m_event_scripts[A4_EVENT_START]) {
            rule->executeEffects(0, 0, this, 0);
        }
    }

	m_zoom = (0.95*m_zoom + 0.05*m_target_zoom);

//	if (!m_current_player->getMap()->visible(m_current_player->getX()/m_tile_dx,
//											 m_current_player->getY()/m_tile_dy)) {
//		m_current_player->getMap()->reevaluateVisibility(m_current_player->getX()/m_tile_dx,
//											 			 m_current_player->getY()/m_tile_dy,
//                                                         m_current_player);
//	}

	// update all the objects in the game:
    {
        // figure out which maps to update:
        std::vector<A4Map *> maps_to_update,*l;
        l = m_current_player->getMap()->getNeighborMaps();
        for(A4Map *m:*l) maps_to_update.push_back(m);
        l->clear();
        delete l;
        for(A4Character *player:m_players) {
            if (std::find(maps_to_update.begin(),maps_to_update.end(),player->getMap()) == maps_to_update.end()) {
                maps_to_update.push_back(player->getMap());
            }
        }
        for(A4Map *map:maps_to_update) {
            map->cycle(this);
        }
    }

	for(WarpRequest *wr:warpRequests) {
        A4Map *m = wr->map;
        bool createRecord = wr->o->isCharacter() || wr->o->isVehicle();
        bool acceptWarp = true;
        if (wr->o->isCharacter() && ((A4Character *)(wr->o))->getVehicle()!=0 &&
            ((A4Character *)(wr->o))->getVehicle()->getMap()==wr->map) {
            acceptWarp = true;
        } else {
            if (createRecord &&
                m!=0 &&
                !m->walkableConsideringVehicles(wr->x, wr->y, wr->o->getPixelWidth(), wr->o->getPixelHeight(), wr->o)) acceptWarp = false;
        }
        if (acceptWarp) {
            bool isCurrentPlayer = wr->o == m_current_player;
            if (m!=0 && createRecord) {
                wr->o->getMap()->addPerceptionBufferRecord(
                    new PerceptionBufferObjectWarpedRecord(wr->o->getID(), wr->o->getSort(), new Symbol(m->getNameSymbol()),
                                                           wr->o->getX(), wr->o->getY(),
                                                           wr->o->getX()+wr->o->getPixelWidth(),
                                                           wr->o->getY()+wr->o->getPixelHeight()));
            }
            if (isCurrentPlayer) {
//                m_current_player->getMap()->reevaluateVisibility(m_current_player->getX()/m_tile_dx,
//                                                                 m_current_player->getY()/m_tile_dy,
//                                                                 m_current_player);
                if (m!=0) addMessage("Welcome to %s",m->getName());
            }
            wr->o->warp(wr->x,wr->y,wr->map,wr->layer);
        } else {
            // can't warp, since there is a collision!
            if (wr->o == m_current_player) addMessage("Something is blocking the way!");
        }
		delete wr;
	}
	warpRequests.clear();
	for(A4Object *o:deletionRequests) {
        o->event(A4_EVENT_END, 0, o->getMap(), this);
		objectRemoved(o);
		delete o;
	}
	deletionRequests.clear();

	// rules:
    for(A4EventRule *r:m_event_scripts[A4_EVENT_TIMER]) r->execute(0,0,this,0);
    for(A4EventRule *r:m_event_scripts[A4_EVENT_STORYSTATE]) r->execute(0,0,this,0);

	if (m_current_player==0) return false;

    executeScriptQueues();

    // check if the AI debugger has to be activated, removed:
    if (k->keyboard[SDL_SCANCODE_B] &&
        k->key_press(SDL_SCANCODE_D) &&
        (k->keyboard[SDL_SCANCODE_LALT] || k->keyboard[SDL_SCANCODE_RALT])) {
        // cycle among the potential characters:
        A4Map *map = m_current_player->getMap();
        std::vector<A4Object *> *l = map->getAllObjects(0, 0, map->getDx()*m_tile_dx, map->getDy()*m_tile_dy);
        std::vector<A4Object *>::iterator it = l->begin();
        bool found = false;
        while(true) {
            if (it == l->end()) {
                AI_debugger_focus = 0;
                break;
            }
            A4Object *o = *it;
            if (o->isAICharacter()) {
                if (found || AI_debugger_focus == 0) {
                    AI_debugger_focus = (A4AICharacter *)o;
                    break;
                } else {
                    if (o == (A4Object *)AI_debugger_focus) found = true;
                }
            }
            it++;
        }
        l->clear();
    }

	m_cycle++;

	return true;
}


void A4Game::executeScriptQueues() {
    std::vector<A4ScriptExecutionQueue *> toDelete;
    for(A4ScriptExecutionQueue *seb:m_script_queues) {
        while(true) {
            A4Script *s = seb->scripts.front();
            int retval = s->execute(seb->object,
                                    seb->map,
                                    (seb->game == 0 ? this:seb->game),
                                    seb->otherCharacter);
            if (retval==SCRIPT_FINISHED) {
                seb->scripts.pop_front();
                delete s;
                if (seb->scripts.empty()) {
                    toDelete.push_back(seb);
                    break;
                }
            } else if (retval==SCRIPT_NOT_FINISHED) {
                break;
            } else if (retval==SCRIPT_FAILED) {
                toDelete.push_back(seb);
                break;
            }
        }
    }
    for(A4ScriptExecutionQueue *seb:toDelete) {
        m_script_queues.remove(seb);
        delete seb;
    }
    toDelete.clear();
}


void A4Game::objectRemoved(A4Object *o)
{
	m_players.remove((A4Character *)o);
	if (m_current_player == o) {
		m_current_player_position = m_players.begin();
		if (m_players.size()>0) {
			m_current_player = *m_current_player_position;
		} else {
			m_current_player = 0;
		}
	}

    for(A4Map *map:m_maps) map->objectRemoved(o);
}


bool A4Game::contains(A4Object *o)
{
    for(A4Object *o2:deletionRequests)
        if (o2==o) return false;
    for(A4Map *map:m_maps) if (map->contains(o)) return true;
    return false;
}


void A4Game::draw(int SCREEN_X,int SCREEN_Y)
{
	// do not draw anything unless we have already executed a cycle:
	if (m_cycle==0) return;

	drawWorld(SCREEN_X, SCREEN_Y);
	drawHUD(SCREEN_X, SCREEN_Y);

    if (AI_debugger_focus!=0) {
        if (!contains((A4Object *)AI_debugger_focus)) {
            AI_debugger_focus = 0;
        } else {
            drawAIDebugger(SCREEN_X, SCREEN_Y, AI_debugger_focus);
        }
    }
}


void A4Game::drawHUD(int SCREEN_X,int SCREEN_Y)
{
    float hud_transparency = 0.66f;
    float target_HUD_height = 32;

	int x = 4;
	int y = 4;

    // draw the characters, and highlight the selected one
	for(A4Character *player:m_players) {
		drawQuad(x,y,target_HUD_height,target_HUD_height,0,0,0,hud_transparency);
		Animation *a = player->getAnimation(A4_ANIMATION_IDLE_RIGHT);
		if (a==0) a = player->getAnimation(A4_ANIMATION_IDLE);
		if (a!=0) {
            float f = target_HUD_height/a->getPixelHeight();
			if (player==m_current_player) a->draw(x,y,f, 1,1,1,1);
									 else a->draw(x,y,f, 1,1,1,0.5);
		}
		x+=target_HUD_height+8;
	}

	// draw health, MP, XP:
    if (m_allowStats) {
        int bar_width = 96;
        int bar_height = (target_HUD_height-8)/3;
        drawQuad(x-1,y-1,bar_width+2,bar_height+2,1.0,1.0,1.0,1.0);
        drawQuad(x,y,bar_width,bar_height,0.0,0.0,0.0,1.0);
        if (m_current_player->getHp()>0) {
            float hp = bar_width*float(m_current_player->getHp())/float(m_current_player->getMaxHp());
            drawQuad(x,y,hp,bar_height,0.8,0.0,0.0,1.0);
        }

        if (m_allowMagic) {
            y+=bar_height+4;
            drawQuad(x-1,y-1,bar_width+2,bar_height+2,1.0,1.0,1.0,1.0);
            drawQuad(x,y,bar_width,bar_height,0.0,0.0,0.0,1.0);
            if (m_current_player->getMp()>0) {
                float mp = bar_width*float(m_current_player->getMp())/float(m_current_player->getMaxMp());
                drawQuad(x,y,mp,bar_height,0.0,0.0,0.8,1.0);
            }
        }
        
        y+=bar_height+4;
        drawQuad(x-1,y-1,bar_width+2,bar_height+2,1.0,1.0,1.0,1.0);
        drawQuad(x,y,bar_width,bar_height,0.0,0.0,0.0,1.0);
        if (m_current_player->getXp()>0) {
            float xp = bar_width*float(m_current_player->getXp())/float(m_current_player->getXpToLevelUp());
            drawQuad(x,y,xp,bar_height,0.8,0.0,0.8,1.0);
        }
        y=4;
        x+=bar_width+8;
    }
    
	// draw equipment:
    if (m_allowInventory) {
        drawQuad(x,y,target_HUD_height,target_HUD_height,0,0,0,hud_transparency);
        if (m_current_player->getEquipment(0)!=0) m_current_player->getEquipment(0)->getAnimation()->draw(x,y,target_HUD_height/m_current_player->getEquipment(0)->getAnimation()->getPixelHeight(), 1,1,1,1);
        BInterface::print_left("A",m_font8,x,y+10);
        x+=target_HUD_height+8;
        drawQuad(x,y,target_HUD_height,target_HUD_height,0,0,0,hud_transparency);
        if (m_current_player->getEquipment(1)!=0) m_current_player->getEquipment(1)->getAnimation()->draw(x,y,target_HUD_height/m_current_player->getEquipment(1)->getAnimation()->getPixelHeight(), 1,1,1,1);
        BInterface::print_left("B",m_font8,x,y+10);
        x+=target_HUD_height+8;
        drawQuad(x,y,target_HUD_height,target_HUD_height,0,0,0,hud_transparency);
        if (m_current_player->getEquipment(2)!=0) m_current_player->getEquipment(2)->getAnimation()->draw(x,y,target_HUD_height/m_current_player->getEquipment(2)->getAnimation()->getPixelHeight(), 1,1,1,1);
        BInterface::print_left("C",m_font8,x,y+10);
        x+=target_HUD_height+8;
    }

	// draw stats / inventory / spells:
    if (m_allowStats) {
        char buffer[256];
        drawQuad(x,y,15*8,target_HUD_height,0.0,0.0,0.0,hud_transparency);
        sprintf(buffer,"LEVEL:");
        BInterface::print_left(buffer,m_font8,x+6,y+4+10 );
        sprintf(buffer,"%i",m_current_player->getLevel());
        BInterface::print_left(buffer,m_font8,x+6+(13-strlen(buffer))*8,y+4+10 );
        sprintf(buffer,"GOLD:");
        BInterface::print_left(buffer,m_font8,x+6,y+4+10+16);
        sprintf(buffer,"%i",m_current_player->getGold());
        BInterface::print_left(buffer,m_font8,x+6+(13-strlen(buffer))*8,y+4+10+16 );
    }

    int info_width = 20*8;
    int info_height = 6 + (m_allowStats ? 10:0) + (m_allowMagic ? 10:0) + (m_allowInventory ? 10:0);
	if (m_HUD_show_stats) info_height += 5*10;
	if (m_allowInventory && m_HUD_show_inventory) info_height += A4_INVENTORY_SIZE*10;
	if (m_allowMagic && m_HUD_show_spells) info_height += A4_N_SPELLS*10;

	x = SCREEN_X - (info_width+8);
    if (m_allowStats || m_allowInventory || m_allowMagic) {
        drawQuad(x,y,info_width,info_height,0.0,0.0,0.0,hud_transparency);
    }

	x+=4;
	y+=4+8;
    if (m_allowStats) {
        BInterface::print_left("STATS ( )",m_font8,x,y);
        BInterface::print_left("       S",m_font8,x,y,1,1,0,1); y+=10;
        if (m_HUD_show_stats) {
            char buffer[256];
            sprintf(buffer,"  HP: %i/%i",m_current_player->getHp(),m_current_player->getMaxHp());
            BInterface::print_left(buffer,m_font8,x,y,0.75,0.75,0.75,1); y+=10;
            if (m_allowMagic) {
                sprintf(buffer,"  MP: %i/%i",m_current_player->getMp(),m_current_player->getMaxMp());
                BInterface::print_left(buffer,m_font8,x,y,0.75,0.75,0.75,1); y+=10;
            }
            sprintf(buffer,"  XP: %i/%i",m_current_player->getXp(),m_current_player->getXpToLevelUp());
            BInterface::print_left(buffer,m_font8,x,y,0.75,0.75,0.75,1); y+=10;
            sprintf(buffer,"  ATTACK: %i",m_current_player->getAttack(true));
            if (m_current_player->getAttack(true) == m_current_player->getAttack(false)) {
                BInterface::print_left(buffer,m_font8,x,y,0.75,0.75,0.75,1); y+=10;
            } else if (m_current_player->getAttack(true) > m_current_player->getAttack(false)) {
                BInterface::print_left(buffer,m_font8,x,y, 0, 1, 1, 1); y+=10;
            } else {
                BInterface::print_left(buffer,m_font8,x,y, 1, 1, 0, 1); y+=10;
            }
            sprintf(buffer,"  DEFENSE: %i",m_current_player->getDefense(true));
            if (m_current_player->getDefense(true) == m_current_player->getDefense(false)) {
                BInterface::print_left(buffer,m_font8,x,y,0.75,0.75,0.75,1); y+=10;
            } else if (m_current_player->getDefense(true) > m_current_player->getDefense(false)) {
                BInterface::print_left(buffer,m_font8,x,y, 0, 1, 1, 1); y+=10;
            } else {
                BInterface::print_left(buffer,m_font8,x,y, 1, 1, 0, 1); y+=10;
            }
        }
    }
    
    if (m_allowInventory) {
        BInterface::print_left("INVENTORY ( )",m_font8,x,y);
        BInterface::print_left("           V",m_font8,x,y,1,1,0,1); y+=10;
        if (m_HUD_show_inventory) {
            char buffer[256];
            std::list<A4Object *> *il = m_current_player->getInventory();
            int i = 0;
            for(A4Object *ii:*il) {
                sprintf(buffer,"  %c:%s",char('1'+i), ii->getName()->get());
                BInterface::print_left(buffer,m_font8,x,y,0.75,0.75,0.75,1); y+=10;
                i++;
                if (i>=A4_INVENTORY_SIZE) break;
            }
            for(;i<A4_INVENTORY_SIZE;i++) {
                sprintf(buffer,"  %c:-",char('1'+i));
                BInterface::print_left(buffer,m_font8,x,y,0.75,0.75,0.75,1); y+=10;
            }
        }
    }
    if (m_allowMagic) {
        BInterface::print_left("SPELLS ( )",m_font8,x,y);
        BInterface::print_left("        P",m_font8,x,y,1,1,0,1); y+=10;
        if (m_HUD_show_spells) {
            char buffer[256];
            for(int i = 0;i<A4_N_SPELLS;i++) {
                if (m_current_player->knowsSpell(i)) {
                    sprintf(buffer,"  %c:%s",char('D'+i),A4Game::spellNames[i]);
                } else {
                    sprintf(buffer,"  %c:-",char('D'+i));
                }
                BInterface::print_left(buffer,m_font8,x,y,0.75,0.75,0.75,1); y+=10;
            }
        }
    }
    
	// messages:
	{
		x = 8;
		y = SCREEN_Y - (8 + 3*10);
        int start = 0;
        if (m_console_first_message==-1) {
            start = (int)m_messages.size() - A4_N_MESSAGES_IN_HUD;
        } else {
            start = m_console_first_message;
        }
        if (start<0) start = 0;

		drawQuad(x-4,SCREEN_Y-(12+4*10),SCREEN_X-8,4*10+8,0,0,0,hud_transparency);
        for(int i = 0;i<A4_N_MESSAGES_IN_HUD && start+i<(int)m_messages.size();i++) {
            char *msg = m_messages.at(start+i);
			BInterface::print_left(msg,m_font8,x,y);
			y+=10;
		}

        if (start>0) BInterface::print_left("PGUP",m_font8,SCREEN_X-(10+4*8),SCREEN_Y - (8 + 3*10), 1, 1, 0, 1);
        if (start+A4_N_MESSAGES_IN_HUD<(int)m_messages.size()) BInterface::print_left("PGDOWN",m_font8,SCREEN_X-(10+6*8),SCREEN_Y - 8, 1, 1, 0, 1);

	}

	// debug:
/*
	{
		char tmp[256];
		sprintf(tmp,"position: %i,%i",m_current_player->getX(),m_current_player->getY());
		BInterface::print_left(tmp,m_font8,60,SCREEN_Y-20);
	}
*/
}


void A4Game::drawWorld(int SCREEN_X,int SCREEN_Y)
{
	if (m_current_player!=0) {
		A4Map *map = m_current_player->getMap();
		int mapx = getCameraX(m_current_player, map->getDx()*m_tile_dx, SCREEN_X);
		int mapy = getCameraY(m_current_player, map->getDy()*m_tile_dy, SCREEN_Y);
        if (m_current_player->hasSpellEffect(A4_SPELL_MAGIC_EYE)) {
            map->draw(mapx,mapy,m_zoom,SCREEN_X, SCREEN_Y, this);
            map->drawSpeechBubbles(mapx,mapy,m_zoom,SCREEN_X, SCREEN_Y, this);
        } else {
            int tx = m_current_player->getX()/m_tile_dx;
            int ty = m_current_player->getY()/m_tile_dy;
            map->draw(mapx,mapy,m_zoom,SCREEN_X, SCREEN_Y, map->visiblilityRegion(tx,ty), this);
            map->drawSpeechBubbles(mapx,mapy,m_zoom,SCREEN_X, SCREEN_Y, map->visiblilityRegion(tx,ty), this);
        }
	} else {
		A4Map *map = m_maps.front();
		map->draw(0,0,m_zoom,SCREEN_X, SCREEN_Y, this);
        map->drawSpeechBubbles(0,0,m_zoom,SCREEN_X, SCREEN_Y, this);
	}
}


void A4Game::playSound(const char *sound_path)
{
    if (sound) {
    	char tmp[256];
    	sprintf(tmp,"%s/%s",m_game_path,sound_path);
    //	output_debug_message("loading sound: %s\n", tmp);
    	Mix_Chunk *chunk = m_SFXM->get(tmp);
    //	output_debug_message("sound loaded: %p\n", chunk);
        int channel = Mix_PlayChannel( -1, chunk, 0);
        Mix_Volume(channel, sfx_volume);
    }
}


GraphicFile *A4Game::getGraphicFile(const char *file)
{
	for(GraphicFile *gf:m_graphicFiles) {
		if (strcmp(file,gf->m_name)==0) return gf;
	}
	return 0;
}


A4Map *A4Game::getMap(int idx)
{
	int i = 0;
	for(A4Map *m:m_maps) {
		if (i==idx) return m;
		i++;
	}
	return 0;
}


A4Map *A4Game::getMap(Symbol *name)
{
    for(A4Map *m:m_maps) {
        if (name->cmp(m->getName())) return m;
    }
    return 0;
}


int A4Game::getCameraX(A4Object *focus, int map_width, int screen_width)
{
	if (map_width<screen_width/m_zoom) {
		return -(screen_width/m_zoom-map_width)/2;
	} else {
		int target_x = focus->getX()+m_tile_dx/2;
		target_x -= (screen_width/2)/m_zoom;

		if (target_x<0) target_x = 0;
		if (map_width - target_x < screen_width/m_zoom) target_x = map_width - screen_width/m_zoom;
		return target_x;
	}
}


int A4Game::getCameraY(A4Object *focus, int map_height, int screen_height)
{
    float top_HUD = 40;
    float bottom_HUD = 56;
    float center_Y = (screen_height - (top_HUD + bottom_HUD))/2 + top_HUD;

	if (map_height<(screen_height-(top_HUD + bottom_HUD))/m_zoom) {
		return -((center_Y*2)/m_zoom-map_height)/2;
	} else {
		int target_y = focus->getY()+m_tile_dy/2;
		target_y -= (center_Y)/m_zoom;

		if (target_y<-top_HUD/m_zoom) target_y = -top_HUD/m_zoom; // 40 pixels to leave space for the HUD
		if (map_height - target_y < (screen_height-bottom_HUD)/m_zoom) target_y = map_height - (screen_height-bottom_HUD)/m_zoom;
		return target_y;
	}
}


A4Object *A4Game::getObject(int ID)
{
    for(A4Map *map:m_maps) {
        A4Object *o = map->getObject(ID);
        if (o!=0) return o;
    }
    return 0;
}


RespawnRecord *A4Game::getRespawnRecord(int ID)
{
    for(A4Map *map:m_maps) {
        RespawnRecord *rr = map->getRespawnRecord(ID);
        if (rr!=0) return rr;
    }
    return 0;
}



void A4Game::requestWarp(A4Object *o, A4Map *map, int x, int y, int layer)
{
//    output_debug_message("Warp request at %i: %p to %i,%i,%s\n",m_cycle,o,x,y,map->getName());
    WarpRequest *wr = new WarpRequest(o,map,x,y,layer);
	warpRequests.push_back(wr);
}


void A4Game::requestDeletion(A4Object *o)
{
	deletionRequests.push_back(o);

    /*
    // remove all the warp buffers referring to this object:
    for(A4Map *m:m_maps) {
        m->removePerceptionBuffersForObject(o, false, true);
    }
    */
}


void A4Game::setDoorGroupState(Symbol *doorGroup, bool state, A4Character *character)
{
    for(A4Map *map:m_maps) {
        map->setDoorGroupState(doorGroup, state, character, map, this);
    }
}


bool A4Game::checkIfDoorGroupStateCanBeChanged(Symbol *doorGroup, bool state, A4Character *character)
{
    for(A4Map *map:m_maps) {
        if (!map->checkIfDoorGroupStateCanBeChanged(doorGroup, state, character, map, this)) return false;
    }
    return true;
}


void A4Game::addMessage(const char *fmt, ...)
{
    char *text = new char[256];
    va_list ap;

    if (fmt == 0)
        return ;

    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);

    // split longer messages into different lines:
    {
        char buffer[A4_MAX_MESSAGE_LENGTH+1];
        int j = 0, last_space = 0;
        int longestLine = 0;

        for(int i=0;text[i]!=0;i++) {
            buffer[j++] = text[i];
            if (text[i]==' ') last_space = i;
            if (j>=A4_MAX_MESSAGE_LENGTH) {
                if (last_space==0) {
                    // a single word doesn't fit, just split it!
                    buffer[j++] = 0;
                    char *tmp = new char[j];
                    strcpy(tmp,buffer);
                    m_messages.push_back(tmp);
                    if ((int)strlen(tmp)>longestLine) longestLine = (int)strlen(tmp);
                    j = 0;
                } else {
                    int backspaces = i - last_space;
                    j-=backspaces;
                    i-=backspaces;
                    buffer[j++] = 0;
                    char *tmp = new char[j];
                    strcpy(tmp,buffer);
                    m_messages.push_back(tmp);
                    if (strlen(tmp)>longestLine) longestLine = (int)strlen(tmp);
                    j = 0;
                }
            }
        }
        if (j!=0) {
            buffer[j++] = 0;
            char *tmp = new char[j];
            strcpy(tmp,buffer);
            m_messages.push_back(tmp);
//            if (strlen(tmp)>longestLine) longestLine = (int)strlen(tmp);
//            j = 0;
        }
    }
    delete []text;
}


void A4Game::addMessage(A4Object *focus, const char *fmt, ...)
{
    if (m_current_player->getMap() != focus->getMap()) return;
    char *text = new char[256];
    va_list ap;

    if (fmt == 0)
        return ;

    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);

    // split longer messages into different lines:
    {
        char buffer[A4_MAX_MESSAGE_LENGTH+1];
        int j = 0, last_space = 0;
        int longestLine = 0;

        for(int i=0;text[i]!=0;i++) {
            buffer[j++] = text[i];
            if (text[i]==' ') last_space = i;
            if (j>=A4_MAX_MESSAGE_LENGTH) {
                if (last_space==0) {
                    // a single word doesn't fit, just split it!
                    buffer[j++] = 0;
                    char *tmp = new char[j];
                    strcpy(tmp,buffer);
                    m_messages.push_back(tmp);
                    if (strlen(tmp)>longestLine) longestLine = (int)strlen(tmp);
                    j = 0;
                } else {
                    int backspaces = i - last_space;
                    j-=backspaces;
                    i-=backspaces;
                    buffer[j++] = 0;
                    char *tmp = new char[j];
                    strcpy(tmp,buffer);
                    m_messages.push_back(tmp);
                    if (strlen(tmp)>longestLine) longestLine = (int)strlen(tmp);
                    j = 0;
                }
            }
        }
        if (j!=0) {
            buffer[j++] = 0;
            char *tmp = new char[j];
            strcpy(tmp,buffer);
            m_messages.push_back(tmp);
            //            if (strlen(tmp)>longestLine) longestLine = (int)strlen(tmp);
            //            j = 0;
        }
    }
    delete []text;
}



void A4Game::playerInput_ToogleZoom() {
	if (m_target_zoom<m_default_zoom) m_target_zoom = m_default_zoom;
	else if (m_target_zoom == m_default_zoom) m_target_zoom = m_default_zoom*2;
	else m_target_zoom = m_default_zoom/2;
}


void A4Game::playerInput_SwitchPlayers() {
	if (m_players.size()>0) {
		m_current_player_position++;
		if (m_current_player_position==m_players.end()) {
			m_current_player_position = m_players.begin();
		}
		m_current_player = *m_current_player_position;
//        m_current_player->getMap()->reevaluateVisibility(m_current_player->getX()/m_tile_dx,
//                                                         m_current_player->getY()/m_tile_dy,
//                                                         m_current_player);
	}
}


void A4Game::messageConsoleUp()
{
    if (m_console_first_message>0) m_console_first_message--;
    if (m_console_first_message==-1 &&
        m_messages.size()>A4_N_MESSAGES_IN_HUD) m_console_first_message = (int) m_messages.size()-(A4_N_MESSAGES_IN_HUD+1);
}


void A4Game::messageConsoleDown()
{
    if (m_console_first_message!=-1) {
        if (m_console_first_message<m_messages.size()-(A4_N_MESSAGES_IN_HUD+1)) {
            m_console_first_message++;
        } else {
            m_console_first_message = -1;
        }
    }
}


void A4Game::playerInput_issueCommand(int cmd, SpeechAct *sa, int direction) {
    m_current_player->issueCommand(cmd, sa, direction, 0, this);
}


int A4Game::playerInput_issueCommand(int cmd, int arg, int direction) {
    if (cmd==A4CHARACTER_COMMAND_WALK) {
        // detect whether we should change "walk" to attack or talk if we walk against an enemy or npc:
        int movement_slack = m_current_player->getPixelWidth()*A4_MOVEMENT_SLACK;
        int v_movement_slack = m_current_player->getPixelHeight()*A4_MOVEMENT_SLACK;
        if (v_movement_slack>movement_slack) movement_slack = v_movement_slack;
        int tmp = m_current_player->canMove(direction, movement_slack);
        // only change action if there is an obstacle and we cannot walk around it:
        if (tmp==INT_MAX) {
            A4Map *map = m_current_player->getMap();
            // detect whether we will collide with another character/object, and decide whether to change to another action:
            std::vector<A4Object *> *collisions = map->getAllObjectCollisions(m_current_player, A4Game::direction_x_inc[direction], A4Game::direction_y_inc[direction]);
            for(A4Object *o:*collisions) {
                if (o->isCharacter()) {
                    // determine whether the character is friendly or unfriendly
                    if (o->isAICharacter()) {
                        A4AI *ai = ((A4AICharacter *)o)->getAI();
                        if (ai->isUnfriendly(m_current_player->getID())) {
                            // attack!
                            m_current_player->issueCommand(A4CHARACTER_COMMAND_ATTACK, arg, direction, o, this);
                            delete collisions;
                            return A4CHARACTER_COMMAND_ATTACK;
                        }
                        if (ai->getConversationGraph()!=0) {
                            // talk:
                            // don't issue anything, the claling code will trigger the talk dialog
                            delete collisions;
                            return A4CHARACTER_COMMAND_TALK;
                        }
                        m_current_player->issueCommand(cmd, arg, direction, 0, this);
                        delete collisions;
                        return cmd;
                    }
                }
            }
            delete collisions;
        }
    } else if (cmd==A4CHARACTER_COMMAND_USE) {
        // using a wand:
        A4Object *o = getCurrentPlayer()->getInventory(arg);
        if (o!=0 && o->isWand()) {
            o->eventWithInteger(A4_EVENT_USE, direction, getCurrentPlayer(), getCurrentPlayer()->getMap(), this);
        }
    }
    m_current_player->issueCommand(cmd, arg, direction, 0, this);
    return cmd;
}


void A4Game::playerInput_issueCommand(int cmd, int arg, A4Object *target) {
    m_current_player->issueCommand(cmd, arg, 0, target, this);
}


void A4Game::addSpeechAct(int performative, const char *keyword, const char *text)
{
	for(SpeechAct *sa:m_known_speech_acts) {
		if (sa->performative==performative &&
			strcmp(sa->keyword,keyword)==0 &&
			strcmp(sa->text,text)==0) return;
	}
	char *tmp = new char[strlen(keyword)+1]; strcpy(tmp,keyword);
	char *tmp2 = new char[strlen(text)+1]; strcpy(tmp2,text);
	m_known_speech_acts.push_back(new SpeechAct(performative, tmp, tmp2));
}


void A4Game::setStoryStateVariable(const char *variable, const char *value)
{
/*
    std::vector<char *>::iterator pos1 = m_storyStateVariables.begin();
    std::vector<char *>::iterator pos2 = m_storyStateValues.begin();

    while(!(pos1==m_storyStateVariables.end()) && strcmp(*pos1,variable)!=0) {
        pos1++;
        pos2++;
    }
    if (pos1==m_storyStateVariables.end()) {
        // store a new value:
        char *tmp1 = new char[strlen(variable)+1];
        strcpy(tmp1, variable);
        char *tmp2 = new char[strlen(value)+1];
        strcpy(tmp2, value);
        m_storyStateVariables.push_back(tmp1);
        m_storyStateValues.push_back(tmp2);
    } else {
        char *tmp2 = new char[strlen(value)+1];
        strcpy(tmp2, value);
        *pos2 = tmp2;
    }
*/

    for(int i = 0;i<m_storyStateVariables.size();i++) {
        if (strcmp(variable, m_storyStateVariables[i])==0) {
            char *tmp2 = new char[strlen(value)+1];
            strcpy(tmp2, value);
            m_storyStateValues[i] = tmp2;
            return;
        }
    }
    // store a new value:
    char *tmp1 = new char[strlen(variable)+1];
    strcpy(tmp1, variable);
    char *tmp2 = new char[strlen(value)+1];
    strcpy(tmp2, value);
    m_storyStateVariables.push_back(tmp1);
    m_storyStateValues.push_back(tmp2);

}


char *A4Game::getStoryStateVariable(const char *variable)
{
    for(int i = 0;i<m_storyStateVariables.size();i++) {
        if (strcmp(variable, m_storyStateVariables[i])==0) {
            return m_storyStateValues[i];
        }
    }
    return 0;
/*

    std::vector<char *>::iterator pos1 = m_storyStateVariables.begin();
    std::vector<char *>::iterator pos2 = m_storyStateValues.begin();

    while(!(pos1==m_storyStateVariables.end()) && strcmp(*pos1,variable)!=0) {
        pos1++;
        pos2++;
    }

    if (pos1==m_storyStateVariables.end()) {
        return 0;
    } else {
        return *pos2;
    }
 */
}



void A4Game::clearOpenGLState()
{
	output_debug_message("A4Game::clearOpenGLState begin\n");
	for(GraphicFile *gf:m_graphicFiles) {
		output_debug_message("  gf: %p\n",gf);
		gf->clearOpenGLState();
	}
	output_debug_message("A4Game::clearOpenGLState ...\n");
	for(A4Map *m:m_maps) {
		m->clearOpenGLState();
	}

	output_debug_message("A4Game::clearOpenGLState ok\n");
}


void A4Game::saveGame(const char *path, const char *saveName)
{
    char fullPath[1024];
    char fullPath2[1024];
#ifdef __EMSCRIPTEN__
    sprintf(fullPath, "/%s/%s",persistent_data_folder,saveName);
#else
    sprintf(fullPath, "%s/%s",path,saveName);
#endif
#ifdef _WIN32
    _mkdir(fullPath);
#else
    mkdir(fullPath,0777);
#endif

    // 2) iterate over the maps, and save every map
    int idx = 1;
    for(A4Map *map:m_maps) {
#ifdef __EMSCRIPTEN__
        sprintf(fullPath2,"/%s/%s/map%i.xml",persistent_data_folder,saveName,idx);
#else
        sprintf(fullPath2,"%s/%s/map%i.xml",path,saveName,idx);
#endif
        XMLfileWriter *writer = new XMLfileWriter(fullPath2,2);

        // save the map:
        map->saveToXML(writer, this);

        delete writer;
        idx++;
    }

    // 3) save the main game file
#ifdef __EMSCRIPTEN__
    sprintf(fullPath2,"/%s/%s/%s.xml",persistent_data_folder,saveName,saveName);
#else
    sprintf(fullPath2,"%s/%s/%s.xml",path,saveName,saveName);
#endif
    XMLfileWriter *writer = new XMLfileWriter(fullPath2,2);

    // save the game:
    saveToXML(writer);

    delete writer;

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
}


void A4Game::saveToXML(XMLwriter *w)
{
    w->openTag("A4Game");
    if (m_gameName!=0) w->setAttribute("name", m_gameName);
    if (m_gameTitle!=0) w->setAttribute("title", m_gameTitle);
    if (m_gameSubtitle!=0) w->setAttribute("subtitle", m_gameSubtitle);
    w->setAttribute("allowSaveGames", m_allowSaveGames);
    w->setAttribute("allowTalking", m_allowTalking);
    w->setAttribute("allowMagic", m_allowMagic);
    w->setAttribute("allowInventory", m_allowInventory);
    w->setAttribute("allowStats", m_allowStats);
    
    // savegame name:
    {
        char tmp[256];
        tmp[0] = 0;
        bool first = true;
        for(A4Character *c:m_players) {
            if (!first) sprintf(tmp+strlen(tmp)," ");
            sprintf(tmp+strlen(tmp),"Lvl%i",c->getLevel());
            first = false;
        }
        w->setAttribute("saveGameName", tmp);
    }

    if (m_gameTitleImage!=0) {
        w->openTag("titleImage");
        w->addContent("%s",m_gameTitleImage);
        w->closeTag("titleImage");
    }
    
    w->openTag("story");
    for(char *text:m_storyText_raw) {
        w->openTag("line");
        w->addContent(text);
        w->closeTag("line");
    }
    w->closeTag("story");

    for(int i = 0;i<m_n_endings;i++) {
        w->openTag("ending");
        w->setAttribute("id", m_endingIDs[i]);
        for(char *text:*(m_endingText_raw[i])) {
            w->openTag("line");
            w->addContent(text);
            w->closeTag("line");
        }
        w->closeTag("ending");
    }
    
    // tiles:
    w->openTag("tiles");
    w->setAttribute("sourcewidth", m_tile_dx);
    w->setAttribute("sourceheight", m_tile_dy);
    w->setAttribute("targetwidth", m_tile_dx*m_default_zoom);
    w->setAttribute("targetheight", m_tile_dy*m_default_zoom);
    for(GraphicFile *gf:m_graphicFiles) {
        w->openTag("types");
        w->setAttribute("file", gf->m_name);
        for(int i = 0;i<gf->m_n_tiles;i+=gf->m_tiles_per_row) {
            w->addContent("\n");
            for(int j = 0;j<gf->m_tiles_per_row;j++) {
                w->addContent("%i,",gf->m_tileTypes[i+j]);
            }
        }
        w->closeTag("types");

        w->openTag("seeThrough");
        w->setAttribute("file", gf->m_name);
        for(int i = 0;i<gf->m_n_tiles;i+=gf->m_tiles_per_row) {
            w->addContent("\n");
            for(int j = 0;j<gf->m_tiles_per_row;j++) {
                w->addContent("%i,",gf->m_tileSeeThrough[i+j]);
            }
        }
        w->closeTag("seeThrough");

        w->openTag("canDig");
        w->setAttribute("file", gf->m_name);
        for(int i = 0;i<gf->m_n_tiles;i+=gf->m_tiles_per_row) {
            w->addContent("\n");
            for(int j = 0;j<gf->m_tiles_per_row;j++) {
                w->addContent("%i,",gf->m_tileCanDig[i+j]);
            }
        }
        w->closeTag("canDig");
    }
    // default animations:
    if (m_coinpurse_animation!=0) m_coinpurse_animation->saveToXML(w,"coinpurse");
    if (m_emotion_animation[A4_EMOTION_HAPPY]!=0) m_emotion_animation[A4_EMOTION_HAPPY]->saveToXML(w,"happy");
    if (m_emotion_animation[A4_EMOTION_ANGRY]!=0) m_emotion_animation[A4_EMOTION_ANGRY]->saveToXML(w,"angry");
    if (m_emotion_animation[A4_EMOTION_SCARED]!=0) m_emotion_animation[A4_EMOTION_SCARED]->saveToXML(w,"scared");
    if (m_emotion_animation[A4_EMOTION_CURIOUS]!=0) m_emotion_animation[A4_EMOTION_CURIOUS]->saveToXML(w,"curious");
    if (m_emotion_animation[A4_EMOTION_TIRED]!=0) m_emotion_animation[A4_EMOTION_TIRED]->saveToXML(w,"tired");

    if (m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING]!=0) m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING]->saveToXML(w,"magic missile");
    if (m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_LEFT]!=0) m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_LEFT]->saveToXML(w,"magic missile-left");
    if (m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_UP]!=0) m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_UP]->saveToXML(w,"magic missile-up");
    if (m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_RIGHT]!=0) m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_RIGHT]->saveToXML(w,"magic missile-right");
    if (m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_DOWN]!=0) m_spell_animation[A4_SPELL_MAGIC_MISSILE][A4_ANIMATION_MOVING_DOWN]->saveToXML(w,"magic missile-down");

    if (m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING]!=0) m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING]->saveToXML(w,"fireball");
    if (m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_LEFT]!=0) m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_LEFT]->saveToXML(w,"fireball-left");
    if (m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_UP]!=0) m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_UP]->saveToXML(w,"fireball-up");
    if (m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_RIGHT]!=0) m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_RIGHT]->saveToXML(w,"fireball-right");
    if (m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_DOWN]!=0) m_spell_animation[A4_SPELL_FIREBALL][A4_ANIMATION_MOVING_DOWN]->saveToXML(w,"fireball-down");

    if (m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING]!=0) m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING]->saveToXML(w,"incinerate");
    if (m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_LEFT]!=0) m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_LEFT]->saveToXML(w,"incinerate-left");
    if (m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_UP]!=0) m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_UP]->saveToXML(w,"incinerate-up");
    if (m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_RIGHT]!=0) m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_RIGHT]->saveToXML(w,"incinerate-right");
    if (m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_DOWN]!=0) m_spell_animation[A4_SPELL_INCINERATE][A4_ANIMATION_MOVING_DOWN]->saveToXML(w,"incinerate-down");

    w->closeTag("tiles");

    // character and object definition files ...
    for(char *tmp:m_characterDefinitionFiles) {
        w->openTag("characterDefinition");
        w->setAttribute("file", tmp);
        w->closeTag("characterDefinition");
    }

    for(char *tmp:m_objectDefinitionFiles) {
        w->openTag("objectDefinition");
        w->setAttribute("file", tmp);
        w->closeTag("objectDefinition");
    }

    // maps
    for(int idx = 0;idx<m_maps.size();idx++) {
        char tmp[16];
        sprintf(tmp,"map%i.xml",idx+1);
        w->openTag("map");
        w->setAttribute("file", tmp);
        w->closeTag("map");
    }

    // players:
    for(A4Character *pc:m_players) {
        int idx = 0;
        for(A4Map *map:m_maps) {
            if (pc->getMap()==map) {
                pc->saveToXMLForMainFile(w,this, idx);
                break;
            }
            idx++;
        }
    }

    // save state:
    bool tagOpen = false;
    for(SpeechAct *sa:m_known_speech_acts) {
        if (sa->performative==A4_TALK_PERFORMATIVE_ASK) {
            if (!tagOpen) {
                w->openTag("onStart");
                tagOpen = true;
            }
            w->openTag("addTopic");
            w->setAttribute("topic", sa->keyword);
            w->setAttribute("text", sa->text);
            w->closeTag("addtopic");
        }
    }
    for(int i = 0;i<m_storyStateVariables.size();i++) {
        if (!tagOpen) {
            w->openTag("onStart");
            tagOpen = true;
        }
        w->openTag("storyState");
        w->setAttribute("variable", m_storyStateVariables[i]);
        w->setAttribute("value", m_storyStateValues[i]);
        w->setAttribute("scope", "game");
        w->closeTag("storyState");
    }
/*
    std::vector<char *>::iterator it1 = m_storyStateVariables.begin();
    std::vector<char *>::iterator it2 = m_storyStateValues.begin();
    while(it1!=m_storyStateVariables.end()) {
        if (!tagOpen) {
            w->openTag("onStart");
            tagOpen = true;
        }
        w->openTag("storyState");
        w->setAttribute("variable", *it1);
        w->setAttribute("value", *it2);
        w->setAttribute("scope", "game");
        w->closeTag("storyState");
        it1++;
        it2++;
    }
 */
    if (tagOpen) w->closeTag("onStart");

    // each execution queue goes to its own "onStart" block:
    for(A4ScriptExecutionQueue *seq:m_script_queues) {
        w->openTag("onStart");
        for(A4Script *s:seq->scripts) s->saveToXML(w);
        w->closeTag("onStart");
    }

    // rules:
    for(int i = 0;i<A4_NEVENTS;i++) {
        for(A4EventRule *er:m_event_scripts[i]) {
            er->saveToXML(w);
        }
    }

    w->closeTag("A4Game");
}


bool A4Game::checkSaveGame(const char *gamePath, const char *saveName, char *info)
{
    char fullPath[1024];
#ifdef __EMSCRIPTEN__
    sprintf(fullPath, "/%s/%s/%s.xml",persistent_data_folder,saveName,saveName);
#else
    sprintf(fullPath, "%s/%s/%s.xml",gamePath,saveName,saveName);
#endif
    if (!fileExists(fullPath)) return false;
    XMLNode *xml = XMLNode::from_file(fullPath);
    sprintf(info,"%s",xml->get_attribute("saveGameName"));
    delete xml;
    return true;
}

