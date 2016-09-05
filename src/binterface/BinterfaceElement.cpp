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
#include "Animation.h"

#include "Binterface.h"



float color1_r=1,color1_g=1,color1_b=1;					// TEXT
float color2_r=0.15f,color2_g=0.15f,color2_b=0.15f;		// FRAME_BG
float color3_r=1,color3_g=1,color3_b=1;					// FRAME_FG

/*
float color4_r=0.25f,color4_g=0.25f,color4_b=0.25f;		// BUTTON STD
float color5_r=0.5f,color5_g=0.5f,color5_b=0.5f;		// BUTTON MOUSE
float color6_r=0.75f,color6_g=0.75f,color6_b=0.75f;		// BUTTON PRESSED
*/

float color4_r=0.16f,color4_g=0.16f,color4_b=0.33f;		// BUTTON STD
float color5_r=0.33f,color5_g=0.33f,color5_b=0.66f;		// BUTTON MOUSE
float color6_r=0.66f,color6_g=0.66f,color6_b=0.85f;		// BUTTON PRESSED


BInterfaceElement::BInterfaceElement() 
{
	m_ID=-1;
	m_enabled=false;
	m_active=false;
	m_modal=false;
	m_to_be_deleted=false;
	m_x=m_y=m_dx=m_dy=0;
    
    createOpenGLBuffers();
} /* BInterfaceElement::BInterfaceElement */


void BInterfaceElement::createOpenGLBuffers(void)
{
    vPositionLocation = UVLocation = -1;
}


BInterfaceElement::~BInterfaceElement() 
{
} /* BInterfaceElement::~BInterfaceElement */ 


bool BInterfaceElement::mouse_over(int mousex,int mousey)
{
	return false;
}


bool BInterfaceElement::check_status(int mousex,int mousey,int button,int button_status,KEYBOARDSTATE *k)
{
	return false;
} /* BInterfaceElement::check_status */ 


void BInterfaceElement::draw(void)
{
} /* BInterfaceElement::draw */ 


void BInterfaceElement::draw(float alpha)
{
} /* BInterfaceElement::draw */ 


std::list<BInterfaceElement *> BInterfaceElement::getChildren(void)
{
	std::list<BInterfaceElement *> tmp;
	return tmp;
}


BText::BText(const char *text,BitmapFont *font,float x,float y,bool centered)
{
	m_centered=centered;
	m_text=new char[strlen(text)+1];
	strcpy(m_text,text);
	m_font=font;
	m_x=x;
	m_y=y;
	m_enabled=true;
	m_active=false;
} // BText::BText 


BText::BText(const char *text,BitmapFont *font,float x,float y,bool centered,int ID) : BInterfaceElement()
{
	m_ID = ID;
	m_centered=centered;
	m_text=new char[strlen(text)+1];
	strcpy(m_text,text);
	m_font=font;
	m_x=x;
	m_y=y;
	m_enabled=true;
	m_active=false;
} // BText::BText 


BText::~BText()
{
	delete []m_text;
	m_text=0;

} // BText::~BText 


void BText::draw(void)
{
	if (m_centered) {
		if (m_enabled) BInterface::print_center(m_text,m_font,m_x,m_y+(m_font->getHeight())/2,color1_r,color1_g,color1_b,1);
				  else BInterface::print_center(m_text,m_font,m_x,m_y+(m_font->getHeight())/2,0.33f*color1_r,0.33f*color1_g,0.33f*color1_b,1);
	} else {
		if (m_enabled) BInterface::print_left(m_text,m_font,m_x,m_y+(m_font->getHeight())/2);
				  else BInterface::print_left(m_text,m_font,m_x,m_y+(m_font->getHeight())/2,0.33f,0.33f,0.33f,1);
	} // if 

} // BText::draw 


void BText::draw(float alpha)
{
	if (m_centered) {
		if (m_enabled) BInterface::print_center(m_text,m_font,m_x,m_y+(m_font->getHeight())/2,color1_r,color1_g,color1_b,alpha);
				  else BInterface::print_center(m_text,m_font,m_x,m_y+(m_font->getHeight())/2,0.33f*color1_r,0.33f*color1_g,0.33f*color1_b,alpha);
	} else {
		if (m_enabled) BInterface::print_left(m_text,m_font,m_x,m_y+(m_font->getHeight())/2,color1_r,color1_g,color1_b,alpha);
				  else BInterface::print_left(m_text,m_font,m_x,m_y+(m_font->getHeight())/2,0.33f*color1_r,0.33f*color1_g,0.33f*color1_b,alpha);
	} // if 

} // BText::draw 


void BText::set_text(const char *text)
{
	if (m_text!=0) 	delete []m_text;
	m_text=new char[strlen(text)+1];
	strcpy(m_text,text);
} // BText::set_text


BInterfaceTile::BInterfaceTile(GLTile *t,float x,float y)
{
    m_tile = t;
    m_x=x;
    m_y=y;
    m_enabled=true;
    m_active=false;
}


BInterfaceTile::BInterfaceTile(GLTile *t,float x,float y,int ID) : BInterfaceElement()
{
    m_ID = ID;
    m_tile = t;
    m_x=x;
    m_y=y;
    m_enabled=true;
    m_active=false;
}


BInterfaceTile::~BInterfaceTile()
{
    m_tile = 0;
}


void BInterfaceTile::draw(void)
{
    if (m_enabled) m_tile->draw(m_x,m_y, 0, 0, 1);
    else m_tile->draw(m_x,m_y, 0, 0, 1, 0.33f,0.33f,0.33f,1);
    
}


void BInterfaceTile::draw(float alpha)
{
    if (m_enabled) m_tile->draw(m_x,m_y, 0, 0, 1, 1, 1, 1, alpha);
    else m_tile->draw(m_x,m_y, 0, 0, 1, 0.33f,0.33f,0.33f,alpha);
}


BInterfaceAnimation::BInterfaceAnimation(Animation *t,float x,float y)
{
    m_animation = t;
    m_x=x;
    m_y=y;
    m_enabled=true;
    m_active=false;
}


BInterfaceAnimation::BInterfaceAnimation(Animation *t,float x,float y,int ID) : BInterfaceElement()
{
    m_ID = ID;
    m_animation = t;
    m_x=x;
    m_y=y;
    m_enabled=true;
    m_active=false;
}


BInterfaceAnimation::~BInterfaceAnimation()
{
    delete m_animation;
    m_animation = 0;
}


void BInterfaceAnimation::draw(void)
{
    if (m_enabled) m_animation->draw(m_x, m_y, 1);
    else m_animation->draw(m_x,m_y, 1, 0.33f,0.33f,0.33f,1);
    
}


void BInterfaceAnimation::draw(float alpha)
{
    if (m_enabled) m_animation->draw(m_x,m_y, 1, 1, 1, 1, alpha);
    else m_animation->draw(m_x,m_y, 1, 0.33f,0.33f,0.33f,alpha);
}



BButton::BButton(const char *text,BitmapFont *font,float x,float y,float dx,float dy,int ID)  : BInterfaceElement()
{
	m_text=new char[strlen(text)+1];
	strcpy(m_text,text);
	m_font=font;
	m_x=x;
	m_y=y;
	m_dx=dx;
	m_dy=dy;
	m_ID=ID;
	m_status=0;
	m_enabled=true;
	m_tile=0;
	m_active=true;
	m_cycle=0;

	createOpenGLBuffers();
} // BButton::BButton 


BButton::BButton(GLTile *icon,float x,float y,float dx,float dy,int ID)  : BInterfaceElement()
{
	m_text=0;
	m_font=0;
	m_x=x;
	m_y=y;
	m_dx=dx;
	m_dy=dy;
	m_ID=ID;
	m_status=0;
	m_enabled=true;
	m_tile=icon;
	m_active=true;
	m_cycle=0;

	createOpenGLBuffers();
} // BButton::BButton 


void BButton::createOpenGLBuffers(void)
{
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	GLfloat *g_vertex_buffer_data = new GLfloat[36]{
			m_x+3,m_y,0,
			m_x+m_dx-3,m_y,0,
			m_x+m_dx-1,m_y+1,0,
			m_x+m_dx,m_y+3,0,
			m_x+m_dx,m_y+m_dy-3,0,
			m_x+m_dx-1,m_y+m_dy-1,0,
			m_x+m_dx-3,m_y+m_dy,0,
			m_x+3,m_y+m_dy,0,
			m_x+1,m_y+m_dy-1,0,
			m_x,m_y+m_dy-3,0,
			m_x,m_y+3,0,
			m_x+1,m_y+1,0
	};
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*36, g_vertex_buffer_data, GL_STATIC_DRAW);
	//output_debug_message("Button created: '%s' %i %i [%lu]\n",m_text, VertexArrayID, vertexbuffer, sizeof(GLfloat)*36);
	delete []g_vertex_buffer_data;
}



BButton::~BButton()
{
	// Only delete the tile if it has been created here:
	if (m_text!=0) {
		if (m_tile!=0) delete m_tile;
		m_tile=0;
	} // if 

	if (m_text!=0) delete []m_text;
	m_text=0;

   	glDeleteBuffers(1,&vertexbuffer);
#ifdef __APPLE__
   	glDeleteVertexArraysAPPLE(1,&VertexArrayID);
#else
   	glDeleteVertexArrays(1,&VertexArrayID);
#endif

} // BButton::~BButton 


void BButton::changeText(char *new_text)
{
    if (m_text!=0) delete []m_text;
    m_text= new char[strlen(new_text)+1];
    strcpy(m_text, new_text);
}


void BButton::draw(void)
{
	draw(1);
} // BButton::draw 


void BButton::draw(float alpha)
{
	if (!m_enabled) alpha/=3;

	switch(m_status) {
	case 1: glUniform4f(inColorID, color5_r,color5_g,color5_b,alpha);
			break;
	case 2: glUniform4f(inColorID, color6_r,color6_g,color6_b,alpha);
			break;
	default:glUniform4f(inColorID, color4_r,color4_g,color4_b,alpha);
	} // switch

	glm::mat4 tmp = glm::mat4(1.0f);
	glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &tmp[0][0]);
    if (vPositionLocation==-1) {
        vPositionLocation = glGetAttribLocation(programID, "vPosition");
        UVLocation = glGetAttribLocation(programID, "UV");
    }
	glUniform1i(useTextureID, 0);
	glBindVertexArray(VertexArrayID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glEnableVertexAttribArray(vPositionLocation);
	glEnableVertexAttribArray(UVLocation);
	glVertexAttribPointer(vPositionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
	glVertexAttribPointer(UVLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 12);
	glDisableVertexAttribArray(vPositionLocation);
	glDisableVertexAttribArray(UVLocation);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (m_tile==0) {
		BInterface::print_center(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2+(m_font->getHeight())/2,color1_r,color1_g,color1_b,alpha);
	} else {
		m_tile->draw(color1_r,color1_g,color1_b,alpha,m_x+m_dx/2,m_y+m_dy/2,0,0,1);
	} // if 
//	output_debug_message("button draw: %i\n", m_ID);
} // BButton::draw 


bool BButton::mouse_over(int mousex,int mousey)
{
	if (mousex>=m_x && mousex<m_x+m_dx &&
		mousey>=m_y && mousey<m_y+m_dy) return true;
	return false;
}


bool BButton::check_status(int mousex,int mousey,int button,int button_status,KEYBOARDSTATE *k)
{
	m_cycle++;

	if (!m_enabled) {
		m_status=BBUTTON_STATE_NORMAL;
		return false;
	} // if

	if (mousex>=m_x && mousex<m_x+m_dx &&
		mousey>=m_y && mousey<m_y+m_dy) {
		if (button==BBUTTON_STATE_MOUSEOVER) {
			m_status=BBUTTON_STATE_PRESSED;
			return true;
		} else {
			m_status=BBUTTON_STATE_MOUSEOVER;
		} // if 
	} else {
		m_status=0;
	} // if 

	return false;
} // BButton::check_status 



BButtonTransparent::BButtonTransparent(const char *text,BitmapFont *font,float x,float y,float dx,float dy,int ID) : BButton(text,font,x,y,dx,dy,ID)
{
    r = g = b = a = 1;
} // BButtonTransparent::BButtonTransparent 


BButtonTransparent::BButtonTransparent(const char *text,BitmapFont *font,float x,float y,float dx,float dy,int ID, float a_r, float a_g, float a_b, float a_a) : BButton(text,font,x,y,dx,dy,ID)
{
    r = a_r;
    g = a_g;
    b = a_b;
    a = a_a;
}


void BButtonTransparent::draw(float alpha)
{
	if (!m_enabled) alpha/=3;
	float f=float(0.5+0.3*sin((m_cycle)*0.3F));
	switch(m_status) {
	case BBUTTON_STATE_MOUSEOVER: 
//			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,0,0,0,alpha*0.5f,0,1.25f);
//			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,1,1,1,alpha*0.5f,0,1.1f);
//			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,1,1,1,alpha,0,1.05f);
//			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,0.75f,0.75f,0.75f,alpha,0,1);
			//output_debug_message("transparent %g,%g -> %i (%g)\n",m_x,m_y,m_cycle,f);
			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,r*f,g*f,b*0.75f,a*alpha*f,0,1);
			break;				
	case BBUTTON_STATE_PRESSED: 
//			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,0,0,0,alpha*0.5f,0,1.25f);
//			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,1,1,1,alpha*0.5f,0,1.1f);
//			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,1,1,1,alpha,0,1.05f);
//			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,1,1,1,alpha,0,1);
			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,r,g,b,a*alpha,0,1);
			break;
	default://BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,0,0,0,alpha*0.5f,0,1.15f);
			BInterface::print_centered(m_text,m_font,m_x+m_dx/2,m_y+m_dy/2,0.75f*r,0.75f*g,0.75f*b,a*alpha,0,1);

	} // switch

} // BButtonTransparent::draw 


BQuad::BQuad(float x,float y,float dx,float dy, float a_r, float a_g, float a_b, float a_a) : BInterfaceElement()
{
    r = a_r;
    g = a_g;
    b = a_b;
    a = a_a;
    m_x=x;
    m_y=y;
    m_dx=dx;
    m_dy=dy;
    m_ID=-1;
    m_enabled=true;
    m_active=false;
    
    createOpenGLBuffers();
    //	output_debug_message("BQuad created");
} // BQuad::BQuad


void BQuad::createOpenGLBuffers(void)
{
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    GLfloat *g_vertex_buffer_data = new GLfloat[3*4]{
        m_x,m_y,0,
        m_x+m_dx,m_y,0,
        m_x+m_dx,m_y+m_dy,0,
        m_x,m_y+m_dy,0,
    };
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*4, g_vertex_buffer_data, GL_STATIC_DRAW);
    delete []g_vertex_buffer_data;
    
    //	output_debug_message("BQuad::createOpenGLBuffers completed (%g,%g,%g,%g)\n",m_x, m_y, m_dx, m_dy);
}


BQuad::~BQuad()
{
   	glDeleteBuffers(1,&vertexbuffer);
#ifdef __APPLE__
   	glDeleteVertexArraysAPPLE(1,&VertexArrayID);
#else
   	glDeleteVertexArrays(1,&VertexArrayID);
#endif
} // BQuad::~BQuad


void BQuad::draw(void)
{
    draw(1);
} // BFrame::draw


void BQuad::draw(float alpha)
{
    if (!m_enabled) alpha*=0.5;
    
    glm::mat4 tmp = glm::mat4(1.0f);
    glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &tmp[0][0]);
    if (vPositionLocation==-1) {
        vPositionLocation = glGetAttribLocation(programID, "vPosition");
        UVLocation = glGetAttribLocation(programID, "UV");
    }
    glUniform1i(useTextureID, 0);
    glBindVertexArray(VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glEnableVertexAttribArray(vPositionLocation);
    glEnableVertexAttribArray(UVLocation);
    glVertexAttribPointer(vPositionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
    glVertexAttribPointer(UVLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
    
    glUniform4f(inColorID, r, g, b, a*alpha);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glDisableVertexAttribArray(vPositionLocation);
    glDisableVertexAttribArray(UVLocation);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
} // BQuad::draw

BFrame::BFrame(float x,float y,float dx,float dy) : BInterfaceElement()
{
	m_x=x;
	m_y=y;
	m_dx=dx;
	m_dy=dy;
	m_ID=-1;
	m_enabled=true;
	m_active=false;

	createOpenGLBuffers();
//	output_debug_message("BFrame created");
} // BFrame::BFrame 


void BFrame::createOpenGLBuffers(void)
{
	GLfloat bar_height=6;
	GLfloat new_y = m_y + m_dy - bar_height;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	GLfloat *g_vertex_buffer_data = new GLfloat[3*36]{
			m_x+2,m_y,0,
			m_x+m_dx-2,m_y,0,
			m_x+m_dx-2,m_y+m_dy,0,
			m_x+2,m_y+m_dy,0,

			m_x+2,m_y,0,
			m_x+m_dx-1,m_y,0,
			m_x+m_dx,m_y+1,0,
			m_x+m_dx,m_y+bar_height-1,0,
			m_x+m_dx-1,m_y+bar_height,0,
			m_x+2,m_y+bar_height,0,
			m_x,m_y+bar_height-1,0,
			m_x,m_y+1,0,

			m_x+3,m_y,0,
			m_x+m_dx-2,m_y,0,
			m_x+m_dx-1,m_y+1,0,
			m_x+m_dx-1,m_y+bar_height-1,0,
			m_x+m_dx-2,m_y+bar_height,0,
			m_x+3,m_y+bar_height,0,
			m_x+1,m_y+bar_height-1,0,
			m_x+1,m_y+1,0,

			m_x+2,new_y,0,
			m_x+m_dx-1,new_y,0,
			m_x+m_dx,new_y+1,0,
			m_x+m_dx,new_y+bar_height-1,0,
			m_x+m_dx-1,new_y+bar_height,0,
			m_x+2,new_y+bar_height,0,
			m_x,new_y+bar_height-1,0,
			m_x,new_y+1,0,

			m_x+3,new_y,0,
			m_x+m_dx-2,new_y,0,
			m_x+m_dx-1,new_y+1,0,
			m_x+m_dx-1,new_y+bar_height-1,0,
			m_x+m_dx-2,new_y+bar_height,0,
			m_x+3,new_y+bar_height,0,
			m_x+1,new_y+bar_height-1,0,
			m_x+1,new_y+1,0,
	};
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*36, g_vertex_buffer_data, GL_STATIC_DRAW);	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    delete []g_vertex_buffer_data;

//	output_debug_message("BFrame::createOpenGLBuffers completed (%g,%g,%g,%g)\n",m_x, m_y, m_dx, m_dy);
}


BFrame::~BFrame()
{
   	glDeleteBuffers(1,&vertexbuffer);
#ifdef __APPLE__
   	glDeleteVertexArraysAPPLE(1,&VertexArrayID);
#else
   	glDeleteVertexArrays(1,&VertexArrayID);
#endif
} // BFrame::~BFrame 


void BFrame::setX(int x)
{
    m_x = x;
    recreateBuffers();
}


void BFrame::setY(int y)
{
    m_y = y;
    recreateBuffers();
}


void BFrame::setDx(int dx)
{
    m_dx = dx;
    recreateBuffers();
}


void BFrame::setDy(int dy)
{
    m_dy = dy;
    recreateBuffers();
}


void BFrame::recreateBuffers()
{
   	glDeleteBuffers(1,&vertexbuffer);
#ifdef __APPLE__
   	glDeleteVertexArraysAPPLE(1,&VertexArrayID);
#else
   	glDeleteVertexArrays(1,&VertexArrayID);
#endif
    createOpenGLBuffers();
}


void BFrame::draw(void)
{
	if (!m_enabled) draw(0.5f);
			   else draw(1);
} // BFrame::draw  


void BFrame::draw(float alpha)
{
	if (!m_enabled) alpha*=0.5;

	glm::mat4 tmp = glm::mat4(1.0f);
	glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &tmp[0][0]);
    if (vPositionLocation==-1) {
        vPositionLocation = glGetAttribLocation(programID, "vPosition");
        UVLocation = glGetAttribLocation(programID, "UV");
    }
	glUniform1i(useTextureID, 0);
	glBindVertexArray(VertexArrayID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glEnableVertexAttribArray(vPositionLocation);
	glEnableVertexAttribArray(UVLocation);
	glVertexAttribPointer(vPositionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
	glVertexAttribPointer(UVLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);

	glUniform4f(inColorID, color2_r,color2_g,color2_b,0.8f*alpha);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform4f(inColorID, color3_r*0.5f,color3_g*0.5f,color3_b*0.5f,alpha);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 8);

	glUniform4f(inColorID, color3_r,color3_g,color3_b,alpha);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 8);

	glUniform4f(inColorID, color3_r*0.5f,color3_g*0.5f,color3_b*0.5f,alpha);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 8);

	glUniform4f(inColorID, color3_r,color3_g,color3_b,alpha);
	glDrawArrays(GL_TRIANGLE_FAN, 28, 8);

	glDisableVertexAttribArray(vPositionLocation);
	glDisableVertexAttribArray(UVLocation);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
} // BFrame::draw  

