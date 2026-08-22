/*
 *  The Mana Client
 *  Copyright (C) 2010-2026  The Mana Developers
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

#include "gui/beingpopup.h"

#include "being.h"
#include "localplayer.h"
#include "playerinfo.h"

#include "gui/gui.h"

#include "gui/widgets/label.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>


BeingPopup::BeingPopup():
    Popup("BeingPopup")
{
    setMinWidth(0);
    setMinHeight(0);

    // Being Name
    mBeingName = new Label("A");
    mBeingName->setFont(boldFont);
    mBeingName->setPosition(0, 0);

    // Being's party
    mBeingParty = new Label("A");

    // Being's level
    mBeingLevel = new Label("A");

    add(mBeingName);
    add(mBeingParty);
    add(mBeingLevel);

    addMouseListener(this);
}

BeingPopup::~BeingPopup() = default;

void BeingPopup::show(int x, int y, Being *b)
{
    if (!b)
    {
        setVisible(false);
        return;
    }

    mBeingName->setCaption(b->getName());
    mBeingName->adjustSize();

    int minWidth = mBeingName->getWidth();
    const int fontHeight = getFont()->getHeight();
    int height = fontHeight;

    if (!(b->getPartyName().empty()))
    {
        mBeingParty->setCaption(strprintf(_("Party: %s"),
                                          b->getPartyName().c_str()));
        mBeingParty->adjustSize();
        mBeingParty->setPosition(0, height);
        minWidth = std::max(minWidth, mBeingParty->getWidth());
        height += fontHeight;
    }
    else
    {
        mBeingParty->setCaption(std::string());
    }

    const int level = b == local_player ? PlayerInfo::getAttribute(LEVEL)
                                        : b->getLevel();
    if (level > 1)
    {
        mBeingLevel->setCaption(strprintf(_("Level: %d"), level));
        mBeingLevel->adjustSize();
        mBeingLevel->setPosition(0, height);
        minWidth = std::max(minWidth, mBeingLevel->getWidth());
        height += fontHeight;
    }
    else
    {
        mBeingLevel->setCaption(std::string());
    }

    setContentSize(minWidth, height);

    position(x, y);
}
