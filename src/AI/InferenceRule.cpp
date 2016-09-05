#include "stdlib.h"
#include <vector>

#include "debug.h"

#include "ExpressionParser.h"
#include "XMLparser.h"
#include "XMLwriter.h"
#include "Ontology.h"
#include "WME.h"
#include "AIMemory.h"
#include "InferenceRule.h"

#include "A4Object.h"
#include "A4Map.h"
#include "A4Game.h"


InferenceRule::InferenceRule(XMLNode *xml, Ontology *o)
{
    char *premiseText = xml->get_attribute("premise");
    char *conclusionText = xml->get_attribute("conclusion");
    m_frequency = atof(xml->get_attribute("frequency"));
    m_activation = atoi(xml->get_attribute("activation"));
    m_coolDown = 1;
    if (xml->get_attribute("cooldown")!=0) {
        m_coolDown = atoi(xml->get_attribute("cooldown"));
    }
    m_last_cycle_executed = -1;
    
    output_debug_message("Creaiting WME from expression (premise): %s\n", premiseText);
    output_debug_message("Creaiting WME from expression (conclusion): %s\n", conclusionText);
    
    std::vector<Expression *> *l = Expression::list_from_string(premiseText);
    Expression *exp = Expression::from_string(conclusionText);

    for(Expression *e:*l) {
        WME *w = new WME(e, o, 0);
        m_premise.push_back(w);
    }
    m_conclusion = new WME(exp, o, 0);

    delete exp;
    for(Expression *e:*l) delete e;
    l->clear();
    delete l;
    
    char *once_text = xml->get_attribute("once");
    if (once_text!=0 && strcmp(once_text,"true")==0) {
        m_once = true;
    } else {
        m_once = false;
    }
    
    for(XMLNode *script_xml:*(xml->get_children())) {
        m_effects.push_back(new A4Script(script_xml));
    }
}


InferenceRule::~InferenceRule() {
    if (m_conclusion!=0) delete m_conclusion;
    m_conclusion = 0;
    for(WME *e:m_premise) delete e;
    m_premise.clear();
    for(A4Script *s:m_effects) delete s;
    m_effects.clear();
}


void InferenceRule::execute(AIMemory *m, A4Object *character, A4Map *map, A4Game *game)
{
    if (m_once && m_last_cycle_executed!=-1) return;
    if (m_last_cycle_executed>=0 && game->getCycle()-m_last_cycle_executed<m_coolDown) return;
    std::list<Binding *> bindings;
    if (matchPremise(m_premise.begin(), m_premise.end(), &bindings, m)) {
        WME *wme = new WME(m_conclusion);
        applyBindings(wme, &bindings);
        wme->setActivation(m_activation);
        m->addShortTermWME(wme);
        executeScriptEffects(character, map, game, 0);
        m_last_cycle_executed = game->getCycle();
    }
    for(Binding *b:bindings) delete b;
    bindings.clear();
}


int InferenceRule::executeScriptEffects(A4Object *p, A4Map *map, A4Game *game, A4Character *otherCharacter)
{
    int retValue = SCRIPT_FINISHED;
    A4ScriptExecutionQueue *seq = 0;
    for(A4Script *s:m_effects) {
        if (seq==0) {
            s->reset();
            retValue = s->execute(p, map, game, otherCharacter);
            if (retValue == SCRIPT_FINISHED) {
                // good, do nothing
            } else if (retValue == SCRIPT_NOT_FINISHED) {
                // script needs more time, create an script queue
                seq = new A4ScriptExecutionQueue(p, map, game, otherCharacter);
                seq->scripts.push_back(new A4Script(s));
                if (p!=0 ) {
                    p->addScriptQueue(seq);
                } else if (map!=0) {
                    map->addScriptQueue(seq);
                } else {
                    game->addScriptQueue(seq);
                }
            } else {
                // failed, stop the script
                break;
            }
        } else {
            s->reset();
            seq->scripts.push_back(new A4Script(s));
        }
    }
    return retValue;
}


bool InferenceRule::matchPremise(std::list<WME *>::iterator it, std::list<WME *>::iterator end,
                                 std::list<Binding *> *bindings, AIMemory *m) {
    if (it==end) return true;

    WME *wme = *it;
    bool free_wme = false;

    if (!bindings->empty()) {
        free_wme = true;
        wme = new WME(wme);
        // apply bindings:
        applyBindings(wme, bindings);
    }

    std::vector<WME *> *l = m->retrieveUnification(wme);
    for(WME *wme2:*l) {
        // add new bindings and do recursive call:
        int n = 0;
        for(int i = 0;i<wme->getNParameters();i++) {
            if (wme->getParameterType(i)==WME_PARAMETER_WILDCARD &&
                wme->getParameter(i).m_integer!=0 &&
                (wme2->getParameterType(i)!=WME_PARAMETER_WILDCARD ||
                 wme2->getParameter(i).m_integer!=0)) {
                Binding *b = new Binding();
                b->m_variable = wme->getParameter(i).m_integer;
                b->m_value = wme2->getParameter(i);
                b->m_value_type = wme2->getParameterType(i);
                bindings->push_front(b);
                n++;
            }
        }

        it++;
        if (matchPremise(it, end, bindings, m)) {
            delete l;
            if (free_wme) delete wme;
            return true;
        }
        it--;

        // remove bindings:
        for(int i = 0;i<n;i++) {
            Binding *b = bindings->front();
            bindings->pop_front();
            delete b;
        }
    }
    delete l;
    if (free_wme) delete wme;
    return false;
}


void InferenceRule::applyBindings(WME *wme, std::list<Binding *> *bindings)
{
    for(int i = 0;i<wme->getNParameters();i++) {
        if (wme->getParameterType(i)==WME_PARAMETER_WILDCARD && wme->getParameter(i).m_integer>0) {
            for(Binding *b:*bindings) {
                if (b->m_variable == wme->getParameter(i).m_integer) {
                    if (wme->getParameterType(i) == WME_PARAMETER_SYMBOL &&
                        wme->getParameter(i).m_symbol != 0) delete wme->getParameter(i).m_symbol;
                    wme->setParameterType(i, b->m_value_type);
                    if (b->m_value_type == WME_PARAMETER_SYMBOL) wme->setParameter(i, WMEParameter(new Symbol(b->m_value.m_symbol)));
                                                            else wme->setParameter(i, b->m_value);
                }
            }
        }
    }
}


void InferenceRule::saveToXML(XMLwriter *w) {
    char tmp[1024];
    
    w->openTag("inferenceRule");
    
    bool first = true;
    for(WME *w:m_premise) {
        if (first) {
            w->toStringNoActivation(tmp);
            first = false;
        } else {
            sprintf(tmp+strlen(tmp),",");
            w->toStringNoActivation(tmp+strlen(tmp));
        }
    }
    w->setAttribute("premise", tmp);

    m_conclusion->toStringNoActivation(tmp);
    w->setAttribute("conclusion", tmp);
    
    sprintf(tmp,"%g",m_frequency);
    w->setAttribute("frequency", tmp);
    w->setAttribute("activation", m_activation);
    w->setAttribute("cooldown", m_coolDown);
    
    w->closeTag("inferenceRule");
}


