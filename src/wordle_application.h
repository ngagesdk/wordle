/** @file wordle_application.h
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef WORDLE_APPLICATION_H
#define WORDLE_APPLICATION_H

#include <aknapp.h>

class CWordleApplication : public CAknApplication
{
public:
    TUid AppDllUid() const;

protected:
    CApaDocument* CreateDocumentL();
};

#endif /* WORDLE_APPLICATION_H */
