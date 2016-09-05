#ifndef _A4ENGINE_CHARACTER
#define _A4ENGINE_CHARACTER

#include <list>
#include <algorithm>

#include "A4WalkingObject.h"

#define A4CHARACTER_STATE_NONE		-1
#define A4CHARACTER_STATE_IDLE		0
#define A4CHARACTER_STATE_WALKING	1
#define A4CHARACTER_STATE_INTERACTING	2
#define A4CHARACTER_STATE_TALKING	3
#define A4CHARACTER_STATE_TALKING_ANGRY	4
#define A4CHARACTER_STATE_ATTACKING	5
#define A4CHARACTER_STATE_CASTING	6
#define A4CHARACTER_STATE_IN_VEHICLE    7
#define A4CHARACTER_STATE_DYING     8

#define A4CHARACTER_COMMAND_IDLE	0
#define A4CHARACTER_COMMAND_WALK	1
// note: "TAKE" turns into "USE" when there is no takeable object, but there is an "useable" object
#define A4CHARACTER_COMMAND_TAKE	2
#define A4CHARACTER_COMMAND_DROP	3
#define A4CHARACTER_COMMAND_DROP_GOLD	4
// note: "USE" turns into "EQUIP" for equipable objects
#define A4CHARACTER_COMMAND_USE		5
#define A4CHARACTER_COMMAND_UNEQUIP	6
#define A4CHARACTER_COMMAND_INTERACT	7
#define A4CHARACTER_COMMAND_TALK    8
#define A4CHARACTER_COMMAND_TALK_ANGRY    9
#define A4CHARACTER_COMMAND_ATTACK  10
#define A4CHARACTER_COMMAND_SPELL   11
#define A4CHARACTER_COMMAND_GIVE    12
#define A4CHARACTER_COMMAND_SELL    13
#define A4CHARACTER_COMMAND_BUY     14


class A4CharacterCommand {
public:
    A4CharacterCommand(int c, int a, int d, class SpeechAct *sa, int p) {
        m_command = c;
        m_argument = a;
        m_direction = d;
        m_target = 0;
        m_speechAct = sa;
        m_priority = p;
    }
    A4CharacterCommand(int c, int a, int d, A4Object *target, class SpeechAct *sa, int p) {
        m_command = c;
        m_argument = a;
        m_direction = d;
        m_target = target;  // if there are multiple objects in the same direction, this allows to specify which one to target
        m_speechAct = sa;
        m_priority = p;
    }
    A4CharacterCommand(int c, int a, A4Object *target, int p) {
        m_command = c;
        m_argument = a;
        m_direction = A4_DIRECTION_NONE;
        m_target = target;
        m_speechAct = 0;
        m_priority = p;
    }
    ~A4CharacterCommand() {
        if (m_speechAct!=0) delete m_speechAct;
        m_speechAct = 0;
    }
    int m_command;
    int m_argument;
    int m_direction;
    A4Object *m_target;
    class SpeechAct *m_speechAct;
    int m_priority;
};


class A4Character : public A4WalkingObject {
public:
	A4Character(Symbol *name, Sort *sort);
	virtual ~A4Character();
    
    virtual void loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit);
    virtual bool loadObjectAttribute(XMLNode *xml);
    virtual void revisitObject(XMLNode *xml, A4Game *game);
    virtual void savePropertiesToXML(class XMLwriter *w, A4Game *game);

	virtual bool cycle(A4Game *game);
    virtual void draw(int offsetx, int offsety, float zoom, class A4Game *game);
    virtual void drawSpeechBubbles(int offsetx, int offsety, float zoom, class A4Game *game);

    bool isIdle();
    int getPreviousState() {return m_previous_state;}    // this returns the state in which the character was the last cycle
	int getState() {return m_state;}
    bool isTalkingIdle() {return m_talking_state == A4CHARACTER_STATE_IDLE;}
    int getTalkingState() {return m_talking_state;}

    virtual void issueCommand(A4CharacterCommand *command, class A4Game *game);
	virtual void issueCommand(int command, class SpeechAct *argument, int direction, A4Character *target, class A4Game *game);
    virtual void issueCommand(int command, int argument, int direction, A4Object *target, class A4Game *game);

    bool take(A4Game *game);
    bool use(A4Game *game); // e.g. activate lever
    bool attack(A4Character *other);
    bool takeDamage(int damage);
    
    virtual void receiveSpeechAct(A4Character *speaker, A4Character *receiver, SpeechAct *sa);

	int getHp() {return m_hp;}
	int getMaxHp() {return m_max_hp;}
	int getMp() {return m_mp;}
	int getMaxMp() {return m_max_mp;}
	int getXp() {return m_xp;}
	int getXpToLevelUp() {return (int)m_experienceToLevelUp;}
    int getLevel() {return m_level;}
    int getAttack(bool considerSpellEffects);
    int getBaseAttack() {return m_attack;}
    int getDefense(bool considerSpellEffects);
    int getBaseDefense() {return m_defense;}
    float getMagicBonus();
    virtual int getWalkSpeed();
    
    void gainXp(int xp);
    void levelUp();

    // embark/disembark vehicles:
    void embark(class A4Vehicle *v);
    void disembark();
    bool isInVehicle() {return m_vehicle!=0;}
    class A4Vehicle *getVehicle() {return m_vehicle;}

	void setExperienceToLevelUp(int v, float m) {
		m_experienceToLevelUp = (float)v;
		m_experienceToLevelUp_multiplier = m;
	}
	void setGivesExperience(int ge) {m_givesExperience = ge;}
    int getGivesExperience() {return m_givesExperience;}
    void recoverHp(int hp) {m_hp = std::min(m_max_hp,m_hp+hp);}

    void setHp(int hp) {m_hp = hp;}
    void setMp(int mp) {m_mp = mp;}
    void setXp(int xp) {m_xp = xp;}
	void setMaxHp(int hp) {m_max_hp = hp;}
	void setHpLevelupadd(int hp_lua) {m_hp_levelupadd = hp_lua;}
	void setMaxMp(int mp) {m_max_mp = mp;}
    void recoverMp(int mp) {m_mp = std::min(m_max_mp,m_mp+mp);}
	void setMpLevelupadd(int mp_lua) {m_mp_levelupadd = mp_lua;}
    void setLevel(int l) {m_level = l;}
	void setAttack(int attack) {m_attack = attack;}
	void setAttackLevelupadd(int attack_lua) {m_attack_levelupadd = attack_lua;}
	void setDefense(int defense) {m_defense = defense;}
	void setDefenseLevelupadd(int defense_lua) {m_defense_levelupadd = defense_lua;}
	void setAttackModifier(float am) {m_attack_modifier = am;}
	void setDefenseModifier(float dm) {m_defense_modifier = dm;}
	void setMagicImmune(bool m) {m_magicImmune = m;}

	void learnSpell(int i) {m_spells[i] = true;}
	bool knowsSpell(int i) {return m_spells[i];}
    void addSpellEffect(int spell, int cycles) {m_spell_effect_counter[spell] += cycles;}
    bool hasSpellEffect(int spell) {return m_spell_effect_counter[spell]>0;}
    bool castSpell(int argument, int direction, A4Game *game, bool usingWand);
    
	void addObjectToInventory(A4Object *o, A4Game *game);
	void removeFromInventory(A4Object *o) {m_inventory.remove(o);}
	std::list<A4Object *> *getInventory() {return &m_inventory;}
	A4Object *getInventory(int i);
	A4Object *getEquipment(int i);
    void setEquipment(int i, A4Object *o);
	int getDirection() {return m_direction;}

	virtual bool isWalkable() {return isInVehicle();}
	virtual bool isHeavy() {return true;}		// this is used by pressure plates
	virtual bool isInteracteable() {return true;}
    virtual bool isCharacter() {return true;}

protected:
	// attributes:
	int m_hp, m_max_hp, m_hp_levelupadd;
	int m_mp, m_max_mp, m_mp_levelupadd;
	int m_xp;
    int m_level;
	float m_experienceToLevelUp, m_experienceToLevelUp_multiplier;
	int m_givesExperience;
	int m_attack, m_attack_levelupadd;
	int m_defense, m_defense_levelupadd;
	float m_attack_modifier;
	float m_defense_modifier;
	bool m_magicImmune;

	bool m_spells[A4_N_SPELLS];
	std::list<A4Object *> m_inventory;
	A4Object *m_equipment[A4_EQUIPABLE_N_SLOTS];

    int m_talking_state;
    int m_talking_state_cycle;

    class A4Vehicle *m_vehicle;

    int m_spell_effect_counter[A4_N_SPELLS];    // number of cycles left of this spell effect
    
    // talking:
    class SpeechAct *m_talkingSpeechAct;
    class A4TextBubble *m_talkingBubble;
    A4Character *m_talkingTarget;
};

#endif

