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

#include <vector>
#include "sound.h"
#include "keyboardstate.h"
#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
#include "BitmapFont.h"

#include "A4auxiliar.h"
#include "A4EngineApp.h"

#include "Binterface.h"


extern int SCREEN_Y;
extern float color1_r,color1_g,color1_b;
extern float color2_r,color2_g,color2_b;
extern float color3_r,color3_g,color3_b;


BTextFrame::BTextFrame(const char *initial_text,bool a_centered,BitmapFont *font,float x,float y,float dx,float dy) : BFrame(x,y,dx,dy)
{
	text = splitByLines((char *)initial_text);

	m_font=font;
	centered = a_centered;
}


BTextFrame::~BTextFrame()
{
	for(char *l:text) delete l;
	text.clear();
}


void BTextFrame::draw(float alpha)
{
	BFrame::draw(alpha);
	int x = m_x + 10;
	int y = m_y + 10 + m_font->getHeight();
	if (centered) x = m_x + m_dx/2;
	for(char *l:text) {
		if (centered) {
			BInterface::print_center(l,m_font,x,y,1,1,1,alpha);
		} else {
			BInterface::print_left(l,m_font,x,y,1,1,1,alpha);
		}
		y += (m_font->getHeight())+4;
	}
//	output_debug_message("drawing text frame %g!\n", alpha);
}




BTextInputFrame::BTextInputFrame(const char *question,const char *initial_text,int max_characters,BitmapFont *font,float x,float y,float dx,float dy,int ID) : BFrame(x,y,dx,dy)
{
	m_question = (char *)question;
	if (initial_text==0) {
		m_editing=new char[max_characters+1];
		m_editing[0]=0;
		m_editing_position=0;
	} else {
		m_editing=new char[max_characters+1];
		strcpy(m_editing,initial_text);
		m_editing_position=(int)strlen(m_editing);
	} // if 
	m_font=font;
	m_focus=false;
	m_cycle=0;
	m_max_characters=max_characters;
	m_ID=ID;
	m_active=true;
	m_change_in_last_cycle=false;
	
	cursor_dx = 4.0f;
	cursor_dy = (m_font->getHeight())-4.0f;

	createOpenGLBuffers();

//	output_debug_message("BTextInputFrame created");
} /* BTextInputFrame::BTextInputFrame */ 


BTextInputFrame::BTextInputFrame(const char *initial_text,int max_characters,BitmapFont *font,float x,float y,float dx,float dy,int ID) : BFrame(x,y,dx,dy)
{
	m_question = 0;
	if (initial_text==0) {
		m_editing=new char[max_characters+1];
		m_editing[0]=0;
		m_editing_position=0;
	} else {
		m_editing=new char[max_characters+1];
		strcpy(m_editing,initial_text);
		m_editing_position=(int)strlen(m_editing);
	} // if 
	m_font=font;
	m_focus=false;
	m_cycle=0;
	m_max_characters=max_characters;
	m_ID=ID;
	m_active=true;
	m_change_in_last_cycle=false;
	
	cursor_dx = 4.0f;
	cursor_dy = (m_font->getHeight())-4.0f;

	createOpenGLBuffers();

//	output_debug_message("BTextInputFrame created");
} /* BTextInputFrame::BTextInputFrame */ 


void BTextInputFrame::createOpenGLBuffers(void)
{
	BFrame::createOpenGLBuffers();
	glGenVertexArrays(1, &VertexArrayID2);
	glBindVertexArray(VertexArrayID2);
	GLfloat *g_vertex_buffer_data = new GLfloat[3*4]{
			0,0,0,
			cursor_dx,0,0,
			cursor_dx,cursor_dy,0,
			0,cursor_dy,0,
	};
	glGenBuffers(1, &vertexbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*4, g_vertex_buffer_data, GL_STATIC_DRAW);	
    delete []g_vertex_buffer_data;
}


BTextInputFrame::~BTextInputFrame()
{
	if (m_editing!=0) delete []m_editing;
	m_editing=0;
} /* BTextInputFrame::~BTextInputFrame */ 


void BTextInputFrame::draw(void)
{
	if (!m_enabled) draw(0.5f);
			   else draw(1);
} /* BTextInputFrame::draw */ 


void BTextInputFrame::draw(float alpha)
{
	BFrame::draw(alpha);

    GLint bb[4];
    glGetIntegerv(GL_SCISSOR_BOX, bb);
	glScissor(int(m_x+4),int(SCREEN_Y-(m_y+4+(m_dy-8))),int(m_dx-8),int(m_dy-8));
    glEnable(GL_SCISSOR_TEST);

	float text_y = m_y+(m_dy/2)+(m_font->getHeight())/2;

    if (m_question!=0) {
    	text_y = m_y+(m_dy/2)-2;
    	if (m_enabled) {
			BInterface::print_left(m_question,m_font,m_x+8,text_y,color1_r,color1_g,color1_b,1);
		} else {
			BInterface::print_left(m_question,m_font,m_x+8,text_y,color1_r,color1_g,color1_b,0.5);
		}
		text_y+=m_font->getHeight()+4;
    }

	// draw the cursor:
	if (m_focus) {
		float start_x = 0,start_y = 0;
		float cursor_x,cursor_y;
		char *tmp = new char[m_max_characters+1];

		strcpy(tmp,m_editing);
		tmp[m_editing_position]=0;
		int tdx = m_font->getTextWidth(tmp);

		cursor_x = m_x+8+tdx;
		cursor_y = text_y-(m_font->getHeight())+2;

		if (cursor_x+cursor_dx>=m_x+m_dx-4) start_x = (m_x+m_dx-4)-(cursor_x + cursor_dx);
		if (cursor_y+cursor_dy>=m_y+m_dy-4) start_y = (m_y+m_dy-4)-(cursor_y + cursor_dy);

		if (m_enabled) {
			BInterface::print_left(m_editing,m_font,m_x+8+start_x,text_y+start_y,color1_r,color1_g,color1_b,1);
		} else {
			BInterface::print_left(m_editing,m_font,m_x+8+start_x,text_y+start_y,color1_r,color1_g,color1_b,0.5f);
		} // if 
		{			
			float f;
			f=float(0.5+0.3*sin((m_cycle)*0.3F));
			glm::mat4 tmp = glm::mat4(1.0f);
			tmp = glm::translate(tmp, glm::vec3(cursor_x,cursor_y,0));
			glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &tmp[0][0]);
			int loc1 = glGetAttribLocation(programID, "vPosition");		
			int loc2 = glGetAttribLocation(programID, "UV");
			glBindVertexArray(VertexArrayID2);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
			glEnableVertexAttribArray(loc1);
			glEnableVertexAttribArray(loc2);
			glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
			glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
			glUniform4f(inColorID, 1,0,0,f);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDisableVertexAttribArray(loc1);
			glDisableVertexAttribArray(loc2);			
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		delete []tmp;
	} else {
		if (m_enabled) {
			BInterface::print_left(m_editing,m_font,m_x+8,text_y,color1_r,color1_g,color1_b,1);
		} else {
			BInterface::print_left(m_editing,m_font,m_x+8,text_y,color1_r,color1_g,color1_b,0.5f);
		} // if 
	} // if
    glDisable(GL_SCISSOR_TEST);
	glScissor(bb[0],bb[1],bb[2],bb[3]);

} /* BTextInputFrame::draw */ 


bool BTextInputFrame::mouse_over(int mousex,int mousey)
{
	if (mousex>=m_x && mousex<m_x+m_dx &&
		mousey>=m_y && mousey<m_y+m_dy) return true;
	return false;
}


bool BTextInputFrame::check_status(int mousex,int mousey,int button,int button_status,KEYBOARDSTATE *k)
{
	m_cycle++;

	m_change_in_last_cycle=false;

	if (m_enabled) {
		if (button!=0) {
			if (mousex>=m_x && mousex<m_x+m_dx &&
				mousey>=m_y && mousey<m_y+m_dy) m_focus=true;
										   else m_focus=false;
		} // if 

		if (m_focus) {
			unsigned int pep=m_editing_position, el=(int)strlen(m_editing);

			string_editor_cycle(m_editing,&m_editing_position,m_max_characters,k);

			if (m_editing_position!=pep || strlen(m_editing)!=el) {
				m_change_in_last_cycle=true;
				// cache the graphic tile for the text for not having to create it during the redraw:
				BInterface::get_text_tile(m_editing,m_font);
			}

			if (k->key_press(SDL_SCANCODE_RETURN)) {
				return true;
			}
		} // if 
	} // if

	return false;
} /* BTextInputFrame::check_status */ 


void BTextInputFrame::string_editor_cycle(char *editing,unsigned int *editing_position,unsigned int max_length,KEYBOARDSTATE *k)
{
	for(SDL_Keysym ke:k->keyevents) {
		if (ke.scancode==SDL_SCANCODE_BACKSPACE) {
			if ((*editing_position)>0) {
				strcpy(editing+(*editing_position)-1,editing+(*editing_position));
				(*editing_position)--;
			} // if
		} // if

		if (ke.scancode==SDL_SCANCODE_DELETE) {
			if ((*editing_position)<strlen(editing)) {
				strcpy(editing+(*editing_position),editing+(*editing_position)+1);
			} // if
		} // if

		if (ke.scancode==SDL_SCANCODE_LEFT) {
			if ((*editing_position)>0) {
				(*editing_position)--;
			} // if
		} // if
        if (ke.scancode==SDL_SCANCODE_RIGHT) {
			if ((*editing_position)<strlen(editing)) {
				(*editing_position)++;
			} // if
		} // if

		if (ke.scancode==SDL_SCANCODE_HOME) (*editing_position)=0;
		if (ke.scancode==SDL_SCANCODE_END) (*editing_position)=(unsigned int)strlen(editing);
	} // while
	k->keyevents.clear();
    
    for(SDL_TextInputEvent ti:k->textevents) {
        for(int i = 0;ti.text[i]!=0;i++) {
            if (strlen(editing)<max_length) {
                if ((*editing_position)<strlen(editing)) {
                    for(unsigned int j= (unsigned)strlen(editing)+1;j>(*editing_position);j--) editing[j]=editing[j-1];
                    editing[(*editing_position)]=ti.text[i];
                    (*editing_position)++;
                } else {
                    editing[(*editing_position)]=ti.text[i];
                    (*editing_position)++;
                    editing[(*editing_position)]=0;
                } // if
            }
        }
    }
    k->textevents.clear();
} // BTextInputFrame::string_editor_cycle
