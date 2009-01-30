/*
 *  Aethyra
 *  Copyright 2009 Aethyra Development Team
 *
 *  This file is part of Aethyra.
 *
 *  Aethyra is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  Aethyra is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Aethyra; if not, write to the Free Software Foundation, 
 *  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string>

#include <guichan/mouseinput.hpp>

#include "button.h"
#include "gui.h"
#include "emotewindow.h"
#include "emotecontainer.h"
#include "scrollarea.h"
#include "textbox.h"

#include "widgets/layout.h"

#include "../localplayer.h"

#include "../utils/gettext.h"      
#include "../utils/tostring.h"

EmoteWindow::EmoteWindow():
    Window(_("Emote"))
{
    setWindowName(_("Emote"));
    setResizable(true);
    setCloseButton(true);
    setMinWidth(80);
    setMinHeight(130);
    setDefaultSize(115, 25, 322, 200);

    mUseButton = new Button(_("Use"), "use", this);

    mEmotes = new EmoteContainer();
    mEmotes->addSelectionListener(this);

    mEmoteScroll = new ScrollArea(mEmotes);
    mEmoteScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    place(0, 0, mEmoteScroll, 5, 4);
    place(4, 4, mUseButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    mUseButton->setSize(60, mUseButton->getHeight());

    loadWindowState();
}

void EmoteWindow::action(const gcn::ActionEvent &event)
{
    int emote = mEmotes->getSelectedEmote();

    if (!emote)
        return;

    player_node->emote(emote);
}

int EmoteWindow::getSelectedEmote() const
{
    return mEmotes->getSelectedEmote();
}
