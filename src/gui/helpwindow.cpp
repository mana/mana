/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "gui/helpwindow.h"

#include "gui/gui.h"
#include "gui/setup.h"

#include "gui/widgets/button.h"
#include "gui/widgets/browserbox.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"

#include "resources/resourcemanager.h"
#include "configuration.h"

#include "utils/gettext.h"

HelpWindow::HelpWindow():
    Window(_("Help"))
{
    setMinWidth(300);
    setMinHeight(250);
    setContentSize(455, 350);
    setWindowName("Help");
    setResizable(true);
    setupWindow->registerWindowForReset(this);

    setDefaultSize(500, 400, ImageRect::CENTER);

    mBrowserBox = new BrowserBox;
    mBrowserBox->setFrameSize(4);
    mScrollArea = new ScrollArea(mBrowserBox);
    auto *okButton = new Button(_("Close"), "close", this);

    mBrowserBox->setLinkHandler(this);
    mBrowserBox->setFont(monoFont);

    place(0, 0, mScrollArea, 5, 3).setPadding(3);
    place(4, 3, okButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    loadWindowState();
}

void HelpWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "close")
        setVisible(false);
}

void HelpWindow::handleLink(const std::string &link)
{
    std::string helpFile = link;
    loadHelp(helpFile);
}

void HelpWindow::loadHelp(const std::string &helpFile)
{
    mBrowserBox->clearRows();

    loadFile("header");
    loadFile(helpFile);

    mScrollArea->setVerticalScrollAmount(0);
    setVisible(true);
}

void HelpWindow::loadFile(const std::string &file)
{
    ResourceManager *resman = ResourceManager::getInstance();
    std::string helpPath = branding.getStringValue("helpPath");
    if (helpPath.empty())
        helpPath = paths.getStringValue("help");

    const auto lines = resman->loadTextFile(helpPath + file + ".txt");
    for (auto &line : lines)
        mBrowserBox->addRow(line);
}
