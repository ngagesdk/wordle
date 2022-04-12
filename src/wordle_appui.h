/** @file wordle_appui.h
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef WORDLE_APPUI_H
#define WORDLE_APPUI_H

#include <aknappui.h>

class CWordleAppView;

class CWordleAppUi : public CAknAppUi
{
public:
    void ConstructL();

    CWordleAppUi();
    ~CWordleAppUi();

public:
    void HandleCommandL(TInt aCommand);

private:
    CWordleAppView* iAppView;
};

#endif /* WORDLE_APPUI_H */
