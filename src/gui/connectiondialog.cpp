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

#include "connectiondialog.h"

#include "main.h"
#include "log.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/progressindicator.h"

#include "utils/gettext.h"

ConnectionDialog::ConnectionDialog(const std::string &text,
                                   State cancelState):
    Window(""),
    mCancelState(cancelState)
{
    setTitleBarHeight(0);
    setMovable(false);
    setMinWidth(0);

    ProgressIndicator *progressIndicator = new ProgressIndicator;
    gcn::Label *label = new Label(text);
    Button *cancelButton = new Button(_("Cancel"), "cancelButton", this);

    place(0, 0, progressIndicator);
    place(0, 1, label);
    place(0, 2, cancelButton).setHAlign(LayoutCell::CENTER);
    reflowLayout();

    center();
    setVisible(true);
}

void ConnectionDialog::action(const gcn::ActionEvent &)
{
    logger->log("Cancel pressed");
    state = mCancelState;
}

void ConnectionDialog::draw(gcn::Graphics *graphics)
{
    // Don't draw the window background, only draw the children
    drawChildren(graphics);
}
