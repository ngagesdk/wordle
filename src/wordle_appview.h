/** @file wordle_appview.h
 *
 *  A clone of Wordle for the Nokie N-Gage.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef WORDLE_APPVIEW_H
#define WORDLE_APPVIEW_H

#include <coecntrl.h>

class CWordleAppView : public CCoeControl
{
public:
    static CWordleAppView* NewL(const TRect& aRect);
    static CWordleAppView* NewLC(const TRect& aRect);

    ~CWordleAppView();

public:
    void Draw(const TRect& aRect) const;

private:
    void ConstructL(const TRect& aRect);

    CWordleAppView();
};

#endif /* WORDLE_APPVIEW_H */
