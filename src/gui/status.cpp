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

#include "../localplayer.h"

#include "../graphics.h"

extern Graphics *graphics;

StatusWindow::StatusWindow(LocalPlayer *player):
    Window(player->getName()),
    mPlayer(player)
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

void StatusWindow::update()
{
    std::stringstream updateText;

    // Status Part
    // -----------
    updateText.str("");
    updateText << "Level: " << mPlayer->lvl;
    lvlLabel->setCaption(updateText.str());
    lvlLabel->adjustSize();

    updateText.str("");
    updateText << "Money: " << mPlayer->gp << " GP";
    gpLabel->setCaption(updateText.str());
    gpLabel->adjustSize();

    updateText.str("");
    updateText << "Job: " << mPlayer->jobLvl;
    jobXpLabel->setCaption(updateText.str());
    jobXpLabel->adjustSize();

    updateText.str("");
    updateText << mPlayer->hp << "/" << mPlayer->maxHp;
    hpValueLabel->setCaption(updateText.str());
    hpValueLabel->adjustSize();

    updateText.str("");
    updateText << mPlayer->mp <<  "/" << mPlayer->maxMp;
    mpValueLabel->setCaption(updateText.str());
    mpValueLabel->adjustSize();

    updateText.str("");
    updateText << (int)mPlayer->xp << "/" << (int)mPlayer->xpForNextLevel;
    xpValueLabel->setCaption(updateText.str());
    xpValueLabel->adjustSize();

    updateText.str("");
    updateText << (int)mPlayer->jobXp << "/" << (int)mPlayer->jobXpForNextLevel;
    jobValueLabel->setCaption(updateText.str());
    jobValueLabel->adjustSize();

    // HP Bar coloration
    if (mPlayer->hp < int(mPlayer->maxHp / 3))
    {
        hpBar->setColor(223, 32, 32); // Red
    }
    else if (mPlayer->hp < int((mPlayer->maxHp / 3) * 2))
    {
        hpBar->setColor(230, 171, 34); // Orange
    }
    else
    {
        hpBar->setColor(0, 171, 34); // Green
    }

    hpBar->setProgress((float)mPlayer->hp / (float)mPlayer->maxHp);
    // mpBar->setProgress((float)mPlayer->mp / (float)mPlayer->maxMp);

    xpBar->setProgress(
            (float)mPlayer->xp / (float)mPlayer->xpForNextLevel);
    jobXpBar->setProgress(
            (float)mPlayer->jobXp / (float)mPlayer->jobXpForNextLevel);

    // Stats Part
    // ----------
    static const std::string attrNames[6] = {
        "Strength",
        "Agility",
        "Vitality",
        "Intelligence",
        "Dexterity",
        "Luck"
    };

    int statusPoints = mPlayer->statsPointsToAttribute;

    updateText.str("");
    updateText << "Remaining Status Points: " << statusPoints;

    // Update labels
    for (int i = 0; i < 6; i++) {
        std::stringstream sstr;

        statsLabel[i]->setCaption(attrNames[i]);
        statsLabel[i]->adjustSize();

        sstr.str("");
        sstr << (int)mPlayer->ATTR[i];
        statsDisplayLabel[i]->setCaption(sstr.str());
        statsDisplayLabel[i]->adjustSize();

        sstr.str("");
        sstr << (int)mPlayer->ATTR_UP[i];
        pointsLabel[i]->setCaption(sstr.str());
        pointsLabel[i]->adjustSize();

        statsButton[i]->setEnabled(mPlayer->ATTR_UP[i] <= statusPoints);
    }
    remainingStatsPointsLabel->setCaption(updateText.str());
    remainingStatsPointsLabel->adjustSize();

    // Derived Stats Points

    // Attack TODO: Count equipped Weapons and items attack bonuses
    updateText.str("");
    updateText << int(mPlayer->ATK + mPlayer->ATK_BONUS);
    statsAttackPoints->setCaption(updateText.str());
    statsAttackPoints->adjustSize();

    // Defense TODO: Count equipped Armors and items defense bonuses
    updateText.str("");
    updateText << int(mPlayer->DEF + mPlayer->DEF_BONUS);
    statsDefensePoints->setCaption(updateText.str());
    statsDefensePoints->adjustSize();

    // Magic Attack TODO: Count equipped items M.Attack bonuses
    updateText.str("");
    updateText << int(mPlayer->MATK + mPlayer->MATK_BONUS);
    statsMagicAttackPoints->setCaption(updateText.str());
    statsMagicAttackPoints->adjustSize();

    // Magic Defense TODO: Count equipped items M.Defense bonuses
    updateText.str("");
    updateText << int(mPlayer->MDEF + mPlayer->MDEF_BONUS);
    statsMagicDefensePoints->setCaption(updateText.str());
    statsMagicDefensePoints->adjustSize();

    // Accuracy %
    updateText.str("");
    updateText << (int)mPlayer->HIT;
    statsAccuracyPoints->setCaption(updateText.str());
    statsAccuracyPoints->adjustSize();

    // Evasion %
    updateText.str("");
    updateText << (int)mPlayer->FLEE;
    statsEvadePoints->setCaption(updateText.str());
    statsEvadePoints->adjustSize();

    // Reflex %
    updateText.str("");
    updateText << ((int)mPlayer->DEX / 4); // + counter
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
        if (eventId == "STR")
        {
            player_node->raiseAttribute(LocalPlayer::STR);
        }
        if (eventId == "AGI")
        {
            player_node->raiseAttribute(LocalPlayer::AGI);
        }
        if (eventId == "VIT")
        {
            player_node->raiseAttribute(LocalPlayer::VIT);
        }
        if (eventId == "INT")
        {
            player_node->raiseAttribute(LocalPlayer::INT);
        }
        if (eventId == "DEX")
        {
            player_node->raiseAttribute(LocalPlayer::DEX);
        }
        if (eventId == "LUK")
        {
            player_node->raiseAttribute(LocalPlayer::LUK);
        }
    }
}
