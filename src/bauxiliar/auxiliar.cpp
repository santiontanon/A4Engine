//
//  auxiliar.cpp
//  Aventura 4
//
//  Created by Santiago Ontanon on 9/6/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//


#ifdef _WIN32
#include <windows.h>
#include <string>
#else
#include "unistd.h"
#include "sys/stat.h"
#include "sys/types.h"
#include <dirent.h>
#endif

#include "debug.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "auxiliar.h"


char *replaceString(const char *orig, const char *rep, const char *with)
{
    char *result; // the return string
    const char *ins;    // the next insert point
    char *tmp;    // varies
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements
    int len_rep = (int)strlen(rep);
    int len_with = (int)strlen(with);
    
    if (!orig) return 0;
    
    ins = orig;
    for (count = 0; (tmp = (char *)strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }
    
    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = new char[(strlen(orig) + (len_with - len_rep) * count + 1)];
    
    if (!result)
        return NULL;
    
    while (count--) {
        ins = strstr(orig, rep);
        len_front = (int)(ins - orig);
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}


int remove_dir(const char *path)
{
    // this code has been adapted from here: http://forums.codeguru.com/showthread.php?239271-Windows-SDK-File-System-How-to-delete-a-directory-and-subdirectories
#ifdef _WIN32
    bool            bSubdirectory = false;       // Flag, indicating whether
    // subdirectories have been found
    HANDLE          hFile;                       // Handle to directory
    std::string     strFilePath;                 // Filepath
    std::string     strPattern;                  // Pattern
    WIN32_FIND_DATA FileInformation;             // File information
    
    std::string refcstrRootDirectory(path);
    strPattern = refcstrRootDirectory + "\\*.*";
    hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(FileInformation.cFileName[0] != '.')
            {
                strFilePath.erase();
                strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;
                
                if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // Delete subdirectory
                    int iRC = remove_dir(strFilePath.c_str());
                    if(iRC)
                        return iRC;
                }
                else
                {
                    // Set file attributes
                    if(::SetFileAttributes(strFilePath.c_str(),
                                           FILE_ATTRIBUTE_NORMAL) == FALSE)
                        return ::GetLastError();
                    
                    // Delete file
                    if(::DeleteFile(strFilePath.c_str()) == FALSE)
                        return ::GetLastError();
                }
            }
        } while(::FindNextFile(hFile, &FileInformation) == TRUE);
        
        // Close handle
        ::FindClose(hFile);
        
        DWORD dwError = ::GetLastError();
        if(dwError != ERROR_NO_MORE_FILES) {
            output_debug_message("cannot delete folder '%'\n", path);
            return dwError;
        } else {
            if(!bSubdirectory)
            {
                // Set directory attributes
                if(::SetFileAttributes(refcstrRootDirectory.c_str(),
                                       FILE_ATTRIBUTE_NORMAL) == FALSE)
                    return ::GetLastError();
                
                // Delete directory
                if(::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
                    return ::GetLastError();
            }
        }
    }
#else
    struct dirent *entry = NULL;
    DIR *dir = NULL;
    dir = opendir(path);
    if (dir==NULL) {
        output_debug_message("cannot delete folder '%'\n", path);
        return 1;
    }
    while((entry = readdir(dir)))
    {
        DIR *sub_dir = NULL;
        FILE *file = NULL;
        char abs_path[100] = {0};
        if(*(entry->d_name) != '.')
        {
            sprintf(abs_path, "%s/%s", path, entry->d_name);
            if((sub_dir = opendir(abs_path)))
            {
                closedir(sub_dir);
                remove_dir(abs_path);
            }
            else
            {
                if((file = fopen(abs_path, "r")))
                {
                    fclose(file);
                    remove(abs_path);
                }
            }
        }
    }
    remove(path);
#endif
    return 0;
}



std::vector<char *> splitByLines(const char *initial_text)
{
    return splitStringBy(initial_text, '\n');
}


std::vector<char *> splitStringBy(const char *str, char separator)
{
    std::vector<char *> l;
    int last = 0, i = 0;
    for(;str[i]!=0;i++) {
        if (str[i]==separator) {
            if (i>last) {
                char *tmp = new char[i-last+1];
                int j = 0;
                for(;j<i-last;j++) tmp[j] = str[j+last];
                tmp[j] = 0;
                l.push_back(tmp);
            }
            last = i+1;
        }
    }
    if (i>last) {
        char *tmp = new char[i-last+1];
        int j = 0;
        for(;j<i-last;j++) tmp[j] = str[j+last];
        tmp[j] = 0;
        l.push_back(tmp);
    }
    return l;
}


/* Note, here, I am only considering a few characters, which are useful for romanic languages, not the whole unicode */

int UTF8ToExtendedASCII(const char *input, int &position)
{
    const char *extendedASCIITable[] = {"Ç","ü","é","â","ä","à","å","ç",
        "ê","ë","è","ï","î","ì","Ä","Å",
        "É","","","ô","ö","ò","û","ù",
        "Ÿ","Ö","Ü","","","","","",
        "á","í","ó","ú","ñ","Ñ","","",
    };
    
    if ((input[position]&0x80) == 0) {
        int c = input[position];
        position++;
        return c;
    }
    for(int i = 0;i<40;i++) {
        int l = (int)strlen(extendedASCIITable[i]);
        if (l>0) {
            bool match = true;
            for(int j = 0;j<l && input[position+j]!=0;j++) {
                if (input[position+j]!=extendedASCIITable[i][j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                position+=l;
                return 128 + i;
            }
        }
    }
    position++;
    return 0;
}


int UTF8StringLength(const char *input)
{
    int l = 0;
    for(int i = 0;input[i]!=0;) {
        UTF8ToExtendedASCII(input,i);
        l++;
    }
    return l;
}

