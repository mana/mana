/*
 *  The Mana Client
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

#ifndef SPECIALSWINDOW_H
#define SPECIALSWINDOW_H

#include <vector>

#include "playerinfo.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

#include <map>

class Label;
class ScrollArea;
class Tab;
class TabbedArea;

struct SpecialEntry;

class SpecialsWindow : public Window, public gcn::ActionListener {
    public:
        SpecialsWindow();

        ~SpecialsWindow();

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &actionEvent);

        void draw(gcn::Graphics *graphics);

        bool hasSpecials()
        { return !mEntries.empty(); }

    private:
        // (re)constructs the list of specials
        void rebuild(const std::map<int, Special> &specialData);

        TabbedArea *mTabs;
        std::map<int, SpecialEntry *> mEntries;
};

extern SpecialsWindow *specialsWindow;

#endif // SPECIALSWINDOW_H
