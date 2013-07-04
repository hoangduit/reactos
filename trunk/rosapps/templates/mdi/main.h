/*
 *  ReactOS Application
 *
 *  main.h
 *
 *  Copyright (C) 2002  Robert Dickenson <robd@reactos.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

#include "../../lib/ros2win/ros2win.h"

////////////////////////////////////////////////////////////////////////////////

#define	SPLIT_WIDTH    5
#define MAX_LOADSTRING 100
//#define   BUFFER_LEN  1024
#define _NO_EXTENSIONS

typedef struct {
    HWND    hWnd;
    HWND    hLeftWnd;
    HWND    hRightWnd;
    int     nFocusPanel;      // 0: left  1: right
	int		nSplitPos;
    WINDOWPLACEMENT pos;
	TCHAR	szPath[MAX_PATH];
} ChildWnd;

void UpdateStatusBar(void);

////////////////////////////////////////////////////////////////////////////////
// Global Variables:

extern HINSTANCE hInst;
extern HACCEL    hAccel;
extern HWND      hFrameWnd;
extern HMENU     hMenuFrame;
extern HWND      hMDIClient;
extern HWND      hStatusBar;
extern HWND      hToolBar;
extern HFONT     hFont;


extern TCHAR szTitle[];
extern TCHAR szFrameClass[];
extern TCHAR szChildClass[];



#ifdef __cplusplus
};
#endif

#endif // __MAIN_H__
