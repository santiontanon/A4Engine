#ifdef WIN32
#include "windows.h"
#else
#include <sys/time.h>
#include <time.h>
#endif
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "math.h"
#include <vector>

#include "debug.h"

#include "ExpressionParser.h"


Expression::Expression() {
    quoted = false;
}

Expression::~Expression() {
	if (m_head!=0) delete m_head;
	m_head = 0;
	for(Expression *e:m_parameters) delete e;
	m_parameters.clear();
}

void Expression::output_debug(int tabs)
{
	for(int i = 0;i<tabs;i++) output_debug_message("  ");
	output_debug_message("%s\n", m_head);
	for(Expression *e:m_parameters) {
		e->output_debug(tabs+1);
	}
}


Expression *Expression::from_string(char *string) {
	int pos = 0;
	return from_string(string,pos);
}

Expression *Expression::from_string(char *string, int &pos) {
	int pos0 = pos;

//	output_debug_message("Expression::from_string: %s\n",string+pos);

	char *token = next_token(string,pos);
	if (token==0) return 0;
	std::vector<Expression *> stack;
	Expression *last = 0;
	Expression *current = 0;

	while(token!=0) {
//		output_debug_message("next token: %s\n",token);
		if (isSymbol(token)) {
			last = new Expression();
			last->m_head = token;
			if (current!=0) current->m_parameters.push_back(last);

			if (strcmp(token,"new")==0) {
				Expression *exp = from_string(string, pos);
				if (exp!=0) {
					last->m_parameters.push_back(exp);
				} else {
					output_debug_message("Expression::from_string: illegal expression after 'new': %s\n",string+pos0);
					break;
				}
				if (stack.size()==0) return last;
			}
		} else if (token[0]=='(') {
            delete token;
			if (last!=0) {
				stack.push_back(last);
				current = last;
			} else {
				output_debug_message("Expression::from_string: illegal expression '(': %s\n",string+pos0);
				break;
			}
		} else if (token[0]==')') {
            delete token;
			stack.pop_back();
			if (stack.size()>0) {
				current = stack.back();
			} else {
				return current;
			}
		} else if (token[0]==',') {
            delete token;
			// ... (ignore for now)
		} else if (token[0]>='0' && token[0]<='9') {
			last = new Expression();
			last->m_head = token;
			if (current!=0) current->m_parameters.push_back(last);
        } else if (token[0]=='?') {
            last = new Expression();
            last->m_head = token;
            if (current!=0) current->m_parameters.push_back(last);
		} else if (token[0]=='\'' || token[0]=='\"') {
			token[strlen(token)-1] = 0;	// remove the last '\"'
			for(int i = 0;i<(int)strlen(token);i++) token[i] = token[i+1];	// remove the first '\"'
            last = new Expression();
			last->m_head = token;
            last->quoted = true;
			if (current!=0) current->m_parameters.push_back(last);
		} else {
			output_debug_message("Expression::from_string: unrecognized token: %s\n",token);
			delete token;
			break;
		}

		token = next_token(string,pos);
	}

	if (stack.size()>0) {
		current = stack.front();
		stack.clear();
		delete current;
	}

	output_debug_message("Expression::from_string: illegal expression start: %s\n",string+pos0);

	return 0;
}


std::vector<Expression *> *Expression::list_from_string(char *expression)
{
    int pos = 0;
    return Expression::list_from_string(expression, pos);
}


std::vector<Expression *> *Expression::list_from_string(char *expression, int &pos){
    std::vector<Expression *> *l = new std::vector<Expression *>();

    while(pos<(int)strlen(expression)) {
        Expression *exp = Expression::from_string(expression, pos);
        if (exp!=0) l->push_back(exp);
        while(expression[pos]==' ' || expression[pos]==',') pos++;
    }

    return l;
}


/*

Tokens:
1) , ( )
2) "..."
3) '...'
4) symbols
5) numbers
6) ? + number (wildcards)

*/

char *Expression::next_token(char *string, int &pos)
{
	int buffer_size = 16;
	char *buffer = new char[buffer_size];

	// skip spaces:
	char c = string[pos++];
	while(c==' ' || c=='\t') c = string[pos++];
	// determine token type:
	if (c==0) {
		return 0;
	} else if (c==',' || c=='(' || c==')') {
		buffer[0] = c;
		buffer[1] = 0;
		return buffer;
	} else if (c=='\"') {
		int j = 0;
		buffer[j++]=c;
		c = string[pos++];
		while(c!='\"') {
			buffer[j++] = c;
			c = string[pos++];
			if (j>=buffer_size-1) {	// (-1 to allow for the additional '\"' at the end)
				char *buffer2 = new char[buffer_size*2];
				for(int i = 0;i<buffer_size;i++) buffer2[i] = buffer[i];
				delete []buffer;
				buffer_size*=2;
				buffer = buffer2;
			}
		}
		buffer[j++]='\"';
		buffer[j] = 0;
		return buffer;
	} else if (c=='\'') {
		int j = 0;
		buffer[j++]=c;
		c = string[pos++];
		while(c!='\'') {
			buffer[j++] = c;
			c = string[pos++];
			if (j>=buffer_size-1) {	// (-1 to allow for the additional '\' at the end)
				char *buffer2 = new char[buffer_size*2];
				for(int i = 0;i<buffer_size;i++) buffer2[i] = buffer[i];
				delete []buffer;
				buffer_size*=2;
				buffer = buffer2;
			}
		}
		buffer[j++]='\'';
		buffer[j] = 0;
		return buffer;
	} else if (isSymbolStartCharacter(c)) {
		int j = 0;
		while(isSymbolCharacter(c)) {
			buffer[j++] = c;
			c = string[pos++];
			if (j>=buffer_size) {
				char *buffer2 = new char[buffer_size*2];
				for(int i = 0;i<buffer_size;i++) buffer2[i] = buffer[i];
				delete []buffer;
				buffer_size*=2;
				buffer = buffer2;
			}
		}
		pos--;
		buffer[j] = 0;
		return buffer;
	} else if (c>='0' && c<='9') {
		int j = 0;
		while((c>='0' && c<='9') || c=='.') {
			buffer[j++] = c;
			c = string[pos++];
			if (j>=buffer_size) {
				char *buffer2 = new char[buffer_size*2];
				for(int i = 0;i<buffer_size;i++) buffer2[i] = buffer[i];
				delete []buffer;
				buffer_size*=2;
				buffer = buffer2;
			}
		}
		pos--;
		buffer[j] = 0;
		return buffer;
	} else if (c=='&' &&
			   string[pos]=='q' &&
			   string[pos+1]=='u' &&
			   string[pos+2]=='o' &&
			   string[pos+3]=='t' &&
			   string[pos+4]==';') {
		pos+=5;
		int j = 0;
		buffer[j++]='\"';
		c = string[pos++];
		while(c!='&') {
			buffer[j++] = c;
			c = string[pos++];
			if (j>=buffer_size-1) {	// (-1 to allow for the additional '\"' at the end)
				char *buffer2 = new char[buffer_size*2];
				for(int i = 0;i<buffer_size;i++) buffer2[i] = buffer[i];
				delete []buffer;
				buffer_size*=2;
				buffer = buffer2;
			}
		}
		buffer[j++]='\"';
		buffer[j] = 0;
		pos+=5;	// skip the "quot;"
		return buffer;
    } else if (c=='?') {
        int j = 0;
        buffer[j++] = c;
        c = string[pos++];
        while(c>='0' && c<='9') {
            buffer[j++] = c;
            c = string[pos++];
            if (j>=buffer_size) {
                char *buffer2 = new char[buffer_size*2];
                for(int i = 0;i<buffer_size;i++) buffer2[i] = buffer[i];
                delete []buffer;
                buffer_size*=2;
                buffer = buffer2;
            }
        }
        pos--;
        buffer[j] = 0;
        return buffer;
	} else {
		output_debug_message("Expression::next_token: Token starts with illegal character '%c'\n", c);
		exit(1);
	}
}


bool Expression::isSymbol(char *token)
{
	return isSymbolStartCharacter(token[0]);
}


bool Expression::isSymbolCharacter(char c)
{
	return (c>='a' && c<='z') || (c>='A' && c<='Z') || c=='.' || c=='_' || (c>='0' && c<='9') || c==':';
}


bool Expression::isSymbolStartCharacter(char c)
{
	return (c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_';
}

