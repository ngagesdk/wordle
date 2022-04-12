/** @file wordle_appui.cpp
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <avkon.hrh>
#include <aknnotewrappers.h>

#include "wordle_appui.h"
#include "wordle_appview.h"

void CWordleAppUi::ConstructL()
{
    BaseConstructL();

    iAppView = CWordleAppView::NewL(ClientRect());

    AddToStackL(iAppView);
}

CWordleAppUi::CWordleAppUi()
{
    RProcess Proc;
    if (KErrNone == Proc.Create(_L("E:\\System\\Apps\\wordle\\game.exe"), _L("")))
    {
        Proc.Resume();
        Proc.Close();
        User::After(10000000);
        Exit();
    }
    else
    {
        Exit();
    }
}

CWordleAppUi::~CWordleAppUi()
{
    if (iAppView)
    {
        RemoveFromStack(iAppView);
        delete iAppView;
        iAppView = NULL;
    }
}

void CWordleAppUi::HandleCommandL(TInt aCommand)
{
    /* No implementation required. */
}
