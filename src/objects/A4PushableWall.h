#ifndef _A4ENGINE_PUSHABLE_WALL
#define _A4ENGINE_PUSHABLE_WALL

class A4PushableWall : public A4Object {
public:
	A4PushableWall(Sort *sort, class Animation *a);
	virtual ~A4PushableWall();

	virtual bool isWalkable() {return false;}
	virtual bool isInteracteable() {return true;}

	virtual void event(int event, class A4Character *character, class A4Map *map, class A4Game *game);

protected:
};

#endif

