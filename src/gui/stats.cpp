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

/**
 * Constructor
 */
StatsWindow::StatsWindow(gcn::Container *parent)
    : Window(parent, "Stats") 
{
    std::stringstream statsStr[6];

    statsStr[0] << "Strenght: "  << char_info->STR;
    statsStr[1] << "Agility: "  << char_info->AGI;
    statsStr[2] << "Vitality: "  << char_info->VIT;
    statsStr[3] << "Inteligence: "  << char_info->INT;
    statsStr[4] << "Dexternity: "  << char_info->DEX;
    statsStr[5] << "Luck: "  << char_info->LUK;
    
    /* New labels */
    for (i = 0; i < 6; i++)
        statsLabel[i] = new gcn::Label(statsStr[i].str());
    
    /* New buttons */
    for (i = 0; i < 6; i++)
        statsButton[i] = new Button("+");
    
    /* Set button events Id */
    statsButton[0]->setEventId("STR");
    statsButton[1]->setEventId("AGI");
    statsButton[2]->setEventId("VIT");
    statsButton[3]->setEventId("INT");
    statsButton[4]->setEventId("DEX");
    statsButton[5]->setEventId("LUK");
    
    /* Set position */
    for (i = 0; i < 6; i++) {
        statsLabel[i]->setPosition(10,(i*22)+10);
        statsButton[i]->setPosition(170,(i*22)+10);
    }
    
    /* Assemble */
    for(i = 0; i < 6; i++) {
        add(statsLabel[i]);
        add(statsButton[i]);
    }
    
    setSize(200, 150);
    setLocationRelativeTo(getParent());
}
/**
 * Method updates stats in window
 */
void StatsWindow::update(){
    std::stringstream statsStr[6];
    
    statsStr[0] << "Strenght: "  << char_info->STR;
    statsStr[1] << "Agility: "  << char_info->AGI;
    statsStr[2] << "Vitality: "  << char_info->VIT;
    statsStr[3] << "Inteligence: "  << char_info->INT;
    statsStr[4] << "Dexternity: "  << char_info->DEX;
    statsStr[5] << "Luck: "  << char_info->LUK;
    
    /* Update labels */
    for (i = 0; i < 6; i++)
        statsLabel[i]->setCaption(statsStr[i].str());
}

/**
 * Destructor
 */
StatsWindow::~StatsWindow() {
    for(int i = 0; i < 6; i++) {
        delete statsLabel[i];
        delete statsButton[i];
    }
}

/**
 * Static method for creating singleton objects
 */
StatsWindow * StatsWindow::ptr = NULL;
StatsWindow * StatsWindow::create_statswindow() {
    if (ptr == NULL)
        ptr = new StatsWindow(guiTop);
    else
        ptr->setVisible(true);
    return ptr;
}

/**
 * Event handling method
 */
void StatsWindow::action(const std::string& eventId) {
    //TODO: update char_info
    if (eventId == "STR") {
        puts("STR");
        update();
    }
    if (eventId == "AGI") {
        puts("AGI");
        update();
    }
    if (eventId == "VIT") {
        puts("VIT");
        update();
    }
    if (eventId == "INT") {
        puts("INT");
        update();    
    }
    if (eventId == "DEX") {
        puts("DEX");
        update();
    }
    if (eventId == "LUK") {
        puts("LUK");
        update();
    }
    puts("default");
}
