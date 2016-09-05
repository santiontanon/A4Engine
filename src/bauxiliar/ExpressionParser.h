#ifndef EXPRESSION_PARSER
#define EXPRESSION_PARSER

/*
	This class parses expressions written in C/C++/Java/JavaScript syntax
*/

class Expression {
public:
	~Expression();

	static Expression *from_string(char *expression);
	static Expression *from_string(char *expression, int &pos);
    static std::vector<Expression *> *list_from_string(char *expression);
    static std::vector<Expression *> *list_from_string(char *expression, int &pos);

	static bool isSymbol(char *token);
	static bool isSymbolCharacter(char c);
	static bool isSymbolStartCharacter(char c);

	char *get_head() {return m_head;}
    bool isQuoted() {return quoted;}
	std::vector<Expression *> *get_parameters() {return &m_parameters;}

	void output_debug(int tabs);

private:
	Expression();

	static char *next_token(char *string, int &pos);

	char *m_head;
    bool quoted;
	std::vector<Expression *> m_parameters;
};

#endif