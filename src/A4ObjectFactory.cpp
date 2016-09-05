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
#include "ExpressionParser.h"
#include "Animation.h"

#include "A4auxiliar.h"
#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4ObjectFactory.h"
#include "A4Game.h"

#include "A4Object.h"
#include "A4Item.h"
#include "A4Container.h"
#include "A4CoinPurse.h"
#include "A4EquipableItem.h"
#include "A4Food.h"
#include "A4Key.h"
#include "A4Scroll.h"
#include "A4Wand.h"
#include "A4Spade.h"
#include "A4Potion.h"
#include "A4Spell.h"
#include "A4Door.h"
#include "A4Lever.h"
#include "A4PressurePlate.h"
#include "A4PushableWall.h"
#include "A4Character.h"
#include "A4PlayerCharacter.h"
#include "A4AICharacter.h"
#include "A4Vehicle.h"
#include "A4Trigger.h"
#include "A4Spell.h"

#include "Ontology.h"
#include "InferenceRule.h"


A4ObjectFactory::A4ObjectFactory()
{
}


A4ObjectFactory::~A4ObjectFactory()
{
    for(XMLNode *xml:objectTypes) delete xml;
	objectTypes.clear();
}


void A4ObjectFactory::addObjectDefinitions(XMLNode *xml, A4Game *game)
{
    Ontology *o = game->getOntology();
	std::vector<XMLNode *> *classes_xml = xml->get_children("ObjectClass");
	for(XMLNode *class_xml:*classes_xml) {
		objectTypes.push_back(new XMLNode(class_xml));

        // create sorts:
        Sort *s = o->getSort(class_xml->get_attribute("class"));
        if (s==0) s = o->newSort(new Symbol(class_xml->get_attribute("class")));
        char *superString = class_xml->get_attribute("super");
        std::vector<char *> superClasses = splitStringBy(superString,',');
        for(char *className:superClasses) {
            if (className[0]=='*') {
                Sort *s2 = o->getSort(className+1);
                if (s2==0) s2 = o->newSort(new Symbol(className+1));
                s->addParent(s2);
            } else {
                Sort *s2 = o->getSort(className);
                if (s2==0) s2 = o->newSort(new Symbol(className));
                s->addParent(s2);
            }
            delete className;
        }

	}
    delete classes_xml;
}


void A4ObjectFactory::addCharacterDefinitions(XMLNode *xml, A4Game *game)
{
    Ontology *o = game->getOntology();
	std::vector<XMLNode *> *classes_xml = xml->get_children("CharacterClass");
	for(XMLNode *class_xml:*classes_xml) {
		objectTypes.push_back(new XMLNode(class_xml));

        // create sorts:
        Sort *s = o->sortExistsP(class_xml->get_attribute("class"));
        if (s==0) s = o->newSort(new Symbol(class_xml->get_attribute("class")));
        char *superString = class_xml->get_attribute("super");
        std::vector<char *> superClasses = splitStringBy(superString,',');
        for(char *className:superClasses) {
            if (className[0]=='*') {
                Sort *s2 = o->getSort(className+1);
                if (s2==0) s2 = o->newSort(new Symbol(className+1));
                s->addParent(s2);
            } else {
                Sort *s2 = o->sortExistsP(className);
                if (s2==0) s2 = o->newSort(new Symbol(className));
                s->addParent(s2);
            }
            delete className;
        }
	}
    delete classes_xml;
}


A4Object *A4ObjectFactory::createObject(char *className, A4Game *game, bool isPlayer, bool completeRedefinition)
{
    XMLNode *xml = getObjectType(className);
    if (xml!=0) return createObjectFromXML(xml, game, isPlayer, completeRedefinition);
    if (strcmp(className,"CoinPurse")==0) {
        A4Object *o = new A4CoinPurse(game->getOntology()->getSort("CoinPurse"), 0, 0, !completeRedefinition);
        return o;
    } else if (strcmp(className,"Door")==0) {
        A4Door *o = new A4Door(game->getOntology()->getSort("Door"), 0, false, true, 0, 0);
        return o;
    } else if (strcmp(className,"Lever")==0) {
        A4Lever *o = new A4Lever(game->getOntology()->getSort("Lever"), 0, false, 0, 0);
        return o;
    } else if (strcmp(className,"PressurePlate")==0) {
        A4PressurePlate *o = new A4PressurePlate(game->getOntology()->getSort("PressurePlate"), 0, 0, 0);
        return o;
    } else if (strcmp(className,"Scroll")==0) {
        A4Scroll *o = new A4Scroll(0, game->getOntology()->getSort("Scroll"), 0, 0, 0);
        return o;
    } else if (strcmp(className,"Wand")==0) {
        A4Wand *o = new A4Wand(0, game->getOntology()->getSort("Wand"), 0, 1, false, 0, 0);
        return o;
    } else if (strcmp(className,"Container")==0) {
        A4Container *o = new A4Container(0, game->getOntology()->getSort("Container"), 0);
        return o;
    } else if (strcmp(className,"ItemHPotion")==0 ||
               strcmp(className,"ItemHPPotion")==0 ||
               strcmp(className,"HPPotion")==0) {
        A4Potion *o = new A4HPPotion(0, game->getOntology()->getSort("HPPotion"), 0, 0);
        return o;
    } else if (strcmp(className,"ItemMPotion")==0 ||
               strcmp(className,"ItemMPPotion")==0 ||
               strcmp(className,"MPPotion")==0) {
        A4Potion *o = new A4MPPotion(0, game->getOntology()->getSort("MPPotion"), 0, 0);
        return o;
    } else if (strcmp(className,"ItemXPotion")==0 ||
               strcmp(className,"ItemXPPotion")==0 ||
               strcmp(className,"XPPotion")==0) {
        A4Potion *o = new A4XPPotion(0, game->getOntology()->getSort("XPPotion"), 0, 0);
        return o;
    } else if (strcmp(className,"StrengthPotion")==0) {
        A4Potion *o = new A4StrengthPotion(0, game->getOntology()->getSort("StrengthPotion"), 0, 0);
        return o;
    } else if (strcmp(className,"ConstitutionPotion")==0) {
        A4Potion *o = new A4ConstitutionPotion(0, game->getOntology()->getSort("ConstitutionPotion"), 0, 0);
        return o;
    } else if (strcmp(className,"LifePotion")==0) {
        A4Potion *o = new A4LifePotion(0, game->getOntology()->getSort("LifePotion"), 0, 0);
        return o;
    } else if (strcmp(className,"PowerPotion")==0) {
        A4Potion *o = new A4PowerPotion(0, game->getOntology()->getSort("PowerPotion"), 0, 0);
        return o;
    } else if (strcmp(className,"EquipableItem")==0) {
        A4EquipableItem *o = new A4EquipableItem(0, game->getOntology()->getSort("EquipableItem"), 0, 0, 0, 0, 1.0f, 1.0f, false, 0);
        return o;
    } else if (strcmp(className,"Key")==0) {
        A4Key *o = new A4Key(0, game->getOntology()->getSort("Key"), 0, 0);
        return o;
    } else if (strcmp(className,"PushableWall")==0) {
        A4PushableWall *o = new A4PushableWall(game->getOntology()->getSort("PushableWall"), 0);
        return o;
    } else if (strcmp(className,"Food")==0) {
        A4Food *o = new A4Food(0, game->getOntology()->getSort("Food"), 0, 0);
        return o;
    } else if (strcmp(className,"Spade")==0) {
        A4Spade *o = new A4Spade(game->getOntology()->getSort("Spade"), 0, 0);
        return o;
    } else if (strcmp(className,"Trigger")==0) {
        A4Trigger *o = new A4Trigger(game->getOntology()->getSort("Trigger"),0,0);
        return o;
    } else if (strcmp(className,"SpellMagicMissile")==0 ||
               strcmp(className,"SpellFireball")==0 ||
               strcmp(className,"SpellIncinerate")==0) {
        A4Spell *o = new A4Spell(0,game->getOntology()->getSort("SpellMagicMissile"),0,0,0,0,0,0);
        // Symbol *name, Sort *sort, int direction, int duration, int damag, float radius, A4Object *caster, Animation *a
        return o;
    }
	return 0;
}


XMLNode *A4ObjectFactory::getObjectType(char *className) {
    for(XMLNode *xml:objectTypes) {
        if (strcmp(xml->get_attribute("class"),className)==0) return xml;
    }
    return 0;
}


A4Object *A4ObjectFactory::createObjectFromXML(XMLNode *xml, A4Game *game, bool isPlayer, bool completeRedefinition)
{
    const char *baseClasses[] = {"Item","Container","Vehicle","Character"};
	A4Object *o;
    Ontology *ontology = game->getOntology();

    char *tmp = xml->get_attribute("completeRedefinition");
    if (tmp!=0 && strcmp(tmp,"true")==0) completeRedefinition = true;
    
    // find base class, and additional definitions to add:
    std::list<XMLNode *> classes;
    Symbol *baseClassName = 0;
    {
        std::list<Symbol *> open;
        open.push_back(new Symbol(xml->get_attribute("class")));
        while(open.size()>0) {
            Symbol *current = open.front();
            open.remove(current);
            char *tmp = current->get();
            assert(tmp!=0);
            bool loadContent = true;
            if (tmp[0]=='*') loadContent = false;
            char *tmp2 = tmp[0]=='*' ? tmp+1:tmp;
            XMLNode *xml = getObjectType(tmp2);
            
            bool found = false;
            for(int i = 0;i<4;i++) {
                if (strcmp(baseClasses[i],tmp2)==0) found = true;
            }
            
            if (xml==0 && found) {
                if (baseClassName==0) {
                    baseClassName = current;
                } else if (baseClassName->cmp(current)) {
                    delete current;
                } else {
                    output_debug_message("A4ObjectFactory::createObject: baseClassName was '%s' and now it's attempted to set to '%s'!!!\n",baseClassName->get(), current->get());
                    exit(1);
                }
            } else {
                delete current;
                if (xml!=0) {
                    if (loadContent) classes.push_front(xml);
                    char *superString = xml->get_attribute("super");
                    //output_debug_message("splitting '%s'\n", superString);
                    std::vector<char *> superClasses = splitStringBy(superString,',');
                    for(char *className:superClasses) {
                        assert(className!=0);
                        //output_debug_message("  '%s'\n", className);
                        open.push_back(new Symbol(className));
                        delete className;
                    }
                }
            }
        }
    }

    if (baseClassName==0) {
        output_debug_message("A4ObjectFactory::createObject: baseClassName null to create '%s'!!!\n",xml->get_attribute("class"));
        exit(1);
    }

    char *name_tmp = xml->get_attribute("name");
    if (name_tmp==0) {
        for(XMLNode *xml2:classes) {
            name_tmp = xml2->get_attribute("name");
            if (name_tmp!=0) break;
        }
    }
    Symbol *o_name = new Symbol(name_tmp);
	if (baseClassName->cmp("Item")) {
        Sort *s = ontology->sortExistsP(xml->get_attribute("class"));
		o = new A4Item(o_name, s);
        delete o_name;
    } else if (baseClassName->cmp("Container")) {
        Sort *s = ontology->sortExistsP(xml->get_attribute("class"));
        o = new A4Container(o_name, s, 0);
        delete o_name;
	} else if (baseClassName->cmp("Vehicle")) {
        Sort *s = ontology->sortExistsP(xml->get_attribute("class"));
		o = new A4Vehicle(o_name,s);
        delete o_name;
	} else if (baseClassName->cmp("Character")) {
        if (isPlayer) {
            Sort *s = ontology->sortExistsP(xml->get_attribute("class"));
            o = new A4PlayerCharacter(o_name,s);
            delete o_name;
        } else {
            Sort *s = ontology->sortExistsP(xml->get_attribute("class"));
            o = new A4AICharacter(o_name,s);
            delete o_name;
        }
	} else {
		output_debug_message("A4ObjectFactory::createObject: Could not create object %s\n", xml->get_attribute("class"));
        exit(1);
	}

    if (!completeRedefinition) {
        for(XMLNode *xml2:classes) {
            //output_debug_message("A4ObjectFactory::createObject: loading content from '%s' to create '%s'\n", xml2->get_attribute("class"), xml->get_attribute("class"));
            o->loadObjectAdditionalContent(xml2, game, this, 0);
        }
    }
    
    delete baseClassName;
	return o;
}
