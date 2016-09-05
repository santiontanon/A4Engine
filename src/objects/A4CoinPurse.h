#ifndef _A4ENGINE_COINPURSE
#define _A4ENGINE_COINPURSE

#include "A4Item.h"

class A4CoinPurse : public A4Item {
public:
	A4CoinPurse(Sort *sort, int gold, class Animation *a, bool createScripts);
	virtual ~A4CoinPurse();

    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);

protected:
};

#endif

