/** @file wordle_document.h
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef WORDLE_DOCUMENT_H
#define WORDLE_DOCUMENT_H

#include <akndoc.h>

class CWordleAppUi;
class CEikApplication;

class CWordleDocument : public CAknDocument
{
public:
    static CWordleDocument* NewL(CEikApplication& aApp);
    static CWordleDocument* NewLC(CEikApplication& aApp);

    ~CWordleDocument();

public:
    CEikAppUi* CreateAppUiL();

private:
    void ConstructL();
    CWordleDocument(CEikApplication& aApp);
};

#endif /* WORDLE_DOCUMENT_H */
