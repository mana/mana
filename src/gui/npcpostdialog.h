/*
 *  The Mana World
 *  Copyright (C) 2008-2010  The Mana World Development Team
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

#ifndef GUI_NPCPOSTDIALOG_H
#define GUI_NPCPOSTDIALOG_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

class TextBox;
class TextField;

class NpcPostDialog : public Window, public gcn::ActionListener
{
public:
    /**
     * Constructor
     */
    NpcPostDialog(int npcId);

    ~NpcPostDialog();

    /**
     * Called when receiving actions from the widgets.
     */
    void action(const gcn::ActionEvent &event);

    void setVisible(bool visible);

    /**
     * Returns true if any instances exist.
     */
    static bool isActive() { return instances.size() > 0; }

    /**
     * Closes all instances.
     */
    static void closeAll();

private:
    typedef std::list<NpcPostDialog*> DialogList;
    static DialogList instances;

    int mNpcId;

    TextBox *mText;
    TextField *mSender;
};

#endif
