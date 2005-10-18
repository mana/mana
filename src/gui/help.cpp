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

#include "help.h"

#include "button.h"
#include "browserbox.h"
#include "scrollarea.h"
#include "textbox.h"

#include "../log.h"

#include "../resources/resourcemanager.h"

HelpWindow::HelpWindow():
    Window("Help")
{
    setContentSize(455, 350);
    setWindowName("Help");

    browserBox = new BrowserBox();
    browserBox->setOpaque(false);
    scrollArea = new ScrollArea(browserBox);
    okButton = new Button("Close");

    scrollArea->setDimension(gcn::Rectangle(
                5, 5, 445, 335 - okButton->getHeight()));
    okButton->setPosition(
            450 - okButton->getWidth(),
            345 - okButton->getHeight());

    okButton->setEventId("close");
    okButton->addActionListener(this);

    browserBox->setLinkHandler(this);

    add(scrollArea);
    add(okButton);

    setLocationRelativeTo(getParent());
}

HelpWindow::~HelpWindow()
{
    delete browserBox;
}

void HelpWindow::action(const std::string& eventId)
{
    if (eventId == "close")
    {
        setVisible(false);
    }
}

void HelpWindow::handleLink(const std::string& link)
{
    std::string helpFile = link;
    loadHelp(helpFile);
}

void HelpWindow::loadHelp(const std::string &helpFile)
{
    browserBox->clearRows();

    loadFile("header");
    loadFile(helpFile);

    scrollArea->setVerticalScrollAmount(0);
    setVisible(true);
}

void HelpWindow::loadFile(const std::string &file)
{
    ResourceManager *resman = ResourceManager::getInstance();
    std::vector<std::string> lines =
        resman->loadTextFile("help/" + file + ".txt");

    for (unsigned int i = 0; i < lines.size(); ++i)
    {
        browserBox->addRow(lines[i]);
    }
}
