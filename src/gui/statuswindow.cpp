/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gui/statuswindow.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/windowcontainer.h"

#include "localplayer.h"

#include "utils/stringutils.h"

StatusWindow::StatusWindow(LocalPlayer *player):
    Window(player->getName()),
    mPlayer(player)
{
    setWindowName("Status");
    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize((windowContainer->getWidth() - 365) / 2,
                   (windowContainer->getHeight() - 255) / 2, 365, 275);
    loadWindowState();

    // ----------------------
    // Status Part
    // ----------------------

    mLvlLabel = new Label("Level:");
    mMoneyLabel = new Label("Money:");

    mHpLabel = new Label("HP:");
    mHpBar = new ProgressBar(0.0f, 80, 15, gcn::Color(0, 171, 34));
    mHpValueLabel = new Label;

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

    y += mHpLabel->getHeight() + 5; // Next Row
    x = 5;

    add(mLvlLabel);
    add(mMoneyLabel);
    add(mHpLabel);
    add(mHpValueLabel);
    add(mHpBar);

    // ----------------------
    // Stats Part
    // ----------------------

    // Static Labels
    gcn::Label *mStatsTitleLabel = new Label("Stats");
    gcn::Label *mStatsTotalLabel = new Label("Total");

    // Derived Stats
/*
    mStatsAttackLabel = new Label("Attack:");
    mStatsDefenseLabel= new Label("Defense:");
    mStatsMagicAttackLabel = new Label("M.Attack:");
    mStatsMagicDefenseLabel = new Label("M.Defense:");
    mStatsAccuracyLabel = new Label("% Accuracy:");
    mStatsEvadeLabel = new Label("% Evade:");
    mStatsReflexLabel = new Label("% Reflex:");

    mStatsAttackPoints = new Label;
    mStatsDefensePoints = new Label;
    mStatsMagicAttackPoints = new Label;
    mStatsMagicDefensePoints = new Label;
    mStatsAccuracyPoints = new Label("% Accuracy:");
    mStatsEvadePoints = new Label("% Evade:");
    mStatsReflexPoints = new Label("% Reflex:");
*/
    // New labels
    for (int i = 0; i < 6; i++) {
        mStatsLabel[i] = new Label;
        mStatsDisplayLabel[i] = new Label;
    }
    mCharacterPointsLabel = new Label;
    mCorrectionPointsLabel = new Label;

    // Set button events Id
    mStatsPlus[0] = new Button("+", "STR+", this);
    mStatsPlus[1] = new Button("+", "AGI+", this);
    mStatsPlus[2] = new Button("+", "DEX+", this);
    mStatsPlus[3] = new Button("+", "VIT+", this);
    mStatsPlus[4] = new Button("+", "INT+", this);
    mStatsPlus[5] = new Button("+", "WIL+", this);

    mStatsMinus[0] = new Button("-", "STR-", this);
    mStatsMinus[1] = new Button("-", "AGI-", this);
    mStatsMinus[2] = new Button("-", "DEX-", this);
    mStatsMinus[3] = new Button("-", "VIT-", this);
    mStatsMinus[4] = new Button("-", "INT-", this);
    mStatsMinus[5] = new Button("-", "WIL-", this);



    // Set position
    mStatsTitleLabel->setPosition(mHpLabel->getX(), mHpLabel->getY() + 23 );
    mStatsTotalLabel->setPosition(110, mStatsTitleLabel->getY() + 15);
    int totalLabelY = mStatsTotalLabel->getY();

    for (int i = 0; i < 6; i++)
    {
        mStatsLabel[i]->setPosition(5,
                                    mStatsTotalLabel->getY() + (i * 23) + 15);
        mStatsMinus[i]->setPosition(85, totalLabelY + (i * 23) + 15);
        mStatsDisplayLabel[i]->setPosition(125,
                                          totalLabelY + (i * 23) + 15);
        mStatsPlus[i]->setPosition(185, totalLabelY + (i * 23) + 15);
    }

    mCharacterPointsLabel->setPosition(5, mStatsDisplayLabel[5]->getY() + 25);
    mCorrectionPointsLabel->setPosition(5, mStatsDisplayLabel[5]->getY() + 35);
/*
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
*/
    // Assemble
    add(mStatsTitleLabel);
    add(mStatsTotalLabel);
    for(int i = 0; i < 6; i++)
    {
        add(mStatsLabel[i]);
        add(mStatsDisplayLabel[i]);
        add(mStatsPlus[i]);
        add(mStatsMinus[i]);
    }/*
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
    add(mStatsReflexPoints);*/

    add(mCharacterPointsLabel);
    add(mCorrectionPointsLabel);
}

void StatusWindow::update()
{
    // Status Part
    // -----------
    mLvlLabel->setCaption(  "Level: " +
                            toString(mPlayer->getLevel()) +
                            " (" +
                            toString(mPlayer->getLevelProgress()) +
                            "%)");
    mLvlLabel->adjustSize();

    mMoneyLabel->setCaption("Money: " + toString(mPlayer->getMoney()) + " GP");
    mMoneyLabel->adjustSize();

    updateHPBar(mHpBar, true);

    // Stats Part
    // ----------
    const std::string attrNames[6] = {
        "Strength",
        "Agility",
        "Dexterity",
        "Vitality",
        "Intelligence",
        "Willpower"
    };
    int characterPoints = mPlayer->getCharacterPoints();
    int correctionPoints = mPlayer->getCorrectionPoints();
    // Update labels
    for (int i = 0; i < 6; i++)
    {
        mStatsLabel[i]->setCaption(attrNames[i]);
        mStatsDisplayLabel[i]->setCaption(
            strprintf("%d / %d",
                mPlayer->getAttributeEffective(CHAR_ATTR_BEGIN + i),
                mPlayer->getAttributeBase(CHAR_ATTR_BEGIN + i)));

        mStatsLabel[i]->adjustSize();
        mStatsDisplayLabel[i]->adjustSize();

        mStatsPlus[i]->setEnabled(characterPoints);
        mStatsMinus[i]->setEnabled(correctionPoints);
    }
    mCharacterPointsLabel->setCaption("Character Points: " +
            toString(characterPoints));
    mCharacterPointsLabel->adjustSize();

    mCorrectionPointsLabel->setCaption("Correction Points: " +
            toString(correctionPoints));
    mCorrectionPointsLabel->adjustSize();
/*
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
*/
    // Update Second column widgets position
    mMoneyLabel->setPosition(mLvlLabel->getX() + mLvlLabel->getWidth() + 20,
                         mLvlLabel->getY());

}

void StatusWindow::draw(gcn::Graphics *g)
{
    update();

    Window::draw(g);
}

void StatusWindow::action(const gcn::ActionEvent &event)
{
    const std::string &eventId = event.getId();

    // Stats Part
    if (eventId == "STR+")
    {
        mPlayer->raiseAttribute(LocalPlayer::STR);
    }
    else if (eventId == "AGI+")
    {
        mPlayer->raiseAttribute(LocalPlayer::AGI);
    }
    else if (eventId == "DEX+")
    {
        mPlayer->raiseAttribute(LocalPlayer::DEX);
    }
    else if (eventId == "VIT+")
    {
        mPlayer->raiseAttribute(LocalPlayer::VIT);
    }
    else if (eventId == "INT+")
    {
        mPlayer->raiseAttribute(LocalPlayer::INT);
    }
    else if (eventId == "WIL+")
    {
        mPlayer->raiseAttribute(LocalPlayer::WIL);
    }

    else if (eventId == "STR-")
    {
        mPlayer->lowerAttribute(LocalPlayer::STR);
    }
    else if (eventId == "AGI-")
    {
        mPlayer->lowerAttribute(LocalPlayer::AGI);
    }
    else if (eventId == "DEX-")
    {
        mPlayer->lowerAttribute(LocalPlayer::DEX);
    }
    else if (eventId == "VIT-")
    {
        mPlayer->lowerAttribute(LocalPlayer::VIT);
    }
    else if (eventId == "INT-")
    {
        mPlayer->lowerAttribute(LocalPlayer::INT);
    }
    else if (eventId == "WIL-")
    {
        mPlayer->lowerAttribute(LocalPlayer::WIL);
    }
}

// WARNING: Duplicated method!

void StatusWindow::updateHPBar(ProgressBar *bar, bool showMax)
{
    if (showMax)
        bar->setText(toString(player_node->getHp()) +
                    "/" + toString(player_node->getMaxHp()));
    else
        bar->setText(toString(player_node->getHp()));

    // HP Bar coloration
    if (player_node->getHp() < player_node->getMaxHp() / 3)
    {
        bar->setColor(223, 32, 32); // Red
    }
    else if (player_node->getHp() < (player_node->getMaxHp() / 3) * 2)
    {
        bar->setColor(230, 171, 34); // Orange
    }
    else
    {
        bar->setColor(0, 171, 34); // Green
    }

    bar->setProgress((float) player_node->getHp() / (float) player_node->getMaxHp());
}
