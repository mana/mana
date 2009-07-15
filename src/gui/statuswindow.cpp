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

StatusWindow::StatusWindow():
    Window(player_node->getName())
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

    // Assemble
    add(mStatsTitleLabel);
    add(mStatsTotalLabel);
    for(int i = 0; i < 6; i++)
    {
        add(mStatsLabel[i]);
        add(mStatsDisplayLabel[i]);
        add(mStatsPlus[i]);
        add(mStatsMinus[i]);
    }

    add(mCharacterPointsLabel);
    add(mCorrectionPointsLabel);
}

void StatusWindow::update()
{
    // Status Part
    // -----------
    mLvlLabel->setCaption(  "Level: " +
                            toString(player_node->getLevel()) +
                            " (" +
                            toString(player_node->getLevelProgress()) +
                            "%)");
    mLvlLabel->adjustSize();

    mMoneyLabel->setCaption("Money: " + toString(player_node->getMoney()) + " GP");
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
    int characterPoints = player_node->getCharacterPoints();
    int correctionPoints = player_node->getCorrectionPoints();
    // Update labels
    for (int i = 0; i < 6; i++)
    {
        mStatsLabel[i]->setCaption(attrNames[i]);
        mStatsDisplayLabel[i]->setCaption(
            strprintf("%d / %d",
                player_node->getAttributeEffective(CHAR_ATTR_BEGIN + i),
                player_node->getAttributeBase(CHAR_ATTR_BEGIN + i)));

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

    // Update Second column widgets position
    mMoneyLabel->setPosition(mLvlLabel->getX() + mLvlLabel->getWidth() + 20,
                         mLvlLabel->getY());

    update();
}

std::string StatusWindow::update(int id)
{
    // TODO: only update what changed
    update();
}

void StatusWindow::addAttribute(int id, const std::string &name,
                                bool modifiable)
{
    // future use
}

void StatusWindow::action(const gcn::ActionEvent &event)
{
    const std::string &eventId = event.getId();

    // Stats Part
    if (eventId == "STR+")
    {
        player_node->raiseAttribute(LocalPlayer::STR);
    }
    else if (eventId == "AGI+")
    {
        player_node->raiseAttribute(LocalPlayer::AGI);
    }
    else if (eventId == "DEX+")
    {
        player_node->raiseAttribute(LocalPlayer::DEX);
    }
    else if (eventId == "VIT+")
    {
        player_node->raiseAttribute(LocalPlayer::VIT);
    }
    else if (eventId == "INT+")
    {
        player_node->raiseAttribute(LocalPlayer::INT);
    }
    else if (eventId == "WIL+")
    {
        player_node->raiseAttribute(LocalPlayer::WIL);
    }

    else if (eventId == "STR-")
    {
        player_node->lowerAttribute(LocalPlayer::STR);
    }
    else if (eventId == "AGI-")
    {
        player_node->lowerAttribute(LocalPlayer::AGI);
    }
    else if (eventId == "DEX-")
    {
        player_node->lowerAttribute(LocalPlayer::DEX);
    }
    else if (eventId == "VIT-")
    {
        player_node->lowerAttribute(LocalPlayer::VIT);
    }
    else if (eventId == "INT-")
    {
        player_node->lowerAttribute(LocalPlayer::INT);
    }
    else if (eventId == "WIL-")
    {
        player_node->lowerAttribute(LocalPlayer::WIL);
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
