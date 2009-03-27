/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include <guichan/widgets/label.hpp>

#include "partytab.h"

#include "net/messageout.h"

#include "net/ea/party.h"
#include "net/ea/protocol.h"

#include "resources/iteminfo.h"
#include "resources/itemdb.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/strprintf.h"
#include "utils/stringutils.h"

PartyTab::PartyTab() : ChatTab(_("Party"))
{
}

PartyTab::~PartyTab()
{
}

void PartyTab::handleInput(std::string &msg) {
    // TODO
}

void PartyTab::handleCommand(std::string &msg) {
    // TODO
}
