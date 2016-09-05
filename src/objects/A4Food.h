#ifndef _A4ENGINE_FOOD
#define _A4ENGINE_FOOD

class A4Food : public A4Item {
public:
	A4Food(Symbol *name, Sort *sort, Animation *a, int gold);
	virtual ~A4Food();

protected:
};

#endif

