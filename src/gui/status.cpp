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

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "progressbar.h"
#include "status.h"
#include "windowcontainer.h"

#include "widgets/layout.h"

#include "../localplayer.h"
#include "../units.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/stringutils.h"

StatusWindow::StatusWindow(LocalPlayer *player):
    Window(player->getName()),
    mPlayer(player),
    mCurrency(0)
{
    setWindowName("Status");
    setCloseButton(true);
    setDefaultSize((windowContainer->getWidth() - 365) / 2,
                   (windowContainer->getHeight() - 255) / 2, 400, 345);

    // ----------------------
    // Status Part
    // ----------------------

    mLvlLabel = new gcn::Label(strprintf(_("Level: %d"), 0));
    mJobLvlLabel = new gcn::Label(strprintf(_("Job: %d"), 0));
    mGpLabel = new gcn::Label(strprintf(_("Money: %s"),
                Units::formatCurrency(mCurrency).c_str()));

    mHpLabel = new gcn::Label(_("HP:"));
    mHpBar = new ProgressBar(1.0f, 80, 15, 0, 171, 34);

    mXpLabel = new gcn::Label(_("Exp:"));
    mXpBar = new ProgressBar(1.0f, 80, 15, 143, 192, 211);

    mMpLabel = new gcn::Label(_("MP:"));
    mMpBar = new ProgressBar(1.0f, 80, 15, 26, 102, 230);

    mJobLabel = new gcn::Label(_("Job:"));
    mJobBar = new ProgressBar(1.0f, 80, 15, 220, 135, 203);

    // ----------------------
    // Stats Part
    // ----------------------

    // Static Labels
    gcn::Label *mStatsTitleLabel = new gcn::Label(_("Stats"));
    gcn::Label *mStatsTotalLabel = new gcn::Label(_("Total"));
    gcn::Label *mStatsCostLabel = new gcn::Label(_("Cost"));
    mStatsTotalLabel->setAlignment(gcn::Graphics::CENTER);

    // Derived Stats
    mStatsAttackLabel = new gcn::Label(_("Attack:"));
    mStatsDefenseLabel= new gcn::Label(_("Defense:"));
    mStatsMagicAttackLabel = new gcn::Label(_("M.Attack:"));
    mStatsMagicDefenseLabel = new gcn::Label(_("M.Defense:"));
    // Gettext flag for next line: xgettext:no-c-format
    mStatsAccuracyLabel = new gcn::Label(_("% Accuracy:"));
    // Gettext flag for next line: xgettext:no-c-format
    mStatsEvadeLabel = new gcn::Label(_("% Evade:"));
    // Gettext flag for next line: xgettext:no-c-format
    mStatsReflexLabel = new gcn::Label(_("% Reflex:"));

    mStatsAttackPoints = new gcn::Label;
    mStatsDefensePoints = new gcn::Label;
    mStatsMagicAttackPoints = new gcn::Label;
    mStatsMagicDefensePoints = new gcn::Label;
    mStatsAccuracyPoints = new gcn::Label;
    mStatsEvadePoints = new gcn::Label;
    mStatsReflexPoints = new gcn::Label;

    // New labels
    for (int i = 0; i < 6; i++)
    {
        mStatsLabel[i] = new gcn::Label("0");
        mStatsLabel[i]->setAlignment(gcn::Graphics::CENTER);
        mStatsDisplayLabel[i] = new gcn::Label;
        mPointsLabel[i] = new gcn::Label("0");
        mPointsLabel[i]->setAlignment(gcn::Graphics::CENTER);
    }
    mRemainingStatsPointsLabel = new gcn::Label;

    // Set button events Id
    mStatsButton[0] = new Button("+", "STR", this);
    mStatsButton[1] = new Button("+", "AGI", this);
    mStatsButton[2] = new Button("+", "VIT", this);
    mStatsButton[3] = new Button("+", "INT", this);
    mStatsButton[4] = new Button("+", "DEX", this);
    mStatsButton[5] = new Button("+", "LUK", this);

    // Assemble
    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mLvlLabel, 3);
    place(5, 0, mJobLvlLabel, 3);
    place(8, 0, mGpLabel, 3);
    place(1, 1, mHpLabel).setPadding(3);
    place(2, 1, mHpBar, 3);
    place(6, 1, mXpLabel).setPadding(3);
    place(7, 1, mXpBar, 3);
    place(1, 2, mMpLabel).setPadding(3);
    place(2, 2, mMpBar, 3);
    place(6, 2, mJobLabel).setPadding(3);
    place(7, 2, mJobBar, 3);
    place.getCell().matchColWidth(0, 1);
    place = getPlacer(0, 3);
    place(0, 0, mStatsTitleLabel, 5);
    place(5, 1, mStatsTotalLabel, 5);
    place(12, 1, mStatsCostLabel, 5);
    for (int i = 0; i < 6; i++)
    {
        place(0, 2 + i, mStatsLabel[i], 7).setPadding(5);
        place(7, 2 + i, mStatsDisplayLabel[i]).setPadding(5);
        place(10, 2 + i, mStatsButton[i]);
        place(12, 2 + i, mPointsLabel[i]).setPadding(5);
    }
    place(14, 2, mStatsAttackLabel, 7).setPadding(5);
    place(14, 3, mStatsDefenseLabel, 7).setPadding(5);
    place(14, 4, mStatsMagicAttackLabel, 7).setPadding(5);
    place(14, 5, mStatsMagicDefenseLabel, 7).setPadding(5);
    place(14, 6, mStatsAccuracyLabel, 7).setPadding(5);
    place(14, 7, mStatsEvadeLabel, 7).setPadding(5);
    place(14, 8, mStatsReflexLabel, 7).setPadding(5);
    place(21, 2, mStatsAttackPoints, 3).setPadding(5);
    place(21, 3, mStatsDefensePoints, 3).setPadding(5);
    place(21, 4, mStatsMagicAttackPoints, 3).setPadding(5);
    place(21, 5, mStatsMagicDefensePoints, 3).setPadding(5);
    place(21, 6, mStatsAccuracyPoints, 3).setPadding(5);
    place(21, 7, mStatsEvadePoints, 3).setPadding(5);
    place(21, 8, mStatsReflexPoints, 3).setPadding(5);
    place(0, 8, mRemainingStatsPointsLabel, 3).setPadding(5);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    loadWindowState();
}

void StatusWindow::update()
{
    // Status Part
    // -----------
    mLvlLabel->setCaption(strprintf(_("Level: %d"), mPlayer->mLevel));
    mLvlLabel->adjustSize();

    mJobLvlLabel->setCaption(strprintf(_("Job: %d"), mPlayer->mJobLevel));
    mJobLvlLabel->adjustSize();

    if (mCurrency != mPlayer->mGp) {
        mCurrency = mPlayer->mGp;
        mGpLabel->setCaption(strprintf(_("Money: %s"),
                    Units::formatCurrency(mCurrency).c_str()));
        mGpLabel->adjustSize();
    }

    updateHPBar(mHpBar, true);

    updateMPBar(mMpBar, true);

    updateXPBar(mXpBar);

    updateJobBar(mJobBar);

    // Stats Part
    // ----------
    static const char *attrNames[6] = {
        N_("Strength"),
        N_("Agility"),
        N_("Vitality"),
        N_("Intelligence"),
        N_("Dexterity"),
        N_("Luck")
    };
    int statusPoints = mPlayer->mStatsPointsToAttribute;

    // Update labels
    for (int i = 0; i < 6; i++)
    {
        mStatsLabel[i]->setCaption(gettext(attrNames[i]));
        mStatsDisplayLabel[i]->setCaption(toString((int) mPlayer->mAttr[i]));
        mPointsLabel[i]->setCaption(toString((int) mPlayer->mAttrUp[i]));

        mStatsLabel[i]->adjustSize();
        mStatsDisplayLabel[i]->adjustSize();
        mPointsLabel[i]->adjustSize();

        mStatsButton[i]->setEnabled(mPlayer->mAttrUp[i] <= statusPoints);
    }
    mRemainingStatsPointsLabel->setCaption(
            strprintf(_("Remaining Status Points: %d"), statusPoints));
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
}

void StatusWindow::draw(gcn::Graphics *g)
{
    update();

    Window::draw(g);
}

void StatusWindow::action(const gcn::ActionEvent &event)
{
    // Stats Part
    if (event.getId().length() == 3)
    {
        if (event.getId() == "STR")
        {
            player_node->raiseAttribute(LocalPlayer::STR);
        }
        if (event.getId() == "AGI")
        {
            player_node->raiseAttribute(LocalPlayer::AGI);
        }
        if (event.getId() == "VIT")
        {
            player_node->raiseAttribute(LocalPlayer::VIT);
        }
        if (event.getId() == "INT")
        {
            player_node->raiseAttribute(LocalPlayer::INT);
        }
        if (event.getId() == "DEX")
        {
            player_node->raiseAttribute(LocalPlayer::DEX);
        }
        if (event.getId() == "LUK")
        {
            player_node->raiseAttribute(LocalPlayer::LUK);
        }
    }
}

void StatusWindow::updateHPBar(ProgressBar *bar, bool showMax)
{
    if (showMax)
        bar->setText(toString(player_node->mHp) +
                    "/" + toString(player_node->mMaxHp));
    else
        bar->setText(toString(player_node->mHp));

    // HP Bar coloration
    if (player_node->mHp < player_node->mMaxHp / 3)
    {
        bar->setColor(223, 32, 32); // Red
    }
    else if (player_node->mHp < (player_node->mMaxHp / 3) * 2)
    {
        bar->setColor(230, 171, 34); // Orange
    }
    else
    {
        bar->setColor(0, 171, 34); // Green
    }

    bar->setProgress((float) player_node->mHp / (float) player_node->mMaxHp);
}

void StatusWindow::updateMPBar(ProgressBar *bar, bool showMax)
{
    if (showMax)
        bar->setText(toString(player_node->mMp) +
                    "/" + toString(player_node->mMaxMp));
    else
        bar->setText(toString(player_node->mMp));


    bar->setProgress((float) player_node->mMp / (float) player_node->mMaxMp);
}

void StatusWindow::updateXPBar(ProgressBar *bar)
{
    if (player_node->mXpForNextLevel == 0) {
        bar->setText(_("Max level"));
        bar->setProgress(1.0);
    } else {
        bar->setText(toString(player_node->getXp()) +
                        "/" + toString(player_node->mXpForNextLevel));

        bar->setProgress((float) player_node->getXp() /
                         (float) player_node->mXpForNextLevel);
    }
}

void StatusWindow::updateJobBar(ProgressBar *bar)
{
    if (player_node->mXpForNextLevel == 0) {
        bar->setText(_("Max level"));
        bar->setProgress(1.0);
    } else {
        bar->setText(toString(player_node->mJobXp) +
                            "/" + toString(player_node->mJobXpForNextLevel));

        bar->setProgress((float) player_node->mJobXp /
                            (float) player_node->mJobXpForNextLevel);
    }
}
