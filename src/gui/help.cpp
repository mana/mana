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
#include "scrollarea.h"
#include "button.h"
#include "textbox.h"
#include "../main.h"
#include <fstream>

HelpWindow::HelpWindow():
    Window("Help")
{
    setContentSize(455, 350);

    textBox = new TextBox();
    textBox->setEditable(false);
    scrollArea = new ScrollArea(textBox);
    okButton = new Button("Close");

    scrollArea->setDimension(gcn::Rectangle(
                5, 5, 445, 335 - okButton->getHeight()));
    okButton->setPosition(
            450 - okButton->getWidth(),
            345 - okButton->getHeight());

    okButton->setEventId("close");
    okButton->addActionListener(this);

    add(scrollArea);
    add(okButton);

    setLocationRelativeTo(getParent());
    textBox->requestFocus();
}

HelpWindow::~HelpWindow()
{
    delete okButton;
    delete textBox;
    delete scrollArea;
    links.clear();
}

void HelpWindow::action(const std::string& eventId)
{
    setVisible(false);
}

void HelpWindow::mousePress(int mx, int my, int button)
{
    getParent()->moveToTop(this);

    if (button == gcn::MouseInput::LEFT)
    {
        int x1 = scrollArea->getX() + 10;
        int y1 = scrollArea->getY() + 15;
        int x2 = x1 + scrollArea->getWidth() - 25;
        int y2 = y1 + scrollArea->getHeight();

        if ((mx >= x1) && (my >= y1) && (mx <= x2) && (my <= y2))
        {
            for (unsigned int i = 0; i < links.size(); i++)
            {
                int y1 = links[i].yPos * textBox->getFont()->getHeight() + 5 -
                    scrollArea->getVerticalScrollAmount();
                int y2 = y1 + textBox->getFont()->getHeight();

                if ((my > y1) && (my < y2))
                {
                    loadHelp(links[i].file);
                }
            }
        }
        else if (hasMouse() && my < (int)(getTitleBarHeight() + getPadding()))
        {
            mMouseDrag = true;
            mMouseXOffset = mx;
            mMouseYOffset = my;
        }
    }
}

void HelpWindow::loadHelp(const std::string &helpFile)
{
    std::string helpPath = TMW_DATADIR "data/help/" + helpFile + ".txt";

    std::ifstream fin;
    fin.open(helpPath.c_str());
    if (fin.fail())
    {
        logger->log("Couldn't load help file: %s", helpPath.c_str());
        return;
    }

    links.clear();
    textBox->setText("");

    while (!fin.eof())
    {
        std::string line = "";
        getline(fin, line);

        // Check for links
        if (line.substr(0, 1) == "@")
        {
            int idx = line.find("->");
            HELP_LINK hlink;
            hlink.yPos = textBox->getNumberOfRows() + 1;
            hlink.file = line.substr(1, idx - 1);
            links.push_back(hlink);

            line = "    " + line.erase(0, idx);
        }

        textBox->addRow(line);
    }

    scrollArea->setVerticalScrollAmount(0);
    setVisible(true);

    fin.close();
}
