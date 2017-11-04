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
#include "A4Object.h"
#include "A4Item.h"
#include "A4ObjectFactory.h"
#include "A4Game.h"
#include "A4Character.h"
#include "A4Trigger.h"
#include "A4Map.h"
#include "A4AICharacter.h"

#include "Ontology.h"

#include "A4Vehicle.h"

PerceptionBufferRecord::PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, int x0, int y0, int x1, int y1)
{
    m_action = action;
    m_subject_ID = subjectID;
    m_subject_sort = subject;
    m_direct_object_ID = 0;
    m_direct_object_sort = 0;
    m_direct_object_symbol = 0;
    m_indirect_object_ID = 0;
    m_indirect_object_sort = 0;
    m_x0 = x0;
    m_y0 = y0;
    m_x1 = x1;
    m_y1 = y1;
}


PerceptionBufferRecord::PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, int objectID, Sort *object, int x0, int y0, int x1, int y1)
{
    m_action = action;
    m_subject_ID = subjectID;
    m_subject_sort = subject;
    m_direct_object_ID = objectID;
    m_direct_object_sort = object;
    m_direct_object_symbol = 0;
    m_indirect_object_ID = 0;
    m_indirect_object_sort = 0;
    m_x0 = x0;
    m_y0 = y0;
    m_x1 = x1;
    m_y1 = y1;
}

PerceptionBufferRecord::PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, int objectID, Sort *object, int indirectID, Sort *indirect, int x0, int y0, int x1, int y1)
{
    m_action = action;
    m_subject_ID = subjectID;
    m_subject_sort = subject;
    m_direct_object_ID = objectID;
    m_direct_object_sort = object;
    m_direct_object_symbol = 0;
    m_indirect_object_ID = indirectID;
    m_indirect_object_sort = indirect;
    m_x0 = x0;
    m_y0 = y0;
    m_x1 = x1;
    m_y1 = y1;
}



PerceptionBufferRecord::PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, Symbol *object, int x0, int y0, int x1, int y1)
{
    m_action = action;
    m_subject_ID = subjectID;
    m_subject_sort = subject;
    m_direct_object_ID = 0;
    m_direct_object_sort = 0;
    m_direct_object_symbol = object;
    m_indirect_object_ID = 0;
    m_indirect_object_sort = 0;
    m_x0 = x0;
    m_y0 = y0;
    m_x1 = x1;
    m_y1 = y1;
}


PerceptionBufferRecord::PerceptionBufferRecord(Symbol *action, int subjectID, class Sort *subject, Symbol *object, int indirectID, Sort *indirect, int x0, int y0, int x1, int y1)
{
    m_action = action;
    m_subject_ID = subjectID;
    m_subject_sort = subject;
    m_direct_object_ID = 0;
    m_direct_object_sort = 0;
    m_direct_object_symbol = object;
    m_indirect_object_ID = indirectID;
    m_indirect_object_sort = indirect;
    m_x0 = x0;
    m_y0 = y0;
    m_x1 = x1;
    m_y1 = y1;
}


PerceptionBufferRecord::~PerceptionBufferRecord()
{
    m_action = 0;   // the action does not need to be deleted
    if (m_direct_object_symbol!=0) delete m_direct_object_symbol;
    m_direct_object_symbol = 0;
}



A4Map::A4Map(XMLNode *tmx_xml, A4Game *game, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit, int &respawnID)
{
    m_name = 0;
    m_name_symbol = 0;
    m_cycle = 0;
    m_xml = tmx_xml;
//	tmx_xml->print(0);
	m_dx = atoi(tmx_xml->get_attribute("width"));
	m_dy = atoi(tmx_xml->get_attribute("height"));
	XMLNode *properties_xml = tmx_xml->get_child("properties");
	for(XMLNode *property:*(properties_xml->get_children())) {
		if (strcmp(property->get_attribute("name"),"name")==0) {
			m_name = new char[strlen(property->get_attribute("value"))+1];
			strcpy(m_name, property->get_attribute("value"));
            m_name_symbol = new Symbol(m_name);
		}
	}
    m_visibility_regions = new int[m_dx*m_dy];
	for(int i = 0;i<m_dx*m_dy;i++) m_visibility_regions[i] = 0;

//	output_debug_message("loading map %s -> %i,%i\n", m_name, m_dx, m_dy);

	// load tileset:
	int n_gfs = 0;
	GraphicFile **gfs = 0;	
	std::vector<XMLNode *> *tilesets_xml = tmx_xml->get_children("tileset");
	for(XMLNode *tileset_xml:*tilesets_xml) {
        n_gfs+=tileset_xml->get_children()->size();
//		for(XMLNode *tileset:*(tileset_xml->get_children())) n_gfs++;
	}
	gfs = new GraphicFile *[n_gfs];
	int i = 0;
	for(XMLNode *tileset_xml:*tilesets_xml) {
		for(XMLNode *tileset:*(tileset_xml->get_children())) {
			char *filename = tileset->get_attribute("source");
			gfs[i] = game->getGraphicFile(filename);
            if (gfs[i]==0) {
                output_debug_message("getGraphicFile '%s' returned null!",filename);
                exit(1);
            }
            i++;
		}
	}
	delete tilesets_xml;

	// load tile layers:
	std::vector<XMLNode *> *layers_xml = tmx_xml->get_children("layer");
	m_n_layers = (int)layers_xml->size();
	m_layers = new A4MapLayer *[A4_N_LAYERS];
	int l_idx = 0;
	for(XMLNode *layer_xml:*layers_xml) {
		XMLNode *data_xml = layer_xml->get_child("data");
		int idx = 0;
		m_layers[l_idx] = new A4MapLayer(m_dx,m_dy,n_gfs,gfs);
		for(XMLNode *tile_xml:*(data_xml->get_children())) {
			m_layers[l_idx]->m_tiles[idx] = atoi(tile_xml->get_attribute("gid"))-1;
			idx++;
		}
//		output_debug_message("layer loaded with %i tiles (out of %i)\n",idx,(m_dx*m_dy));
		l_idx++;
	}
	delete layers_xml;

	// create the additional layers:
	for(int i = m_n_layers;i<A4_N_LAYERS;i++) {
		m_layers[i] = new A4MapLayer(m_dx,m_dy,n_gfs,gfs);
	}

    // load respawn records:
    bool createRespawnRecords = true;   // if there are no "respawnRecord" in the map, then we have to create them from the enemy data
    std::vector<XMLNode *> *respawn_xml_l = tmx_xml->get_children("respawnRecord");
    for(XMLNode *respawn_xml:*respawn_xml_l) {
        createRespawnRecords = false;
        RespawnRecord *rr = new RespawnRecord(atoi(respawn_xml->get_attribute("ID")),
                                              atoi(respawn_xml->get_attribute("x")),
                                              atoi(respawn_xml->get_attribute("y")),
                                              atoi(respawn_xml->get_attribute("layer")),
                                              respawn_xml->get_child("object"),
                                              atof(respawn_xml->get_attribute("respawn_probability")));
        rr->time_to_respawn = atoi(respawn_xml->get_attribute("time_to_respawn"));
        m_respawnRecords.push_back(rr);
    }
    delete respawn_xml_l;
    
	// load object layers:
	std::vector<XMLNode *> *objectgroups_xml = tmx_xml->get_children("objectgroup");
	for(XMLNode *objectgroup_xml:*objectgroups_xml) {
		for(XMLNode *object_xml:*(objectgroup_xml->get_children())) {
			A4Object *o = loadObjectFromXML(object_xml, game, objectsToRevisit);
			if (o!=0) {
				o->setX(atoi(object_xml->get_attribute("x")));
				o->setY(atoi(object_xml->get_attribute("y")));
				addObject(o,o->getLayer());

                if (o->respawns() && createRespawnRecords) {
                    RespawnRecord *rr = new RespawnRecord(respawnID++, o->getX(), o->getY(), o->getLayer(), object_xml, ((A4AICharacter *)o)->getRespawn());
                    ((A4AICharacter *)o)->setRespawnRecord(rr);
                    m_respawnRecords.push_back(rr);
                } else if (o->respawns()) {
                    XMLNode *copy = new XMLNode(object_xml);
                    objectsToRevisit->push_back(std::pair<XMLNode *, A4Object *>(copy, o));
                }
			}
		}
	}
	delete objectgroups_xml;
    
	// loading scripts:
	{
		// on start:
		std::vector<XMLNode *> *onstarts_xml = tmx_xml->get_children("onStart");
		for(XMLNode *onstart_xml:*onstarts_xml) {
            A4ScriptExecutionQueue *tmp = 0;
            for(XMLNode *script_xml:*(onstart_xml->get_children())) {
                A4Script *s = new A4Script(script_xml);
                if (tmp==0) tmp = new A4ScriptExecutionQueue(0, this, 0, 0);
                tmp->scripts.push_back(s);
            }
            if (tmp!=0) m_script_queues.push_back(tmp);
		}		
		delete onstarts_xml;

		// event rules:
		std::vector<XMLNode *> *eventrules_xml = tmx_xml->get_children("eventRule");
		for(XMLNode *rule_xml:*eventrules_xml) {
			A4EventRule *r = new A4EventRule(rule_xml);
			m_event_scripts[r->getEvent()].push_back(r);
		}		
		delete eventrules_xml;
	}

	delete []gfs;
    
    reevaluateVisibility();
}


A4Object *A4Map::loadObjectFromXML(XMLNode *object_xml, A4Game *game, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit) {
    A4Object *o = 0;
    A4ObjectFactory *of = game->getObjectFactory();
    int layer = A4_LAYER_FG;
    char *o_class = object_xml->get_attribute("class");
    bool completeRedefinition = false;
    char *tmp = object_xml->get_attribute("completeRedefinition");
    if (tmp!=0 && strcmp(tmp,"true")==0) completeRedefinition = true;
    if (strcmp(o_class,"Bridge")==0) {
        A4MapBridge *mb = new A4MapBridge(object_xml, this);
        mb->loadObjectAdditionalContent(object_xml, game, of, objectsToRevisit);
        m_bridges.push_back(mb);
        return 0;
    } else if (strcmp(o_class,"BridgeDestination")==0) {
        A4MapBridge *mb = new A4MapBridge(object_xml, this);
        mb->loadObjectAdditionalContent(object_xml, game, of, objectsToRevisit);
        m_bridgeDestinations.push_back(mb);
        return 0;
    } else if (strcmp(o_class,"Trigger")==0) {
        o = new A4Trigger(game->getOntology()->getSort("Trigger"),
                          atoi(object_xml->get_attribute("width")),
                          atoi(object_xml->get_attribute("height")));
        o->loadObjectAdditionalContent(object_xml, game, of, objectsToRevisit);
        bool once = true;
        if (object_xml->get_attribute("repeat")!=0 &&
            strcmp(object_xml->get_attribute("repeat"),"true")==0) {
            once = false;
        }
        XMLNode *scripts_xml = object_xml->get_child("script");
        if (scripts_xml) {
            for(XMLNode *script_xml:*(scripts_xml->get_children())) {
                A4Script *s = new A4Script(script_xml);
                o->addEventRule(A4_EVENT_ACTIVATE, new A4EventRule(A4_EVENT_ACTIVATE, s, once));
            }
        }
    } else {
        o = of->createObject(o_class, game, false, completeRedefinition);
        if (o==0) {
            output_debug_message("Unknown object class: %s\n", o_class);
            exit(1);
        }
        o->loadObjectAdditionalContent(object_xml, game, of, objectsToRevisit);
        if (o->isCharacter()) layer = A4_LAYER_CHARACTERS;
    }
    o->setLayer(layer);
    return o;
}


A4Map::~A4Map()
{
    for(PerceptionBufferRecord *pbr:m_perception_buffer) {
        delete pbr;
    }
    m_perception_buffer.clear();
    for(PerceptionBufferObjectWarpedRecord *pbr:m_warp_perception_buffer) {
        delete pbr;
    }
    m_warp_perception_buffer.clear();
    
	if (m_visibility_regions!=0) delete m_visibility_regions;
	m_visibility_regions = 0;
	if (m_name!=0) delete m_name;
	m_name = 0;
    if (m_name_symbol!=0) delete m_name_symbol;
    m_name_symbol = 0;
	if (m_layers!=0) {
		for(int i = 0;i<A4_N_LAYERS;i++) {
			delete m_layers[i];
			m_layers[i] = 0;
		}
		delete m_layers;
	}
	for(A4MapBridge *mb:m_bridges) delete mb;
	m_bridges.clear();
	for(A4MapBridge *mb:m_bridgeDestinations) delete mb;
	m_bridgeDestinations.clear();
	for(int i = 0;i<A4_NEVENTS;i++) {
		for(A4EventRule *s:m_event_scripts[i]) delete s;
		m_event_scripts[i].clear();
	}
    if (m_xml!=0) delete m_xml;
    m_xml = 0;
    
    for(RespawnRecord *rr:m_respawnRecords) delete rr;
    m_respawnRecords.clear();

    for(DamageVisualization *dv:m_damageVisualizations) delete dv;
    m_damageVisualizations.clear();
    
    for(A4ScriptExecutionQueue *dv:m_script_queues) delete dv;
    m_script_queues.clear();
    
    for(char *a:m_storyStateVariables) delete a;
    m_storyStateVariables.clear();
    for(char *a:m_storyStateValues) delete a;
    m_storyStateValues.clear();
    
    for(PerceptionBufferRecord *a:m_perception_buffer) delete a;
    m_perception_buffer.clear();
    for(PerceptionBufferObjectWarpedRecord *a:m_warp_perception_buffer) delete a;
    m_warp_perception_buffer.clear();
}


std::vector<A4Map *> *A4Map::getNeighborMaps()
{
    std::vector<A4Map *> *l = new std::vector<A4Map *>();

    for(A4MapBridge *mb:m_bridges) {
        bool found = false;
        for(int i = 0;i<l->size();i++) {
            if (l->at(i) == mb->m_linkedTo->m_map) {
                found = true;
                break;
            }
        }
        if (!found) l->push_back(mb->m_linkedTo->m_map);
    }

    return l;
}


void A4Map::cycle(A4Game *game)
{
    if (m_cycle==0) {
        for(A4EventRule *rule:m_event_scripts[A4_EVENT_START]) {
            rule->executeEffects(0, this, game, 0);
        }
    }

	for(int i = 0;i<A4_N_LAYERS;i++) {
		m_layers[i]->cycle(game);
	}

    // respawns:
    for(RespawnRecord *rr:m_respawnRecords) {
        if (rr->time_to_respawn>=0) {
            if (rr->time_to_respawn>0) {
                rr->time_to_respawn--;
            } else {
                // respawn time! check that there are no players in the map
                bool found = false;
                for(A4Character *player:*(game->getPlayers())) {
                    if (player->getMap() == this) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    A4Object *o = loadObjectFromXML(rr->xml, game, 0);
                    o->setX(rr->x);
                    o->setY(rr->y);
                    o->setLayer(rr->layer);
                    o->setMap(this);
                    // check that there is no object blocking the respawn point:
                    if (walkableConsideringVehicles(rr->x,rr->y,o->getPixelWidth(),o->getPixelHeight(),o)) {
                        // respawn!!
                        ((A4AICharacter *)o)->setRespawnRecord(rr);
                        addObject(o,o->getLayer());
                        rr->time_to_respawn = -1;
                    } else {
                        delete o;
                    }
                }
            }
        }
    }

    // scripts:
    for(A4EventRule *r:m_event_scripts[A4_EVENT_TIMER]) r->execute(0,this,game,0);
    for(A4EventRule *r:m_event_scripts[A4_EVENT_STORYSTATE]) r->execute(0,this,game,0);
	executeScriptQueues(game);

    // visual effects:
    {
        std::vector<DamageVisualization *> toDelete;
        for(DamageVisualization *dv:m_damageVisualizations) {
            if (dv->timer%2==0) dv->y--;
            dv->timer--;
            if (dv->timer<0) toDelete.push_back(dv);
        }
        for(DamageVisualization *dv:toDelete) {
            m_damageVisualizations.remove(dv);
            delete dv;
        }
        toDelete.clear();
    }

    // perception buffers:
    {
        std::vector<PerceptionBufferRecord *> toDelete;
        for(PerceptionBufferRecord *pbr:m_perception_buffer) {
            if ((pbr->m_time+CYCLES_IN_PERCEPTION_BUFFER)<m_cycle) toDelete.push_back(pbr);
        }
        for(PerceptionBufferRecord *pbr:toDelete) {
            m_perception_buffer.remove(pbr);
            delete pbr;
        }
        toDelete.clear();
    }
    {
        std::vector<PerceptionBufferObjectWarpedRecord *> toDelete;
        for(PerceptionBufferObjectWarpedRecord *pbr:m_warp_perception_buffer) {
            if ((pbr->m_time+CYCLES_IN_PERCEPTION_BUFFER)<m_cycle) toDelete.push_back(pbr);
        }
        for(PerceptionBufferObjectWarpedRecord *pbr:toDelete) {
            m_warp_perception_buffer.remove(pbr);
            delete pbr;
        }
        toDelete.clear();
    }

	m_cycle++;
}


void A4Map::executeScriptQueues(A4Game *game) {
    std::vector<A4ScriptExecutionQueue *> toDelete;
    for(A4ScriptExecutionQueue *seb:m_script_queues) {
        while(true) {
            A4Script *s = seb->scripts.front();
            int retval = s->execute(seb->object,
                                    (seb->map == 0 ? this:seb->map),
                                    (seb->game == 0 ? game:seb->game),
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


void A4Map::draw(int offsetx, int offsety, float zoom, int SCREEN_X,int SCREEN_Y, A4Game *game)
{
	for(int i = 0;i<A4_N_LAYERS;i++) {
		m_layers[i]->draw(offsetx, offsety, zoom, SCREEN_X, SCREEN_Y, game);
	}

    // damage visualizations:
    for(DamageVisualization *dv:m_damageVisualizations) {
        int x = (dv->x - offsetx)*zoom;
        int y = (dv->y - offsety)*zoom;

        char tmp[16];
        sprintf(tmp,"%i",dv->damage);
        float f = float(dv->timer)/25;
        if (f<0) f = 0;
        if (f>1) f = 1;
        if (dv->damage>0) {
            BInterface::print_left(tmp, game->getFont(), x, y, 1, 0, 0, f);
        } else {
            BInterface::print_left(tmp, game->getFont(), x, y, 1, 1, 0, f);
        }
    }
}


void A4Map::draw(int offsetx, int offsety, float zoom, int SCREEN_X,int SCREEN_Y, int region, A4Game *game)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        m_layers[i]->draw(offsetx, offsety, zoom, SCREEN_X, SCREEN_Y, m_visibility_regions, region, game);
    }
    
    // damage visualizations:
    for(DamageVisualization *dv:m_damageVisualizations) {
        int x = (dv->x - offsetx)*zoom;
        int y = (dv->y - offsety)*zoom;
        
        char tmp[16];
        sprintf(tmp,"%i",dv->damage);
        float f = float(dv->timer)/25;
        if (f<0) f = 0;
        if (f>1) f = 1;
        if (dv->damage>0) {
            BInterface::print_left(tmp, game->getFont(), x, y, 1, 0, 0, f);
        } else {
            BInterface::print_left(tmp, game->getFont(), x, y, 1, 1, 0, f);
        }
    }
}


void A4Map::drawSpeechBubbles(int offsetx, int offsety, float zoom, int SCREEN_X,int SCREEN_Y, A4Game *game)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        m_layers[i]->drawSpeechBubbles(offsetx, offsety, zoom, SCREEN_X, SCREEN_Y, game);
    }
}


void A4Map::drawSpeechBubbles(int offsetx, int offsety, float zoom, int SCREEN_X,int SCREEN_Y, int region, A4Game *game)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        m_layers[i]->drawSpeechBubbles(offsetx, offsety, zoom, SCREEN_X, SCREEN_Y, m_visibility_regions, region, game);
    }
}



void A4Map::clearOpenGLState()
{
	for(int i = 0;i<A4_N_LAYERS;i++) {
		m_layers[i]->clearOpenGLState();
	}
}


void A4Map::removeObject(A4Object *o)
{
    if (o->getLayer()>=0 && o->getLayer()<A4_N_LAYERS) {
        m_layers[o->getLayer()]->removeObject(o);
    }
}


void A4Map::removePerceptionBuffersForObject(A4Object *o, bool actions, bool warps) {
    // remove the corresponding perception buffers:
    if (actions) {
        std::vector<PerceptionBufferRecord *> toDelete;
        for(PerceptionBufferRecord *pbr:m_perception_buffer) {
            if (pbr->m_subject_ID == o->getID() ||
                pbr->m_direct_object_ID == o->getID() ||
                pbr->m_indirect_object_ID == o->getID()) {
                toDelete.push_back(pbr);
            }
        }
        for(PerceptionBufferRecord *pbr:toDelete) m_perception_buffer.remove(pbr);
    }
    if (warps) {
        std::vector<PerceptionBufferObjectWarpedRecord *> toDelete;
        for(PerceptionBufferObjectWarpedRecord *pbr:m_warp_perception_buffer) {
            if (pbr->m_ID == o->getID()) toDelete.push_back(pbr);
        }
        for(PerceptionBufferObjectWarpedRecord *pbr:toDelete) m_warp_perception_buffer.remove(pbr);
    }
}

	
void A4Map::removeObject(A4Object *o, int layer)
{
	m_layers[layer]->removeObject(o);
}


void A4Map::addObject(A4Object *o, int layer)
{
    assert(layer>=0 && layer<A4_N_LAYERS);
	m_layers[layer]->addObject(o);
	o->setLayer(layer);
	o->setMap(this);
}


bool A4Map::contains(class A4Object *o)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (m_layers[i]->contains(o)) return true;
    }

    return false;
}


void A4Map::objectRemoved(A4Object *o)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        m_layers[i]->objectRemoved(o);
    }
}


RespawnRecord *A4Map::getRespawnRecord(int ID)
{
    for(RespawnRecord *rr:m_respawnRecords) {
        if (rr->ID == ID) return rr;
    }
    return 0;
}


bool A4Map::walkableOnlyBackground(int x, int y, int dx, int dy, A4Object *subject)
{
	for(int i = 0;i<A4_N_LAYERS;i++) {
        if (!m_layers[i]->walkableOnlyBackground(x,y,dx,dy, subject)) return false;
	}

	return true;
}


bool A4Map::walkable(int x, int y, int dx, int dy, A4Object *subject)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (!m_layers[i]->walkable(x,y,dx,dy, subject)) return false;
    }
    
    return true;
}


bool A4Map::walkableConsideringVehicles(int x, int y, int dx, int dy, A4Object *subject)
{
    int granularity = 4;
    bool rettiles = true;
    bool retobjects = true;
    A4Object *buffer[16];
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (rettiles && !m_layers[i]->walkableOnlyBackground(x,y,dx,dy, subject)) rettiles = false;
        if (retobjects && !m_layers[i]->walkableOnlyObjects(x,y,dx,dy, subject)) retobjects = false;
    }

    // if there is a vehicle, characters can always walk on them (unless there is a collision with an object):
    if (!rettiles && retobjects && subject->isCharacter()) {
//        std::vector<A4Object *> *l = getAllObjects(x, y, dx, dy);
//        for(A4Object *o:*l) {
        int n = getAllObjects(x, y, dx, dy, buffer,16);
        for(int i = 0;i<n;i++) {
            A4Object *o = buffer[i];
            if (o!=subject && o->isVehicle() && ((A4Vehicle *)o)->isEmpty()) {
                // see if the vehicle covers all the area that was not walkable:
                for(int xoff = 0;xoff<dx;xoff+=granularity) {
                    for(int yoff = 0;yoff<dy;yoff+=granularity) {
                        if (!walkable(x+xoff,y+yoff,1,1,subject)) {
                            if (!o->collision(x+xoff, y+yoff, 1, 1)) {
//                                l->clear();
//                                delete l;
                                return false;
                            }
                        }
                    }
                }
//                l->clear();
//                delete l;
                return true;
            }
        }
//        l->clear();
//        delete l;
    }

    return rettiles && retobjects;
}


A4MapBridge *A4Map::getBridge(int x, int y)
{
	for(A4MapBridge *b:m_bridges) {
//		output_debug_message("A4Map::getBridge: comparing %i,%i to %i,%i-%i,%i\n",x,y,b->m_x,b->m_y,b->m_dx,b->m_dy);
		if (b->m_x<=x && b->m_x+b->m_dx>x &&
			b->m_y<=y && b->m_y+b->m_dy>y) {
			return b;
		}
	}
	return 0;
}


A4Object *A4Map::getTakeableObject(int x, int y, int dx, int dy)
{
	for(int i = 0;i<A4_N_LAYERS;i++) {
		A4Object *o = m_layers[i]->getTakeableObject(x,y,dx,dy);
		if (o!=0) return o;
	}
	return 0;
}


A4Object *A4Map::getBurrowedObject(int x, int y, int dx, int dy)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        A4Object *o = m_layers[i]->getBurrowedObject(x,y,dx,dy);
        if (o!=0) return o;
    }
    return 0;
}


A4Object *A4Map::getUsableObject(int x, int y, int dx, int dy)
{
	for(int i = 0;i<A4_N_LAYERS;i++) {
		A4Object *o = m_layers[i]->getUsableObject(x,y,dx,dy);
		if (o!=0) return o;
	}
	return 0;
}


A4Object *A4Map::getVehicleObject(int x, int y, int dx, int dy)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        A4Object *o = m_layers[i]->getVehicleObject(x,y,dx,dy);
        if (o!=0) return o;
    }
    return 0;
}



std::vector<A4Object *> *A4Map::getAllObjects(int x, int y, int dx, int dy)
{
    std::vector<A4Object *> *l = new std::vector<A4Object *>();
    for(int i = 0;i<A4_N_LAYERS;i++) {
        m_layers[i]->getAllObjects(x, y, dx, dy, l);
    }
    return l;
}


std::vector<A4Object *> *A4Map::getAllObjectCollisions(A4Object *o)
{
    return getAllObjectCollisions(o, 0, 0);
}


std::vector<A4Object *> *A4Map::getAllObjectCollisions(A4Object *o, int xoffs, int yoffs)
{
    std::vector<A4Object *> *l = new std::vector<A4Object *>();
    for(int i = 0;i<A4_N_LAYERS;i++) {
        m_layers[i]->getAllObjectCollisions(o, xoffs, yoffs, l);
    }
    return l;
}


int A4Map::getAllObjects(int x, int y, int dx, int dy, A4Object **l, int max_l)
{
    int n = 0;
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (n>=max_l) break;
        n+=m_layers[i]->getAllObjects(x, y, dx, dy, l+n, max_l);
    }
    return n;
}


int A4Map::getAllObjectsInRegion(int x, int y, int dx, int dy, int region, A4Object **l, int max_l)
{
    int n = 0;
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (n>=max_l) break;
        n+=m_layers[i]->getAllObjectsInRegion(x, y, dx, dy, this, region, l+n, max_l);
    }
    return n;
}


int A4Map::getAllObjectCollisions(A4Object *o, A4Object **l, int max_l)
{
    return getAllObjectCollisions(o, 0, 0, l, max_l);
}


int A4Map::getAllObjectCollisions(A4Object *o, int xoffs, int yoffs, A4Object **l, int max_l)
{
    int n = 0;
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (n>=max_l) break;
        n+=m_layers[i]->getAllObjectCollisions(o, xoffs, yoffs, l+n, max_l);
    }
    return n;
}



bool A4Map::spellCollision(A4Object *spell, A4Object *caster)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (m_layers[i]->spellCollision(spell, caster)) return true;
    }
    return false;
}


bool A4Map::spellCollision(int x0, int y0, int w, int h, A4Object *caster)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (m_layers[i]->spellCollision(x0,y0,w,h, caster)) return true;
    }
    return false;
}



A4Object *A4Map::getObject(int ID)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        A4Object *o = m_layers[i]->getObject(ID);
        if (o!=0) return o;
    }
    return 0;
}


bool A4Map::visible(int tilex, int tiley, int region)
{
	return m_visibility_regions[tilex + tiley*m_dx] == region;
}


int A4Map::visiblilityRegion(int tilex, int tiley) {
    return m_visibility_regions[tilex + tiley*m_dx];
}


void A4Map::reevaluateVisibility()
{
    int x,y;
    int nextRegion = 1;
	bool *inOpen = new bool[m_dx*m_dy];
	for(int i = 0;i<m_dx*m_dy;i++) {
		m_visibility_regions[i] = 0;
		inOpen[i] = false;
	}

    for(int start_y = 0;start_y<m_dy;start_y++) {
        for(int start_x = 0;start_x<m_dx;start_x++) {
            if (m_visibility_regions[start_x+start_y*m_dx]==0 && seeThrough(start_x, start_y)) {
                std::list<int> stack;
                stack.push_back(start_x+start_y*m_dx);
                inOpen[start_x+start_y*m_dx] = true;
                
                while(stack.size()>0) {
                    int tmp = stack.front();
                    x = tmp%m_dx;
                    y = tmp/m_dx;
                    stack.pop_front();
                    
                    if (seeThrough(x,y)) {
                        m_visibility_regions[tmp] = nextRegion;
                        if (x>0 && m_visibility_regions[x+y*m_dx-1]==0 && !inOpen[x+y*m_dx-1]) {
                            stack.push_back(x+y*m_dx-1);
                            inOpen[x+y*m_dx-1] = true;
                        }
                        if (y>0 && m_visibility_regions[x+(y-1)*m_dx]==0 && !inOpen[x+(y-1)*m_dx]) {
                            stack.push_back(x+(y-1)*m_dx);
                            inOpen[x+(y-1)*m_dx] = true;
                        }
                        if (x<(m_dx-1) && m_visibility_regions[x+y*m_dx+1]==0 && !inOpen[x+y*m_dx+1]) {
                            stack.push_back(x+y*m_dx+1);
                            inOpen[x+y*m_dx+1] = true;
                        }
                        if (y<(m_dy-1) && m_visibility_regions[x+(y+1)*m_dx]==0 && !inOpen[x+(y+1)*m_dx]) {
                            stack.push_back(x+(y+1)*m_dx);
                            inOpen[x+(y+1)*m_dx] = true;
                        }
                        if (x>0 && y>0 && m_visibility_regions[x+(y-1)*m_dx-1]==0 && !inOpen[x+(y-1)*m_dx-1]) {
                            stack.push_back(x+(y-1)*m_dx-1);
                            inOpen[x+(y-1)*m_dx-1] = true;
                        }
                        if (x<(m_dx-1) && y>0 && m_visibility_regions[x+(y-1)*m_dx+1]==0 && !inOpen[x+(y-1)*m_dx+1]) {
                            stack.push_back(x+(y-1)*m_dx+1);
                            inOpen[x+(y-1)*m_dx+1] = true;
                        }
                        if (x>0 && y<(m_dy-1) && m_visibility_regions[x+(y+1)*m_dx-1]==0 && !inOpen[x+(y+1)*m_dx-1]) {
                            stack.push_back(x+(y+1)*m_dx-1);
                            inOpen[x+(y+1)*m_dx-1] = true;
                        }
                        if (x<(m_dx-1) && y<(m_dy-1) && m_visibility_regions[x+(y+1)*m_dx+1]==0 && !inOpen[x+(y+1)*m_dx+1]) {
                            stack.push_back(x+(y+1)*m_dx+1);
                            inOpen[x+(y+1)*m_dx+1] = true;
                        }
                    }
                }
                nextRegion++;
            }
        }
    }
    delete []inOpen;
    
    // debug:
    /*
    output_debug_message("reevaluateVisibility\n");
    for(int start_y = 0;start_y<m_dy;start_y++) {
        for(int start_x = 0;start_x<m_dx;start_x++) {
            if (m_visibility_regions[start_x+start_y*m_dx]==0) {
                output_debug_message(".");
            } else {
                output_debug_message("%c",'a'+m_visibility_regions[start_x+start_y*m_dx]);
            }
        }
        output_debug_message("\n");
    }
     */
}


bool A4Map::seeThrough(int x, int y)
{
	for(int i = 0;i<A4_N_LAYERS;i++) {
		if (!m_layers[i]->seeThrough(x,y)) return false;
	}
	return true;
}


void A4Map::triggerObjectsEvent(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game)
{
	for(int i = 0;i<A4_N_LAYERS;i++) {
		m_layers[i]->triggerObjectsEvent(event,otherCharacter,map,game);
	}
}


void A4Map::triggerObjectsEventWithID(int event, char *ID, class A4Character *otherCharacter, class A4Map *map, class A4Game *game)
{
	for(int i = 0;i<A4_N_LAYERS;i++) {
		m_layers[i]->triggerObjectsEventWithID(event,ID,otherCharacter,map,game);
	}
}


bool A4Map::chopTree(A4Character *o, A4Object *tool, A4Game *game, int direction)
{
    int x = o->getX() + A4Game::direction_x_inc[direction];
    int y = o->getY() + A4Game::direction_y_inc[direction];
    int dx = o->getPixelWidth();
    int dy = o->getPixelHeight();
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (m_layers[i]->chopTree(x,y,dx,dy)) {
            tool->event(A4_EVENT_USE, o, this, game);
            return true;
        }
    }
    return false;
}


bool A4Map::checkIfDoorGroupStateCanBeChanged(Symbol *doorGroup, bool state, A4Character *character, A4Map *map, A4Game *game)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        if (!m_layers[i]->checkIfDoorGroupStateCanBeChanged(doorGroup, state, character, map, game)) return false;
    }
    return true;
}


void A4Map::setDoorGroupState(Symbol *doorGroup, bool state, A4Character *character, A4Map *map, A4Game *game)
{
    for(int i = 0;i<A4_N_LAYERS;i++) {
        m_layers[i]->setDoorGroupState(doorGroup, state, character, map, game);
    }
}


void A4Map::setStoryStateVariable(const char *variable, const char *value)
{
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
}


char *A4Map::getStoryStateVariable(const char *variable)
{
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
}


void A4Map::saveToXML(XMLwriter *w, A4Game *game)
{
    w->openTag("map");
    w->setAttribute("version", "1.0");
    w->setAttribute("orientation", "orthogonal");
    w->setAttribute("width", m_dx);
    w->setAttribute("height", m_dy);
    w->setAttribute("tilewidth", getTileWidth());
    w->setAttribute("tileheight", getTileHeight());
    
    w->openTag("properties");
    w->openTag("property");
    w->setAttribute("name", "name");
    w->setAttribute("value", m_name);
    w->closeTag("property");
    w->closeTag("properties");

    int firstID = 1;
    for(GraphicFile *gf:*game->getGraphicFiles()) {
        w->openTag("tileset");
        w->setAttribute("firstgid", firstID);
        w->setAttribute("name", "graphics");
        w->setAttribute("tilewidth", gf->m_tile_dx);
        w->setAttribute("tileheight", gf->m_tile_dy);
        w->openTag("image");
        w->setAttribute("source", gf->m_name);
        w->setAttribute("width", gf->m_tiles_per_row*gf->m_tile_dx);
        w->setAttribute("height", gf->m_tile_dy*(gf->m_n_tiles/gf->m_tiles_per_row));
        w->closeTag("image");
        w->closeTag("tileset");
        firstID += gf->m_n_tiles;
    }
    
    // tile layers:
    // "m_n_layers" is the number of tile layers
    // A4_N_LAYERS is the total number of layers, which might be higher
    for(int i = 0;i<m_n_layers;i++) {
        char tmp[256];
        A4MapLayer *ml = m_layers[i];
        w->openTag("layer");
        sprintf(tmp,"Tile Layer %i",i+1);
        w->setAttribute("name", tmp);
        w->setAttribute("width", m_dx);
        w->setAttribute("height", m_dy);
        w->openTag("data");
        for(int y = 0;y<m_dy;y++) {
            for(int x = 0;x<m_dx;x++) {
                w->openTag("tile");
                w->setAttribute("gid", ml->m_tiles[x+y*m_dx]+1);
                w->closeTag("tile");
            }
        }
        w->closeTag("data");
        w->closeTag("layer");
    }
    
    // object layers:
    w->openTag("objectgroup");
    w->setAttribute("name", "objects");
    w->setAttribute("width", m_dx);
    w->setAttribute("height", m_dy);
    for(A4MapBridge *b:m_bridges) b->saveToXML(w,game,1,true);
    for(A4MapBridge *b:m_bridgeDestinations) b->saveToXML(w,game,2,true);
    for(int i = 0;i<A4_N_LAYERS;i++) {
        A4MapLayer *ml = m_layers[i];
        for(A4Object *o:ml->m_objects) {
            if (!o->isPlayer())
                o->saveToXML(w,game,0,true);
        }
    }
    w->closeTag("objectgroup");
    
    // respawn records:
    for(RespawnRecord *r:m_respawnRecords) {
        w->openTag("respawnRecord");
        w->setAttribute("ID", r->ID);
        w->setAttribute("x", r->x);
        w->setAttribute("y", r->y);
        w->setAttribute("layer", r->layer);
        w->setAttribute("respawn_probability", r->respawn_probability);
        w->setAttribute("time_to_respawn", r->time_to_respawn);
        r->xml->saveToXML(w);
        w->closeTag("respawnRecord");
    }
    
    if (!m_storyStateVariables.empty()) {
        w->openTag("onStart");
        std::vector<char *>::iterator it1 = m_storyStateVariables.begin();
        std::vector<char *>::iterator it2 = m_storyStateValues.begin();
        while(it1!=m_storyStateVariables.end()) {
            w->openTag("storyState");
            w->setAttribute("variable", *it1);
            w->setAttribute("value", *it2);
            w->setAttribute("scope", "map");
            w->closeTag("storyState");
            it1++;
            it2++;
        }
        w->closeTag("onStart");
    }
    
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
    
    w->closeTag("map");
    
}
