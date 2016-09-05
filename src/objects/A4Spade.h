#ifndef _A4ENGINE_SPADE
#define _A4ENGINE_SPADE

class A4Spade : public A4Item {
public:
	A4Spade(Sort *sort, class Animation *a, int gold);
	virtual ~A4Spade();

    virtual void event(int event, class A4Character *otherCharacter, class A4Map *map, class A4Game *game);

protected:
};

#endif

