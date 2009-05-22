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

#include "gui/status.h"
#include "gui/palette.h"

#include "localplayer.h"
#include "units.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/windowcontainer.h"

#include "net/net.h"
#include "net/ea/playerhandler.h"

#include "utils/gettext.h"
#include "utils/mathutils.h"
#include "utils/stringutils.h"

StatusWindow::StatusWindow(LocalPlayer *player):
    Window(player->getName()),
    mPlayer(player),
    mCurrency(0)
{
    setWindowName("Status");
    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize(400, 345, ImageRect::CENTER);

    // ----------------------
    // Status Part
    // ----------------------

    mLvlLabel = new Label(strprintf(_("Level: %d"), 0));
    mJobLvlLabel = new Label(strprintf(_("Job: %d"), 0));
    mGpLabel = new Label(strprintf(_("Money: %s"),
                Units::formatCurrency(mCurrency).c_str()));

    mHpLabel = new Label(_("HP:"));
    mHpBar = new ProgressBar(0.0f, 80, 15, gcn::Color(0, 171, 34));

    mXpLabel = new Label(_("Exp:"));
    mXpBar = new ProgressBar(0.0f, 80, 15, gcn::Color(143, 192, 211));

    mMpLabel = new Label(_("MP:"));
    mMpBar = new ProgressBar(0.0f, 80, 15, gcn::Color(26, 102, 230));

    mJobLabel = new Label(_("Job:"));
    mJobBar = new ProgressBar(0.0f, 80, 15, gcn::Color(220, 135, 203));

    // ----------------------
    // Stats Part
    // ----------------------

    // Static Labels
    gcn::Label *mStatsTitleLabel = new Label(_("Stats"));
    gcn::Label *mStatsTotalLabel = new Label(_("Total"));
    gcn::Label *mStatsCostLabel = new Label(_("Cost"));
    mStatsTotalLabel->setAlignment(gcn::Graphics::CENTER);

    // Derived Stats
    mStatsAttackLabel = new Label(_("Attack:"));
    mStatsDefenseLabel= new Label(_("Defense:"));
    mStatsMagicAttackLabel = new Label(_("M.Attack:"));
    mStatsMagicDefenseLabel = new Label(_("M.Defense:"));
    // Gettext flag for next line: xgettext:no-c-format
    mStatsAccuracyLabel = new Label(_("% Accuracy:"));
    // Gettext flag for next line: xgettext:no-c-format
    mStatsEvadeLabel = new Label(_("% Evade:"));
    // Gettext flag for next line: xgettext:no-c-format
    mStatsReflexLabel = new Label(_("% Reflex:"));

    mStatsAttackPoints = new Label;
    mStatsDefensePoints = new Label;
    mStatsMagicAttackPoints = new Label;
    mStatsMagicDefensePoints = new Label;
    mStatsAccuracyPoints = new Label;
    mStatsEvadePoints = new Label;
    mStatsReflexPoints = new Label;

    // New labels
    for (int i = 0; i < 6; i++)
    {
        mStatsLabel[i] = new Label("0");
        mStatsLabel[i]->setAlignment(gcn::Graphics::CENTER);
        mStatsDisplayLabel[i] = new Label;
        mPointsLabel[i] = new Label("0");
        mPointsLabel[i]->setAlignment(gcn::Graphics::CENTER);
    }
    mRemainingStatsPointsLabel = new Label;

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
    place(0, 1, mHpLabel).setPadding(3);
    place(1, 1, mHpBar, 4);
    place(5, 1, mXpLabel).setPadding(3);
    place(6, 1, mXpBar, 5);
    place(0, 2, mMpLabel).setPadding(3);
    place(1, 2, mMpBar, 4);
    place(5, 2, mJobLabel).setPadding(3);
    place(6, 2, mJobBar, 5);
    place.getCell().matchColWidth(0, 1);
    place = getPlacer(0, 3);
    place(0, 1, mStatsTitleLabel, 5);
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
    mLvlLabel->setCaption(strprintf(_("Level: %d"), mPlayer->getLevel()));
    mLvlLabel->adjustSize();

    mJobLvlLabel->setCaption(strprintf(_("Job: %d"), mPlayer->mJobLevel));
    mJobLvlLabel->adjustSize();

    if (mCurrency != mPlayer->getMoney()) {
        mCurrency = mPlayer->getMoney();
        mGpLabel->setCaption(strprintf(_("Money: %s"),
                    Units::formatCurrency(mCurrency).c_str()));
        mGpLabel->adjustSize();
    }

    updateHPBar(mHpBar, true);

    updateMPBar(mMpBar, true);

    updateXPBar(mXpBar, false);

    updateJobBar(mJobBar, false);

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
    // Net::getPlayerHandler()->increaseStat(?);
    if (event.getId().length() == 3)
    {
        if (event.getId() == "STR")
            Net::getPlayerHandler()->increaseStat(LocalPlayer::STR);
        if (event.getId() == "AGI")
            Net::getPlayerHandler()->increaseStat(LocalPlayer::AGI);
        if (event.getId() == "VIT")
            Net::getPlayerHandler()->increaseStat(LocalPlayer::VIT);
        if (event.getId() == "INT")
            Net::getPlayerHandler()->increaseStat(LocalPlayer::INT);
        if (event.getId() == "DEX")
            Net::getPlayerHandler()->increaseStat(LocalPlayer::DEX);
        if (event.getId() == "LUK")
            Net::getPlayerHandler()->increaseStat(LocalPlayer::LUK);
    }
}

void StatusWindow::updateHPBar(ProgressBar *bar, bool showMax)
{
    if (showMax)
        bar->setText(toString(player_node->getHp()) +
                    "/" + toString(player_node->getMaxHp()));
    else
        bar->setText(toString(player_node->getHp()));

    // HP Bar coloration
    float r1 = 255;
    float g1 = 255;
    float b1 = 255;

    float r2 = 255;
    float g2 = 255;
    float b2 = 255;

    float weight = 1.0f;

    int curHP = player_node->getHp();
    int thresholdLevel = player_node->getMaxHp() / 4;
    int thresholdProgress = curHP % thresholdLevel;
    weight = 1-((float)thresholdProgress) / ((float)thresholdLevel);

    if (curHP < (thresholdLevel))
    {
        gcn::Color color1 = guiPalette->getColor(Palette::HPBAR_ONE_HALF);
        gcn::Color color2 = guiPalette->getColor(Palette::HPBAR_ONE_QUARTER);
        r1 = color1.r; r2 = color2.r;
        g1 = color1.g; g2 = color2.g;
        b1 = color1.b; b2 = color2.b;
    }
    else if (curHP < (thresholdLevel*2))
    {
        gcn::Color color1 = guiPalette->getColor(Palette::HPBAR_THREE_QUARTERS);
        gcn::Color color2 = guiPalette->getColor(Palette::HPBAR_ONE_HALF);
        r1 = color1.r; r2 = color2.r;
        g1 = color1.g; g2 = color2.g;
        b1 = color1.b; b2 = color2.b;
    }
    else if (curHP < thresholdLevel*3)
    {
        gcn::Color color1 = guiPalette->getColor(Palette::HPBAR_FULL);
        gcn::Color color2 = guiPalette->getColor(Palette::HPBAR_THREE_QUARTERS);
        r1 = color1.r; r2 = color2.r;
        g1 = color1.g; g2 = color2.g;
        b1 = color1.b; b2 = color2.b;
    }
    else
    {
        gcn::Color color1 = guiPalette->getColor(Palette::HPBAR_FULL);
        gcn::Color color2 = guiPalette->getColor(Palette::HPBAR_FULL);
        r1 = color1.r; r2 = color2.r;
        g1 = color1.g; g2 = color2.g;
        b1 = color1.b; b2 = color2.b;
    }

    // Safety checks
    if (weight > 1.0f) weight = 1.0f;
    if (weight < 0.0f) weight = 0.0f;

    // Do the color blend
    r1 = (int) weightedAverage(r1, r2,weight);
    g1 = (int) weightedAverage(g1, g2, weight);
    b1 = (int) weightedAverage(b1, b2, weight);

    // More safety checks
    if (r1 > 255) r1 = 255;
    if (g1 > 255) g1 = 255;
    if (b1 > 255) b1 = 255;

    bar->setColor(r1, g1, b1);

    bar->setProgress((float) player_node->getHp() / (float) player_node->getMaxHp());
}

void StatusWindow::updateMPBar(ProgressBar *bar, bool showMax)
{
    if (showMax)
        bar->setText(toString(player_node->mMp) +
                    "/" + toString(player_node->mMaxMp));
    else
        bar->setText(toString(player_node->mMp));

    if (player_node->MATK <= 0)
        bar->setColor(100, 100, 100); // grey, to indicate that we lack magic
    else
        bar->setColor(26, 102, 230); // blue, to indicate that we have magic

    bar->setProgress((float) player_node->mMp / (float) player_node->mMaxMp);
}

static void updateProgressBar(ProgressBar *bar, int value, int max,
                              bool percent)
{
    if (max == 0)
    {
        bar->setText(_("Max level"));
        bar->setProgress(1.0);
    }
    else
    {
        float progress = (float) value / max;

        if (percent)
            bar->setText(strprintf("%2.2f", 100 * progress) + "%");
        else
            bar->setText(toString(value) + "/" + toString(max));

        bar->setProgress(progress);
    }
}

void StatusWindow::updateXPBar(ProgressBar *bar, bool percent)
{
    updateProgressBar(bar,
                      player_node->getXp(),
                      player_node->mXpForNextLevel,
                      percent);
}

void StatusWindow::updateJobBar(ProgressBar *bar, bool percent)
{
    updateProgressBar(bar,
                      player_node->mJobXp,
                      player_node->mJobXpForNextLevel,
                      percent);
}
