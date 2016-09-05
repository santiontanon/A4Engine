#ifndef __A4_ANIMATION
#define __A4_ANIMATION

class Animation {
public:
	Animation();
	Animation(int tile, const char *file, class A4Game *game);
	Animation(XMLNode *xml, A4Game *game);
	Animation(Animation *a);
	Animation(class Expression *exp, A4Game *game);
	~Animation();
    
    void saveToXML(class XMLwriter *w, const char *name);

	void reset();
	bool cycle();
	void draw(int x, int y, float zoom);
	void draw(int x, int y, float zoom, float r, float g, float b, float a);
	bool isCompleted();
    
	int getWidth() {
		return m_dx;
	}

	int getHeight() {
		return m_dy;
	}

	int getTile() {
		if (m_state>=0) return m_sequence[m_state];
		return -1;
	}

	class GraphicFile *getGraphicFile() {return m_gf;}

	int getPixelWidth();
	int getPixelHeight();

private:
	char *m_file;
	class GraphicFile *m_gf;

	int m_dx,m_dy;	// in tiles (NOT in pixels!)
	int m_period;
	bool m_looping;
	int m_length;
	int *m_sequence;

	int m_cycle;
	int m_state;
	bool m_completed;
};

#endif
