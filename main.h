#ifndef _VH_MAIN_H_
#define _VH_MAIN_H_

#include <windows.h>
#include <wingdi.h>

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <vector>

using namespace std;

#define uchar8 unsigned char
#define uint16 unsigned short

#define F1K 1024L
#define u8 unsigned char 
#define u16 unsigned __int16

typedef struct { int offs_top; int offs_left; int charw; int charh; } stCharProps;

#include "bfn2.h"

void DrawSymbols(HWND hwnd, HFONT hCustomFont);

#endif
