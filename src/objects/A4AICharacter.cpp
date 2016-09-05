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
#include "XMLwriter.h"
#include "Animation.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4Object.h"
#include "A4Game.h"

#include "A4Character.h"
#include "A4AICharacter.h"
#include "A4Vehicle.h"

#include "A4Behavior.h"
#include "A4AI.h"
#include "AIMemory.h"

#include "ConversationGraph.h"

A4AICharacter::A4AICharacter(Symbol *name, Sort *sort) : A4Character(name, sort)
{
	m_sightRadius = 5;
    m_respawn = 0;
    m_emotion = A4_EMOTION_DEFAULT;
    m_emotionAnimation = 0;
    m_respawnRecord = 0;

    m_AI = new A4AI(this, m_sightRadius);
}


A4AICharacter::~A4AICharacter()
{
//    output_debug_message("Deleting ~A4AICharacter %p (%s)\n",this,(m_name==0 ? "-":m_name->get()));
    delete m_AI;
    m_AI = 0;
    if (m_emotionAnimation!=0) delete m_emotionAnimation;
    m_emotionAnimation = 0;
}


void A4AICharacter::receiveSpeechAct(A4Character *speaker, A4Character *receiver, SpeechAct *sa) {
    m_AI->receiveSpeechAct(speaker, receiver, sa);
}


bool A4AICharacter::cycle(A4Game *game)
{
    m_AI->cycle(game);

    bool ret = A4Character::cycle(game);

    if (m_emotionAnimation!=0) m_emotionAnimation->cycle();

    if (!ret) {
        // character will disappear, notify the respawn record:
        if (m_respawnRecord!=0) {
            if (((float)rand() / RAND_MAX)<m_respawn) {
                m_respawnRecord->time_to_respawn = A4_RESPAWN_TIME;
            }
        }
    }

    return ret;
}

int A4AICharacter::getEmotion() {
    AIMemory *m = m_AI->getMemory();
    int highest_activation = 0;
    int highest_emotion = A4_EMOTION_DEFAULT;
    for(int i = 0;i<A4_N_EMOTIONS;i++) {
        if (A4Game::emotionNames[i]!=0) {
            int activation = m->maxActivationOfWMEwithFunctor(A4Game::emotionNames[i], getID());
            if (activation>highest_activation) {
                highest_activation = activation;
                highest_emotion = i;
            }
        }
    }
    return highest_emotion;
}


void A4AICharacter::draw(int offsetx, int offsety, float zoom, A4Game *game)
{
    A4Character::draw(offsetx, offsety, zoom, game);

    // emotions:
    int highest_emotion = getEmotion();

    Animation *a = game->getEmotionAnimation(highest_emotion);
    if (m_emotion != highest_emotion) {
        m_emotion = highest_emotion;
        if (m_emotionAnimation!=0) delete m_emotionAnimation;
        if (a==0) {
            m_emotionAnimation = 0;
        } else {
            m_emotionAnimation = new Animation(a);
        }
    }
    if (m_emotionAnimation!=0) m_emotionAnimation->draw((m_x + offsetx)*zoom, (m_y + offsety)*zoom, zoom);
}


void A4AICharacter::loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit)
{
    A4Character::loadObjectAdditionalContent(xml,game,of, objectsToRevisit);
    
    // conversation graph:
    XMLNode *conversationgraph_xml = xml->get_child("conversationGraph");
    if (conversationgraph_xml!=0) {
        char *file = conversationgraph_xml->get_attribute("name");
        if (file!=0) {
            // it's defined in an external file:
            char *fullpath = new char[strlen(game->get_game_path()) + strlen(file) + 2];
            sprintf(fullpath,"%s/%s",game->get_game_path(),file);
            XMLNode *xml2 = XMLNode::from_file(fullpath);
            getAI()->setConversationGraph(new ConversationGraph(xml2));
            delete xml2;
            delete []fullpath;
        } else {
            // it's defined on the spot:
            getAI()->setConversationGraph(new ConversationGraph(conversationgraph_xml));
        }
    }
    
    // conversation rules:
    std::vector<XMLNode *> *conversationrules_xml = xml->get_children("conversationGraphTransition");
    for(XMLNode *rule_xml:*conversationrules_xml) {
        getAI()->getConversationGraph()->addConversationGraphTransition(rule_xml);
    }
    delete conversationrules_xml;
    
    // inference rules:
    std::vector<XMLNode *> *inferencerules_xml = xml->get_children("inferenceRule");
    for(XMLNode *rule_xml:*inferencerules_xml) {
        InferenceRule *r = new InferenceRule(rule_xml, game->getOntology());
        getAI()->addInferenceRule(r);
    }
    delete inferencerules_xml;
}


bool A4AICharacter::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4Character::loadObjectAttribute(attribute_xml)) return true;
    
    char *a_name = attribute_xml->get_attribute("name");
    
    if (strcmp(a_name,"sightRadius")==0) {
        setSightRadius(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"respawn")==0) {
        setRespawn(atof(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"AI.period")==0) {
        getAI()->setPeriod(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"AI.cycle")==0) {
        getAI()->setCycle(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"respawnRecordID")==0) {
        // this will be handled later in the map
        return true;
    }
    return false;
}


void A4AICharacter::revisitObject(XMLNode *xml, A4Game *game)
{
    A4Character::revisitObject(xml, game);
    
    std::vector<XMLNode *> *attributes_xml = xml->get_children("attribute");
    for(XMLNode *attribute_xml:*attributes_xml) {
        char *a_name = attribute_xml->get_attribute("name");
        if (strcmp(a_name,"respawnRecordID")==0) {
            int rID = atoi(attribute_xml->get_attribute("value"));
            RespawnRecord *rr = game->getRespawnRecord(rID);
            assert(rr!=0);
            setRespawnRecord(rr);
        }
    }
    delete attributes_xml;
}


void A4AICharacter::objectRemoved(A4Object *o)
{
    A4Character::objectRemoved(o);
    
    m_AI->getMemory()->objectRemoved(o);
}



void A4AICharacter::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4Character::savePropertiesToXML(w,game);
    char tmp[1024];
    
    saveObjectAttributeToXML(w,"sightRadius",m_sightRadius);
    saveObjectAttributeToXML(w,"respawn",m_respawn);

    saveObjectAttributeToXML(w,"AI.period",m_AI->getPeriod());
    saveObjectAttributeToXML(w,"AI.cycle",m_AI->getCycle());
    
    bool tagOpen = false;
    for(WME *wme:*(m_AI->getMemory()->getShortTermWMEs())) {
        if (!tagOpen) {
            w->openTag("onStart");
            tagOpen = true;
        }
        w->openTag("addWME");
        wme->toStringNoActivation(tmp);
        w->setAttribute("wme",tmp);
        w->setAttribute("activation",wme->getActivation());
        w->closeTag("addWME");
    }
    for(WME *wme:*(m_AI->getMemory()->getLongTermWMEs())) {
        if (!tagOpen) {
            w->openTag("onStart");
            tagOpen = true;
        }
        w->openTag("addWME");
        wme->toStringNoActivation(tmp);
        w->setAttribute("wme",tmp);
        w->setAttribute("activation",wme->getActivation());
        w->closeTag("addWME");
    }
    for(A4Behavior *b:*(m_AI->getBehaviors())) {
        if (!tagOpen) {
            w->openTag("onStart");
            tagOpen = true;
        }
        w->openTag("addBehavior");
        if (b->getID()!=0) w->setAttribute("ID", b->getID());
        w->setAttribute("priority", b->getPriority());
        b->saveToXML(w);
        w->addContent(tmp);
        w->closeTag("addBehavior");
    }
    for(A4Script *pt:*(m_AI->getPendingTalk())) {
        if (!tagOpen) {
            w->openTag("onStart");
            tagOpen = true;
        }
        pt->saveToXML(w);
    }
    if (tagOpen) w->closeTag("onStart");
    
    for(InferenceRule *ir:*(m_AI->getInferenceRules())) {
        ir->saveToXML(w);
    }

    if (m_AI->getConversationGraph()!=0) m_AI->getConversationGraph()->saveToXML(w);
    
    if (m_respawnRecord!=0) saveObjectAttributeToXML(w,"respawnRecordID",m_respawnRecord->ID);
}
