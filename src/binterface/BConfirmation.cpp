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

#include "A4EngineApp.h"

#include "Binterface.h"

extern int SCREEN_X, SCREEN_Y;

BConfirmation::BConfirmation(const char *message,BitmapFont *font,float x,float y,int ID,bool cancelOption) : BInterfaceElement()
{
	m_font = font;
	{
		char tmp[256];
		int i,j=0;
		for(i=0;message[i]!=0;i++) {
			if (message[i]=='\n') {
				tmp[j]=0;
				char *tmp2 = new char[strlen(tmp)+1];
				strcpy(tmp2,tmp);
				m_message_lines.push_back(tmp2);
				j = 0;
			} else {
				tmp[j++] = message[i];
			}
		}
		if (j!=0) {
			tmp[j]=0;
			char *tmp2 = new char[strlen(tmp)+1];
			strcpy(tmp2,tmp);
			m_message_lines.push_back(tmp2);
		}
	}
	
	m_x=x;
	m_y=y;
	m_ID=ID;
	m_enabled=true;
	m_active=true;
	m_modal=true;
	m_state=0;
	m_cycle=0;

	if (cancelOption) {
		m_ok_button = new BButton("ok",font,x-100,y+10,80,32,-1);
		m_cancel_button = new BButton("cancel",font,x+20,y+10,80,32,-1);
	} else {
		m_ok_button = new BButton("ok",font,x-40,y+10,80,32,-1);
		m_cancel_button = NULL;
	}

	createOpenGLBuffers();
} /* BConfirmation::BConfirmation */ 


void BConfirmation::createOpenGLBuffers(void) 
{
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	GLfloat *g_vertex_buffer_data = new GLfloat[3*4]{
			0,0,0,
			(GLfloat)SCREEN_X,0,0,
			(GLfloat)SCREEN_X,(GLfloat)SCREEN_Y,0,
			0,(GLfloat)SCREEN_Y,0,
	};
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*4, g_vertex_buffer_data, GL_STATIC_DRAW);
    delete []g_vertex_buffer_data;
}


BConfirmation::~BConfirmation()
{
	delete m_ok_button;
	delete m_cancel_button;
	for(char *message:m_message_lines) {
		delete message;
	}
	m_message_lines.clear();
} /* BConfirmation::~BConfirmation */ 


void BConfirmation::draw(void)
{
	draw(1);
} /* BConfirmation::draw */ 


void BConfirmation::draw(float alpha)
{
	if (!m_enabled) alpha*=0.33f;

	if (m_state==0 && m_cycle<25) alpha*=(m_cycle/25.0f);
	if (m_state==1) {
		if (m_cycle<25) {
			alpha*=((25-m_cycle)/25.0f);
		} else {
			alpha=0;
		} // if 
	} // if

    {
		glm::mat4 tmp = glm::mat4(1.0f);
		glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &tmp[0][0]);
		int loc1 = glGetAttribLocation(programID, "vPosition");		
		int loc2 = glGetAttribLocation(programID, "UV");
		glUniform1i(useTextureID, 0);
		glBindVertexArray(VertexArrayID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glEnableVertexAttribArray(loc1);
		glEnableVertexAttribArray(loc2);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
		glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
        glUniform4f(inColorID, 0, 0, 0, alpha*0.75f);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glDisableVertexAttribArray(loc1);
		glDisableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
    }
	int y = m_y - (m_font->getHeight())*m_message_lines.size();
    glUniform4f(inColorID, 1, 1, 1, alpha);
	for(char *message:m_message_lines) {
		BInterface::print_center(message,m_font,m_x,y);
		y+=m_font->getHeight();
	}

	m_ok_button->draw(alpha);
	if (m_cancel_button!=NULL) m_cancel_button->draw(alpha);

} /* BConfirmation::draw */ 



bool BConfirmation::check_status(int mousex,int mousey,int button,int button_status,KEYBOARDSTATE *k)
{
	if (!m_enabled) {
		return false;
	} // if

	m_cycle++;

	if (m_ok_button->check_status(mousex,mousey,button,button_status,k)) {
		m_state = 1;
		m_cycle=0;
		return true;
	} // if 
	if (m_cancel_button!=NULL && m_cancel_button->check_status(mousex,mousey,button,button_status,k)) {
		m_state = 1;
		m_cycle=0;
	} // if 

	if (m_state == 1 && m_cycle>25) m_to_be_deleted = true;

	return false;
} /* BConfirmation::check_status */ 


std::list<BInterfaceElement *> BConfirmation::getChildren(void)
{
	std::list<BInterfaceElement *> tmp;
	if (m_ok_button!=0) tmp.push_back(m_ok_button);
	if (m_cancel_button!=0) tmp.push_back(m_cancel_button);
	return tmp;
}
