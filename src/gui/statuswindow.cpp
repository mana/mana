/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/statuswindow.h"

#include "localplayer.h"
#include "units.h"

#include "gui/ministatus.h"
#include "gui/setup.h"
#include "gui/theme.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/vertcontainer.h"
#include "gui/widgets/windowcontainer.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "utils/gettext.h"
#include "utils/mathutils.h"
#include "utils/stringutils.h"

class AttrDisplay : public Container
{
    public:
        virtual std::string update();

    protected:
        AttrDisplay(int id, const std::string &name);

        const int mId;
        const std::string mName;

        LayoutHelper *mLayout;
        Label *mLabel;
        Label *mValue;
};

class DerDisplay : public AttrDisplay
{
    public:
        DerDisplay(int id, const std::string &name);
};

class ChangeDisplay : public AttrDisplay, gcn::ActionListener
{
    public:
        ChangeDisplay(int id, const std::string &name);
        std::string update();
        void setPointsNeeded(int needed);

    private:
        void action(const gcn::ActionEvent &event);

        int mNeeded;

        Label *mPoints;
        Button *mDec;
        Button *mInc;
};

StatusWindow::StatusWindow():
    Window(player_node->getName())
{
    setWindowName("Status");
    setupWindow->registerWindowForReset(this);
    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize((windowContainer->getWidth() - 365) / 2,
                   (windowContainer->getHeight() - 255) / 2, 365, 275);

    // ----------------------
    // Status Part
    // ----------------------

    mLvlLabel = new Label(strprintf(_("Level: %d"), 0));
    mMoneyLabel = new Label(strprintf(_("Money: %s"), ""));

    int max = player_node->getMaxHp();
    mHpLabel = new Label(_("HP:"));
    mHpBar = new ProgressBar(max ? (float) player_node->getHp() / max: 0,
                             80, 15, Theme::PROG_HP);

    max = player_node->getExpNeeded();
    mXpLabel = new Label(_("Exp:"));
    mXpBar = new ProgressBar(max ? (float) player_node->getExp() / max : 0,
                             80, 15, Theme::PROG_EXP);

    max = player_node->getMaxMP();
    mMpLabel = new Label(_("MP:"));
    mMpBar = new ProgressBar(max ? (float) player_node->getMaxMP() / max : 0,
                             80, 15, Net::getPlayerHandler()->canUseMagic() ?
                             Theme::PROG_MP : Theme::PROG_NO_MP);

    place(0, 0, mLvlLabel, 3);
    // 5, 0 Job Level
    place(8, 0, mMoneyLabel, 3);
    place(0, 1, mHpLabel).setPadding(3);
    place(1, 1, mHpBar, 4);
    place(5, 1, mXpLabel).setPadding(3);
    place(6, 1, mXpBar, 5);
    place(0, 2, mMpLabel).setPadding(3);
    // 5, 2 and 6, 2 Job Progress Bar
    place(1, 2, mMpBar, 4);

    if (Net::getPlayerHandler()->getJobLocation() > 0)
    {
        mJobLvlLabel = new Label(strprintf(_("Job: %d"), 0));
        mJobLabel = new Label(_("Job:"));
        mJobBar = new ProgressBar(0.0f, 80, 15, Theme::PROG_JOB);

        place(5, 0, mJobLvlLabel, 3);
        place(5, 2, mJobLabel).setPadding(3);
        place(6, 2, mJobBar, 5);
    }

    // ----------------------
    // Stats Part
    // ----------------------

    mAttrCont = new VertContainer(32);
    mAttrScroll = new ScrollArea(mAttrCont);
    mAttrScroll->setOpaque(false);
    mAttrScroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mAttrScroll->setVerticalScrollPolicy(ScrollArea::SHOW_AUTO);
    place(0, 3, mAttrScroll, 5, 3);

    mDAttrCont = new VertContainer(32);
    mDAttrScroll = new ScrollArea(mDAttrCont);
    mDAttrScroll->setOpaque(false);
    mDAttrScroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mDAttrScroll->setVerticalScrollPolicy(ScrollArea::SHOW_AUTO);
    place(6, 3, mDAttrScroll, 5, 3);

    getLayout().setRowHeight(3, Layout::AUTO_SET);

    mCharacterPointsLabel = new Label("C");
    place(0, 6, mCharacterPointsLabel, 5);

    if (Net::getPlayerHandler()->canCorrectAttributes())
    {
        mCorrectionPointsLabel = new Label("C");
        place(0, 7, mCorrectionPointsLabel, 5);
    }

    loadWindowState();

    update(HP);
    update(MP);
    update(EXP);
    update(MONEY);
    update(CHAR_POINTS); // This also updates all attributes (none atm)
    update(LEVEL);
    int job = Net::getPlayerHandler()->getJobLocation();
    if (job > 0)
    {
        update(job);
    }
}

std::string StatusWindow::update(int id)
{
    if (miniStatusWindow)
        miniStatusWindow->update(id);

    if (id == HP)
    {
        updateHPBar(mHpBar, true);

        return _("HP");
    }
    else if (id == MP)
    {
        updateMPBar(mMpBar, true);

        return _("MP");
    }
    else if (id == EXP)
    {
        updateXPBar(mXpBar, false);

        return _("Exp");
    }
    else if (id == MONEY)
    {
        int money = player_node->getMoney();
        mMoneyLabel->setCaption(strprintf(_("Money: %s"),
                                        Units::formatCurrency(money).c_str()));
        mMoneyLabel->adjustSize();

        return _("Money");
    }
    else if (id == Net::getPlayerHandler()->getJobLocation())
    {
        mJobLvlLabel->setCaption(strprintf(_("Job: %d"),
                                        player_node->getAttributeBase(id)));
        mJobLvlLabel->adjustSize();

        updateProgressBar(mJobBar, id, false);

        return _("Job");
    }
    else if (id == CHAR_POINTS)
    {
        mCharacterPointsLabel->setCaption(strprintf(_("Character points: %d"),
                                        player_node->getCharacterPoints()));
        mCharacterPointsLabel->adjustSize();

        if (Net::getPlayerHandler()->canCorrectAttributes())
        {
            mCorrectionPointsLabel->setCaption(strprintf(_("Correction points: %d"),
                                            player_node->getCorrectionPoints()));
            mCorrectionPointsLabel->adjustSize();
        }

        for (Attrs::iterator it = mAttrs.begin(); it != mAttrs.end(); it++)
        {
            it->second->update();
        }
    }
    else if (id == LEVEL)
    {
        mLvlLabel->setCaption(strprintf(_("Level: %d"),
                                        player_node->getLevel()));
        mLvlLabel->adjustSize();

        return _("Level");
    }
    else
    {
        Attrs::iterator it = mAttrs.find(id);

        if (it != mAttrs.end())
        {
            return it->second->update();
        }
    }

    return "";
}

void StatusWindow::setPointsNeeded(int id, int needed)
{
    Attrs::iterator it = mAttrs.find(id);

    if (it != mAttrs.end())
    {
        ChangeDisplay *disp = dynamic_cast<ChangeDisplay*>(it->second);
        if (disp)
            disp->setPointsNeeded(needed);
    }
}

void StatusWindow::addAttribute(int id, const std::string &name,
                                bool modifiable)
{
    AttrDisplay *disp;

    if (modifiable)
    {
        disp = new ChangeDisplay(id, name);
        mAttrCont->add(disp);
    }
    else
    {
        disp = new DerDisplay(id, name);
        mDAttrCont->add(disp);
    }

    mAttrs[id] = disp;
}

void StatusWindow::updateHPBar(ProgressBar *bar, bool showMax)
{

    if (showMax)
        bar->setText(toString(player_node->getHp()) +
                    "/" + toString(player_node->getMaxHp()));
    else
        bar->setText(toString(player_node->getHp()));

    float prog = 1.0;

    if (player_node->getMaxHp() > 0)
        prog = (float) player_node->getHp() / player_node->getMaxHp();
    bar->setProgress(prog);
}

void StatusWindow::updateMPBar(ProgressBar *bar, bool showMax)
{
    if (showMax)
        bar->setText(toString(player_node->getMP()) +
                    "/" + toString(player_node->getMaxMP()));
    else
        bar->setText(toString(player_node->getMP()));

    float prog = 1.0f;

    if (player_node->getMaxMP() > 0)
        prog = (float) player_node->getMP() / player_node->getMaxMP();

    if (Net::getPlayerHandler()->canUseMagic())
        bar->setProgressPalette(Theme::PROG_MP);
    else
        bar->setProgressPalette(Theme::PROG_NO_MP);

    bar->setProgress(prog);
}

void StatusWindow::updateProgressBar(ProgressBar *bar, int value, int max,
                              bool percent)
{
    if (max == 0)
    {
        bar->setText(_("Max"));
        bar->setProgress(1.0);
    }
    else
    {
        float progress = ((float) value) / max;

        if (percent)
            bar->setText(strprintf("%2.2f", 100 * progress) + "%");
        else
            bar->setText(toString(value) + "/" + toString(max));

        bar->setProgress(progress);
    }
}

void StatusWindow::updateXPBar(ProgressBar *bar, bool percent)
{
    updateProgressBar(bar, player_node->getExp(),
                      player_node->getExpNeeded(), percent);
}

void StatusWindow::updateProgressBar(ProgressBar *bar, int id, bool percent)
{
    std::pair<int, int> exp =  player_node->getExperience(id);
    updateProgressBar(bar, exp.first, exp.second, percent);
}

AttrDisplay::AttrDisplay(int id, const std::string &name):
        mId(id),
        mName(name)
{
    setSize(100, 32);
    mLabel = new Label(name);
    mValue = new Label("1");

    mLabel->setAlignment(Graphics::CENTER);
    mValue->setAlignment(Graphics::CENTER);

    mLayout = new LayoutHelper(this);
}

std::string AttrDisplay::update()
{
    int base = player_node->getAttributeBase(mId);
    int bonus = player_node->getAttributeEffective(mId) - base;
    std::string value = toString(base);
    if (bonus)
        value += strprintf(" (%+d)", bonus);
    mValue->setCaption(value);

    return mName;
}

DerDisplay::DerDisplay(int id, const std::string &name):
        AttrDisplay(id, name)
{
    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = mLayout->getPlacer(0, 0);

    place(0, 0, mLabel, 3);
    place(3, 0, mValue, 2);

    update();
}

ChangeDisplay::ChangeDisplay(int id, const std::string &name):
        AttrDisplay(id, name), mNeeded(1)
{
    mPoints = new Label(_("Max"));
    mInc = new Button(_("+"), "inc", this);

    // Do the layout
    ContainerPlacer place = mLayout->getPlacer(0, 0);

    place(0, 0, mLabel, 3);
    place(4, 0, mValue, 2);
    place(6, 0, mInc);
    place(7, 0, mPoints);

    if (Net::getPlayerHandler()->canCorrectAttributes())
    {
        mDec = new Button(_("-"), "dec", this);
        mDec->setWidth(mInc->getWidth());

        place(3, 0, mDec);
    }
    else
    {
        mDec = 0;
    }

    update();
}

std::string ChangeDisplay::update()
{
    if (mNeeded > 0)
    {
        mPoints->setCaption(toString(mNeeded));
    }
    else
    {
        mPoints->setCaption(_("Max"));
    }

    if (mDec)
    {
        mDec->setEnabled(player_node->getCorrectionPoints());
    }
    mInc->setEnabled(player_node->getCharacterPoints() >= mNeeded &&
                     mNeeded > 0);

    return AttrDisplay::update();
}

void ChangeDisplay::setPointsNeeded(int needed)
{
    mNeeded = needed;

    update();
}

void ChangeDisplay::action(const gcn::ActionEvent &event)
{
    if (Net::getPlayerHandler()->canCorrectAttributes() &&
        event.getSource() == mDec)
    {
        int newcorpoints = player_node->getCorrectionPoints() - 1;
        player_node->setCorrectionPoints(newcorpoints);
        int newpoints = player_node->getCharacterPoints() + 1;
        player_node->setCharacterPoints(newpoints);
        int newbase = player_node->getAttributeBase(mId) - 1;
        player_node->setAttributeBase(mId, newbase);
        int newmod = player_node->getAttributeEffective(mId) - 1;
        player_node->setAttributeEffective(mId, newmod);
        Net::getPlayerHandler()->decreaseAttribute(mId);
    }
    else if (event.getSource() == mInc)
    {
        int newpoints = player_node->getCharacterPoints() - 1;
        player_node->setCharacterPoints(newpoints);
        int newbase = player_node->getAttributeBase(mId) + 1;
        player_node->setAttributeBase(mId, newbase);
        int newmod = player_node->getAttributeEffective(mId) + 1;
        player_node->setAttributeEffective(mId, newmod);
        Net::getPlayerHandler()->increaseAttribute(mId);
    }
}
