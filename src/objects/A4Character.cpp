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
#include "A4Map.h"
#include "A4Object.h"
#include "A4Game.h"
#include "A4Item.h"
#include "A4CoinPurse.h"
#include "A4Character.h"
#include "A4EquipableItem.h"
#include "A4TextBubble.h"
#include "A4Spell.h"
#include "A4Vehicle.h"

#include "Ontology.h"
#include "A4AI.h"

#include "A4ObjectFactory.h"

// This is a hack, I need to find a way so that I don't need to include these variables here (but without having to pass them
// via parameter to all the A4Object.draw methods...)
extern int SCREEN_X,SCREEN_Y;

#define TEXT_SPEED  5


A4Character::A4Character(Symbol *name, Sort *sort) : A4WalkingObject(name, sort)
{
	m_currentAnimation = A4_ANIMATION_IDLE_RIGHT;

	m_xp = 0;
    m_level = 1;
	m_experienceToLevelUp = 10;
	m_experienceToLevelUp_multiplier = 1.5f;
	m_givesExperience = 1;
	m_hp = m_max_hp = 10;
	m_hp_levelupadd = 1;
	m_mp = m_max_mp =10;
	m_mp_levelupadd = 1;
	m_attack = 2;
	m_attack_levelupadd = 1;
	m_defense = 2;
	m_defense_levelupadd = 1;
	m_attack_modifier = 1.0f;
	m_defense_modifier = 1.0f;
	m_canSwim = 0;
	m_magicImmune = false;

    for(int i = 0;i<A4_N_SPELLS;i++) {
        m_spells[i] = false;
        m_spell_effect_counter[i] = 0;
    }
    for(int i = 0;i<A4_EQUIPABLE_N_SLOTS;i++) m_equipment[i] = 0;

    m_talking_state =A4CHARACTER_STATE_IDLE;
    m_talking_state_cycle = 0;

	m_walking_counter = 0;

    m_talkingSpeechAct = 0;
    m_talkingBubble = 0;
    m_talkingTarget = 0;

    m_vehicle = 0;
}


A4Character::~A4Character()
{
    for(A4Object *o:m_inventory) {
 //       output_debug_message("deleting inventory item: %p\n",o);
        delete o;
    }
	m_inventory.clear();
	for(int i = 0;i<A4_EQUIPABLE_N_SLOTS;i++) {
		if (m_equipment[i]!=0) delete m_equipment[i];
		m_equipment[i] = 0;
	}
    if (m_talkingBubble!=0) delete m_talkingBubble;
    m_talkingBubble = 0;
    if (m_talkingSpeechAct!=0) delete m_talkingSpeechAct;
    m_talkingSpeechAct = 0;
}


bool A4Character::isIdle()
{
    if (m_vehicle==0) {
        return m_state == A4CHARACTER_STATE_IDLE;
    } else {
        return m_state == A4CHARACTER_STATE_IN_VEHICLE && m_vehicle->getState() == A4CHARACTER_STATE_IDLE;
    }
}



void A4Character::issueCommand(A4CharacterCommand *command, class A4Game *game)
{
    if (command->m_speechAct!=0) {
        issueCommand(command->m_command, command->m_speechAct, command->m_direction, (A4Character *)command->m_target, game);
    } else {
        issueCommand(command->m_command, command->m_argument, command->m_direction, command->m_target, game);
    }
}


void A4Character::issueCommand(int command, int argument, int direction, A4Object *target, A4Game *game)
{
	if (m_state==A4CHARACTER_STATE_IN_VEHICLE) {
        if (command==A4CHARACTER_COMMAND_WALK ||
            command==A4CHARACTER_COMMAND_ATTACK ||
            command==A4CHARACTER_COMMAND_INTERACT) {
            m_vehicle->issueCommand(command, argument, direction, target, game);
            return;
        } else {
            if (command!=A4CHARACTER_COMMAND_TAKE) return;
        }
    } else {
        if (m_state!=A4CHARACTER_STATE_IDLE) return;
    }
	switch(command) {
        case A4CHARACTER_COMMAND_IDLE:
            break;
		case A4CHARACTER_COMMAND_WALK:
            m_direction_command_received_this_cycle[direction] = true;
            m_continuous_direction_command_max_movement[direction] = argument;
			break;
		case A4CHARACTER_COMMAND_TAKE:
			{
                if (isInVehicle()) {
                    getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_disembark, getID(), getSort(), getVehicle()->getID(), getVehicle()->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                    disembark();
                } else {
                    if (!take(game)) {
                        if (!use(game)) {
                            // see if there is a vehicle:
                            A4Object *v = m_map->getVehicleObject(m_x + getPixelWidth()/2 - 1, m_y + getPixelHeight()/2 - 1, 2, 2);
                            if (v!=0) {
                                embark((A4Vehicle *)v);
                                getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_embark, getID(), getSort(), v->getID(), v->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                            }
                        }
                    }
                }
			}
			break;
		case A4CHARACTER_COMMAND_DROP:
			{
				A4Object *o = getInventory(argument);
				if (o!=0) {
					// drop:
					m_inventory.remove(o);
					game->requestWarp(o, m_map, m_x, m_y, A4_LAYER_FG);
                    getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_drop, getID(), getSort(), o->getID(), o->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                    o->event(A4_EVENT_DROP, this, m_map, game);
                    eventWithObject(A4_EVENT_ACTION_DROP, 0, o, getMap(), game);
					break;
				}
			}
			break;
		case A4CHARACTER_COMMAND_DROP_GOLD:
			if (m_gold>=argument) {
				m_gold-=argument;
                A4Object *o = new A4CoinPurse(game->getOntology()->getSort("CoinPurse"), argument, new Animation(game->getCoinPurseAnimation()), true);
				game->requestWarp(o, m_map, m_x, m_y, A4_LAYER_FG);
                getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_drop, getID(), getSort(), o->getID(), o->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                event(A4_EVENT_ACTION_DROP_GOLD, 0, getMap(), game);
			} else {
				game->addMessage(this, "Not enough gold!");
			}
			break;
		case A4CHARACTER_COMMAND_USE:
			{
				A4Object *o = getInventory(argument);
				if (o!=0) {
					if (o->isEquipable()) {
						A4EquipableItem *ei = (A4EquipableItem *)o;
						int slot = ei->getEquipableSlot();
						if (m_equipment[slot]!=0) {
							// unequip first:
							A4Object *tmp = m_equipment[slot];
							m_equipment[slot] = 0;
							addObjectToInventory(tmp, game);
							tmp->event(A4_EVENT_UNEQUIP, this, m_map, game);
                            eventWithObject(A4_EVENT_ACTION_UNEQUIP, 0, tmp, getMap(), game);
						}
						if (m_equipment[slot]==0) {
							// equip:
							m_inventory.remove(o);	// make space to unequip
							m_equipment[slot] = o;
							o->event(A4_EVENT_EQUIP, this, m_map, game);
                            eventWithObject(A4_EVENT_ACTION_EQUIP, 0, o, getMap(), game);
						}
					} else if (o->getUsable()) {
						o->event(A4_EVENT_USE, this, m_map, game);
                        eventWithObject(A4_EVENT_ACTION_USE, 0, o, getMap(), game);
					}
				}
			}
			break;
		case A4CHARACTER_COMMAND_UNEQUIP:
			{
				A4Object *tmp = m_equipment[argument];
				if (tmp!=0) {
					m_equipment[argument] = 0;
					addObjectToInventory(tmp, game);
					tmp->event(A4_EVENT_UNEQUIP, this, m_map, game);
                    eventWithObject(A4_EVENT_ACTION_UNEQUIP, 0, tmp, getMap(), game);
				}
			}
			break;
		case A4CHARACTER_COMMAND_INTERACT:
			{
				// get the object to interact with:
				std::vector<A4Object *> *collisions = m_map->getAllObjectCollisions(this, A4Game::direction_x_inc[direction], A4Game::direction_y_inc[direction]);
				for(A4Object *o:*collisions) {
					if (o->isInteracteable()) {
						// interact:
						m_direction = direction;
						delete collisions;
                        m_state = A4CHARACTER_STATE_INTERACTING;
                        m_state_cycle = 0;
                        getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_interact, getID(), getSort(), o->getID(), o->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                        o->event(A4_EVENT_INTERACT,this,m_map,game);
                        eventWithObject(A4_EVENT_ACTION_INTERACT, 0, o, getMap(), game);
						return;
					}
				}
				delete collisions;
				// nothing interacteable, check if we have a weapon that can chop, and there is a chopeable tree:
                A4EquipableItem *o = (A4EquipableItem *)getEquipment(A4_EQUIPABLE_WEAPON);
                if (o!=0 && o->getCanChop()) {
                    if (m_map->chopTree(this, o, game,  direction)) {
                        m_state = A4CHARACTER_STATE_INTERACTING;
                        m_state_cycle = 0;
                        eventWithObject(A4_EVENT_ACTION_CHOP, 0, o, getMap(), game);
                        break;
                    }
                }

                // just default to a walk:
				issueCommand(A4CHARACTER_COMMAND_WALK, argument, direction, target, game);
			}
			break;
        case A4CHARACTER_COMMAND_ATTACK:
            {
                // get the object to interact with:
                std::vector<A4Object *> *collisions = m_map->getAllObjectCollisions(this, A4Game::direction_x_inc[direction], A4Game::direction_y_inc[direction]);
                for(A4Object *o:*collisions) {
                    if (target!=0 && o!=target) continue;
                    if (o->isCharacter()) {
                        // attack!
                        m_direction = direction;
                        bool hit = attack((A4Character *)o);
                        delete collisions;
                        m_state = A4CHARACTER_STATE_ATTACKING;
                        m_state_cycle = 0;
                        getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_attack, getID(), getSort(), o->getID(), o->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                        eventWithInteger(A4_EVENT_ACTION_ATTACK, hit ? 1:0, (A4Character *)o, getMap(), game);
                        return;
                    }
                }
                delete collisions;

                // nothing attackable, check if we have a weapon that can chop, and there is a chopeable tree:
                A4EquipableItem *o = (A4EquipableItem *)getEquipment(A4_EQUIPABLE_WEAPON);
                if (o!=0 && o->getCanChop()) {
                    if (m_map->chopTree(this, o, game, m_direction)) {
                        m_state = A4CHARACTER_STATE_INTERACTING;
                        m_state_cycle = 0;
                        eventWithObject(A4_EVENT_ACTION_CHOP, 0, o, getMap(), game);
                        break;
                    }
                }

                // just default to a walk:
                issueCommand(A4CHARACTER_COMMAND_WALK, argument, direction, target, game);
            }
            break;

        case A4CHARACTER_COMMAND_SPELL:
        {
            castSpell(argument, direction, game, false);
        }
            break;

        case A4CHARACTER_COMMAND_GIVE:
            {
                A4Object *item_to_give = getInventory(argument);
                if (item_to_give==0) {
                    // error!
                    output_debug_message("Character %s trying to give item %i, which it does not have...\n", getName()->get(), argument);
                    exit(1);
                } else {
                    int x2 = target->getX() + target->getPixelWidth()/2;
                    int y2 = target->getY() + target->getPixelHeight()/2;
                    int dx = (getX() + getPixelWidth()/2) - x2;
                    int dy = (getY() + getPixelHeight()/2) - y2;
                    int d = dx*dx + dy*dy;
                    int maxd = std::max(game->getTileDx(), game->getTileDy())*5;
                    if (d>maxd*maxd) {
                        // too far!
                        output_debug_message("Character %s trying to give item %i to a character that is too far...\n", getName()->get(), argument);
                        if (this == game->getCurrentPlayer()) game->addMessage(this, "I need to get closer!");
                    } else {
                        A4Character *target_c = (A4Character *)target;
                        if (target_c->getInventory()->size()>=A4_INVENTORY_SIZE) {
                            if (this == game->getCurrentPlayer()) game->addMessage(this, "The other's inventory is full!");
                        } else {
                            // give!
                            m_inventory.remove(item_to_give);
                            target_c->addObjectToInventory(item_to_give, game);
                            if (this == game->getCurrentPlayer()) game->addMessage(this, "Here, take this.");
                            getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_give,
                                    getID(), getSort(),
                                    target_c->getID(), target_c->getSort(),
                                    item_to_give->getID(), item_to_give->getSort(),
                                    getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                            target_c->eventWithObject(A4_EVENT_RECEIVE, this, item_to_give, getMap(), game);
                            eventWithObject(A4_EVENT_ACTION_GIVE, target_c, item_to_give, getMap(), game);
                        }
                    }
                }
            }
            break;

        case A4CHARACTER_COMMAND_SELL:
        {
            A4Object *item_to_give = getInventory(argument);
            if (item_to_give==0) {
                // error!
                output_debug_message("Character %s trying to sell item %i, which it does not have...\n", getName()->get(), argument);
                exit(1);
            } else {
                int x2 = target->getX() + target->getPixelWidth()/2;
                int y2 = target->getY() + target->getPixelHeight()/2;
                int dx = (getX() + getPixelWidth()/2) - x2;
                int dy = (getY() + getPixelHeight()/2) - y2;
                int d = dx*dx + dy*dy;
                int maxd = std::max(game->getTileDx(), game->getTileDy())*5;
                if (d>maxd*maxd) {
                    // too far!
                    output_debug_message("Character %s trying to sell item %i to a character that is too far...\n", getName()->get(), argument);
                    if (this == game->getCurrentPlayer()) game->addMessage(this, "I need to get closer!");
                } else {
                    A4Character *target_c = (A4Character *)target;
                    if (target_c->getInventory()->size()>=A4_INVENTORY_SIZE) {
                        if (this == game->getCurrentPlayer()) game->addMessage(this, "The other's inventory is full!");
                    } else {
                        if (target_c->getGold()<item_to_give->getGold()) {
                            if (this == game->getCurrentPlayer()) game->addMessage(this, "not enough gold!");
                        } else {
                            // sell!
                            m_inventory.remove(item_to_give);
                            target_c->addObjectToInventory(item_to_give, game);
                            if (this == game->getCurrentPlayer()) game->addMessage(this, "Thanks!");
                            target_c->setGold(target_c->getGold()-item_to_give->getGold());
                            setGold(getGold()+item_to_give->getGold());
                            getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_buy,
                                                                                           target_c->getID(), target_c->getSort(),
                                                                                           getID(), getSort(),
                                                                                           item_to_give->getID(), item_to_give->getSort(),
                                                                                           getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                            target_c->eventWithObject(A4_EVENT_RECEIVE, this, item_to_give, getMap(), game);
                            eventWithObject(A4_EVENT_ACTION_SELL, target_c, item_to_give, getMap(), game);
                        }
                    }
                }
            }
        }
            break;

        case A4CHARACTER_COMMAND_BUY:
        {
            A4Character *target_c = (A4Character *)target;
            A4Object *item_to_give = target_c->getInventory(argument);
            if (item_to_give==0) {
                // error!
                output_debug_message("Character %s trying to buy item %i from %s, which it does not have...\n", getName()->get(), argument, target_c->getName()->get());
                exit(1);
            } else {
                int x2 = target->getX() + target->getPixelWidth()/2;
                int y2 = target->getY() + target->getPixelHeight()/2;
                int dx = (getX() + getPixelWidth()/2) - x2;
                int dy = (getY() + getPixelHeight()/2) - y2;
                int d = dx*dx + dy*dy;
                int maxd = std::max(game->getTileDx(), game->getTileDy())*5;
                if (d>maxd*maxd) {
                    // too far!
                    output_debug_message("Character %s trying to buy item %i from a character that is too far...\n", getName()->get(), argument);
                    if (this == game->getCurrentPlayer()) game->addMessage(this, "I need to get closer!");
                } else {
                    if (getInventory()->size()>=A4_INVENTORY_SIZE) {
                        if (this == game->getCurrentPlayer()) game->addMessage(this, "My inventory is full!");
                    } else {
                        if (getGold()<item_to_give->getGold()) {
                            if (this == game->getCurrentPlayer()) game->addMessage(this, "not enough gold!");
                        } else {
                            // buy!
                            target_c->m_inventory.remove(item_to_give);
                            addObjectToInventory(item_to_give, game);
                            if (this == game->getCurrentPlayer()) game->addMessage(this, "Thanks!");
                            setGold(getGold()-item_to_give->getGold());
                            target_c->setGold(target_c->getGold()+item_to_give->getGold());
                            getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_buy,
                                                                                           getID(), getSort(),
                                                                                           target_c->getID(), target_c->getSort(),
                                                                                           item_to_give->getID(), item_to_give->getSort(),
                                                                                           getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                            eventWithObject(A4_EVENT_RECEIVE, target_c, item_to_give, getMap(), game);
                            eventWithObject(A4_EVENT_ACTION_BUY, target_c, item_to_give, getMap(), game);
                        }
                    }
                }
            }
        }
            break;
	}
}


void A4Character::issueCommand(int command, class SpeechAct *argument, int direction, A4Character *target, class A4Game *game)
{
    if (m_talking_state!=A4CHARACTER_STATE_IDLE) return;
    switch(command) {
        case A4CHARACTER_COMMAND_TALK:
        case A4CHARACTER_COMMAND_TALK_ANGRY:
        {
            if (target==0 && argument->performative!=A4_TALK_PERFORMATIVE_NONE) {
                if (direction==A4_DIRECTION_NONE) {
                    std::vector<A4Object *> *candidates = m_map->getAllObjects(m_x - 5*m_map->getTileWidth(), m_y - 5*m_map->getTileWidth(),
                                                                             getPixelWidth() + 10*m_map->getTileWidth(),
                                                                             getPixelHeight() + 10*m_map->getTileHeight());
                    A4Object *closest = 0;
                    int closest_d = 0;
                    for(A4Object *o:*candidates) {
                        if (o!=this && o->isCharacter()) {
                            int d = o->pixelDistance(this);
                            if (closest==0 || d<closest_d) {
                                closest = o;
                                closest_d = d;
                            }
                        }
                    }
                    if (closest!=0) target = (A4Character *)closest;
                    delete candidates;
                } else {
                    std::vector<A4Object *> *collisions = m_map->getAllObjectCollisions(this, A4Game::direction_x_inc[direction], A4Game::direction_y_inc[direction]);
                    for(A4Object *o:*collisions) {
                        if (o!=this && o->isCharacter()) {
                            target = (A4Character *)o;
                            break;
                        }
                    }
                    delete collisions;
                }
            }
            if (target!=0 && argument->performative==A4_TALK_PERFORMATIVE_NONE) {
                target = 0;
            }

            m_talkingSpeechAct = new SpeechAct(argument);
            m_talkingBubble = new A4TextBubble(m_talkingSpeechAct->text, 24, game->getFont());
            m_talkingTarget = target;
            if (command == A4CHARACTER_COMMAND_TALK) {
                m_talking_state = A4CHARACTER_STATE_TALKING;
            } else {
                m_talking_state = A4CHARACTER_STATE_TALKING_ANGRY;
            }
            m_talking_state_cycle = 0;
            // only when the player is around, we see the message printed in the console:
            if (getMap()==game->getCurrentPlayer()->getMap()) game->addMessage(this, "%s: %s",getName()->get(),m_talkingSpeechAct->text);
            if (target==0) {
                if (m_talkingSpeechAct->keyword==0) {
                    getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_talk_performatives[m_talkingSpeechAct->performative], getID(), getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                } else {
                    getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_talk_performatives[m_talkingSpeechAct->performative], getID(), getSort(), new Symbol(m_talkingSpeechAct->keyword), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                }
            } else {
                if (m_talkingSpeechAct->keyword==0) {
                    getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_talk_performatives[m_talkingSpeechAct->performative], getID(), getSort(), target->getID(), target->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                } else {
                    getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_talk_performatives[m_talkingSpeechAct->performative], getID(), getSort(), new Symbol(m_talkingSpeechAct->keyword), target->getID(), target->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                }
            }
            eventWithSpeechAct(A4_EVENT_ACTION_TALK, m_talkingSpeechAct,
                               command==A4CHARACTER_COMMAND_TALK_ANGRY, target, m_map, game);
        }
        break;
    }
}


bool A4Character::take(A4Game *game) {
    A4Object *item = m_map->getTakeableObject(m_x + getPixelWidth()/2 - 1, m_y + getPixelHeight()/2 - 1, 2, 2);
    if (item!=0) {
        if (m_inventory.size()<A4_INVENTORY_SIZE) {
            game->requestWarp(item, 0, 0, 0, 0);
            addObjectToInventory(item, game);
            getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_take, getID(), getSort(), item->getID(), item->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
            item->event(A4_EVENT_PICKUP, this, m_map, game);
            eventWithObject(A4_EVENT_ACTION_TAKE, 0, item, getMap(), game);
            return true;
        } else {
            if (this == game->getCurrentPlayer()) game->addMessage(this, "Inventory full!");
            return false;
        }
    }
    return false;
}


bool A4Character::use(A4Game *game) {
    A4Object *object = m_map->getUsableObject(m_x + getPixelWidth()/2 - 1, m_y + getPixelHeight()/2 - 1, 2, 2);
    if (object!=0) {
        m_state = A4CHARACTER_STATE_INTERACTING;
        getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_interact, getID(), getSort(), object->getID(), object->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
        object->event(A4_EVENT_USE, this, m_map, game);
        eventWithObject(A4_EVENT_ACTION_USE, 0, object, getMap(), game);
        return true;
    }
    return false;
}


bool A4Character::attack(A4Character *other)
{
    // do not attack if it's already dead:
    if (other->getHp()>0) {
        int attack = getAttack(true);
        int defense = other->getDefense(true);

        double r = (double)rand() / RAND_MAX;
        int rawAttack = int(attack/2 + attack * r);
        if (rawAttack<=0) rawAttack = 0;
        double defenseModifier = pow(0.97,defense);
        int damage = int(rawAttack*defenseModifier);
        if (other->takeDamage(damage)) gainXp(other->getGivesExperience());
        return damage>0;
    }
    return false;
}


bool A4Character::takeDamage(int damage) {
    if (isInVehicle()) {
        m_vehicle->takeDamage(damage);
        return false;
    } else {
        m_map->addDamageVisualization(new DamageVisualization(damage, m_x + getPixelWidth()/2, m_y, 50));
        m_hp -= damage;
        if (m_hp<=0) return true;
        return false;
    }
}



void A4Character::receiveSpeechAct(A4Character *speaker, A4Character *receiver, SpeechAct *sa) {
    // nothing (for now)
    // ...
}



void A4Character::draw(int offsetx, int offsety, float zoom, A4Game *game)
{
    if (!isInVehicle()) {
        A4WalkingObject::draw(offsetx, offsety, zoom, game);
    }
}


void A4Character::drawSpeechBubbles(int offsetx, int offsety, float zoom, A4Game *game)
{
    if (m_talkingBubble!=0) {
        int px = (m_x + offsetx + getPixelWidth()/2)*zoom;
        int bx = px - m_talkingBubble->getWidth()/2;
        if (bx<0) bx = 0;
        if (bx+m_talkingBubble->getWidth()>=SCREEN_X) bx = SCREEN_X - m_talkingBubble->getWidth();
        int py = (m_y + offsety)*zoom;
        int by = py - (32 + m_talkingBubble->getHeight());
        if (by<0 || py<SCREEN_Y/3) {
            py = (m_y + offsety + getPixelHeight())*zoom;
            by = py + 32;
        }
        double f = 1;
        int fade_speed = 15;
        if (m_talking_state_cycle<fade_speed) f = m_talking_state_cycle/float(fade_speed);
        int limit = int(strlen(m_talkingSpeechAct->text)*TEXT_SPEED);
        if (m_talking_state_cycle>limit-fade_speed) f = (limit - m_talking_state_cycle)/float(fade_speed);
        if (f<0) f = 0;
        if (f>1) f = 1;
        m_talkingBubble->draw(bx,by,px,py,
                              m_talking_state==A4CHARACTER_STATE_TALKING_ANGRY,f);
    }
}


bool A4Character::cycle(A4Game *game)
{
    bool ret = A4WalkingObject::cycle(game);

    int movement_slack = getPixelWidth()*A4_MOVEMENT_SLACK;
    int v_movement_slack = getPixelHeight()*A4_MOVEMENT_SLACK;
    if (v_movement_slack>movement_slack) movement_slack = v_movement_slack;
    int max_movement_pixels_requested = 0;

    if (m_hp<=0) {
        getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_die, getID(), getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
        m_state = A4CHARACTER_STATE_DYING;
    }

    // direction control:
    for(int i = 0;i<A4_NDIRECTIONS;i++) {
        if (m_direction_command_received_this_cycle[i]) {
            m_continuous_direction_command_timers[i]++;
        } else {
            m_continuous_direction_command_timers[i] = 0;
        }
    }
    if (m_state == A4CHARACTER_STATE_IDLE) {
        int most_recent_viable_walk_command = A4_DIRECTION_NONE;
        int timer = 0;
        for(int i = 0;i<A4_NDIRECTIONS;i++) {
            if (m_direction_command_received_this_cycle[i] && canMove(i, movement_slack)!=INT_MAX) {
                if (most_recent_viable_walk_command==A4_DIRECTION_NONE ||
                    m_continuous_direction_command_timers[i]<timer) {
                    most_recent_viable_walk_command = i;
                    timer = m_continuous_direction_command_timers[i];
                }
            }
        }
        if (most_recent_viable_walk_command!=A4_DIRECTION_NONE) {
            m_state = A4CHARACTER_STATE_WALKING;
            m_direction = most_recent_viable_walk_command;
            max_movement_pixels_requested = m_continuous_direction_command_max_movement[most_recent_viable_walk_command];
        }
    }
    for(int i = 0;i<A4_NDIRECTIONS;i++) m_direction_command_received_this_cycle[i] = false;

	if (m_state!=m_previous_state || m_direction!=m_previous_direction) m_state_cycle = 0;
	m_previous_state = m_state;
	m_previous_direction = m_direction;
	switch(m_state) {
		case A4CHARACTER_STATE_IDLE:
			if (m_state_cycle==0) {
				if (m_animations[A4_ANIMATION_IDLE_LEFT+m_direction]!=0) {
					m_currentAnimation = A4_ANIMATION_IDLE_LEFT+m_direction;
				} else {
					m_currentAnimation = A4_ANIMATION_IDLE;
				}
				m_animations[m_currentAnimation]->reset();
			} else {
//				m_animations[m_currentAnimation]->cycle();
			}
            m_state_cycle++;
			break;
		case A4CHARACTER_STATE_WALKING:
            {
                if (m_state_cycle==0) {
                    if (m_animations[A4_ANIMATION_MOVING_LEFT+m_direction]!=0) {
                        m_currentAnimation = A4_ANIMATION_MOVING_LEFT+m_direction;
                    } else if (m_animations[A4_ANIMATION_MOVING]!=0) {
                        m_currentAnimation = A4_ANIMATION_MOVING;
                    } else if (m_animations[A4_ANIMATION_IDLE_LEFT+m_direction]!=0) {
                        m_currentAnimation = A4_ANIMATION_IDLE_LEFT+m_direction;
                    } else {
                        m_currentAnimation = A4_ANIMATION_IDLE;
                    }
                    m_animations[m_currentAnimation]->reset();
                } else {
//                    m_animations[m_currentAnimation]->cycle();
                }
                m_state_cycle++;

                // the following kind of messy code just makes characters walk at the proper speed
                // it follows the idea of Brsenham's algorithms for proportionally scaling the speed of
                // the characters without using any floating point calculations.
                // it also makes the character move sideways a bit, if they need to align to fit through a corridor
                int step = game->getTileDx();
                if (m_direction==A4_DIRECTION_UP || m_direction==A4_DIRECTION_DOWN) step = game->getTileDy();
                A4MapBridge *bridge = 0;
                int pixelsMoved = 0;
                int old_x = m_x;
                int old_y = m_y;
                while(m_walking_counter<=step) {
                    int dir = m_direction;
                    if (!WALK_TILE_BY_TILE) {
                        int tmp = canMove(m_direction, movement_slack);
                        if (tmp==INT_MAX) {
                            // we cannot move!
                            break;
                        } else {
                            if (tmp!=0) {
                                if ((m_direction==A4_DIRECTION_LEFT || m_direction==A4_DIRECTION_RIGHT) && tmp<0) dir = A4_DIRECTION_UP;
                                if ((m_direction==A4_DIRECTION_LEFT || m_direction==A4_DIRECTION_RIGHT) && tmp>0) dir = A4_DIRECTION_DOWN;
                                if ((m_direction==A4_DIRECTION_UP || m_direction==A4_DIRECTION_DOWN) && tmp<0) dir = A4_DIRECTION_LEFT;
                                if ((m_direction==A4_DIRECTION_UP || m_direction==A4_DIRECTION_DOWN) && tmp>0) dir = A4_DIRECTION_RIGHT;
                            }
                        }
                    }
                    m_x += A4Game::direction_x_inc[dir];
                    m_y += A4Game::direction_y_inc[dir];
                    m_walking_counter += getWalkSpeed();
                    pixelsMoved++;
                    if (!WALK_TILE_BY_TILE ||
                        ((m_x%game->getTileDx())==0 && (m_y%game->getTileDy())==0)) {
                        m_state = A4CHARACTER_STATE_IDLE;
                        if (WALK_TILE_BY_TILE) m_walking_counter = 0;
                        bridge = m_map->getBridge(m_x+getPixelWidth()/2,m_y+getPixelHeight()/2);
                        if (bridge!=0) {
                            // if we enter a bridge, but it's not with the first pixel we moved, then stop and do not go through the bridfge,
                            // to give the AI a chance to decide whether to go through the bridge or not
                            if (pixelsMoved>1) {
                                m_x -= A4Game::direction_x_inc[dir];
                                m_y -= A4Game::direction_y_inc[dir];
                                bridge = 0;
                            }
                            break;
                          }
                    }

                    // walk in blocks of a tile wide:
                    if (A4Game::direction_x_inc[dir]!=0 && (m_x%game->getTileDx())==0) {
                        m_walking_counter = 0;
                        break;
                    }
                    if (A4Game::direction_y_inc[dir]!=0 && (m_y%game->getTileDy())==0) {
                        m_walking_counter = 0;
                        break;
                    }
                    if (max_movement_pixels_requested>0) {
                        max_movement_pixels_requested--;
                        if (max_movement_pixels_requested<=0) break;
                    }
//                    if (WALK_TILE_BY_TILE) if ((m_x%game->getTileDx())==0 && (m_y%game->getTileDy())==0) break;
                }
                if (m_walking_counter>=step) m_walking_counter-=step;
                if (bridge!=0) {
//                        output_debug_message("Stepped over a bridge!\n");
                    // teleport!
                    int targetx,targety;
                    if (bridge->m_linkedTo->findAvailableTargetLocation(this, game->getTileDx(), game->getTileDy(), targetx, targety)) {
                        game->requestWarp(this,bridge->m_linkedTo->getMap(), targetx, targety, m_layer);
                    } else {
//						output_debug_message("Cannot find available target location to go through bridge!\n");
                        m_x = old_x;
                        m_y = old_y;
                        if (this == game->getCurrentPlayer()) game->addMessage("Something is blocking the way!");
                    }
                }
                break;
            }
        case A4CHARACTER_STATE_INTERACTING:
            if (m_state_cycle==0) {
                if (m_animations[A4_ANIMATION_INTERACTING_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_INTERACTING_LEFT+m_direction;
                } else if (m_animations[A4_ANIMATION_INTERACTING]!=0) {
                    m_currentAnimation = A4_ANIMATION_INTERACTING;
                } else if (m_animations[A4_ANIMATION_IDLE_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_IDLE_LEFT+m_direction;
                } else {
                    m_currentAnimation = A4_ANIMATION_IDLE;
                }
                m_animations[m_currentAnimation]->reset();
            } else {
//                m_animations[m_currentAnimation]->cycle();
            }
            m_state_cycle++;
            if (m_state_cycle>=getWalkSpeed()) {
                m_state = A4CHARACTER_STATE_IDLE;
            }
            break;
        case A4CHARACTER_STATE_ATTACKING:
            if (m_state_cycle==0) {
                if (m_animations[A4_ANIMATION_ATTACKING_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_ATTACKING_LEFT+m_direction;
                } else if (m_animations[A4_ANIMATION_ATTACKING]!=0) {
                    m_currentAnimation = A4_ANIMATION_ATTACKING;
                } else if (m_animations[A4_ANIMATION_IDLE_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_IDLE_LEFT+m_direction;
                } else {
                    m_currentAnimation = A4_ANIMATION_IDLE;
                }
                m_animations[m_currentAnimation]->reset();
            } else {
//                m_animations[m_currentAnimation]->cycle();
            }
            m_state_cycle++;
            if (m_state_cycle>=getWalkSpeed()*2) {
                m_state = A4CHARACTER_STATE_IDLE;
            }
            break;
        case A4CHARACTER_STATE_CASTING:
            if (m_state_cycle==0) {
                if (m_animations[A4_ANIMATION_CASTING_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_CASTING_LEFT+m_direction;
                } else if (m_animations[A4_ANIMATION_CASTING]!=0) {
                    m_currentAnimation = A4_ANIMATION_CASTING;
                } else if (m_animations[A4_ANIMATION_IDLE_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_IDLE_LEFT+m_direction;
                } else {
                    m_currentAnimation = A4_ANIMATION_IDLE;
                }
                m_animations[m_currentAnimation]->reset();
            } else {
//                m_animations[m_currentAnimation]->cycle();
            }
            m_state_cycle++;
            if (m_state_cycle>=getWalkSpeed()*2) {
                m_state = A4CHARACTER_STATE_IDLE;
            }
            break;
        case A4CHARACTER_STATE_DYING:
            if (m_state_cycle==0) {
                if (m_animations[A4_ANIMATION_DEATH_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_DEATH_LEFT+m_direction;
                } else if (m_animations[A4_ANIMATION_DEATH]!=0) {
                    m_currentAnimation = A4_ANIMATION_DEATH;
                } else if (m_animations[A4_ANIMATION_IDLE_LEFT+m_direction]!=0) {
                    m_currentAnimation = A4_ANIMATION_IDLE_LEFT+m_direction;
                } else {
                    m_currentAnimation = A4_ANIMATION_IDLE;
                }
                m_animations[m_currentAnimation]->reset();
            } else {
//                m_animations[m_currentAnimation]->cycle();
            }
            m_state_cycle++;
            if (m_state_cycle>=getWalkSpeed()) {
                if (getGold()>0) {
                    game->requestWarp(new A4CoinPurse(game->getOntology()->getSort("CoinPurse"), getGold(), new Animation(game->getCoinPurseAnimation()), true),
                                      m_map, m_x, m_y, A4_LAYER_FG);
                }
                // drop all the items:
                for(A4Object *o:m_inventory) {
                    game->requestWarp(o, m_map, m_x, m_y, A4_LAYER_FG);
                    o->event(A4_EVENT_DROP, 0, m_map, game);    // pass '0' as the character, since this character is dead
                }
                for(int i = 0;i<A4_EQUIPABLE_N_SLOTS;i++) {
                    if (m_equipment[i]!=0) {
                        game->requestWarp(m_equipment[i], m_map, m_x, m_y, A4_LAYER_FG);
                        m_equipment[i]->event(A4_EVENT_DROP, 0, m_map, game);    // pass '0' as the character, since this character is dead
                        m_equipment[i] = 0;
                    }
                }
                m_inventory.clear();
                return false;
            }
            break;
        case A4CHARACTER_STATE_IN_VEHICLE:
            if (m_map!=m_vehicle->getMap()) {
                game->requestWarp(this, m_vehicle->getMap(), m_vehicle->getX(), m_vehicle->getY(), m_layer);
            } else {
                m_x = m_vehicle->getX();
                m_y = m_vehicle->getY();
            }
            break;
    }
    switch(m_talking_state) {
        case A4CHARACTER_STATE_IDLE:
            break;
        case A4CHARACTER_STATE_TALKING:
            m_talking_state_cycle++;
            if (m_talking_state_cycle>=(int)strlen(m_talkingSpeechAct->text)*TEXT_SPEED) {
                if (m_talkingTarget!=0 && game->contains(m_talkingTarget)) {
                    m_talkingTarget->receiveSpeechAct(this, m_talkingTarget, m_talkingSpeechAct);
                    this->receiveSpeechAct(this, m_talkingTarget, m_talkingSpeechAct);
                }
                delete m_talkingSpeechAct;
                m_talkingSpeechAct = 0;
                delete m_talkingBubble;
                m_talkingBubble = 0;
                m_talking_state = A4CHARACTER_STATE_IDLE;
                m_talkingTarget = 0;
            }
            break;
        case A4CHARACTER_STATE_TALKING_ANGRY:
            m_talking_state_cycle++;
            if (m_talking_state_cycle>=(int)strlen(m_talkingSpeechAct->text)*5) {
                if (m_talkingTarget!=0 && game->contains(m_talkingTarget)) {
                    m_talkingTarget->receiveSpeechAct(this, m_talkingTarget, m_talkingSpeechAct);
                    this->receiveSpeechAct(this, m_talkingTarget, m_talkingSpeechAct);
                }
                delete m_talkingSpeechAct;
                m_talkingSpeechAct = 0;
                delete m_talkingBubble;
                m_talkingBubble = 0;
                m_talking_state = A4CHARACTER_STATE_IDLE;
                m_talkingTarget = 0;
            }
            break;
	}

    for(int i = 0;i<A4_N_SPELLS;i++) {
        if (m_spell_effect_counter[i]>0) {
            m_spell_effect_counter[i]--;
//            if (i==A4_SPELL_MAGIC_EYE && m_spell_effect_counter[i]<=0) {
//                m_map->reevaluateVisibility(getX()/game->getTileDx(),
//                                            getY()/game->getTileDy(),
//                                            this);
//            }
        }
    }

	return ret;
}


void A4Character::gainXp(int xp)
{
    m_xp += xp;
    while(m_xp>=getXpToLevelUp()) {
        m_xp -= getXpToLevelUp();
        levelUp();
    }
}


void A4Character::levelUp() {
    m_level++;
    m_max_hp += m_hp_levelupadd;
    m_hp += m_hp_levelupadd;
    m_max_mp += m_mp_levelupadd;
    m_mp += m_mp_levelupadd;
    m_experienceToLevelUp *= m_experienceToLevelUp_multiplier;
    m_attack += m_attack_levelupadd;
    m_defense += m_defense_levelupadd;
}


void A4Character::embark(A4Vehicle *v)
{
    m_vehicle = v;
    m_vehicle->embark(this);
    m_state = A4CHARACTER_STATE_IN_VEHICLE;
}


void A4Character::disembark()
{
    m_vehicle->disembark(this);
    m_state = A4CHARACTER_STATE_IDLE;

    if (m_vehicle->getHp()==0 && !m_map->walkable(m_x, m_y, getPixelWidth(), getPixelHeight(), this)) {
        // drown!
        m_hp = 0;
    }

    m_vehicle = 0;
}


A4Object *A4Character::getInventory(int n)
{
	int i = 0;
	for(A4Object *o:m_inventory) {
		if (i==n) return o;
		i++;
	}
	return 0;
}




void A4Character::addObjectToInventory(A4Object *o, A4Game *game)
{
	if (m_inventory.size()>=A4_INVENTORY_SIZE) {
        if (m_map!=0) {
            game->requestWarp(o, m_map, m_x, m_y, A4_LAYER_FG);
            o->event(A4_EVENT_DROP, this, m_map, game);
            m_map->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_drop, getID(), getSort(), o->getID(), o->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
        } else {
            output_debug_message("Adding additional an item ('%s') to a character that do not fit in the inventory, and the map of the character is null!!\n",o->getName()->get());
        }
	} else {
		m_inventory.push_back(o);
	}
}


A4Object *A4Character::getEquipment(int i)
{
	if (i>=0 && i<A4_EQUIPABLE_N_SLOTS) return m_equipment[i];
	return 0;
}


void A4Character::setEquipment(int i, A4Object *o) {
    if (i>=0 && i<A4_EQUIPABLE_N_SLOTS) m_equipment[i] = o;
}


int A4Character::getAttack(bool considerSpellEffects)
{
    float a = m_attack;

    if (getEquipment(A4_EQUIPABLE_WEAPON)!=0) {
        a += ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_WEAPON))->getAttack();
    }
    if (getEquipment(A4_EQUIPABLE_OFF_HAND)!=0) {
        a += ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_OFF_HAND))->getAttack();
    }
    if (getEquipment(A4_EQUIPABLE_RING)!=0) {
        a += ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_RING))->getAttack();
    }

    if (considerSpellEffects) {
        if (m_spell_effect_counter[A4_SPELL_INCREASE]>0) a += 4*getMagicBonus();
        if (m_spell_effect_counter[A4_SPELL_DECREASE]>0) a -= 4*getMagicBonus();
    }

    if (a<0) a = 0;

    return int(a * m_attack_modifier);
}


int A4Character::getDefense(bool considerSpellEffects)
{
    float d = m_defense;

    if (getEquipment(A4_EQUIPABLE_WEAPON)!=0) {
        d += ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_WEAPON))->getDefense();
    }
    if (getEquipment(A4_EQUIPABLE_OFF_HAND)!=0) {
        d += ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_OFF_HAND))->getDefense();
    }
    if (getEquipment(A4_EQUIPABLE_RING)!=0) {
        d += ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_RING))->getDefense();
    }

    if (considerSpellEffects) {
        if (m_spell_effect_counter[A4_SPELL_INCREASE]>0) d += 2*getMagicBonus();
        if (m_spell_effect_counter[A4_SPELL_DECREASE]>0) d -= 2*getMagicBonus();
        if (m_spell_effect_counter[A4_SPELL_SHIELD]>0) d += 5*getMagicBonus();
    }

    if (d<0) d = 0;

    return int(d * m_defense_modifier);
}


float A4Character::getMagicBonus()
{
    float b = 1.0f;
    if (getEquipment(A4_EQUIPABLE_WEAPON)!=0) {
        b *= ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_WEAPON))->getMagicBonus();
    }
    if (getEquipment(A4_EQUIPABLE_OFF_HAND)!=0) {
        b *= ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_OFF_HAND))->getMagicBonus();
    }
    if (getEquipment(A4_EQUIPABLE_RING)!=0) {
        b *= ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_RING))->getMagicBonus();
    }
    return b;
}


int A4Character::getWalkSpeed()
{
    int base = A4WalkingObject::getWalkSpeed();
    
    // add bonuses:
    float b = 1.0f;
    if (getEquipment(A4_EQUIPABLE_WEAPON)!=0) {
        b *= ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_WEAPON))->getMovementBonus();
    }
    if (getEquipment(A4_EQUIPABLE_OFF_HAND)!=0) {
        b *= ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_OFF_HAND))->getMovementBonus();
    }
    if (getEquipment(A4_EQUIPABLE_RING)!=0) {
        b *= ((A4EquipableItem *)getEquipment(A4_EQUIPABLE_RING))->getMovementBonus();
    }
    
    return base*b;
}



void A4Character::loadObjectAdditionalContent(XMLNode *xml, A4Game *game, A4ObjectFactory *of, std::vector<std::pair<XMLNode *, A4Object *>> *objectsToRevisit)
{
    A4WalkingObject::loadObjectAdditionalContent(xml,game,of, objectsToRevisit);

    // add spells:
    std::vector<XMLNode *> *spells_xml = xml->get_children("spell");
    for(XMLNode *spell_xml:*spells_xml) {
        for(int i = 0;i<A4_N_SPELLS;i++) {
            if (strcmp(spell_xml->get_value(),A4Game::spellNames[i])==0) {
                ((A4Character *)this)->learnSpell(i);
            }
        }
    }
    delete spells_xml;

    XMLNode *items_xml = xml->get_child("items");
    if (items_xml!=0) {
        for(XMLNode *item_xml:*(items_xml->get_children())) {
            char *tmp = item_xml->get_attribute("probability");
            if (tmp!=0) {
                float p = atof(tmp);
                if (((float)rand() / RAND_MAX)>=p) continue;
            }
            bool completeRedefinition = false;
            tmp = item_xml->get_attribute("completeRedefinition");
            if (tmp!=0 && strcmp(tmp,"true")==0) completeRedefinition = true;
            A4Object *item = of->createObject(item_xml->get_attribute("class"), game, false, completeRedefinition);
            item->loadObjectAdditionalContent(item_xml, game, of, objectsToRevisit);
            addObjectToInventory(item, game);
        }
    }
    std::vector<XMLNode *> *equipment_xml_l = xml->get_children("equipment");
    if (equipment_xml_l!=0) {
        for(XMLNode *equipment_xml:*equipment_xml_l) {
            int slot = atoi(equipment_xml->get_attribute("slot"));
            XMLNode *object_xml = equipment_xml->get_child("object");
            char *tmp = object_xml->get_attribute("probability");
            if (tmp!=0) {
                float p = atof(tmp);
                if (((float)rand() / RAND_MAX)>=p) continue;
            }
            bool completeRedefinition = false;
            tmp = object_xml->get_attribute("completeRedefinition");
            if (tmp!=0 && strcmp(tmp,"true")==0) completeRedefinition = true;
            A4Object *item = of->createObject(object_xml->get_attribute("class"), game, false, completeRedefinition);
            item->loadObjectAdditionalContent(object_xml, game, of, objectsToRevisit);
            if (m_equipment[slot]!=0) {
                delete m_equipment[slot];
                output_debug_message("WARNING: loading equipment to slot %i, which already has an item!",slot);
            }
            m_equipment[slot] = item;
        }
        delete equipment_xml_l;
    }



    // check if the character is in a vehicle:
    std::vector<XMLNode *> *attributes_xml = xml->get_children("attribute");
    for(XMLNode *attribute_xml:*attributes_xml) {
        char *a_name = attribute_xml->get_attribute("name");
        if (strcmp(a_name,"vehicle")==0) {
            XMLNode *copy = new XMLNode(xml);
            objectsToRevisit->push_back(std::pair<XMLNode *, A4Object *>(copy, this));
            break;
        }
    }
    delete attributes_xml;
}


bool A4Character::loadObjectAttribute(XMLNode *attribute_xml)
{
    if (A4WalkingObject::loadObjectAttribute(attribute_xml)) return true;

    char *a_name = attribute_xml->get_attribute("name");

    if (strcmp(a_name,"experienceToLevelUp")==0) {
        if (attribute_xml->get_attribute("levelupmultiply")!=0) {
            setExperienceToLevelUp(atoi(attribute_xml->get_attribute("value")),
                                   atof(attribute_xml->get_attribute("levelupmultiply")));
        } else {
            setExperienceToLevelUp(atoi(attribute_xml->get_attribute("value")),1.0f);
        }
        return true;
    } else if (strcmp(a_name,"gives_experience")==0) {
        setGivesExperience(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"hp")==0) {
        setHp(atoi(attribute_xml->get_attribute("value")));
        setMaxHp(atoi(attribute_xml->get_attribute("value")));
        if (attribute_xml->get_attribute("levelupadd")!=0) {
            setHpLevelupadd(atoi(attribute_xml->get_attribute("levelupadd")));
        } else {
            setHpLevelupadd(0);
        }
        return true;
    } else if (strcmp(a_name,"max_hp")==0) {
        setMaxHp(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"mp")==0) {
        setMp(atoi(attribute_xml->get_attribute("value")));
        setMaxMp(atoi(attribute_xml->get_attribute("value")));
        if (attribute_xml->get_attribute("levelupadd")!=0) {
            setMpLevelupadd(atoi(attribute_xml->get_attribute("levelupadd")));
        } else {
            setMpLevelupadd(0);
        }
        return true;
    } else if (strcmp(a_name,"max_mp")==0) {
        setMaxMp(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"xp")==0) {
        setXp(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"level")==0) {
        setLevel(atoi(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"attack")==0) {
        setAttack(atoi(attribute_xml->get_attribute("value")));
        if (attribute_xml->get_attribute("levelupadd")!=0) {
            setAttackLevelupadd(atoi(attribute_xml->get_attribute("levelupadd")));
        } else {
            setAttackLevelupadd(0);
        }
        return true;
    } else if (strcmp(a_name,"defense")==0) {
        setDefense(atoi(attribute_xml->get_attribute("value")));
        if (attribute_xml->get_attribute("levelupadd")!=0) {
            setDefenseLevelupadd(atoi(attribute_xml->get_attribute("levelupadd")));
        } else {
            setDefenseLevelupadd(0);
        }
        return true;
    } else if (strcmp(a_name,"attack_modifier")==0) {
        setAttackModifier(atof(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"defense_modifier")==0) {
        setDefenseModifier(atof(attribute_xml->get_attribute("value")));
        return true;
    } else if (strcmp(a_name,"magicImmune")==0) {
        if (strcmp(attribute_xml->get_attribute("value"),"true")==0) {
            setMagicImmune(true);
        } else {
            setMagicImmune(false);
        }
        return true;
    } else if (strcmp(a_name,"vehicle")==0) {
        // this is loaded in "revisitObject"
        return true;
    }
    return false;
}


void A4Character::revisitObject(XMLNode *xml, A4Game *game)
{
    A4WalkingObject::revisitObject(xml, game);

    std::vector<XMLNode *> *attributes_xml = xml->get_children("attribute");
    for(XMLNode *attribute_xml:*attributes_xml) {
        char *a_name = attribute_xml->get_attribute("name");
        if (strcmp(a_name,"vehicle")==0) {
            int o_ID = atoi(attribute_xml->get_attribute("value"));
            A4Object *o = game->getObject(o_ID);
            if (o==0) {
                output_debug_message("Revisiting A4Character, and cannot find object with ID %i\n",o_ID);
            } else {
                m_vehicle = (A4Vehicle *)o;
            }
            break;
        }
    }
    delete attributes_xml;
}


void A4Character::savePropertiesToXML(XMLwriter *w, A4Game *game)
{
    A4WalkingObject::savePropertiesToXML(w,game);

    w->openTag("attribute");
    w->setAttribute("name", "hp");
    w->setAttribute("value", m_hp);
    w->setAttribute("levelupadd", m_hp_levelupadd);
    w->closeTag("attribute");
    saveObjectAttributeToXML(w,"max_hp",m_max_hp);

    w->openTag("attribute");
    w->setAttribute("name", "mp");
    w->setAttribute("value", m_mp);
    w->setAttribute("levelupadd", m_mp_levelupadd);
    w->closeTag("attribute");

    saveObjectAttributeToXML(w,"max_mp",m_max_mp);
    saveObjectAttributeToXML(w,"xp",m_xp);
    saveObjectAttributeToXML(w,"level",m_level);
    w->openTag("attribute");
    w->setAttribute("name", "experienceToLevelUp");
    w->setAttribute("value", m_experienceToLevelUp);
    w->setAttribute("levelupmultiply", m_experienceToLevelUp_multiplier);
    w->closeTag("attribute");
    saveObjectAttributeToXML(w,"gives_experience",m_givesExperience);

    w->openTag("attribute");
    w->setAttribute("name", "attack");
    w->setAttribute("value", m_attack);
    w->setAttribute("levelupadd", m_attack_levelupadd);
    w->closeTag("attribute");

    w->openTag("attribute");
    w->setAttribute("name", "defense");
    w->setAttribute("value", m_defense);
    w->setAttribute("levelupadd", m_defense_levelupadd);
    w->closeTag("attribute");

    saveObjectAttributeToXML(w,"attack_modifier",m_attack_modifier);
    saveObjectAttributeToXML(w,"defense_modifier",m_defense_modifier);
    saveObjectAttributeToXML(w,"magicImmune",m_magicImmune);
    if (m_vehicle!=0) saveObjectAttributeToXML(w,"vehicle",m_vehicle->getID());

    for(int i = 0 ;i<A4_N_SPELLS;i++) {
        if (m_spells[i]) {
            w->openTag("spell");
            w->addContent(A4Game::spellNames[i]);
            w->closeTag("spell");
        }
        if (m_spell_effect_counter[i]>0) {
            w->openTag("spellEffect");
            w->setAttribute("spell", A4Game::spellNames[i]);
            w->setAttribute("counter", m_spell_effect_counter[i]);
            w->closeTag("spellEffect");
        }
    }
    if (!m_inventory.empty()) {
        w->openTag("items");
        for(A4Object *o:m_inventory) {
            o->saveToXML(w, game, 0, false);
        }
        w->closeTag("items");
    }
    for(int i = 0 ;i<A4_EQUIPABLE_N_SLOTS;i++) {
        if (m_equipment[i]!=0) {
            w->openTag("equipment");
            w->setAttribute("slot", i);
            m_equipment[i]->saveToXML(w, game, 0, false);
            w->closeTag("equipment");
        }
    }
}


bool A4Character::castSpell(int argument, int direction, A4Game *game, bool usingWand)
{
    if (usingWand || knowsSpell(argument)) {
        if (!usingWand && m_mp<A4Game::spellCost[argument]) {
            if (this == game->getCurrentPlayer()) game->addMessage(this, "I don't have enough magic points!");
            return false;
        } else {
            switch(argument) {
                case A4_SPELL_HEAL:
                case A4_SPELL_REGENERATE:
                case A4_SPELL_SHIELD:
                case A4_SPELL_INCREASE:
                case A4_SPELL_DECREASE:
                case A4_SPELL_MAGIC_EYE:
                {
                    A4Character *target = 0;
                    if (direction==A4_DIRECTION_NONE) target = this;
                    else {
                        std::vector<A4Object *> *collisions = m_map->getAllObjectCollisions(this, A4Game::direction_x_inc[direction], A4Game::direction_y_inc[direction]);
                        for(A4Object *o:*collisions) {
                            if (o->isCharacter()) {
                                target = (A4Character *)o;
                                break;
                            }
                        }
                        delete collisions;
                    }
                    if (target==0) {
                        if (this == game->getCurrentPlayer()) game->addMessage(this, "There is no one there to cast the spell!");
                        return false;
                    } else {
                        if (argument==A4_SPELL_HEAL) {
                            if (target->getHp()<target->getMaxHp()) {
                                target->recoverHp(int(5*getMagicBonus()));
                                if (!usingWand) m_mp-=A4Game::spellCost[argument];
                                m_state = A4CHARACTER_STATE_CASTING;
                                m_state_cycle = 0;
                                game->addMessage(this, "%s casts heal!",getName()->get());
                                getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_cast, getID(), getSort(), 0, game->getOntology()->getSort("SpellHeal"), target->getID(), target->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                                eventWithInteger(A4_EVENT_ACTION_SPELL, argument, this, getMap(), game);
                                return true;
                            } else {
                                if (this == game->getCurrentPlayer()) game->addMessage(this, "It would be a waste!");
                                return false;
                            }
                        } else if (argument==A4_SPELL_REGENERATE) {
                            if (target->getHp()<target->getMaxHp()) {
                                target->recoverHp(int(50*getMagicBonus()));
                                if (!usingWand) m_mp-=A4Game::spellCost[argument];
                                m_state = A4CHARACTER_STATE_CASTING;
                                m_state_cycle = 0;
                                game->addMessage(this, "%s casts regenerate!",getName()->get());
                                getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_cast, getID(), getSort(), 0, game->getOntology()->getSort("SpellRegenerate"), target->getID(), target->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                                eventWithInteger(A4_EVENT_ACTION_SPELL, argument, this, getMap(), game);
                                return true;
                            } else {
                                if (this == game->getCurrentPlayer()) game->addMessage(this, "It would be a waste!");
                                return false;
                            }
                        } else if (argument==A4_SPELL_SHIELD) {
                            target->addSpellEffect(argument, 1500);
                            if (!usingWand) m_mp-=A4Game::spellCost[argument];
                            m_state = A4CHARACTER_STATE_CASTING;
                            m_state_cycle = 0;
                            game->addMessage(this, "%s casts shield!",getName()->get());
                            getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_cast, getID(), getSort(), 0, game->getOntology()->getSort("SpellShield"), target->getID(), target->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                            eventWithInteger(A4_EVENT_ACTION_SPELL, argument, this, getMap(), game);
                            return true;
                        } else if (argument==A4_SPELL_INCREASE) {
                            target->addSpellEffect(argument, 1500);
                            if (!usingWand) m_mp-=A4Game::spellCost[argument];
                            m_state = A4CHARACTER_STATE_CASTING;
                            m_state_cycle = 0;
                            game->addMessage(this, "%s casts increase!",getName()->get());
                            getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_cast, getID(), getSort(), 0, game->getOntology()->getSort("SpellIncrease"), target->getID(), target->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                            eventWithInteger(A4_EVENT_ACTION_SPELL, argument, this, getMap(), game);
                            return true;
                        } else if (argument==A4_SPELL_DECREASE) {
                            target->addSpellEffect(argument, 1500);
                            if (!usingWand) m_mp-=A4Game::spellCost[argument];
                            m_state = A4CHARACTER_STATE_CASTING;
                            m_state_cycle = 0;
                            game->addMessage(this, "%s casts decrease!",getName()->get());
                            getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_cast, getID(), getSort(), 0, game->getOntology()->getSort("SpellDecrease"), target->getID(), target->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                            eventWithInteger(A4_EVENT_ACTION_SPELL, argument, this, getMap(), game);
                            return true;
                        } else if (argument==A4_SPELL_MAGIC_EYE) {
                            target->addSpellEffect(argument, 1500);
                            if (!usingWand) m_mp-=A4Game::spellCost[argument];
                            m_state = A4CHARACTER_STATE_CASTING;
                            m_state_cycle = 0;
                            game->addMessage(this, "%s casts magic eye!",getName()->get());
                            getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_cast, getID(), getSort(), 0, game->getOntology()->getSort("SpellMagicEye"), target->getID(), target->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                            eventWithInteger(A4_EVENT_ACTION_SPELL, argument, this, getMap(), game);
                            return true;
                        }
                    }
                }
                    break;
                case A4_SPELL_MAGIC_MISSILE:
                case A4_SPELL_FIREBALL:
                case A4_SPELL_INCINERATE:
                    if (direction == A4_DIRECTION_NONE) {
                        if (this == game->getCurrentPlayer()) game->addMessage(this, "I cannot cast that on myself!");
                        return false;
                    } else {
                        Animation *a = game->getSpellAnimation(argument, A4_ANIMATION_MOVING_LEFT+direction);
                        if (a==0) a = game->getSpellAnimation(argument, A4_ANIMATION_MOVING);
                        if (a==0) a = game->getSpellAnimation(argument, A4_ANIMATION_IDLE_LEFT+direction);
                        if (a==0) a = game->getSpellAnimation(argument, A4_ANIMATION_IDLE);
                        A4Spell *spell = 0;
                        if (argument == A4_SPELL_MAGIC_MISSILE) {
                            Symbol *name = new Symbol("magic missile");
                            spell = new A4Spell(name,game->getOntology()->getSort("SpellMagicMissile"), direction, 6, getMagicBonus()*4, 0, this, new Animation(a));
                            delete name;
                        } else if (argument == A4_SPELL_FIREBALL) {
                            Symbol *name = new Symbol("fireball");
                            spell = new A4Spell(new Symbol("fireball"),game->getOntology()->getSort("SpellFireball"), direction, 10, getMagicBonus()*12, 1.5f, this, new Animation(a));
                            delete name;
                        } else {
                            Symbol *name = new Symbol("incinerate");
                            spell = new A4Spell(new Symbol("incinerate"),game->getOntology()->getSort("SpellIncinerate"), direction, 24, getMagicBonus()*32, 0, this, new Animation(a));
                            delete name;
                        }
                        game->requestWarp(spell, m_map, m_x + (getPixelWidth()-spell->getPixelWidth())/2,
                                          m_y + (getPixelHeight()-spell->getPixelHeight())/2, A4_LAYER_FG);
                        if (!usingWand) m_mp-=A4Game::spellCost[argument];
                        m_state = A4CHARACTER_STATE_CASTING;
                        m_state_cycle = 0;
                        game->addMessage(this, "%s casts %s!",getName()->get(), A4Game::spellNames[argument]);
                        getMap()->addPerceptionBufferRecord(new PerceptionBufferRecord(A4AI::s_action_cast, getID(), getSort(), spell->getID(), spell->getSort(), getX(), getY(), getX()+getPixelWidth(), getY()+getPixelHeight()));
                        eventWithInteger(A4_EVENT_ACTION_SPELL, argument, this, getMap(), game);
                        return true;
                    }
                    break;
            }
            return false;
        }
    } else {
        if (this == game->getCurrentPlayer()) game->addMessage(this, "I do not know that spell!");
        return false;
    }
}
