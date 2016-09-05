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
#include "SDL/SDL_image.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
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
#include "BB2D.h"
#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
#include "XMLparser.h"
#include "XMLwriter.h"
#include "ExpressionParser.h"

#include "Animation.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Game.h"


Animation::Animation()
{
	m_file = 0;
	m_gf = 0;

	m_dx = m_dy = 1;
	m_period = 8;
	m_looping = false;
	m_length = 1;
	m_sequence = new int[1];
	m_sequence[0] = 1;

	m_cycle = 0;
	m_state = 0;
	m_completed = false;
}


Animation::Animation(int tile, const char *file, A4Game *game)
{
	m_file = new char[strlen(file)+1];
	strcpy(m_file,file);
    if (m_file[0]=='\"') {
        strcpy(m_file,m_file+1);
        m_file[strlen(m_file)-1]=0;
    }
	m_gf = game->getGraphicFile(file);

	m_dx = m_dy = 1;
	m_period = 8;
	m_looping = false;
	m_length = 1;
	m_sequence = new int[1];
	m_sequence[0] = 1;

	m_cycle = 0;
	m_state = 0;
	m_completed = false;
}


Animation::Animation(XMLNode *xml, A4Game *game)
{
	// <animation name="curious" dx="1" dy="1" period="8" looping="true" file="graphics2x.png">74,-1</animation>
	m_file = new char[strlen(xml->get_attribute("file"))+1];
	strcpy(m_file,xml->get_attribute("file"));
	m_gf = game->getGraphicFile(m_file);
	if (m_gf==0) output_debug_message("Animation: cannot get graphic file '%s'\n",m_file);

	m_dx = atoi(xml->get_attribute("dx"));
	m_dy = atoi(xml->get_attribute("dy"));
	m_period = atoi(xml->get_attribute("period"));
	m_looping = false;
	if (strcmp(xml->get_attribute("looping"),"true")==0) m_looping = true;

	// parse sequence:
	char *sequence_text = xml->get_value();
	m_length = 1;
	for(int i = 0;sequence_text[i]!=0;i++) {
		if (sequence_text[i]==',') m_length++;
	}
	m_sequence = new int[m_length];
	int position = 0;
	int j = 0;
	char buffer[256];
	for(int i = 0;sequence_text[i]!=0;i++) {
		if (sequence_text[i]==',') {
//			sequence_text[j++] = 0;
			buffer[j++] = 0;
			m_sequence[position] = atoi(buffer);
//			output_debug_message("tile(%i,xml): '%s'\n",position,buffer);
			position++;
			j = 0;
		} else if (sequence_text[i]!=' ' && sequence_text[i]!='\t' && sequence_text[i]!='\n' && sequence_text[i]!='\r') {
			buffer[j++] = sequence_text[i];
		}
		if (j>=256) {
			output_debug_message("sequence_text longer than 256 characters while reading types definition.");
			break;
		}
	}
	if (j>0) {
		buffer[j++] = 0;
		m_sequence[position] = atoi(buffer);
		position++;
//		output_debug_message("tile(%i,xml): '%s'\n",position,buffer);
	}
	if (m_length!=position) {
		output_debug_message("Animation: m_length != position!!!\n");
	}

	m_cycle = 0;
	m_state = 0;
	m_completed = false;
}


Animation::Animation(Animation *a)
{
	m_file = new char[strlen(a->m_file)+1];
	strcpy(m_file,a->m_file);
	m_gf = a->m_gf;

	m_dx = a->m_dx;
	m_dy = a->m_dy;
	m_period = a->m_period;
	m_looping = a->m_looping;
	m_length = a->m_length;
	m_sequence = new int[a->m_length];
	for(int i = 0;i<m_length;i++) m_sequence[i] = a->m_sequence[i];

	m_cycle = a->m_cycle;
	m_state = a->m_state;
	m_completed = a->m_completed;
}

Animation::Animation(class Expression *exp, A4Game *game)
{
	if (strcmp(exp->get_head(),"Animation.fromTile")==0) {
		std::vector<Expression *>::iterator it = exp->get_parameters()->begin();
//		output_debug_message("Animation: getting tile from '%s'\n",(*it)->get_head());
		int tile = atoi((*it)->get_head());
		it++;
		m_file = new char[strlen((*it)->get_head())+1];
		strcpy(m_file,(*it)->get_head());
		if (m_file[0]=='\"') {
            strcpy(m_file,m_file+1);
			m_file[strlen(m_file)-1]=0;
		}
		m_gf = game->getGraphicFile(m_file);
		if (m_gf==0) output_debug_message("Animation: cannot get graphic file '%s'\n",m_file);

		m_dx = 1;
		m_dy = 1;
		m_period = 1;
		m_looping = false;
		m_length = 1;
		m_sequence = new int[1];
		m_sequence[0] = tile;
//		output_debug_message("tile: %i\n",tile);

		m_cycle = 0;
		m_state = 0;
		m_completed = false;
    } else if (strcmp(exp->get_head(),"Animation.fromTile2")==0) {
            std::vector<Expression *>::iterator it = exp->get_parameters()->begin();
            //		output_debug_message("Animation: getting tile from '%s'\n",(*it)->get_head());
            int tile = atoi((*it)->get_head());
            it++;
            m_file = new char[strlen((*it)->get_head())+1];
            strcpy(m_file,(*it)->get_head());
            if (m_file[0]=='\"') {
                strcpy(m_file,m_file+1);
                m_file[strlen(m_file)-1]=0;
            }
            m_gf = game->getGraphicFile(m_file);
            if (m_gf==0) output_debug_message("Animation: cannot get graphic file '%s'\n",m_file);
            it++;
            m_dx = atoi((*it)->get_head());
            it++;
            m_dy = atoi((*it)->get_head());
            m_period = 1;
            m_looping = false;
            m_length = 1;
            m_sequence = new int[1];
            m_sequence[0] = tile;
            //		output_debug_message("tile: %i\n",tile);
            
            m_cycle = 0;
            m_state = 0;
            m_completed = false;
    } else {
		output_debug_message("Cannot create animation from expression!\n");
		exp->output_debug(0);
		exit(1);
	}
}


Animation::~Animation()
{
	if (m_file!=0) delete m_file;
	m_file = 0;
	m_gf = 0;
	if (m_sequence!=0) delete m_sequence;
	m_sequence = 0;
}



void Animation::reset()
{
	m_cycle = 0;
	m_state = 0;
	m_completed = false;
}


bool Animation::cycle()
{
	m_cycle++;
	if (m_cycle>=m_period) {
		m_cycle-=m_period;
		m_state++;
		if (m_state>=m_length) {
			if (m_looping) {
				m_state = 0;
			} else {
			 	m_state = m_length-1;
			 	m_completed = true;
			}
		}
	}
	return m_completed;
}


void Animation::draw(int x, int y, float zoom)
{
	int t = getTile();
//	output_debug_message("Animation::draw %i\n",t);
	if (t<0) return;
	for(int i = 0;i<m_dy;i++) {
		for(int j = 0;j<m_dx;j++) {
			GLTile *tile = m_gf->getTile(t+j + i*m_gf->getTilesPerRow());
//			output_debug_message("Animation::draw(2) %p\n",tile);
			tile->draw(x+(j*tile->get_dx())*zoom,y+(i*tile->get_dy())*zoom,0,0,zoom);
		}
	}
}


void Animation::draw(int x, int y, float zoom, float r, float g, float b, float a)
{
	int t = getTile();
	if (t<0) return;
	for(int i = 0;i<m_dy;i++) {
		for(int j = 0;j<m_dx;j++) {
			GLTile *tile = m_gf->getTile(t+j + i*m_gf->getTilesPerRow());
			tile->draw(r,g,b,a,x+(j*tile->get_dx())*zoom,y+(i*tile->get_dy())*zoom,0,0,zoom);
		}
	}
}


bool Animation::isCompleted()
{
	return m_completed;
}


int Animation::getPixelWidth() {
	return m_dx*m_gf->m_tile_dx;
}


int Animation::getPixelHeight() {
	return m_dy*m_gf->m_tile_dy;
}


void Animation::saveToXML(class XMLwriter *w, const char *name)
{
    w->openTag("animation");
    w->setAttribute("name", name);
    w->setAttribute("dx", m_dx);
    w->setAttribute("dy", m_dy);
    w->setAttribute("period", m_period);
    w->setAttribute("looping", (m_looping ? "true":"false"));
    w->setAttribute("file", m_file);
    for(int i = 0;i<m_length;i++) {
        if (i==0) {
            w->addContent("%i",m_sequence[i]);
        } else {
            w->addContent(",%i",m_sequence[i]);
        }
    }
    w->closeTag("animation");
}

