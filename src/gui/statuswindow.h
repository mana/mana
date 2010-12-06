/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STATUS_H
#define STATUS_H

#include "listener.h"

#include "gui/widgets/window.h"

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
class StatusWindow : public Window, public Mana::Listener
{
    public:
        /**
         * Constructor.
         */
        StatusWindow();

        void event(Channels channel, const Mana::Event &event);

        void updateAttrs();

        void setPointsNeeded(int id, int needed);

        void addAttribute(int id, const std::string &name, bool modifiable,
                          const std::string &description);

        static void updateHPBar(ProgressBar *bar, bool showMax = false);
        static void updateMPBar(ProgressBar *bar, bool showMax = false);
        static void updateXPBar(ProgressBar *bar, bool percent = true);
        static void updateProgressBar(ProgressBar *bar, int value, int max,
                                      bool percent);
        void updateProgressBar(ProgressBar *bar, int id,
                               bool percent = true);

    private:
        /**
         * Status Part
         */
        gcn::Label *mLvlLabel, *mMoneyLabel;
        gcn::Label *mHpLabel, *mMpLabel, *mXpLabel;
        ProgressBar *mHpBar, *mMpBar, *mXpBar;

        gcn::Label *mJobLvlLabel, *mJobLabel;
        ProgressBar *mJobBar;

        VertContainer *mAttrCont;
        ScrollArea *mAttrScroll;
        VertContainer *mDAttrCont;
        ScrollArea *mDAttrScroll;

        gcn::Label *mCharacterPointsLabel;
        gcn::Label *mCorrectionPointsLabel;

        typedef std::map<int, AttrDisplay*> Attrs;
        Attrs mAttrs;
};

extern StatusWindow *statusWindow;

#endif
