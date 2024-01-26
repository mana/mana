/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef TA_PARTYTAB_H
#define TA_PARTYTAB_H

#include "gui/widgets/chattab.h"

namespace TmwAthena {

/**
 * A tab for a party chat channel.
 */
class PartyTab : public ChatTab
{
    public:
        PartyTab();
        ~PartyTab() override;

        void showHelp() override;

        bool handleCommand(const std::string &type, const std::string &args) override;

        void saveToLogFile(std::string &msg) override;

    protected:
        void handleInput(const std::string &msg) override;

        void getAutoCompleteList(std::vector<std::string>&) const override;
};

extern PartyTab *partyTab;

} // namespace TmwAthena

#endif // TA_PARTYTAB_H
