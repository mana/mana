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

#include "updatewindow.h"
#include <sstream>

UpdateWindow::UpdateWindow()
    : Window("Updating")
{
    setContentSize(320, 96);

    vbox = new VBox();
    label = new gcn::Label("Default text");
    progressBar = new ProgressBar(0.0, 0, 0, 0, 0, 0, 191, 63);

    vbox->setPosition(4, 0);
    vbox->setSize(getWidth() - 12, getHeight() - 24);

    vbox->add(label);
    vbox->add(progressBar);

    add(vbox);
}

UpdateWindow::~UpdateWindow()
{
    delete label;
    delete progressBar;
    delete vbox;
}

void UpdateWindow::setProgress(double p)
{
    progress = p;
}

void UpdateWindow::setLabel(const std::string &str)
{
    labelText = str;
}

void UpdateWindow::draw(gcn::Graphics *graphics)
{
    std::stringstream ss;
    ss << labelText << " (" << progress * 100 << "%)";

    label->setCaption(ss.str());
    progressBar->setProgress(progress);

    Window::draw(graphics);
}
