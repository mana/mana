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

#include "stats.h"

extern PLAYER_INFO *char_info;

StatsWindow::StatsWindow():
    Window("Stats")
{
    // New labels
    for (i = 0; i < 6; i++) {
        statsLabel[i] = new gcn::Label();
        statsDisplayLabel[i] = new gcn::Label();
    }
    remainingStatsPointsLabel = new gcn::Label();

    // New buttons
    for (i = 0; i < 6; i++) {
        statsButton[i] = new Button("+");
    }

    // Set button events Id
    statsButton[0]->setEventId("STR");
    statsButton[1]->setEventId("AGI");
    statsButton[2]->setEventId("VIT");
    statsButton[3]->setEventId("INT");
    statsButton[4]->setEventId("DEX");
    statsButton[5]->setEventId("LUK");

    // Set position
    for (i = 0; i < 6; i++) {
        statsLabel[i]->setPosition(10,(i*23)+10);
        statsDisplayLabel[i]->setPosition(120,(i*23)+10);
        statsButton[i]->setPosition(170,(i*23)+10);
    }
    remainingStatsPointsLabel->setPosition(10, 156);

    // Assemble
    for(i = 0; i < 6; i++) {
        add(statsLabel[i]);
        add(statsDisplayLabel[i]);
        add(statsButton[i]);
        statsButton[i]->addActionListener(this);
    }
    add(remainingStatsPointsLabel);

    update();

    setSize(200, 180);
    setLocationRelativeTo(getParent());
}

void StatsWindow::update(){
    std::stringstream statsStr[6];
    std::stringstream figureStr[6];
    std::stringstream remainingStatsPointsStr;

    statsStr[0] << "Strength:";
    figureStr[0] << (int)char_info->STR;
    statsStr[1] << "Agility:";
    figureStr[1] << (int)char_info->AGI;
    statsStr[2] << "Vitality:";
    figureStr[2] << (int)char_info->VIT;
    statsStr[3] << "Intelligence:";
    figureStr[3] << (int)char_info->INT;
    statsStr[4] << "Dexterity:";
    figureStr[4] << (int)char_info->DEX;
    statsStr[5] << "Luck:";
    figureStr[5] << (int)char_info->LUK;

    // for testing only...

    //remainingStatsPointsStr << "Remaining Status Points : " << char_info->statsPointsToAttribute;

    // Update labels
    for (i = 0; i < 6; i++) {
        statsLabel[i]->setCaption(statsStr[i].str());
        statsLabel[i]->adjustSize();
        statsDisplayLabel[i]->setCaption(figureStr[i].str());
        statsDisplayLabel[i]->adjustSize();
    }
    remainingStatsPointsLabel->setCaption(remainingStatsPointsStr.str());
    remainingStatsPointsLabel->adjustSize();
}

StatsWindow::~StatsWindow() {
    for (int i = 0; i < 6; i++) {
        delete statsLabel[i];
        delete statsDisplayLabel[i];
        delete statsButton[i];
    }
    delete remainingStatsPointsLabel;
}

void StatsWindow::action(const std::string& eventId) {
    WFIFOW(0) = net_w_value(0x00bb);

    if (eventId == "STR") {
        WFIFOW(2) = net_w_value(0x000d);
    }
    if (eventId == "AGI") {
        WFIFOW(2) = net_w_value(0x000e);
    }
    if (eventId == "VIT") {
        WFIFOW(2) = net_w_value(0x000f);
    }
    if (eventId == "INT") {
        WFIFOW(2) = net_w_value(0x0010);
    }
    if (eventId == "DEX") {
        WFIFOW(2) = net_w_value(0x0011);
    }
    if (eventId == "LUK") {
        WFIFOW(2) = net_w_value(0x0012);
    }

    flush();
    WFIFOW(4) = net_b_value(1);
    WFIFOSET(5);
}
