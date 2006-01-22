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
 *
 *  $Id$
 */

#include "connection.h"

#include <guichan/actionlistener.hpp>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "progressbar.h"

#include "../main.h"

class ConnectionActionListener : public gcn::ActionListener
{
    public:
        void action(const std::string& eventId)
        {
            if (eventId == "cancel")
            {
                state = EXIT_STATE;
            }
        }
} connectionActionListener;

ConnectionDialog::ConnectionDialog():
    Window("Info"), mProgress(0)
{
    setContentSize(200, 100);

    Button *cancelButton;
    cancelButton = new Button("Cancel");
    cancelButton->setPosition(5, 100 - 5 - cancelButton->getHeight());
    cancelButton->setEventId("cancel");
    cancelButton->addActionListener(&connectionActionListener);

    mProgressBar = new ProgressBar(0.0, 5, cancelButton->getY() - 25,
                                   200 - 10, 20, 128, 128, 128);
    gcn::Label *label;
    label = new gcn::Label("Connecting...");
    label->setPosition(5, mProgressBar->getY() - 25);

    add(label);
    add(cancelButton);
    add(mProgressBar);

    setLocationRelativeTo(getParent());
}

void ConnectionDialog::logic()
{
    mProgress += 0.005f;
    if (mProgress > 1.0f)
    {
        mProgress = 0.0f;
    }
    mProgressBar->setProgress(mProgress);
    Window::logic();
}
