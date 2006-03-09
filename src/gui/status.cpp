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

    mLvlLabel = new gcn::Label("Level:");
    mGpLabel = new gcn::Label("Money:");
    mHpLabel = new gcn::Label("HP:");
    mHpValueLabel = new gcn::Label("");
    mMpLabel = new gcn::Label("MP:");
    mMpValueLabel = new gcn::Label("");
    mXpLabel = new gcn::Label("Exp:");
    mXpValueLabel = new gcn::Label("");
    mJobXpLabel = new gcn::Label("Job:");
    mJobValueLabel = new gcn::Label("");

    mLvlLabel->setPosition(5, 3);
    mGpLabel->setPosition(mLvlLabel->getX() + mLvlLabel->getWidth() + 40, 3);
    mHpLabel->setPosition(5, mLvlLabel->getY() + mLvlLabel->getHeight() + 5);
    mHpBar = new ProgressBar(1.0f,
                            mHpLabel->getX() + mHpLabel->getWidth() + 5,
                            mHpLabel->getY(), 80, 15, 0, 171, 34);
    mHpValueLabel->setPosition(mHpBar->getX() + mHpBar->getWidth() + 5,
                              mHpBar->getY());
    mMpLabel->setPosition(5, mHpLabel->getY() + mHpLabel->getHeight() + 5);
    mMpBar = new ProgressBar(1.0f,
                            mHpBar->getX(),
                            mMpLabel->getY(), 80, 15, 26, 102, 230);
    mMpValueLabel->setPosition(mHpValueLabel->getX(), mMpBar->getY());

    mXpLabel->setPosition(175, mHpLabel->getY());
    mXpBar = new ProgressBar(1.0f, 205, mXpLabel->getY(), 80, 15,
                            143, 192, 211);
    mXpValueLabel->setPosition(290, mXpBar->getY());
    mJobXpLabel->setPosition(175, mMpLabel->getY());
    mJobXpBar = new ProgressBar(1.0f,
                               mXpBar->getX() + mXpBar->getWidth() - 60,
                               mJobXpLabel->getY(),
                               60, 15,
                               220, 135, 203);
    mJobValueLabel->setPosition(290, mJobXpBar->getY());

    add(mLvlLabel);
    add(mGpLabel);
    add(mHpLabel);
    add(mHpValueLabel);
    add(mMpLabel);
    add(mMpValueLabel);
    add(mXpLabel);
    add(mXpValueLabel);
    add(mJobXpLabel);
    add(mJobValueLabel);
    add(mHpBar);
    add(mMpBar);
    add(mXpBar);
    add(mJobXpBar);

    // ----------------------
    // Stats Part
    // ----------------------

    // Static Labels
    gcn::Label *mStatsTitleLabel = new gcn::Label("Stats");
    gcn::Label *mStatsTotalLabel = new gcn::Label("Total");
    gcn::Label *mStatsCostLabel = new gcn::Label("Cost");

    // Derived Stats
    mStatsAttackLabel = new gcn::Label("Attack:");
    mStatsDefenseLabel= new gcn::Label("Defense:");
    mStatsMagicAttackLabel = new gcn::Label("M.Attack:");
    mStatsMagicDefenseLabel = new gcn::Label("M.Defense:");
    mStatsAccuracyLabel = new gcn::Label("% Accuracy:");
    mStatsEvadeLabel = new gcn::Label("% Evade:");
    mStatsReflexLabel = new gcn::Label("% Reflex:");

    mStatsAttackPoints = new gcn::Label("");
    mStatsDefensePoints = new gcn::Label("");
    mStatsMagicAttackPoints = new gcn::Label("");
    mStatsMagicDefensePoints = new gcn::Label("");
    mStatsAccuracyPoints = new gcn::Label("% Accuracy:");
    mStatsEvadePoints = new gcn::Label("% Evade:");
    mStatsReflexPoints = new gcn::Label("% Reflex:");

    // New labels
    for (int i = 0; i < 6; i++) {
        mStatsLabel[i] = new gcn::Label();
        mStatsDisplayLabel[i] = new gcn::Label();
        mPointsLabel[i] = new gcn::Label("0");
    }
    mRemainingStatsPointsLabel = new gcn::Label();

    // Set button events Id
    mStatsButton[0] = new Button("+", "STR", this);
    mStatsButton[1] = new Button("+", "AGI", this);
    mStatsButton[2] = new Button("+", "VIT", this);
    mStatsButton[3] = new Button("+", "INT", this);
    mStatsButton[4] = new Button("+", "DEX", this);
    mStatsButton[5] = new Button("+", "LUK", this);


    // Set position
    mStatsTitleLabel->setPosition(mMpLabel->getX(), mMpLabel->getY() + 23 );
    mStatsTotalLabel->setPosition(110, mStatsTitleLabel->getY() + 15);
    int totalLabelY = mStatsTotalLabel->getY();
    mStatsCostLabel->setPosition(170, totalLabelY);

    for (int i = 0; i < 6; i++)
    {
        mStatsLabel[i]->setPosition(5, mStatsTotalLabel->getY() + (i * 23) + 15);
        mStatsDisplayLabel[i]->setPosition(115,
                                          totalLabelY + (i * 23) + 15);
        mStatsButton[i]->setPosition(145, totalLabelY + (i * 23) + 10);
        mPointsLabel[i]->setPosition(175, totalLabelY + (i * 23) + 15);
    }

    mRemainingStatsPointsLabel->setPosition(5, mPointsLabel[5]->getY() + 25);

    mStatsAttackLabel->setPosition(220, mStatsLabel[0]->getY());
    mStatsDefenseLabel->setPosition(220, mStatsLabel[1]->getY());
    mStatsMagicAttackLabel->setPosition(220, mStatsLabel[2]->getY());
    mStatsMagicDefenseLabel->setPosition(220, mStatsLabel[3]->getY());
    mStatsAccuracyLabel->setPosition(220, mStatsLabel[4]->getY());
    mStatsEvadeLabel->setPosition(220, mStatsLabel[5]->getY());
    mStatsReflexLabel->setPosition(220, mRemainingStatsPointsLabel->getY());

    mStatsAttackPoints->setPosition(310, mStatsLabel[0]->getY());
    mStatsDefensePoints->setPosition(310, mStatsLabel[1]->getY());
    mStatsMagicAttackPoints->setPosition(310, mStatsLabel[2]->getY());
    mStatsMagicDefensePoints->setPosition(310, mStatsLabel[3]->getY());
    mStatsAccuracyPoints->setPosition(310, mStatsLabel[4]->getY());
    mStatsEvadePoints->setPosition(310, mStatsLabel[5]->getY());
    mStatsReflexPoints->setPosition(310, mRemainingStatsPointsLabel->getY());

    // Assemble
    add(mStatsTitleLabel);
    add(mStatsTotalLabel);
    add(mStatsCostLabel);
    for(int i = 0; i < 6; i++)
    {
        add(mStatsLabel[i]);
        add(mStatsDisplayLabel[i]);
        add(mStatsButton[i]);
        add(mPointsLabel[i]);
    }
    add(mStatsAttackLabel);
    add(mStatsDefenseLabel);
    add(mStatsMagicAttackLabel);
    add(mStatsMagicDefenseLabel);
    add(mStatsAccuracyLabel);
    add(mStatsEvadeLabel);
    add(mStatsReflexLabel);

    add(mStatsAttackPoints);
    add(mStatsDefensePoints);
    add(mStatsMagicAttackPoints);
    add(mStatsMagicDefensePoints);
    add(mStatsAccuracyPoints);
    add(mStatsEvadePoints);
    add(mStatsReflexPoints);

    add(mRemainingStatsPointsLabel);
}

void StatusWindow::update()
{
    std::stringstream updateText;

    // Status Part
    // -----------
    updateText.str("");
    updateText << "Level: " << mPlayer->mLevel;
    mLvlLabel->setCaption(updateText.str());
    mLvlLabel->adjustSize();

    updateText.str("");
    updateText << "Money: " << mPlayer->mGp << " GP";
    mGpLabel->setCaption(updateText.str());
    mGpLabel->adjustSize();

    updateText.str("");
    updateText << "Job: " << mPlayer->mJobLevel;
    mJobXpLabel->setCaption(updateText.str());
    mJobXpLabel->adjustSize();

    updateText.str("");
    updateText << mPlayer->mHp << "/" << mPlayer->mMaxHp;
    mHpValueLabel->setCaption(updateText.str());
    mHpValueLabel->adjustSize();

    updateText.str("");
    updateText << mPlayer->mMp <<  "/" << mPlayer->mMaxMp;
    mMpValueLabel->setCaption(updateText.str());
    mMpValueLabel->adjustSize();

    updateText.str("");
    updateText << (int)mPlayer->mXp << "/" << (int)mPlayer->mXpForNextLevel;
    mXpValueLabel->setCaption(updateText.str());
    mXpValueLabel->adjustSize();

    updateText.str("");
    updateText << (int)mPlayer->mJobXp << "/" << (int)mPlayer->mJobXpForNextLevel;
    mJobValueLabel->setCaption(updateText.str());
    mJobValueLabel->adjustSize();

    // HP Bar coloration
    if (mPlayer->mHp < int(mPlayer->mMaxHp / 3))
    {
        mHpBar->setColor(223, 32, 32); // Red
    }
    else if (mPlayer->mHp < int((mPlayer->mMaxHp / 3) * 2))
    {
        mHpBar->setColor(230, 171, 34); // Orange
    }
    else
    {
        mHpBar->setColor(0, 171, 34); // Green
    }

    mHpBar->setProgress((float)mPlayer->mHp / (float)mPlayer->mMaxHp);
    // mMpBar->setProgress((float)mPlayer->mp / (float)mPlayer->maxMp);

    mXpBar->setProgress(
            (float)mPlayer->mXp / (float)mPlayer->mXpForNextLevel);
    mJobXpBar->setProgress(
            (float)mPlayer->mJobXp / (float)mPlayer->mJobXpForNextLevel);

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

    int statusPoints = mPlayer->mStatsPointsToAttribute;

    updateText.str("");
    updateText << "Remaining Status Points: " << statusPoints;

    // Update labels
    for (int i = 0; i < 6; i++) {
        std::stringstream sstr;

        mStatsLabel[i]->setCaption(attrNames[i]);
        mStatsLabel[i]->adjustSize();

        sstr.str("");
        sstr << (int)mPlayer->mAttr[i];
        mStatsDisplayLabel[i]->setCaption(sstr.str());
        mStatsDisplayLabel[i]->adjustSize();

        sstr.str("");
        sstr << (int)mPlayer->mAttrUp[i];
        mPointsLabel[i]->setCaption(sstr.str());
        mPointsLabel[i]->adjustSize();

        mStatsButton[i]->setEnabled(mPlayer->mAttrUp[i] <= statusPoints);
    }
    mRemainingStatsPointsLabel->setCaption(updateText.str());
    mRemainingStatsPointsLabel->adjustSize();

    // Derived Stats Points

    // Attack TODO: Count equipped Weapons and items attack bonuses
    updateText.str("");
    updateText << int(mPlayer->ATK + mPlayer->ATK_BONUS);
    mStatsAttackPoints->setCaption(updateText.str());
    mStatsAttackPoints->adjustSize();

    // Defense TODO: Count equipped Armors and items defense bonuses
    updateText.str("");
    updateText << int(mPlayer->DEF + mPlayer->DEF_BONUS);
    mStatsDefensePoints->setCaption(updateText.str());
    mStatsDefensePoints->adjustSize();

    // Magic Attack TODO: Count equipped items M.Attack bonuses
    updateText.str("");
    updateText << int(mPlayer->MATK + mPlayer->MATK_BONUS);
    mStatsMagicAttackPoints->setCaption(updateText.str());
    mStatsMagicAttackPoints->adjustSize();

    // Magic Defense TODO: Count equipped items M.Defense bonuses
    updateText.str("");
    updateText << int(mPlayer->MDEF + mPlayer->MDEF_BONUS);
    mStatsMagicDefensePoints->setCaption(updateText.str());
    mStatsMagicDefensePoints->adjustSize();

    // Accuracy %
    updateText.str("");
    updateText << (int)mPlayer->HIT;
    mStatsAccuracyPoints->setCaption(updateText.str());
    mStatsAccuracyPoints->adjustSize();

    // Evasion %
    updateText.str("");
    updateText << (int)mPlayer->FLEE;
    mStatsEvadePoints->setCaption(updateText.str());
    mStatsEvadePoints->adjustSize();

    // Reflex %
    updateText.str("");
    updateText << ((int)mPlayer->DEX / 4); // + counter
    mStatsReflexPoints->setCaption(updateText.str());
    mStatsReflexPoints->adjustSize();

    // Update Second column widgets position
    mGpLabel->setPosition(mLvlLabel->getX() + mLvlLabel->getWidth() + 20,
                         mLvlLabel->getY());

    mXpLabel->setPosition(mHpValueLabel->getX() + mHpValueLabel->getWidth() + 10,
                         mHpLabel->getY());
    mXpBar->setPosition(mXpLabel->getX() + mXpLabel->getWidth() + 5,
                       mXpLabel->getY());
    mXpValueLabel->setPosition(mXpBar->getX() + mXpBar->getWidth() + 5,
                              mXpLabel->getY());

    mJobXpLabel->setPosition(mXpLabel->getX(), mMpLabel->getY());
    mJobXpBar->setPosition(mXpBar->getX() + mXpBar->getWidth() -
                          mJobXpBar->getWidth(), mJobXpLabel->getY());
    mJobValueLabel->setPosition(290, mJobXpLabel->getY());
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
