/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef CHANNELTAB_H
#define CHANNELTAB_H

#include "gui/widgets/chattab.h"

/**
 * A tab for a party chat channel.
 */
class PartyTab : public ChatTab
{
    public:
        PartyTab();
        ~PartyTab();

        void showHelp();

        bool handleCommand(const std::string &type, const std::string &args);

    protected:
        void handleInput(const std::string &msg);
};

extern PartyTab *partyTab;

#endif // CHANNELTAB_H
