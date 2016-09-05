#ifndef _A4ENGINE_AI_CHARACTER
#define _A4ENGINE_AI_CHARACTER

#include "A4AI.h"

class A4AICharacter : public A4Character {
public:
	A4AICharacter(Symbol *name, Sort *sort);
	virtual ~A4AICharacter();
    
    virtual void loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit);
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void revisitObject(XMLNode *xml, A4Game *game);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

    virtual bool cycle(A4Game *game);
    virtual void draw(int offsetx, int offsety, float zoom, class A4Game *game);

    void setSightRadius(int sr) {m_sightRadius = sr;}
    int getSightRadius() {return m_sightRadius;}
	void setRespawn(float r) {m_respawn = r;}

    float getRespawn() {return m_respawn;}
    virtual bool respawns() {return m_respawn>0;}
    virtual bool isAICharacter() {return true;}
    void setRespawnRecord(RespawnRecord *rr) {m_respawnRecord = rr;}

    A4AI *getAI() {return m_AI;}
    int getEmotion();

    void addBehavior(A4Behavior *b) {m_AI->addBehavior(b);}

    virtual void receiveSpeechAct(A4Character *speaker, A4Character *receiver, SpeechAct *sa);
    
    virtual void objectRemoved(A4Object *o);
    
protected:
	// attributes:
	int m_sightRadius;
	float m_respawn;
    int m_emotion;
    Animation *m_emotionAnimation;

    RespawnRecord *m_respawnRecord;

    A4AI *m_AI;
};

#endif

