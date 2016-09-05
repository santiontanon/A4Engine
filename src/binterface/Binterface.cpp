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
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
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
#include "auxiliar.h"

#include <algorithm>
#include <vector>
#include "sound.h"
#include "keyboardstate.h"
#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
#include "BitmapFont.h"

#include "Binterface.h"

#define TEXT_TILE_BUFFER_SIZE	256

extern SDL_Window *appwindow;

class PrintBuffer {
public:
    ~PrintBuffer() {
        delete []text;
        text = 0;
        delete tile;
        tile = 0;
    } //
    
    BitmapFont *font;
    char *text;
    GLTile *tile;
};

std::list<PrintBuffer *> text_tile_buffer;
std::list<BInterfaceElement *> BInterface::m_elements;

std::list<BInterfaceElement *> BInterface::m_added_since_last_push;
std::list<std::list<BInterfaceElement *>> BInterface::m_stack;

BitmapFont *BInterface::s_last_used_font = 0;

void BInterface::add_element(BInterfaceElement *e)
{
    m_elements.push_back(e);
    m_added_since_last_push.push_back(e);
} /* BInterface::add_element */


void BInterface::remove_element(BInterfaceElement *e)
{
    m_elements.remove(e);
    m_added_since_last_push.remove(e);
} /* BInterface::remove_element */


void BInterface::remove_element(int ID)
{
    for(BInterfaceElement *e:m_elements) {
        if (e->m_ID==ID) {
            m_elements.remove(e);
            m_added_since_last_push.remove(e);
            delete e;
            return;
        } // if
    } // while
} /* BInterface::remove_element */



void BInterface::substitute_element(BInterfaceElement *old,BInterfaceElement *e)
{
    std::list<BInterfaceElement *>::iterator it;
    it = std::find(m_elements.begin(), m_elements.end(), old);
    if (it != m_elements.end()) {
        *it = e;
    } // if
} /* BInterface::substitute_element */


void BInterface::reset(void)
{
    output_debug_message("Resetting BInterface...\n");
    for(BInterfaceElement *e:m_elements) delete e;
    m_elements.clear();
    m_added_since_last_push.clear();
    m_stack.clear();
    output_debug_message("Done.\n");
} /* BInterface::reset */


bool BInterface::mouse_over_element(int mousex, int mousey)
{
    for(BInterfaceElement *e:m_elements) {
        if (e->mouse_over(mousex,mousey)) return true;
    } // while
    return false;
}


int BInterface::update_state(std::list<SDL_MouseButtonEvent> *m_mouse_clicks,KEYBOARDSTATE *k)
{
    int mouse_x=0,mouse_y=0,button=0,button_status=0;
    //	int ID=-1;
    if (!m_mouse_clicks->empty()) {
        SDL_MouseButtonEvent event = m_mouse_clicks->front();
        m_mouse_clicks->pop_front();
        mouse_x = event.x;
        mouse_y = event.y;
        button = event.button;
    } else {
        button_status=SDL_GetMouseState(&mouse_x,&mouse_y);
        button=0;
    } // if
    if (k->key_press(SDL_SCANCODE_SPACE) || k->key_press(SDL_SCANCODE_RETURN)) button=1;
    return BInterface::update_state(mouse_x,mouse_y,button,button_status,k);
}


int BInterface::update_state(int mousex,int mousey,int button,int button_status,KEYBOARDSTATE *k)
{
    int ret_val=-1;
    BInterfaceElement *modal=0;
    std::vector<BInterfaceElement *> to_delete;
    
    for(BInterfaceElement *e:m_elements) {
        if (e->m_modal && e->m_active && e->m_enabled) {
            modal = e;
            break;
        }
    }
    
    if (k->key_press(SDL_SCANCODE_TAB) || k->key_press(SDL_SCANCODE_DOWN)) {
        //        output_debug_message("BInterface::update_state: TAB/DOWN pressed...\n");
        std::list<BInterfaceElement *> elements;
        if (modal==0) {
            elements = m_elements;
        } else {
            elements = modal->getChildren();
        }
        
        BInterfaceElement *found=0;
        bool any_active=false;
        
        // Find if the mouse is currently over one of the active components:
        for(BInterfaceElement *e:elements) {
            if (e->m_active && e->m_enabled) {
                any_active=true;
                if (mousex>=e->m_x && mousex<e->m_x+e->m_dx &&
                    mousey>=e->m_y && mousey<e->m_y+e->m_dy) {
                    found=e;
                    break;
                } // if
            } // if
        } // while
        
        //		output_debug_message("found: %p\n",found);
        
        // find the next active component:
        if (any_active) {
            std::list<BInterfaceElement *>::iterator pos;
            if (found==0) {
                pos=elements.begin();
                BInterfaceElement *e = *pos;
                SDL_WarpMouseInWindow(appwindow, int(e->m_x+e->m_dx/2),int(e->m_y+e->m_dy/2));
                while(!e->m_active || !e->m_enabled) {
                    pos++;
                    if (pos==elements.end()) pos=elements.begin();
                    e=*pos;
                }
                SDL_WarpMouseInWindow(appwindow, int(e->m_x+e->m_dx/2),int(e->m_y+e->m_dy/2));
            } else {
                pos=std::find(elements.begin(), elements.end(), found);
                BInterfaceElement *e = 0;
                do {
                    pos++;
                    if (pos==elements.end()) pos=elements.begin();
                    e=*pos;
                } while(!e->m_active || !e->m_enabled);
                SDL_WarpMouseInWindow(appwindow, int(e->m_x+e->m_dx/2),int(e->m_y+e->m_dy/2));
            }
        } // if
    } else if (k->key_press(SDL_SCANCODE_UP)) {
        std::list<BInterfaceElement *> elements;
        if (modal==0) {
            elements = m_elements;
        } else {
            elements = modal->getChildren();
        }
        
        BInterfaceElement *found=0;
        bool any_active=false;
        
        // Find if the mouse is currently over one of the active components:
        for(BInterfaceElement *e:elements) {
            if (e->m_active && e->m_enabled) {
                any_active=true;
                if (mousex>=e->m_x && mousex<e->m_x+e->m_dx &&
                    mousey>=e->m_y && mousey<e->m_y+e->m_dy) {
                    found=e;
                    break;
                } // if
            } // if
        } // while
        
        //		output_debug_message("found: %p\n",found);
        
        // find the previous active component:
        if (any_active) {
            std::list<BInterfaceElement *>::iterator pos;
            if (found==0) {
                pos=elements.end();
                pos--;
                BInterfaceElement *e = *pos;
                SDL_WarpMouseInWindow(appwindow, int(e->m_x+e->m_dx/2),int(e->m_y+e->m_dy/2));
                while(!e->m_active || !e->m_enabled) {
                    if (pos==elements.begin()) pos=elements.end();
                    pos--;
                    e=*pos;
                }
                SDL_WarpMouseInWindow(appwindow, int(e->m_x+e->m_dx/2),int(e->m_y+e->m_dy/2));
            } else {
                pos=std::find(elements.begin(), elements.end(), found);
                BInterfaceElement *e = 0;
                do {
                    if (pos==elements.begin()) pos=elements.end();
                    pos--;
                    e=*pos;
                } while(!e->m_active || !e->m_enabled);
                SDL_WarpMouseInWindow(appwindow, int(e->m_x+e->m_dx/2),int(e->m_y+e->m_dy/2));
            }
        } // if
    } // if
    
    if (modal!=0) {
        if (modal->check_status(mousex,mousey,button,button_status,k)) ret_val=modal->m_ID;
        if (modal->m_to_be_deleted) to_delete.push_back(modal);
    } else {
        for(BInterfaceElement *e:m_elements) {
            if (e->check_status(mousex,mousey,button,button_status,k)) ret_val=e->m_ID;
            if (e->m_to_be_deleted) to_delete.push_back(e);
        } // while
    } // if
    
    for(BInterfaceElement *e:to_delete) {
        m_elements.remove(e);
        delete e;
    } // while
    
    return ret_val;
} /* BInterface::update_state */


void BInterface::draw(void)
{
    for(BInterfaceElement *e:m_elements) {
        //		output_debug_message("  %p\n",e);
        e->draw();
    }
} /* BInterface::draw */


void BInterface::draw(float alpha)
{
    for(BInterfaceElement *e:m_elements) e->draw(alpha);
} /* BInterface::draw */


void BInterface::createOpenGLBuffers()
{
    for(BInterfaceElement *e:m_elements) e->createOpenGLBuffers();
}


void BInterface::enable(int ID)
{
    for(BInterfaceElement *e:m_elements) {
        if (e->m_ID==ID) e->m_enabled=true;
    } // while
} /* BInterface::enable */


void BInterface::disable(int ID)
{
    for(BInterfaceElement *e:m_elements) {
        if (e->m_ID==ID) e->m_enabled=false;
    } // while
} /* BInterface::disable */


bool BInterface::is_enabled(int ID)
{
    for(BInterfaceElement *e:m_elements) {
        if (e->m_ID==ID) return e->m_enabled;
    } // while
    
    return false;
} /* BInterface::is_enabled */


BInterfaceElement *BInterface::get(int ID)
{
    for(BInterfaceElement *e:m_elements) {
        if (e->m_ID==ID) return e;
    } // while
    return 0;
} /* BInterface::get */


void BInterface::setFocus(int ID)
{
    BInterfaceElement *e = get(ID);
    if (e!=0) {
        SDL_WarpMouseInWindow(appwindow, int(e->m_x+e->m_dx/2),int(e->m_y+e->m_dy/2));
    }
}


void BInterface::clear_print_cache(void)
{
    output_debug_message("Clearing print cache...\n");
    for(PrintBuffer *pb:text_tile_buffer) delete pb;
    text_tile_buffer.clear();
} /* clear_print_cache */



GLTile *BInterface::get_text_tile(const char *text,BitmapFont *font)
{
    s_last_used_font = font;
    
    for(PrintBuffer *pb:text_tile_buffer) {
        //		if (pb->tile->get_texture()<0) output_debug_message("TGLinterface: Weird stuff just happened!\n");
        if (pb->font==font && strcmp(pb->text,text)==0) {
            // Move it the text to the top of the list to indicate that it has been used:
            text_tile_buffer.remove(pb);
            text_tile_buffer.push_back(pb);
            //			if (pb->tile->get_texture()<0) output_debug_message("TGLinterface: Weird stuff just happened (2)!\n");
            return pb->tile;
        } // if
    } // if
    
    while(text_tile_buffer.size()>=TEXT_TILE_BUFFER_SIZE) {
        PrintBuffer *pb=text_tile_buffer.front();
        text_tile_buffer.pop_front();
        delete pb;
    } // while
    
    {
        GLTile *tile;
        SDL_Surface *sfc;
        sfc=font->render(text);
        //		output_debug_message("text surface: %i x %i\n", sfc->w, sfc->h);
        tile=new GLTile(sfc);
        //		tile->set_smooth();
        
        PrintBuffer *pb=new PrintBuffer();
        pb->font=font;
        pb->text=new char[strlen(text)+1];
        strcpy(pb->text,text);
        pb->tile=tile;
        text_tile_buffer.push_back(pb);
        
        //		if (pb->tile->get_texture(0)<0) output_debug_message("TGLinterface: Even weirdest stuff just happened!\n");
        
        return tile;
    }
} /* BInterface::get_text_tile */


void BInterface::print_left(const char *text,BitmapFont *font,float x,float y)
{
    GLTile *tile;
    if (strlen(text)==0) return;
    tile=get_text_tile(text,font);
    tile->set_hotspot_nobb(0,tile->get_dy());
    tile->draw(x,y,0,0,1);
} /* print_left */


void BInterface::print_right(const char *text,BitmapFont *font,float x,float y)
{
    GLTile *tile;
    if (strlen(text)==0) return;
    tile=get_text_tile(text,font);
    tile->set_hotspot_nobb(tile->get_dx(),tile->get_dy());
    tile->draw(x,y,0,0,1);
} /* print_right */


void BInterface::print_center(const char *text,BitmapFont *font,float x,float y)
{
    GLTile *tile;
    if (strlen(text)==0) return;
    tile=get_text_tile(text,font);
    tile->set_hotspot_nobb(tile->get_dx()/2,tile->get_dy());
    tile->draw(x,y,0,0,1);
} /* print_center */


void BInterface::print_left(const char *text,BitmapFont *font,float x,float y,float r,float g,float b,float a)
{
    GLTile *tile;
    if (strlen(text)==0) return;
    tile=get_text_tile(text,font);
    tile->set_hotspot_nobb(0,tile->get_dy());
    tile->draw(r,g,b,a,x,y,0,0,1);
} // print_left


void BInterface::print_right(const char *text,BitmapFont *font,float x,float y,float r,float g,float b,float a)
{
    GLTile *tile;
    if (strlen(text)==0) return;
    tile=get_text_tile(text,font);
    tile->set_hotspot_nobb(tile->get_dx(),tile->get_dy());
    tile->draw(r,g,b,a,x,y,0,0,1);
} // print_center


void BInterface::print_center(const char *text,BitmapFont *font,float x,float y,float r,float g,float b,float a)
{
    GLTile *tile;
    if (strlen(text)==0) return;
    tile=get_text_tile(text,font);
    tile->set_hotspot_nobb(tile->get_dx()/2,tile->get_dy());
    tile->draw(r,g,b,a,x,y,0,0,1);
} // print_center


void BInterface::print_centered(const char *text,BitmapFont *font,float x,float y,float r,float g,float b,float a,float angle,float scale)
{
    GLTile *tile;
    if (strlen(text)==0) return;
    tile=get_text_tile(text,font);
    tile->set_hotspot_nobb(tile->get_dx()/2,tile->get_dy()/2);
    tile->draw(r,g,b,a,x,y,0,angle,scale);
} // print_center 


void BInterface::print_left_multiline(const char *text, BitmapFont *font, float x, float y, float y_interval)
{
    std::vector<char *> lines = splitByLines(text);
    for(char *line:lines) {
        BInterface::print_left(line,font,x,y);
        y+=y_interval;
        delete line;
    }
    lines.clear();
}


void BInterface::push(void) 
{
    for(BInterfaceElement *e:m_elements) e->m_enabled = false;
    std::list<BInterfaceElement *> tmp;
    for(BInterfaceElement *e:m_added_since_last_push) tmp.push_back(e);
    m_stack.push_back(tmp);
    m_added_since_last_push.clear();
}


void BInterface::pop(void)
{
    if (m_stack.empty()) return;
    for(BInterfaceElement *e:m_added_since_last_push) {
        delete e;
        m_elements.remove(e);
    }
    m_added_since_last_push.clear();
    std::list<BInterfaceElement *> tmp = m_stack.back();
    m_stack.pop_back();
    for(BInterfaceElement *e:tmp) {
        e->m_enabled = true;
        m_added_since_last_push.push_back(e);
    }
}


void BInterface::createMenu(const char *options, BitmapFont *font, int x, int y, int dx, int dy, int starting_ID)
{
	std::vector<char *> lines = splitByLines((char *)options);
	BInterface::add_element(new BFrame(x,y,dx,dy));
	int by = y + 10;
	int bdy = font->getHeight();
	for(char *line:lines) {
		BInterface::add_element(new BButtonTransparent((const char *)line,font,x,by,dx,bdy,starting_ID++));
		by += bdy+4;
		delete line;
	}
	lines.clear();
}


void BInterface::createMenu(const char *options, BitmapFont *font, int x, int y, int dx, int dy, int interline_space, int starting_ID)
{
	std::vector<char *> lines = splitByLines((char *)options);
	BInterface::add_element(new BFrame(x,y,dx,dy));
	int by = y + 10;
	int bdy = font->getHeight();
	for(char *line:lines) {
		BInterface::add_element(new BButtonTransparent((const char *)line,font,x,by,dx,bdy,starting_ID++));
		by += bdy+interline_space;
		delete line;
	}
	lines.clear();
}

