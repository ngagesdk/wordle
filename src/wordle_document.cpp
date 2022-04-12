/** @file wordle_document.cpp
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "wordle_appui.h"
#include "wordle_document.h"

CWordleDocument* CWordleDocument::NewL(CEikApplication& aApp)
{
    CWordleDocument* self = NewLC(aApp);
    CleanupStack::Pop(self);
    return self;
}

CWordleDocument* CWordleDocument::NewLC(CEikApplication& aApp)
{
    CWordleDocument* self = new (ELeave) CWordleDocument(aApp);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

void CWordleDocument::ConstructL()
{
    /* No implementation required. */
}

CWordleDocument::CWordleDocument(CEikApplication& aApp) : CAknDocument(aApp)
{
    /* No implementation required. */
}

CWordleDocument::~CWordleDocument()
{
    /* No implementation required. */
}

CEikAppUi* CWordleDocument::CreateAppUiL()
{
    CEikAppUi* appUi = new (ELeave) CWordleAppUi;
    return appUi;
}
