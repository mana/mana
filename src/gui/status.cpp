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

#include "status.h"

#include <guichan/widgets/label.hpp>
#include <sstream>

#include "button.h"
#include "progressbar.h"

#include "../playerinfo.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../graphics.h"

extern Graphics *graphics;

StatusWindow::StatusWindow():
    Window(player_info->name)
{
    setWindowName("Status");
    setResizable(true);
    setDefaultSize((graphics->getWidth() - 365) / 2,
                   (graphics->getHeight() - 255) / 2, 365, 255);
    loadWindowState();

    // ----------------------
    // Status Part
    // ----------------------

    lvlLabel = new gcn::Label("Level:");
    gpLabel = new gcn::Label("Money:");
    hpLabel = new gcn::Label("HP:");
    hpValueLabel = new gcn::Label("");
    mpLabel = new gcn::Label("MP:");
    mpValueLabel = new gcn::Label("");
    xpLabel = new gcn::Label("Exp:");
    xpValueLabel = new gcn::Label("");
    jobXpLabel = new gcn::Label("Job:");
    jobValueLabel = new gcn::Label("");

    lvlLabel->setPosition(5, 3);
    gpLabel->setPosition(lvlLabel->getX() + lvlLabel->getWidth() + 40, 3);
    hpLabel->setPosition(5, lvlLabel->getY() + lvlLabel->getHeight() + 5);
    hpBar = new ProgressBar(1.0f,
                            hpLabel->getX() + hpLabel->getWidth() + 5,
                            hpLabel->getY(), 80, 15, 0, 171, 34);
    hpValueLabel->setPosition(hpBar->getX() + hpBar->getWidth() + 5,
                              hpBar->getY());
    mpLabel->setPosition(5, hpLabel->getY() + hpLabel->getHeight() + 5);
    mpBar = new ProgressBar(1.0f,
                            hpBar->getX(),
                            mpLabel->getY(), 80, 15, 26, 102, 230);
    mpValueLabel->setPosition(hpValueLabel->getX(), mpBar->getY());

    xpLabel->setPosition(175, hpLabel->getY());
    xpBar = new ProgressBar(1.0f, 205, xpLabel->getY(), 80, 15,
                            143, 192, 211);
    xpValueLabel->setPosition(290, xpBar->getY());
    jobXpLabel->setPosition(175, mpLabel->getY());
    jobXpBar = new ProgressBar(1.0f,
                               xpBar->getX() + xpBar->getWidth() - 60,
                               jobXpLabel->getY(),
                               60, 15,
                               220, 135, 203);
    jobValueLabel->setPosition(290, jobXpBar->getY());

    add(lvlLabel);
    add(gpLabel);
    add(hpLabel);
    add(hpValueLabel);
    add(mpLabel);
    add(mpValueLabel);
    add(xpLabel);
    add(xpValueLabel);
    add(jobXpLabel);
    add(jobValueLabel);
    add(hpBar);
    add(mpBar);
    add(xpBar);
    add(jobXpBar);

    // ----------------------
    // Stats Part
    // ----------------------

    // Static Labels
    statsTitleLabel = new gcn::Label("Stats");
    statsTotalLabel = new gcn::Label("Total");
    statsCostLabel = new gcn::Label("Cost");

    // Derived Stats
    statsAttackLabel = new gcn::Label("Attack:");
    statsDefenseLabel= new gcn::Label("Defense:");
    statsMagicAttackLabel = new gcn::Label("M.Attack:");
    statsMagicDefenseLabel = new gcn::Label("M.Defense:");
    statsAccuracyLabel = new gcn::Label("% Accuracy:");
    statsEvadeLabel = new gcn::Label("% Evade:");
    statsReflexLabel = new gcn::Label("% Reflex:");

    statsAttackPoints = new gcn::Label("");
    statsDefensePoints = new gcn::Label("");
    statsMagicAttackPoints = new gcn::Label("");
    statsMagicDefensePoints = new gcn::Label("");
    statsAccuracyPoints = new gcn::Label("% Accuracy:");
    statsEvadePoints = new gcn::Label("% Evade:");
    statsReflexPoints = new gcn::Label("% Reflex:");

    // New labels
    for (int i = 0; i < 6; i++) {
        statsLabel[i] = new gcn::Label();
        statsDisplayLabel[i] = new gcn::Label();
        pointsLabel[i] = new gcn::Label("0");
    }
    remainingStatsPointsLabel = new gcn::Label();

    // New buttons
    for (int i = 0; i < 6; i++) {
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
    statsTitleLabel->setPosition(mpLabel->getX(), mpLabel->getY() + 23 );
    statsTotalLabel->setPosition(110, statsTitleLabel->getY() + 15);
    int totalLabelY = statsTotalLabel->getY();
    statsCostLabel->setPosition(170, totalLabelY);

    for (int i = 0; i < 6; i++)
    {
        statsLabel[i]->setPosition(5, statsTotalLabel->getY() + (i * 23) + 15);
        statsDisplayLabel[i]->setPosition(115,
                                          totalLabelY + (i * 23) + 15);
        statsButton[i]->setPosition(145, totalLabelY + (i * 23) + 10);
        pointsLabel[i]->setPosition(175, totalLabelY + (i * 23) + 15);
    }

    remainingStatsPointsLabel->setPosition(5, pointsLabel[5]->getY() + 25);

    statsAttackLabel->setPosition(220, statsLabel[0]->getY());
    statsDefenseLabel->setPosition(220, statsLabel[1]->getY());
    statsMagicAttackLabel->setPosition(220, statsLabel[2]->getY());
    statsMagicDefenseLabel->setPosition(220, statsLabel[3]->getY());
    statsAccuracyLabel->setPosition(220, statsLabel[4]->getY());
    statsEvadeLabel->setPosition(220, statsLabel[5]->getY());
    statsReflexLabel->setPosition(220, remainingStatsPointsLabel->getY());

    statsAttackPoints->setPosition(310, statsLabel[0]->getY());
    statsDefensePoints->setPosition(310, statsLabel[1]->getY());
    statsMagicAttackPoints->setPosition(310, statsLabel[2]->getY());
    statsMagicDefensePoints->setPosition(310, statsLabel[3]->getY());
    statsAccuracyPoints->setPosition(310, statsLabel[4]->getY());
    statsEvadePoints->setPosition(310, statsLabel[5]->getY());
    statsReflexPoints->setPosition(310, remainingStatsPointsLabel->getY());

    // Assemble
    add(statsTitleLabel);
    add(statsTotalLabel);
    add(statsCostLabel);
    for(int i = 0; i < 6; i++)
    {
        add(statsLabel[i]);
        add(statsDisplayLabel[i]);
        add(statsButton[i]);
        add(pointsLabel[i]);
        statsButton[i]->addActionListener(this);
    }
    add(statsAttackLabel);
    add(statsDefenseLabel);
    add(statsMagicAttackLabel);
    add(statsMagicDefenseLabel);
    add(statsAccuracyLabel);
    add(statsEvadeLabel);
    add(statsReflexLabel);

    add(statsAttackPoints);
    add(statsDefensePoints);
    add(statsMagicAttackPoints);
    add(statsMagicDefensePoints);
    add(statsAccuracyPoints);
    add(statsEvadePoints);
    add(statsReflexPoints);

    add(remainingStatsPointsLabel);
}

StatusWindow::~StatusWindow()
{
    // Status Part
    delete lvlLabel;
    delete gpLabel;
    delete hpLabel;
    delete hpValueLabel;
    delete mpLabel;
    delete mpValueLabel;
    delete xpLabel;
    delete xpValueLabel;
    delete jobXpLabel;
    delete jobValueLabel;
    delete hpBar;
    delete mpBar;
    delete xpBar;
    delete jobXpBar;

    // Stats Part
    delete statsTitleLabel;
    delete statsTotalLabel;
    delete statsCostLabel;
    for (int i = 0; i < 6; i++)
    {
        delete statsLabel[i];
        delete pointsLabel[i];
        delete statsDisplayLabel[i];
        delete statsButton[i];
    }
    delete statsAttackLabel;
    delete statsDefenseLabel;
    delete statsMagicAttackLabel;
    delete statsMagicDefenseLabel;
    delete statsAccuracyLabel;
    delete statsEvadeLabel;
    delete statsReflexLabel;

    delete statsAttackPoints;
    delete statsDefensePoints;
    delete statsMagicAttackPoints;
    delete statsMagicDefensePoints;
    delete statsAccuracyPoints;
    delete statsEvadePoints;
    delete statsReflexPoints;

    delete remainingStatsPointsLabel;
}

void StatusWindow::update()
{
    std::stringstream updateText;

    // Status Part
    // -----------
    updateText.str("");
    updateText << "Level: " << player_info->lvl;
    lvlLabel->setCaption(updateText.str());
    lvlLabel->adjustSize();

    updateText.str("");
    updateText << "Money: " << player_info->gp << " GP";
    gpLabel->setCaption(updateText.str());
    gpLabel->adjustSize();

    updateText.str("");
    updateText << "Job: " << player_info->jobLvl;
    jobXpLabel->setCaption(updateText.str());
    jobXpLabel->adjustSize();

    updateText.str("");
    updateText << player_info->hp << "/" << player_info->maxHp;
    hpValueLabel->setCaption(updateText.str());
    hpValueLabel->adjustSize();

    updateText.str("");
    updateText << player_info->mp <<  "/" << player_info->maxMp;
    mpValueLabel->setCaption(updateText.str());
    mpValueLabel->adjustSize();

    updateText.str("");
    updateText << (int)player_info->xp << "/" << (int)player_info->xpForNextLevel;
    xpValueLabel->setCaption(updateText.str());
    xpValueLabel->adjustSize();

    updateText.str("");
    updateText << (int)player_info->jobXp << "/" << (int)player_info->jobXpForNextLevel;
    jobValueLabel->setCaption(updateText.str());
    jobValueLabel->adjustSize();

    // HP Bar coloration
    if (player_info->hp < int(player_info->maxHp / 3))
    {
        hpBar->setColor(223, 32, 32); // Red
    }
    else
    {
        if (player_info->hp < int((player_info->maxHp / 3) * 2))
        {
            hpBar->setColor(230, 171, 34); // Orange
        }
        else
        {
            hpBar->setColor(0, 171, 34); // Green
        }
    }

    hpBar->setProgress((float)player_info->hp / (float)player_info->maxHp);
    // mpBar->setProgress((float)player_info->mp / (float)player_info->maxMp);

    xpBar->setProgress(
            (float)player_info->xp / (float)player_info->xpForNextLevel);
    jobXpBar->setProgress(
            (float)player_info->jobXp / (float)player_info->jobXpForNextLevel);

    // Stats Part
    // ----------
    std::stringstream statsStr[6];
    std::stringstream figureStr[6];
    std::stringstream pointsStr[6];

    statsStr[0] << "Strength:";
    figureStr[0] << (int)player_info->STR;
    statsStr[1] << "Agility:";
    figureStr[1] << (int)player_info->AGI;
    statsStr[2] << "Vitality:";
    figureStr[2] << (int)player_info->VIT;
    statsStr[3] << "Intelligence:";
    figureStr[3] << (int)player_info->INT;
    statsStr[4] << "Dexterity:";
    figureStr[4] << (int)player_info->DEX;
    statsStr[5] << "Luck:";
    figureStr[5] << (int)player_info->LUK;

    int statusPoints = player_info->statsPointsToAttribute;

    updateText.str("");
    updateText << "Remaining Status Points: " << statusPoints;

    pointsStr[0] << (int)player_info->STRUp;
    pointsStr[1] << (int)player_info->AGIUp;
    pointsStr[2] << (int)player_info->VITUp;
    pointsStr[3] << (int)player_info->INTUp;
    pointsStr[4] << (int)player_info->DEXUp;
    pointsStr[5] << (int)player_info->LUKUp;

    // Enable buttons for which there are enough status points
    statsButton[0]->setEnabled(player_info->STRUp <= statusPoints);
    statsButton[1]->setEnabled(player_info->AGIUp <= statusPoints);
    statsButton[2]->setEnabled(player_info->VITUp <= statusPoints);
    statsButton[3]->setEnabled(player_info->INTUp <= statusPoints);
    statsButton[4]->setEnabled(player_info->DEXUp <= statusPoints);
    statsButton[5]->setEnabled(player_info->LUKUp <= statusPoints);

    // Update labels
    for (int i = 0; i < 6; i++) {
        statsLabel[i]->setCaption(statsStr[i].str());
        statsLabel[i]->adjustSize();
        statsDisplayLabel[i]->setCaption(figureStr[i].str());
        statsDisplayLabel[i]->adjustSize();
        pointsLabel[i]->setCaption(pointsStr[i].str());
        pointsLabel[i]->adjustSize();
    }
    remainingStatsPointsLabel->setCaption(updateText.str());
    remainingStatsPointsLabel->adjustSize();

    // Derived Stats Points

    // Attack TODO: Count equipped Weapons and items attack bonuses
    updateText.str("");
    updateText << int(player_info->ATK + player_info->ATKBonus);
    statsAttackPoints->setCaption(updateText.str());
    statsAttackPoints->adjustSize();

    // Defense TODO: Count equipped Armors and items defense bonuses
    updateText.str("");
    updateText << int(player_info->DEF + player_info->DEFBonus);
    statsDefensePoints->setCaption(updateText.str());
    statsDefensePoints->adjustSize();

    // Magic Attack TODO: Count equipped items M.Attack bonuses
    updateText.str("");
    updateText << int(player_info->MATK + player_info->MATKBonus);
    statsMagicAttackPoints->setCaption(updateText.str());
    statsMagicAttackPoints->adjustSize();

    // Magic Defense TODO: Count equipped items M.Defense bonuses
    updateText.str("");
    updateText << int(player_info->MDEF + player_info->MDEFBonus);
    statsMagicDefensePoints->setCaption(updateText.str());
    statsMagicDefensePoints->adjustSize();

    // Accuracy %
    updateText.str("");
    updateText << (int)player_info->HIT;
    statsAccuracyPoints->setCaption(updateText.str());
    statsAccuracyPoints->adjustSize();

    // Evasion %
    updateText.str("");
    updateText << (int)player_info->FLEE;
    statsEvadePoints->setCaption(updateText.str());
    statsEvadePoints->adjustSize();

    // Reflex %
    updateText.str("");
    updateText << ((int)player_info->DEX / 4); // + counter
    statsReflexPoints->setCaption(updateText.str());
    statsReflexPoints->adjustSize();

    // Update Second column widgets position
    gpLabel->setPosition(lvlLabel->getX() + lvlLabel->getWidth() + 20,
                         lvlLabel->getY());

    xpLabel->setPosition(hpValueLabel->getX() + hpValueLabel->getWidth() + 10,
                         hpLabel->getY());
    xpBar->setPosition(xpLabel->getX() + xpLabel->getWidth() + 5,
                       xpLabel->getY());
    xpValueLabel->setPosition(xpBar->getX() + xpBar->getWidth() + 5,
                              xpLabel->getY());

    jobXpLabel->setPosition(xpLabel->getX(), mpLabel->getY());
    jobXpBar->setPosition(xpBar->getX() + xpBar->getWidth() -
                          jobXpBar->getWidth(), jobXpLabel->getY());
    jobValueLabel->setPosition(290, jobXpLabel->getY());
}

void StatusWindow::draw(gcn::Graphics *g)
{
    update();

    Window::draw(g);
}

void StatusWindow::action(const std::string& eventId)
{
    // Stats Part
    if (eventId.length() == 3)
    {
        MessageOut outMsg;
        outMsg.writeInt16(CMSG_STAT_UPDATE_REQUEST);

        if (eventId == "STR")
        {
            outMsg.writeInt16(0x000d);
        }
        if (eventId == "AGI")
        {
            outMsg.writeInt16(0x000e);
        }
        if (eventId == "VIT")
        {
            outMsg.writeInt16(0x000f);
        }
        if (eventId == "INT")
        {
            outMsg.writeInt16(0x0010);
        }
        if (eventId == "DEX")
        {
            outMsg.writeInt16(0x0011);
        }
        if (eventId == "LUK")
        {
            outMsg.writeInt16(0x0012);
        }
        outMsg.writeInt8(1);
    }
}
