//
//  auxiliar.h
//  Aventura 4
//
//  Created by Santiago Ontanon on 9/6/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifndef __Brain__auxiliar__
#define __Brain__auxiliar__

#include <vector>

int remove_dir(const char *path);
char *replaceString(const char *string, const char *replaceThis, const char *withThis);
std::vector<char *> splitByLines(const char *);
std::vector<char *> splitStringBy(const char *str, char separator);

int UTF8ToExtendedASCII(const char *input, int &position);
int UTF8StringLength(const char *input);



#endif /* defined(__Brain__auxiliar__) */
