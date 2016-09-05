#ifndef B_INTERFACE
#define B_INTERFACE

#include <list>

#define BBUTTON_STATE_NORMAL	0
#define BBUTTON_STATE_MOUSEOVER	1
#define BBUTTON_STATE_PRESSED	2

class BInterfaceElement {
public:
	BInterfaceElement();
	virtual ~BInterfaceElement();
	virtual void createOpenGLBuffers(void);

	virtual bool mouse_over(int mousex,int mousey);
	virtual bool check_status(int mousex,int mousey,int button,int button_status,class KEYBOARDSTATE *k);
	virtual void draw(float alpha);
	virtual void draw(void);
	virtual std::list<BInterfaceElement *> getChildren(void);
    
	int m_ID;
	bool m_modal;	/* If any element is modal, only him has the control until it is destroyed (the rest of the interface is faded) */ 
	bool m_enabled;	/* whether the element can b interacted with or not */
	bool m_active;	/* This indicates whether the component is active or passive (passive elements are only decorative) */ 
					/* e.g.: BText and BFrame are passive					*/ 
	bool m_to_be_deleted;

	float m_x,m_y,m_dx,m_dy;

    int vPositionLocation, UVLocation;
};


class BText : public BInterfaceElement {
public:

	BText(const char *text,BitmapFont *font,float x,float y,bool centered);
	BText(const char *text,BitmapFont *font,float x,float y,bool centered,int ID);
	virtual ~BText();

	virtual void draw(float alpha);
	virtual void draw(void);

	void set_text(const char *text);

	bool m_centered;
	char *m_text;
	BitmapFont *m_font;
};

class BInterfaceTile : public BInterfaceElement {
public:
    
    BInterfaceTile(GLTile *t,float x,float y);
    BInterfaceTile(GLTile *t,float x,float y,int ID);
    virtual ~BInterfaceTile();
    
    virtual void draw(float alpha);
    virtual void draw(void);

    GLTile *m_tile;
};

class BInterfaceAnimation : public BInterfaceElement {
public:
    
    BInterfaceAnimation(class Animation *t,float x,float y);
    BInterfaceAnimation(Animation *t,float x,float y,int ID);
    virtual ~BInterfaceAnimation();
    
    virtual void draw(float alpha);
    virtual void draw(void);
    
    Animation *m_animation;
};


class BButton : public BInterfaceElement {
public:

	BButton(const char *text,BitmapFont *font,float x,float y,float dx,float dy,int ID);
	BButton(GLTile *icon,float x,float y,float dx,float dy,int ID);
	virtual ~BButton();
	virtual void createOpenGLBuffers(void);
    void changeText(char *new_text);
    
	virtual bool mouse_over(int mousex,int mousey);
	virtual bool check_status(int mousex,int mousey,int button,int button_status,KEYBOARDSTATE *k);
	virtual void draw(float alpha);
	virtual void draw(void);

	char *m_text;
	BitmapFont *m_font;
	int m_status;
	int m_cycle;
	GLTile *m_tile;
    GLuint VertexArrayID;
    GLuint vertexbuffer;	
};

class BButtonTransparent : public BButton {
public:
	BButtonTransparent(const char *text,BitmapFont *font,float x,float y,float dx,float dy,int ID);
    BButtonTransparent(const char *text,BitmapFont *font,float x,float y,float dx,float dy,int ID, float r, float g, float b, float a);

	virtual void draw(float alpha);
    float r, g, b, a;
};


class BQuad : public BInterfaceElement {
public:
    
    BQuad(float x,float y,float dx,float dy, float r, float g, float b, float a);
    virtual ~BQuad();
    virtual void createOpenGLBuffers(void);
    
    virtual void draw(float alpha);
    virtual void draw(void);
    
    float r, g, b, a;
    GLuint VertexArrayID;
    GLuint vertexbuffer;
};


class BFrame : public BInterfaceElement {
public:

	BFrame(float x,float y,float dx,float dy);
	virtual ~BFrame();
	virtual void createOpenGLBuffers(void);

    void setX(int x);
    void setY(int y);
    void setDx(int dx);
    void setDy(int dy);
    void recreateBuffers();
    
	virtual void draw(float alpha);
	virtual void draw(void);

	GLfloat cursor_dx, cursor_dy;
    GLuint VertexArrayID;
    GLuint vertexbuffer;
};


class BTextFrame : public BFrame {
public:

	BTextFrame(const char *initial_text,bool a_centered,BitmapFont *font,float x,float y,float dx,float dy);
	virtual ~BTextFrame();

	virtual void draw(float alpha);

	bool centered;
	BitmapFont *m_font;
	std::vector<char *> text;
};


class BTextInputFrame : public BFrame {
public:

	BTextInputFrame(const char *question, const char *initial_text,int max_characters,BitmapFont *font,float x,float y,float dx,float dy,int ID);
	BTextInputFrame(const char *initial_text,int max_characters,BitmapFont *font,float x,float y,float dx,float dy,int ID);
	virtual ~BTextInputFrame();
	virtual void createOpenGLBuffers(void);

	virtual bool mouse_over(int mousex,int mousey);
	virtual bool check_status(int mousex,int mousey,int button,int button_status,KEYBOARDSTATE *k);
	virtual void draw(float alpha);
	virtual void draw(void);
	void setFocus() {m_focus = true;}

	void string_editor_cycle(char *editing_text,unsigned int *editing_position,unsigned int max_length,KEYBOARDSTATE *k);

	BitmapFont *m_font;
	char *m_question;
	char *m_editing;
	int m_max_characters;
	unsigned int m_editing_position;
	bool m_focus;
	int m_cycle;
	bool m_change_in_last_cycle;

    GLuint VertexArrayID2;
    GLuint vertexbuffer2;	
};


class BConfirmation : public BInterfaceElement {
public:

	// Separate the different lines of the message with '\n' characters:
	BConfirmation(const char *message,BitmapFont *font,float x,float y,int ID,bool cancelOption);
	virtual ~BConfirmation();
	virtual void createOpenGLBuffers(void);

	virtual bool check_status(int mousex,int mousey,int button,int button_status,KEYBOARDSTATE *k);
	virtual void draw(float alpha);
	virtual void draw(void);
	virtual std::list<BInterfaceElement *> getChildren(void);

	BitmapFont *m_font;
	std::vector<char *> m_message_lines;
	int m_state,m_cycle;
	BButton *m_ok_button,*m_cancel_button;

    GLuint VertexArrayID;
    GLuint vertexbuffer;	
};


class BInterface {
public:
	static std::list<BInterfaceElement *> m_elements;
	static std::list<BInterfaceElement *> m_added_since_last_push;
	static std::list<std::list<BInterfaceElement *>> m_stack;
    static BitmapFont *s_last_used_font;

	static void push(void);
	static void pop(void);
	static void createMenu(const char *options, BitmapFont *font, int x, int y, int dx, int dy, int starting_ID);
	static void createMenu(const char *options, BitmapFont *font, int x, int y, int dx, int dy, int interline_space, int starting_ID);

	static void add_element(BInterfaceElement *b);
	static void remove_element(BInterfaceElement *b);
	static void remove_element(int ID);
	static void substitute_element(BInterfaceElement *old,BInterfaceElement *n);
	static void reset(void);
	static bool mouse_over_element(int mousex, int mousey);
	static int update_state(int mousex,int mousey,int button,int button_status,KEYBOARDSTATE *k);
	static int update_state(std::list<SDL_MouseButtonEvent> *m_mouse_clicks,KEYBOARDSTATE *k);
	static void draw(float alpha);
	static void draw(void);

	static void createOpenGLBuffers();	// this should be called when the OpenGL context is recreated.
										// For example, when toggling from windowed to fullscreen

	static BInterfaceElement *get(int ID);
	static void enable(int ID);
	static void disable(int ID);
	static bool is_enabled(int ID);
	static void setFocus(int ID);

    static void clear_print_cache(void);
    static GLTile *get_text_tile(const char *text,BitmapFont *font);
    static BitmapFont *get_last_used_font() {return s_last_used_font;};
    static void print_left(const char *text,BitmapFont *font,float x,float y);
    static void print_right(const char *text,BitmapFont *font,float x,float y);
    static void print_center(const char *text,BitmapFont *font,float x,float y);
    static void print_left(const char *text,BitmapFont *font,float x,float y,float r,float g,float b,float a);
    static void print_right(const char *text,BitmapFont *font,float x,float y,float r,float g,float b,float a);
    static void print_center(const char *text,BitmapFont *font,float x,float y,float r,float g,float b,float a);
    
    // Centered in both X and Y:
    static void print_centered(const char *text,BitmapFont *font,float x,float y,float r,float g,float b,float a,float angle,float scale);
    
    static void print_left_multiline(const char *text, BitmapFont *font, float x, float y, float y_interval);
};

#endif