//
//  WME.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#include <assert.h>
#include "debug.h"
#include "string.h"
#include "stdlib.h"
#include "Ontology.h"
#include "WME.h"
#include "AIMemory.h"
#include "ExpressionParser.h"
#include "auxiliar.h"


WME::WME(Expression *exp, Ontology *o, int activation)
{
    assert(exp!=0);
//    exp->output_debug(1);
    m_functor = new Symbol(exp->get_head());
    m_deleteFunctor = true;
    m_sourceObject = 0;

    std::vector<Expression *>::iterator it = exp->get_parameters()->begin();
    m_n_parameters = 0;
    
    while(it!=exp->get_parameters()->end()) {
        char *tmp = (*it)->get_head();
//        output_debug_message("parsing parameter: %s\n",tmp);
        
        if (tmp[0]=='?') {
            // wildcard:
            if (tmp[1] == 0) {
                m_parameters[m_n_parameters].m_integer = 0;
            } else {
                m_parameters[m_n_parameters].m_integer = atoi(tmp+1);
            }
            m_parameter_types[m_n_parameters] = WME_PARAMETER_WILDCARD;
        } else if (tmp[0]=='\"') {
            // symbol:
            char *tmp2 = new char[strlen(tmp)+1];
            strcpy(tmp2, tmp+1);
            tmp2[strlen(tmp2)-1] = 0;
            char *tmp3 = replaceString(tmp2,"\\039","\'");
            m_parameters[m_n_parameters].m_symbol = new Symbol(tmp3);
            delete tmp3;
            m_parameter_types[m_n_parameters] = WME_PARAMETER_SYMBOL;
            delete []tmp2;
        } else if ((*it)->isQuoted()) {
            // symbol:
            char *tmp3 = replaceString(tmp,"\\039","\'");
            m_parameters[m_n_parameters].m_symbol = new Symbol(tmp3);
            delete tmp3;
            m_parameter_types[m_n_parameters] = WME_PARAMETER_SYMBOL;
        } else if ((tmp[0]>='0' && tmp[0]<='9') || tmp[0]=='-') {
            // integer:
            m_parameters[m_n_parameters].m_integer = atoi(tmp);
            m_parameter_types[m_n_parameters] = WME_PARAMETER_INTEGER;
        } else {
            // sort:
            m_parameters[m_n_parameters].m_sort = o->getSort(tmp);
            m_parameter_types[m_n_parameters] = WME_PARAMETER_SORT;
        }

        m_n_parameters++;
        it++;
    }
    
//    output_debug_message("done\n");

    m_activation = activation;
    m_startTime = 0;
    assert(parameterSanityCheck());
}


WME::WME(Symbol *f, int activation)
{
    m_functor = f;
    m_deleteFunctor = false;
    m_n_parameters = 0;
    m_activation = activation;
    m_startTime = 0;
    m_sourceObject = 0;
}


WME::WME(Symbol *f, WMEParameter p1, int p1_type, int activation)
{
    m_functor = f;
    m_deleteFunctor = false;
    m_n_parameters = 1;
    m_parameters[0] = p1;
    m_parameter_types[0] = p1_type;
    m_activation = activation;
    m_startTime = 0;
    m_sourceObject = 0;
    assert(parameterSanityCheck());
}


WME::WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, int activation)
{
    m_functor = f;
    m_deleteFunctor = false;
    m_n_parameters = 2;
    m_parameters[0] = p1;
    m_parameter_types[0] = p1_type;
    m_parameters[1] = p2;
    m_parameter_types[1] = p2_type;
    m_activation = activation;
    m_startTime = 0;
    m_sourceObject = 0;
    assert(parameterSanityCheck());
}


WME::WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, WMEParameter p3, int p3_type, int activation)
{
    m_functor = f;
    m_deleteFunctor = false;
    m_n_parameters = 3;
    m_parameters[0] = p1;
    m_parameter_types[0] = p1_type;
    m_parameters[1] = p2;
    m_parameter_types[1] = p2_type;
    m_parameters[2] = p3;
    m_parameter_types[2] = p3_type;
    m_activation = activation;
    m_startTime = 0;
    m_sourceObject = 0;
    assert(parameterSanityCheck());
}


WME::WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, WMEParameter p3, int p3_type,
                    WMEParameter p4, int p4_type, int activation)
{
    m_functor = f;
    m_deleteFunctor = false;
    m_n_parameters = 4;
    m_parameters[0] = p1;
    m_parameter_types[0] = p1_type;
    m_parameters[1] = p2;
    m_parameter_types[1] = p2_type;
    m_parameters[2] = p3;
    m_parameter_types[2] = p3_type;
    m_parameters[3] = p4;
    m_parameter_types[3] = p4_type;
    m_activation = activation;
    m_startTime = 0;
    m_sourceObject = 0;
    assert(parameterSanityCheck());
}


WME::WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, WMEParameter p3, int p3_type,
         WMEParameter p4, int p4_type, WMEParameter p5, int p5_type, int activation)
{
    m_functor = f;
    m_deleteFunctor = false;
    m_n_parameters = 5;
    m_parameters[0] = p1;
    m_parameter_types[0] = p1_type;
    m_parameters[1] = p2;
    m_parameter_types[1] = p2_type;
    m_parameters[2] = p3;
    m_parameter_types[2] = p3_type;
    m_parameters[3] = p4;
    m_parameter_types[3] = p4_type;
    m_parameters[4] = p5;
    m_parameter_types[4] = p5_type;
    m_activation = activation;
    m_startTime = 0;
    m_sourceObject = 0;
    assert(parameterSanityCheck());
}


WME::WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, WMEParameter p3, int p3_type,
                    WMEParameter p4, int p4_type, WMEParameter p5, int p5_type, WMEParameter p6, int p6_type, int activation)
{
    m_functor = f;
    m_deleteFunctor = false;
    m_n_parameters = 6;
    m_parameters[0] = p1;
    m_parameter_types[0] = p1_type;
    m_parameters[1] = p2;
    m_parameter_types[1] = p2_type;
    m_parameters[2] = p3;
    m_parameter_types[2] = p3_type;
    m_parameters[3] = p4;
    m_parameter_types[3] = p4_type;
    m_parameters[4] = p5;
    m_parameter_types[4] = p5_type;
    m_parameters[5] = p6;
    m_parameter_types[5] = p6_type;
    m_activation = activation;
    m_startTime = 0;
    m_sourceObject = 0;
    assert(parameterSanityCheck());
}


WME::WME(WME *wme)
{
    m_functor = wme->m_functor;
    m_deleteFunctor = false;
    m_n_parameters = wme->m_n_parameters;
    for(int i = 0;i<m_n_parameters;i++) {
        m_parameter_types[i] = wme->m_parameter_types[i];
        if (m_parameter_types[i]==WME_PARAMETER_SYMBOL) {
            m_parameters[i].m_symbol = new Symbol(wme->m_parameters[i].m_symbol);
        } else {
            m_parameters[i] = wme->m_parameters[i];
        }
    }
    m_activation = wme->m_activation;
    m_startTime = wme->m_startTime;
    assert(parameterSanityCheck());
}


WME::WME(Symbol *f, std::vector<WMEParameter> *lp, std::vector<int> *lt, int activation)
{
    m_functor = f;
    m_deleteFunctor = false;
    m_n_parameters = (int)lp->size();
    m_activation = activation;
    m_startTime = 0;
    m_sourceObject = 0;

    std::vector<WMEParameter>::iterator ip;
    std::vector<int>::iterator it;

    ip = lp->begin();
    it = lt->begin();
    for(int i = 0;i<m_n_parameters;i++) {
        m_parameters[i] = *ip;
        m_parameter_types[i] = *it;
        ip++;
        it++;
    }
    assert(parameterSanityCheck());
}


WME::~WME()
{
//    output_debug_message("Deleting WME %p (%s/%i)... ",this,m_functor->get(),m_n_parameters);
//    for(int i = 0;i<m_n_parameters;i++) output_debug_message(" %i ",m_parameter_types[i]);
//    {
//        char buffer[256];
//        toString(buffer);
//        output_debug_message("%s ",buffer);
//    }
    if (m_deleteFunctor) {
        delete m_functor;
        m_functor = 0;
    }
    for(int i = 0;i<m_n_parameters;i++) {
        if (m_parameter_types[i]==WME_PARAMETER_SYMBOL && m_parameters[i].m_symbol!=0) {
            delete m_parameters[i].m_symbol;
            m_parameters[i].m_symbol = 0;
        }
    }
//    output_debug_message("Ok\n");
}


bool WME::equivalents(WME *wme)
{
    if (!m_functor->cmp(wme->m_functor)) return false;
    if (m_n_parameters!=wme->m_n_parameters) return false;
    for(int i = 0;i<m_n_parameters;i++) {
        if (!equivalentParameters(m_parameters[i], m_parameter_types[i],
                                  wme->m_parameters[i], wme->m_parameter_types[i])) return false;
    }

    return true;
}


bool WME::subsumption(WME *wme)
{
    if (!m_functor->cmp(wme->m_functor)) return false;
    if (m_n_parameters!=wme->m_n_parameters) return false;
    for(int i = 0;i<m_n_parameters;i++) {
        switch(m_parameter_types[i]) {
            case WME_PARAMETER_INTEGER:
                if (wme->m_parameter_types[i]!=WME_PARAMETER_INTEGER) return false;
                if (m_parameters[i].m_integer!=wme->m_parameters[i].m_integer) return false;
                break;
            case WME_PARAMETER_SYMBOL:
                if (wme->m_parameter_types[i]!=WME_PARAMETER_SYMBOL) return false;
                if (!m_parameters[i].m_symbol->cmp(wme->m_parameters[i].m_symbol)) return false;
                break;
            case WME_PARAMETER_SORT:
                if (wme->m_parameter_types[i]!=WME_PARAMETER_SORT) return false;
                if (!m_parameters[i].m_sort->subsumes(wme->m_parameters[i].m_sort)) return false;
                break;
            case WME_PARAMETER_WILDCARD:
                break;
        }
    }
    return true;
}


bool WME::relativeSubsumption(WME *wme, AIMemory *m)
{
    if (!m_functor->cmp(wme->m_functor)) return false;
    if (m_n_parameters!=wme->m_n_parameters) return false;
    for(int i = 0;i<m_n_parameters;i++) {
        switch(m_parameter_types[i]) {
            case WME_PARAMETER_INTEGER:
                if (wme->m_parameter_types[i]!=WME_PARAMETER_INTEGER) return false;
                if (m_parameters[i].m_integer!=wme->m_parameters[i].m_integer) return false;
                break;
            case WME_PARAMETER_SYMBOL:
                if (wme->m_parameter_types[i]!=WME_PARAMETER_SYMBOL) return false;
                if (!m_parameters[i].m_symbol->cmp(wme->m_parameters[i].m_symbol)) return false;
                break;
            case WME_PARAMETER_SORT:
                if (wme->m_parameter_types[i]==WME_PARAMETER_SORT) {
                    if (!m_parameters[i].m_sort->subsumes(wme->m_parameters[i].m_sort)) return false;
                } else if (wme->m_parameter_types[i]==WME_PARAMETER_INTEGER) {
                    // check for "is_a" wmes:
                    WME *isaPattern = new WME(AIMemory::s_is_a_symbol, wme->m_parameters[i], wme->m_parameter_types[i],
						WMEParameter(0), WME_PARAMETER_WILDCARD, 0);
                    std::vector<WME *> *l = m->retrieveSubsumption(isaPattern);
                    bool found = false;
                    for(WME *isaWME:*l) {
                        if (isaWME->m_parameter_types[1]==WME_PARAMETER_SORT &&
                            m_parameters[i].m_sort->subsumes(isaWME->m_parameters[1].m_sort)) {
                            found = true;
                            break;
                        }
                    }
                    delete l;
                    delete isaPattern;
                    if (!found) return false;
                } else if (wme->m_parameter_types[i]==WME_PARAMETER_SYMBOL) {
                    // check for "is_a" wmes:
                    WME *isaPattern = new WME(AIMemory::s_is_a_symbol,
											  WMEParameter(new Symbol(wme->m_parameters[i].m_symbol)), wme->m_parameter_types[i],
											  WMEParameter(0), WME_PARAMETER_WILDCARD, 0);
                    std::vector<WME *> *l = m->retrieveSubsumption(isaPattern);
                    bool found = false;
                    for(WME *isaWME:*l) {
                        if (isaWME->m_parameter_types[1]==WME_PARAMETER_SORT &&
                            m_parameters[i].m_sort->subsumes(isaWME->m_parameters[1].m_sort)) {
                            found = true;
                            break;
                        }
                    }
                    delete l;
                    delete isaPattern;
                    if (!found) return false;
                } else {
                    // it's a wildcard... no subsumption...
                    return false;
                }
                break;
            case WME_PARAMETER_WILDCARD:
                break;
        }
    }
    return true;
}


WME *WME::unification(WME *wme)
{
    if (!m_functor->cmp(wme->m_functor)) return 0;
    if (m_n_parameters!=wme->m_n_parameters) return 0;
    WME *unifier = new WME(m_functor,0);
    for(int i = 0;i<m_n_parameters;i++) {
        if (m_parameter_types[i] == WME_PARAMETER_WILDCARD) {
            unifier->m_parameters[i] = wme->m_parameters[i];
            unifier->m_parameter_types[i] = wme->m_parameter_types[i];
        } else {
            if (wme->m_parameter_types[i] == WME_PARAMETER_WILDCARD) {
                unifier->m_parameters[i] = m_parameters[i];
                unifier->m_parameter_types[i] = m_parameter_types[i];
            } else {
                switch(m_parameter_types[i]) {
                    case WME_PARAMETER_INTEGER:
                        if (wme->m_parameter_types[i]!=WME_PARAMETER_INTEGER ||
                            m_parameters[i].m_integer!=wme->m_parameters[i].m_integer) {
                            delete unifier;
                            return 0;
                        }
                        unifier->m_parameters[i] = m_parameters[i];
                        unifier->m_parameter_types[i] = m_parameter_types[i];
                        break;
                    case WME_PARAMETER_SYMBOL:
                        if (wme->m_parameter_types[i]!=WME_PARAMETER_SYMBOL ||
                            !m_parameters[i].m_symbol->cmp(wme->m_parameters[i].m_symbol)) {
                            delete unifier;
                            return 0;
                        }
                        unifier->m_parameters[i].m_symbol = new Symbol(m_parameters[i].m_symbol);
                        unifier->m_parameter_types[i] = m_parameter_types[i];
                        break;
                    case WME_PARAMETER_SORT:
                        if (wme->m_parameter_types[i]!=WME_PARAMETER_SORT) {
                            delete unifier;
                            return 0;
                        }
                        if (m_parameters[i].m_sort->subsumes(wme->m_parameters[i].m_sort)) {
                            unifier->m_parameters[i] = wme->m_parameters[i];
                            unifier->m_parameter_types[i] = wme->m_parameter_types[i];
                        } else if (wme->m_parameters[i].m_sort->subsumes(m_parameters[i].m_sort)) {
                            unifier->m_parameters[i] = m_parameters[i];
                            unifier->m_parameter_types[i] = m_parameter_types[i];
                        } else {
                            delete unifier;
                            return 0;
                        }
                        break;
                }
            }
        }
        unifier->m_n_parameters++;
    }

    return unifier;
}


void WME::toString(char *buffer)
{
    sprintf(buffer, "%s(",m_functor->get());
    bool first = true;
    for(int i = 0;i<m_n_parameters;i++) {
        if (!first) sprintf(buffer+strlen(buffer),",");
        if (m_parameter_types[i]==WME_PARAMETER_INTEGER) {
            sprintf(buffer+strlen(buffer),"%i",m_parameters[i].m_integer);
        } else if (m_parameter_types[i]==WME_PARAMETER_SYMBOL) {
            sprintf(buffer+strlen(buffer),"'%s'",m_parameters[i].m_symbol->get());
        } else if (m_parameter_types[i]==WME_PARAMETER_SORT) {
            sprintf(buffer+strlen(buffer),"%s",m_parameters[i].m_sort->getName()->get());
        } else if (m_parameter_types[i]==WME_PARAMETER_WILDCARD) {
            if (m_parameters[i].m_integer==0) sprintf(buffer+strlen(buffer),"?");
                               else sprintf(buffer+strlen(buffer),"?%i",m_parameters[i].m_integer);
        }
        first = false;
    }
    if (!first) sprintf(buffer+strlen(buffer),",");
    sprintf(buffer+strlen(buffer),"%i)",m_activation);
}



void WME::toStringNoActivation(char *buffer)
{
    sprintf(buffer, "%s(",m_functor->get());
    bool first = true;
    for(int i = 0;i<m_n_parameters;i++) {
        if (!first) sprintf(buffer+strlen(buffer),",");
        if (m_parameter_types[i]==WME_PARAMETER_INTEGER) {
            sprintf(buffer+strlen(buffer),"%i",m_parameters[i].m_integer);
        } else if (m_parameter_types[i]==WME_PARAMETER_SYMBOL) {
            char *tmp2 = replaceString(m_parameters[i].m_symbol->get(),"\'","\039");
            sprintf(buffer+strlen(buffer),"'%s'",tmp2);
            delete tmp2;
        } else if (m_parameter_types[i]==WME_PARAMETER_SORT) {
            sprintf(buffer+strlen(buffer),"%s",m_parameters[i].m_sort->getName()->get());
        } else if (m_parameter_types[i]==WME_PARAMETER_WILDCARD) {
            if (m_parameters[i].m_integer==0) sprintf(buffer+strlen(buffer),"?");
            else sprintf(buffer+strlen(buffer),"?%i",m_parameters[i].m_integer);
        }
        first = false;
    }
    sprintf(buffer+strlen(buffer),")");
}


void WME::parseParameterString(const char *s, Ontology *o, WMEParameter &p, int &type)
{
    if ((s[0]>='0' && s[0]<='9') || s[0]=='-') {
        // it's a number:
        p.m_integer = atoi(s);
        type = WME_PARAMETER_INTEGER;
    } else if (s[0]=='?' && s[1]==0) {
        // wildcard:
        p.m_symbol = 0;
        type = WME_PARAMETER_WILDCARD;
    } else {
        Sort *sort = o->getSort(s);
        if (sort==0) {
            p.m_symbol = new Symbol(s);
            type = WME_PARAMETER_SYMBOL;
        } else {
            p.m_sort = sort;
            type = WME_PARAMETER_SORT;
        }
    }
}


bool WME::parameterSanityCheck()
{
    for(int i = 0;i<m_n_parameters;i++) {
        switch(m_parameter_types[i]) {
            case WME_PARAMETER_INTEGER:
                break;
            case WME_PARAMETER_WILDCARD:
                break;
            case WME_PARAMETER_SYMBOL:
                if (m_parameters[i].m_symbol==0) {
                    return false;
                }
                break;
            case WME_PARAMETER_SORT:
                if (m_parameters[i].m_sort==0) {
                    return false;
                }
        }
    }

    return true;
}

