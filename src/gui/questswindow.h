/*
 *  The Mana Client
 *  Copyright (C) 2025  The Mana Developers
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

#pragma once

#include "eventlistener.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/selectionlistener.hpp>

class BrowserBox;
class CheckBox;
class LinkHandler;
class QuestsListBox;
class QuestsModel;
class ScrollArea;

/**
 * Quests window.
 *
 * \ingroup Interface
 */
class QuestsWindow final : public Window,
                           public gcn::ActionListener,
                           public gcn::SelectionListener,
                           public EventListener
{
public:
    QuestsWindow();

    void action(const gcn::ActionEvent &event) override;

    void valueChanged(const gcn::SelectionEvent &event) override;

    void event(Event::Channel channel, const Event &event) override;

private:
    void refreshQuestList();
    void updateQuestDetails();

    int mSelectedQuestIndex = -1;
    std::unique_ptr<QuestsModel> mQuestsModel;
    QuestsListBox *mQuestsListBox;
    CheckBox *mHideCompletedCheckBox;
    BrowserBox *mQuestDetails;
    ScrollArea *mQuestDetailsScrollArea;
    std::unique_ptr<LinkHandler> mLinkHandler;
};

extern QuestsWindow *questsWindow;
