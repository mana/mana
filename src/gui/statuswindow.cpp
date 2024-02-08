/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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
#include "playerinfo.h"
#include "units.h"

#include "gui/setup.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/vertcontainer.h"
#include "gui/widgets/windowcontainer.h"

#include "net/net.h"
#include "net/playerhandler.h"
#include "net/gamehandler.h"

#include "net/tmwa/protocol.h"

#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/mathutils.h"
#include "utils/stringutils.h"

class AttrDisplay : public Container
{
    public:
        enum Type {
            DERIVED, CHANGEABLE, UNKNOWN
        };

        ~AttrDisplay() override;

        virtual std::string update();
        virtual Type getType() { return UNKNOWN; }

    protected:
        AttrDisplay(int id, const std::string &name);

        const int mId;
        const std::string mName;

        Label *mLabel;
        Label *mValue;
};

class DerDisplay : public AttrDisplay
{
    public:
        DerDisplay(int id, const std::string &name);
        Type getType() override { return DERIVED; }
};

class ChangeDisplay : public AttrDisplay, gcn::ActionListener
{
    public:
        ChangeDisplay(int id, const std::string &name);
        std::string update() override;
        Type getType() override { return CHANGEABLE; }
        void setPointsNeeded(int needed);

    private:
        void action(const gcn::ActionEvent &event) override;

        int mNeeded = 1;

        Label *mPoints;
        Button *mDec;
        Button *mInc;
};

StatusWindow::StatusWindow():
    Window(local_player->getName())
{
    listen(Event::AttributesChannel);

    setWindowName("Status");
    setupWindow->registerWindowForReset(this);
    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize((windowContainer->getWidth() - 365) / 2,
                   (windowContainer->getHeight() - 255) / 2, 480, 275);
    setMinWidth(480);
    setMinHeight(131);

    // ----------------------
    // Status Part
    // ----------------------

    mLvlLabel = new Label(strprintf(_("Level: %d"), 0));
    mMoneyLabel = new Label(strprintf(_("Money: %s"), ""));

    int max = PlayerInfo::getAttribute(MAX_HP);
    mHpLabel = new Label(_("HP:"));
    mHpBar = new ProgressBar(max ? (float) PlayerInfo::getAttribute(HP) / max :
                             0, 80, 15, Theme::PROG_HP);

    max = PlayerInfo::getAttribute(EXP_NEEDED);
    mXpLabel = new Label(_("Exp:"));
    mXpBar = new ProgressBar(max ? (float) PlayerInfo::getAttribute(EXP) / max :
                             0, 80, 15, Theme::PROG_EXP);

    bool magicBar = Net::getGameHandler()->canUseMagicBar();
    if (magicBar)
    {
        max = PlayerInfo::getAttribute(MAX_MP);
        mMpLabel = new Label(_("MP:"));
        mMpBar = new ProgressBar(max ?
                             (float) PlayerInfo::getAttribute(MAX_MP) / max :
                             0, 80, 15, Net::getPlayerHandler()->canUseMagic() ?
                             Theme::PROG_MP : Theme::PROG_NO_MP);
    }

    place(0, 0, mLvlLabel, 3);
    // 5, 0 Job Level
    place(8, 0, mMoneyLabel, 3);
    place(0, 1, mHpLabel).setPadding(3);
    place(1, 1, mHpBar, 4);
    place(5, 1, mXpLabel).setPadding(3);
    place(6, 1, mXpBar, 5);

    int attributesFirstRow = 2;
    if (magicBar)
    {
        place(0, 2, mMpLabel).setPadding(3);
        // 5, 2 and 6, 2 Job Progress Bar
        place(1, 2, mMpBar, 4);

        // We move the attribute row to the next one
        attributesFirstRow = 3;
    }

    if (Net::getPlayerHandler()->getJobLocation() > 0)
    {
        mJobLvlLabel = new Label(strprintf(_("Job: %d"), 0));
        mJobLabel = new Label(_("Job:"));
        mJobBar = new ProgressBar(0.0f, 80, 15, Theme::PROG_JOB);

        place(5, 0, mJobLvlLabel, 3);
        place(5, 2, mJobLabel).setPadding(3);
        place(6, 2, mJobBar, 5);

        // We move the attribute row to the next one
        attributesFirstRow = 3;
    }

    // ----------------------
    // Stats Part
    // ----------------------

    mAttrCont = new VertContainer(28);
    mAttrScroll = new ScrollArea(mAttrCont);
    mAttrScroll->setOpaque(false);
    mAttrScroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mAttrScroll->setVerticalScrollPolicy(ScrollArea::SHOW_AUTO);
    place(0, attributesFirstRow, mAttrScroll, 5, 3);

    mDAttrCont = new VertContainer(28);
    mDAttrScroll = new ScrollArea(mDAttrCont);
    mDAttrScroll->setOpaque(false);
    mDAttrScroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mDAttrScroll->setVerticalScrollPolicy(ScrollArea::SHOW_AUTO);
    place(6, attributesFirstRow, mDAttrScroll, 5, 3);

    getLayout().setRowHeight(attributesFirstRow, Layout::AUTO_SET);

    mCharacterPointsLabel = new Label("Character points: 0");
    place(0, attributesFirstRow + 3, mCharacterPointsLabel, 4);

    if (Net::getPlayerHandler()->canCorrectAttributes())
    {
        mCorrectionPointsLabel = new Label("Correction points: 0");
        place(4, attributesFirstRow + 3, mCorrectionPointsLabel, 4);
    }

    loadWindowState();

    // Update bars
    updateHPBar(mHpBar, true);
    if (magicBar)
        updateMPBar(mMpBar, true);
    updateXPBar(mXpBar, false);


    mMoneyLabel->setCaption(strprintf(_("Money: %s"),
               Units::formatCurrency(PlayerInfo::getAttribute(MONEY)).c_str()));
    mMoneyLabel->adjustSize();
    mCharacterPointsLabel->setCaption(strprintf(_("Character points: %d"),
                                      PlayerInfo::getAttribute(CHAR_POINTS)));
    mCharacterPointsLabel->adjustSize();

    mLvlLabel->setCaption(strprintf(_("Level: %d"),
                          PlayerInfo::getAttribute(LEVEL)));
    mLvlLabel->adjustSize();
}

void StatusWindow::event(Event::Channel channel,
                         const Event &event)
{
    if (event.getType() == Event::UpdateAttribute)
    {
        switch(event.getInt("id"))
        {
            case HP: case MAX_HP:
                updateHPBar(mHpBar, true);
            break;

            case MP: case MAX_MP:
                updateMPBar(mMpBar, true);
            break;

            case EXP: case EXP_NEEDED:
                updateXPBar(mXpBar, false);
            break;

            case MONEY:
                mMoneyLabel->setCaption(strprintf(_("Money: %s"),
                                Units::formatCurrency(
                                event.getInt("newValue")).c_str()));
                mMoneyLabel->adjustSize();
            break;

            case CHAR_POINTS:
                mCharacterPointsLabel->setCaption(strprintf(
                                              _("Character points: %d"),
                                              event.getInt("newValue")));
                mCharacterPointsLabel->adjustSize();
                updateAttrs();
            break;

            case CORR_POINTS:
                mCorrectionPointsLabel->setCaption(strprintf(
                                               _("Correction points: %d"),
                                               event.getInt("newValue")));
                mCorrectionPointsLabel->adjustSize();
                updateAttrs();
            break;

            case LEVEL:
                mLvlLabel->setCaption(strprintf(_("Level: %d"),
                                  event.getInt("newValue")));
                mLvlLabel->adjustSize();
            break;
        }
    }
    else if (event.getType() == Event::UpdateStat)
    {
        int id = event.getInt("id");

        if (id == Net::getPlayerHandler()->getJobLocation())
        {

            mJobLvlLabel->setCaption(strprintf(_("Job: %d"),
                                            PlayerInfo::getStatBase(id)));
            mJobLvlLabel->adjustSize();

            updateProgressBar(mJobBar, id, false);
        }
        else
        {
            auto it = mAttrs.find(id);
            if (it != mAttrs.end())
            {
                it->second->update();
            }

            if (Net::getNetworkType() == ServerInfo::TMWATHENA &&
                    id == TmwAthena::MATK)
            {
                updateMPBar(mMpBar, true);
            }
        }
    }
}

void StatusWindow::updateAttrs()
{
    for (auto &attr : mAttrs)
    {
        attr.second->update();
    }
}

void StatusWindow::setPointsNeeded(int id, int needed)
{
    auto it = mAttrs.find(id);

    if (it != mAttrs.end())
    {
        AttrDisplay *disp = it->second;
        if (disp->getType() == AttrDisplay::CHANGEABLE)
            static_cast<ChangeDisplay*>(disp)->setPointsNeeded(needed);
    }
}

void StatusWindow::addAttribute(int id, const std::string &name,
                                bool modifiable,
                                const std::string &description)
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
    if (!bar)
        return;

    if (showMax)
        bar->setText(toString(PlayerInfo::getAttribute(HP)) +
                    "/" + toString(PlayerInfo::getAttribute(MAX_HP)));
    else
        bar->setText(toString(PlayerInfo::getAttribute(HP)));

    float prog = 1.0;

    if (PlayerInfo::getAttribute(MAX_HP) > 0)
        prog = (float) PlayerInfo::getAttribute(HP)
               / PlayerInfo::getAttribute(MAX_HP);
    bar->setProgress(prog);
}

void StatusWindow::updateMPBar(ProgressBar *bar, bool showMax)
{
    if (!bar)
        return;

    if (showMax)
        bar->setText(toString(PlayerInfo::getAttribute(MP)) +
                    "/" + toString(PlayerInfo::getAttribute(MAX_MP)));
    else
        bar->setText(toString(PlayerInfo::getAttribute(MP)));

    float prog = 1.0f;

    if (PlayerInfo::getAttribute(MAX_MP) > 0)
        prog = (float) PlayerInfo::getAttribute(MP)
               / PlayerInfo::getAttribute(MAX_MP);

    if (Net::getPlayerHandler()->canUseMagic())
        bar->setProgressPalette(Theme::PROG_MP);
    else
        bar->setProgressPalette(Theme::PROG_NO_MP);

    bar->setProgress(prog);
}

void StatusWindow::updateProgressBar(ProgressBar *bar, int value, int max,
                                     bool percent)
{
    if (!bar)
        return;

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
    if (!bar)
        return;

    updateProgressBar(bar, PlayerInfo::getAttribute(EXP),
                      PlayerInfo::getAttribute(EXP_NEEDED), percent);
}

void StatusWindow::updateProgressBar(ProgressBar *bar, int id, bool percent)
{
    std::pair<int, int> exp =  PlayerInfo::getStatExperience(id);
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
}

AttrDisplay::~AttrDisplay()
{
}

std::string AttrDisplay::update()
{
    int base = PlayerInfo::getStatBase(mId);
    int bonus = PlayerInfo::getStatMod(mId);
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
    place(0, 0, mLabel, 3);
    place(3, 0, mValue, 2);

    update();
}

ChangeDisplay::ChangeDisplay(int id, const std::string &name):
    AttrDisplay(id, name)
{
    mPoints = new Label(_("Max"));
    mInc = new Button(_("+"), "inc", this);

    // Do the layout
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
        mDec = nullptr;
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
        mDec->setEnabled(PlayerInfo::getAttribute(CORR_POINTS));
    }
    mInc->setEnabled(PlayerInfo::getAttribute(CHAR_POINTS) >= mNeeded &&
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
        int newcorpoints = PlayerInfo::getAttribute(CORR_POINTS) - 1;
        PlayerInfo::setAttribute(CORR_POINTS, newcorpoints);

        int newpoints = PlayerInfo::getAttribute(CHAR_POINTS) + 1;
        PlayerInfo::setAttribute(CHAR_POINTS, newpoints);

        int newbase = PlayerInfo::getStatBase(mId) - 1;
        PlayerInfo::setStatBase(mId, newbase);

        Net::getPlayerHandler()->decreaseAttribute(mId);
    }
    else if (event.getSource() == mInc)
    {
        int newpoints = PlayerInfo::getAttribute(CHAR_POINTS) - 1;
        PlayerInfo::setAttribute(CHAR_POINTS, newpoints);

        int newbase = PlayerInfo::getStatBase(mId) + 1;
        PlayerInfo::setStatBase(mId, newbase);

        Net::getPlayerHandler()->increaseAttribute(mId);
    }
}
