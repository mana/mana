/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _TMW_GUI_SETUP_PLAYERS_H
#define _TMW_GUI_SETUP_PLAYERS_H

#include <guichan/actionlistener.hpp>

#include "button.h"
#include "scrollarea.h"
#include "setuptab.h"
#include "table.h"

#include "../guichanfwd.h"
#include "../player_relations.h"

class PlayerTableModel;

class Setup_Players : public SetupTab, public gcn::ActionListener, public PlayerRelationsListener
{
public:
    Setup_Players();
    virtual ~Setup_Players();

    void apply();
    void cancel();

    void reset();

    void action(const gcn::ActionEvent &event);

    virtual void updatedPlayer(const std::string &name);

private:
    StaticTableModel *mPlayerTableTitleModel;
    PlayerTableModel *mPlayerTableModel;
    GuiTable *mPlayerTable;
    GuiTable *mPlayerTitleTable;
    ScrollArea *mPlayerScrollArea;

    gcn::CheckBox *mPersistIgnores;
    gcn::CheckBox *mDefaultTrading;
    gcn::CheckBox *mDefaultWhisper;

    Button *mDeleteButton;
    gcn::DropDown *mIgnoreActionChoicesBox;
};

#endif
