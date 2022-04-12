/** @file wordle_appview.cpp
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <coemain.h>
#include "wordle_appview.h"

CWordleAppView* CWordleAppView::NewL(const TRect& aRect)
{
    CWordleAppView* self = CWordleAppView::NewLC(aRect);
    CleanupStack::Pop(self);
    return self;
}

CWordleAppView* CWordleAppView::NewLC(const TRect& aRect)
{
    CWordleAppView* self = new (ELeave) CWordleAppView;
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    return self;
}

CWordleAppView::CWordleAppView()
{
    /* No implementation required. */
}

CWordleAppView::~CWordleAppView()
{
    /* No implementation required. */
}

void CWordleAppView::ConstructL(const TRect& aRect)
{
    CreateWindowL();
    SetRect(aRect);
    ActivateL();
}

// Draw this application's view to the screen
void CWordleAppView::Draw(const TRect& /*aRect*/) const
{
    CWindowGc& gc   = SystemGc();
    TRect      rect = Rect();

    gc.Clear(rect);
}
