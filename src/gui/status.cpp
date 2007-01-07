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

#include "button.h"
#include "progressbar.h"
#include "windowcontainer.h"

#include "../localplayer.h"

#include "../utils/tostring.h"

StatusWindow::StatusWindow(LocalPlayer *player):
    Window(player->getName()),
    mPlayer(player)
{
    setWindowName("Status");
    setResizable(true);
    setDefaultSize((windowContainer->getWidth() - 365) / 2,
                   (windowContainer->getHeight() - 255) / 2, 365, 280);
    loadWindowState();

    // ----------------------
    // Status Part
    // ----------------------

    mLvlLabel = new gcn::Label("Level:");
    mMoneyLabel = new gcn::Label("Money:");

    mHpLabel = new gcn::Label("HP:");
    mHpBar = new ProgressBar(1.0f, 80, 15, 0, 171, 34);
    mHpValueLabel = new gcn::Label("");

    mXpLabel = new gcn::Label("Exp:");
    mXpBar = new ProgressBar(1.0f, 80, 15, 143, 192, 211);
    mXpValueLabel = new gcn::Label("");

    mMpLabel = new gcn::Label("MP:");
    mMpBar = new ProgressBar(1.0f, 80, 15, 26, 102, 230);
    mMpValueLabel = new gcn::Label("");

    int y = 3;
    int x = 5;

    mLvlLabel->setPosition(x, y);
    x += mLvlLabel->getWidth() + 40;
    mMoneyLabel->setPosition(x, y);

    y += mLvlLabel->getHeight() + 5; // Next Row
    x = 5;

    mHpLabel->setPosition(x, y);
    x += mHpLabel->getWidth() + 5;
    mHpBar->setPosition(x, y);
    x += mHpBar->getWidth() + 5;
    mHpValueLabel->setPosition(x, y);

    mXpLabel->setPosition(175, y);
    mXpBar->setPosition(205, y);
    mXpValueLabel->setPosition(290, y);

    y += mHpLabel->getHeight() + 5; // Next Row
    x = 5;

    mMpLabel->setPosition(x, y);
    x += mMpLabel->getWidth() + 5;
    mMpBar->setPosition(x, y);
    x += mMpBar->getWidth() + 5;
    mMpValueLabel->setPosition(x, y);

    add(mLvlLabel);
    add(mMoneyLabel);
    add(mHpLabel);
    add(mHpValueLabel);
    add(mMpLabel);
    add(mMpValueLabel);
    add(mXpLabel);
    add(mXpValueLabel);
    add(mHpBar);
    add(mMpBar);
    add(mXpBar);

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
    for (int i = 0; i < 7; i++) {
        mStatsLabel[i] = new gcn::Label();
        mStatsDisplayLabel[i] = new gcn::Label();
        mPointsLabel[i] = new gcn::Label("0");
    }
    mRemainingStatsPointsLabel = new gcn::Label();

    // Set button events Id
    mStatsButton[0] = new Button("+", "STR", this);
    mStatsButton[1] = new Button("+", "AGI", this);
    mStatsButton[2] = new Button("+", "DEX", this);
    mStatsButton[3] = new Button("+", "VIT", this);
    mStatsButton[4] = new Button("+", "INT", this);
    mStatsButton[5] = new Button("+", "WIL", this);
    mStatsButton[6] = new Button("+", "CHR", this);


    // Set position
    mStatsTitleLabel->setPosition(mMpLabel->getX(), mMpLabel->getY() + 23 );
    mStatsTotalLabel->setPosition(110, mStatsTitleLabel->getY() + 15);
    int totalLabelY = mStatsTotalLabel->getY();
    mStatsCostLabel->setPosition(170, totalLabelY);

    for (int i = 0; i < 7; i++)
    {
        mStatsLabel[i]->setPosition(5,
                                    mStatsTotalLabel->getY() + (i * 23) + 15);
        mStatsDisplayLabel[i]->setPosition(115,
                                          totalLabelY + (i * 23) + 15);
        mStatsButton[i]->setPosition(145, totalLabelY + (i * 23) + 10);
        mPointsLabel[i]->setPosition(175, totalLabelY + (i * 23) + 15);
    }

    mRemainingStatsPointsLabel->setPosition(5, mPointsLabel[6]->getY() + 25);

    mStatsAttackLabel->setPosition(220, mStatsLabel[0]->getY());
    mStatsDefenseLabel->setPosition(220, mStatsLabel[1]->getY());
    mStatsMagicAttackLabel->setPosition(220, mStatsLabel[2]->getY());
    mStatsMagicDefenseLabel->setPosition(220, mStatsLabel[3]->getY());
    mStatsAccuracyLabel->setPosition(220, mStatsLabel[4]->getY());
    mStatsEvadeLabel->setPosition(220, mStatsLabel[5]->getY());
    mStatsReflexLabel->setPosition(220, mStatsLabel[6]->getY());

    mStatsAttackPoints->setPosition(310, mStatsLabel[0]->getY());
    mStatsDefensePoints->setPosition(310, mStatsLabel[1]->getY());
    mStatsMagicAttackPoints->setPosition(310, mStatsLabel[2]->getY());
    mStatsMagicDefensePoints->setPosition(310, mStatsLabel[3]->getY());
    mStatsAccuracyPoints->setPosition(310, mStatsLabel[4]->getY());
    mStatsEvadePoints->setPosition(310, mStatsLabel[5]->getY());
    mStatsReflexPoints->setPosition(310, mStatsLabel[6]->getY());

    // Assemble
    add(mStatsTitleLabel);
    add(mStatsTotalLabel);
    add(mStatsCostLabel);
    for(int i = 0; i < 7; i++)
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
    // Status Part
    // -----------
    mLvlLabel->setCaption("Level: " + toString(mPlayer->mLevel));
    mLvlLabel->adjustSize();

    mMoneyLabel->setCaption("Money: " + toString(mPlayer->mMoney) + " GP");
    mMoneyLabel->adjustSize();

    mHpValueLabel->setCaption(toString(mPlayer->mHp) +
            "/" + toString(mPlayer->mMaxHp));
    mHpValueLabel->adjustSize();

    mMpValueLabel->setCaption(toString(mPlayer->mMp) +
            "/" + toString(mPlayer->mMaxMp));
    mMpValueLabel->adjustSize();

    mXpValueLabel->setCaption(toString(mPlayer->mXp) +
            "/" + toString(mPlayer->mXpForNextLevel));
    mXpValueLabel->adjustSize();

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
            (float) mPlayer->mXp / (float) mPlayer->mXpForNextLevel);

    // Stats Part
    // ----------
    const std::string attrNames[7] = {
        "Strength",
        "Agility",
        "Dexterity",
        "Vitality",
        "Intelligence",
        "Willpower",
        "Charisma"
    };
    int statusPoints = mPlayer->mStatsPointsToAttribute;

    // Update labels
    for (int i = 0; i < 7; i++) {
        mStatsLabel[i]->setCaption(attrNames[i]);
        mStatsDisplayLabel[i]->setCaption(toString((int) mPlayer->mAttr[i]));
        mPointsLabel[i]->setCaption(toString((int) mPlayer->mAttrUp[i]));

        mStatsLabel[i]->adjustSize();
        mStatsDisplayLabel[i]->adjustSize();
        mPointsLabel[i]->adjustSize();

        mStatsButton[i]->setEnabled(mPlayer->mAttrUp[i] <= statusPoints);
    }
    mRemainingStatsPointsLabel->setCaption("Remaining Status Points: " +
            toString(statusPoints));
    mRemainingStatsPointsLabel->adjustSize();

    // Derived Stats Points

    // Attack TODO: Count equipped Weapons and items attack bonuses
    mStatsAttackPoints->setCaption(
            toString(mPlayer->ATK + mPlayer->ATK_BONUS));
    mStatsAttackPoints->adjustSize();

    // Defense TODO: Count equipped Armors and items defense bonuses
    mStatsDefensePoints->setCaption(
            toString(mPlayer->DEF + mPlayer->DEF_BONUS));
    mStatsDefensePoints->adjustSize();

    // Magic Attack TODO: Count equipped items M.Attack bonuses
    mStatsMagicAttackPoints->setCaption(
            toString(mPlayer->MATK + mPlayer->MATK_BONUS));
    mStatsMagicAttackPoints->adjustSize();

    // Magic Defense TODO: Count equipped items M.Defense bonuses
    mStatsMagicDefensePoints->setCaption(
            toString(mPlayer->MDEF + mPlayer->MDEF_BONUS));
    mStatsMagicDefensePoints->adjustSize();

    // Accuracy %
    mStatsAccuracyPoints->setCaption(toString(mPlayer->HIT));
    mStatsAccuracyPoints->adjustSize();

    // Evasion %
    mStatsEvadePoints->setCaption(toString(mPlayer->FLEE));
    mStatsEvadePoints->adjustSize();

    // Reflex %
    mStatsReflexPoints->setCaption(toString(mPlayer->DEX / 4)); // + counter
    mStatsReflexPoints->adjustSize();

    // Update Second column widgets position
    mMoneyLabel->setPosition(mLvlLabel->getX() + mLvlLabel->getWidth() + 20,
                         mLvlLabel->getY());

    mXpLabel->setPosition(
            mHpValueLabel->getX() + mHpValueLabel->getWidth() + 10,
            mHpLabel->getY());
    mXpBar->setPosition(
            mXpLabel->getX() + mXpLabel->getWidth() + 5,
            mXpLabel->getY());
    mXpValueLabel->setPosition(
            mXpBar->getX() + mXpBar->getWidth() + 5,
            mXpLabel->getY());
}

void StatusWindow::draw(gcn::Graphics *g)
{
    update();

    Window::draw(g);
}

void StatusWindow::action(const std::string &eventId, gcn::Widget *widget)
{
    // Stats Part
    if (eventId.length() == 3)
    {
        if (eventId == "STR")
        {
            mPlayer->raiseAttribute(LocalPlayer::STR);
        }
        else if (eventId == "AGI")
        {
            mPlayer->raiseAttribute(LocalPlayer::AGI);
        }
        else if (eventId == "DEX")
        {
            mPlayer->raiseAttribute(LocalPlayer::DEX);
        }
        else if (eventId == "VIT")
        {
            mPlayer->raiseAttribute(LocalPlayer::VIT);
        }
        else if (eventId == "INT")
        {
            mPlayer->raiseAttribute(LocalPlayer::INT);
        }
        else if (eventId == "WIL")
        {
            mPlayer->raiseAttribute(LocalPlayer::WIL);
        }
        else if (eventId == "CHR")
        {
            mPlayer->raiseAttribute(LocalPlayer::CHR);
        }
    }
}
