/** @file wordle_application.cpp
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "wordle_document.h"
#include "wordle_application.h"

static const TUid KUidWordleApp = { UID3 };

CApaDocument* CWordleApplication::CreateDocumentL()
{
    CApaDocument* document = CWordleDocument::NewL(*this);
    return document;
}

TUid CWordleApplication::AppDllUid() const
{
    return KUidWordleApp;
}
