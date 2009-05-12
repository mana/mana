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

#ifndef GUI_SETUP_PLAYERS_H
#define GUI_SETUP_PLAYERS_H

#include "guichanfwd.h"

#include "gui/setuptab.h"

#include "playerrelations.h"

#include <guichan/actionlistener.hpp>

class GuiTable;
class PlayerTableModel;
class StaticTableModel;

class Setup_Players : public SetupTab,
                      public gcn::ActionListener,
                      public PlayerRelationsListener
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
    gcn::ScrollArea *mPlayerScrollArea;

    gcn::CheckBox *mDefaultTrading;
    gcn::CheckBox *mDefaultWhisper;

    gcn::Button *mDeleteButton;

    gcn::ListModel *mIgnoreActionChoicesModel;
    gcn::DropDown *mIgnoreActionChoicesBox;

    bool mWhisperTab;
    gcn::CheckBox *mWhisperTabCheckBox;
};

#endif
