/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef STATUS_H
#define STATUS_H

#include "gui/widgets/window.h"

#include "guichanfwd.h"

#include <guichan/actionlistener.hpp>

#include <map>

class AttrDisplay;
class ProgressBar;
class ScrollArea;
class VertContainer;

/**
 * The player status dialog.
 *
 * \ingroup Interface
 */
class StatusWindow : public Window
{
    public:
        enum { // Some update constants
            HP = -1,
            MP = -2,
            EXP = -3,
            MONEY = -4,
            CHAR_POINTS = -5,
            LEVEL = -6
        };

        /**
         * Constructor.
         */
        StatusWindow();

        std::string update(int id);

        void setPointsNeeded(int id, int needed);

        void addAttribute(int id, const std::string &name, bool modifiable);

        static void updateHPBar(ProgressBar *bar, bool showMax = false);
        static void updateMPBar(ProgressBar *bar, bool showMax = false);
        static void updateXPBar(ProgressBar *bar, bool percent = true);
        static void updateProgressBar(ProgressBar *bar, int value, int max,
                              bool percent);
        static void updateProgressBar(ProgressBar *bar, int id,
                                      bool percent = true);

    private:
        /**
         * Status Part
         */
        gcn::Label *mLvlLabel, *mMoneyLabel;
        gcn::Label *mHpLabel, *mMpLabel, *mXpLabel;
        ProgressBar *mHpBar, *mMpBar, *mXpBar;

#ifdef EATHENA_SUPPORT
        gcn::Label *mJobLvlLabel, *mJobLabel;
        ProgressBar *mJobBar;
#endif

        VertContainer *mAttrCont;
        ScrollArea *mAttrScroll;
        VertContainer *mDAttrCont;
        ScrollArea *mDAttrScroll;

        gcn::Label *mCharacterPointsLabel, *mCorrectionPointsLabel;

        typedef std::map<int, AttrDisplay*> Attrs;
        Attrs mAttrs;
};

extern StatusWindow *statusWindow;

#endif
