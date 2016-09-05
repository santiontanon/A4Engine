//
//  WME.h
//  A4Engine
//
//  Created by Santiago Ontanon on 3/25/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __A4Engine__WME__
#define __A4Engine__WME__

#include "Symbol.h"
#include "Sort.h"
#include "Ontology.h"

#define MAX_WME_PARAMETERS          6

#define WME_PARAMETER_INTEGER       0
#define WME_PARAMETER_WILDCARD      1
#define WME_PARAMETER_SYMBOL        2
#define WME_PARAMETER_SORT          3


typedef union WMEParameter {
    int m_integer;
    Symbol *m_symbol;
    Sort *m_sort;

	WMEParameter() {};
	WMEParameter(int i) { m_integer = i; }
	WMEParameter(Symbol *s) { m_symbol = s; }
	WMEParameter(Sort *s) { m_sort = s; }
} WMEParameter;


class WME {
public:
    WME(class Expression *exp, Ontology *o, int activation);
    WME(Symbol *f, int activation);
    WME(Symbol *f, WMEParameter p1, int p1_type, int activation);
    WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, int activation);
    WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, WMEParameter p3, int p3_type, int activation);
    WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, WMEParameter p3, int p3_type,
                   WMEParameter p4, int p4_type, int activation);
    WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, WMEParameter p3, int p3_type,
                   WMEParameter p4, int p4_type, WMEParameter p5, int p5_type, int activation);
    WME(Symbol *f, WMEParameter p1, int p1_type, WMEParameter p2, int p2_type, WMEParameter p3, int p3_type,
                   WMEParameter p4, int p4_type, WMEParameter p5, int p5_type, WMEParameter p6, int p6_type, int activation);
    WME(WME *wme);

    // note: be mindful when using this function, that WMEs have a maximum parameter size (MAX_WME_PARAMETERS), for efficiency:
    WME(Symbol *f, std::vector<WMEParameter> *lp, std::vector<int> *lt, int activation);

    // note: this destructor will NOT free the functor, it is assumed that a single copy of the functor is kept somewhere else.
    ~WME();

    bool equivalents(WME *wme);
    bool subsumption(WME *wme);
    WME *unification(WME *wme);

    bool relativeSubsumption(WME *wme, class AIMemory *m);  // subsumption, but takn into account the "is_a" WMEs in the short and long term memory

    bool equivalentParameters(WMEParameter p1, int t1, WMEParameter p2, int t2) {
        if (t1!=t2) return false;
        if (t1==WME_PARAMETER_INTEGER) {
            return p1.m_integer==p2.m_integer;
        }
        if (t1==WME_PARAMETER_WILDCARD) return true;
        if (t1==WME_PARAMETER_SYMBOL) {
            return p1.m_symbol->cmp(p2.m_symbol);
        }
        if (t1==WME_PARAMETER_SORT) {
            return p1.m_sort==p2.m_sort;
        }
        return false;
    }
    
    void setSource(class A4Object *o) {m_sourceObject = o;}
    A4Object *getSource() {return m_sourceObject;}

    Symbol *getFunctor() {return m_functor;}
    int getNParameters() {return m_n_parameters;}
    WMEParameter getParameter(int n) {return m_parameters[n];}
    int getParameterType(int n) {return m_parameter_types[n];}

    void setParameter(int n, WMEParameter v) {m_parameters[n] = v;}
    void setParameter(int n, WMEParameter v, int type) {m_parameters[n] = v;m_parameter_types[n] = type;}
    void setParameterType(int n, int type) {m_parameter_types[n] = type;}

    int getActivation() {return m_activation;}
    void setActivation(int a) {m_activation = a;}
    int getStartTime() {return m_startTime;}
    void setStartTime(int st) {m_startTime = st;}

    void toString(char *buffer);
    void toStringNoActivation(char *buffer);

    static void parseParameterString(const char *s, Ontology *o, WMEParameter &p, int &t);

    bool parameterSanityCheck();
    
protected:
    bool m_deleteFunctor;
    Symbol *m_functor;
    int m_n_parameters;
    int m_parameter_types[MAX_WME_PARAMETERS];      // hard-coded max number of parameters to prevent memory allocations.
                                                    // If this becomes inconvenient, I can replace this by a variable size array.
    WMEParameter m_parameters[MAX_WME_PARAMETERS];

    A4Object *m_sourceObject;   // for perception WMEs, we cache here the object that triggered the creation of this WME
                                // (notice that this pointer is not safe, since the object might hav disappeared from the
                                // actual game, so it has to be verified before using it).
    
    int m_activation;
    int m_startTime;
};

#endif /* defined(__A4Engine__WME__) */
