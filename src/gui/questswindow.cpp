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

#include "questswindow.h"

#include "configuration.h"

#include "gui/setup.h"

#include "gui/widgets/browserbox.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/itemlinkhandler.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "resources/questdb.h"

#include "utils/gettext.h"

#include <guichan/font.hpp>

#include <algorithm>

class QuestsModel final : public gcn::ListModel
{
public:
    int getNumberOfElements() override
    { return mQuests.size(); }

    std::string getElementAt(int i) override
    { return mQuests[i].name(); }

    const std::vector<QuestEntry> &getQuests() const
    { return mQuests; }

    void setQuests(const std::vector<QuestEntry> &quests)
    { mQuests = quests; }

private:
    std::vector<QuestEntry> mQuests;
};


class QuestsListBox final : public ListBox
{
public:
    QuestsListBox(QuestsModel *model)
        : ListBox(model)
    {}

    unsigned getRowHeight() const override;

    void draw(gcn::Graphics *graphics) override;
};

unsigned QuestsListBox::getRowHeight() const
{
    auto rowHeight = ListBox::getRowHeight();

    if (auto icon = gui->getTheme()->getIcon("complete"))
        rowHeight = std::max<unsigned>(rowHeight, icon->getHeight() + 2);

    return rowHeight;
}

void QuestsListBox::draw(gcn::Graphics *gcnGraphics)
{
    if (!mListModel)
        return;

    auto *graphics = static_cast<Graphics *>(gcnGraphics);
    auto *model = static_cast<QuestsModel *>(getListModel());

    const int rowHeight = getRowHeight();

    auto theme = gui->getTheme();
    auto completeIcon = theme->getIcon("complete");
    auto incompleteIcon = theme->getIcon("incomplete");

    // Draw filled rectangle around the selected list element
    if (mSelected >= 0)
    {
        auto highlightColor = Theme::getThemeColor(Theme::HIGHLIGHT);
        highlightColor.a = gui->getTheme()->getGuiAlpha();
        graphics->setColor(highlightColor);
        graphics->fillRectangle(gcn::Rectangle(0, rowHeight * mSelected,
                                               getWidth(), rowHeight));
    }

    // Draw the list elements
    graphics->setFont(getFont());
    graphics->setColor(Theme::getThemeColor(Theme::TEXT));

    const int fontHeight = getFont()->getHeight();

    for (int i = 0, y = 0; i < model->getNumberOfElements();
         ++i, y += rowHeight)
    {
        if (mSelected == i)
            graphics->setColor(Theme::getThemeColor(Theme::HIGHLIGHT_TEXT));
        else
            graphics->setColor(Theme::getThemeColor(Theme::TEXT));

        auto &quest = model->getQuests()[i];
        int x = 1;

        if (const Image *icon = quest.completed ? completeIcon : incompleteIcon)
        {
            graphics->drawImage(icon, x, y + (rowHeight - icon->getHeight()) / 2);
            x += icon->getWidth() + 4;
        }

        graphics->drawText(quest.name(), x, y + (rowHeight - fontHeight) / 2);
    }
}


QuestsWindow::QuestsWindow()
    : Window(_("Quests"))
    , mQuestsModel(std::make_unique<QuestsModel>())
    , mQuestsListBox(new QuestsListBox(mQuestsModel.get()))
    , mHideCompletedCheckBox(new CheckBox(_("Hide completed"), config.hideCompletedQuests))
    , mQuestDetails(new BrowserBox(BrowserBox::AUTO_WRAP))
    , mLinkHandler(std::make_unique<ItemLinkHandler>())
{
    setWindowName("Quests");
    setupWindow->registerWindowForReset(this);
    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);

    setDefaultSize(387, 307, WindowAlignment::Center);
    setMinWidth(316);
    setMinHeight(179);

    mQuestsListBox->addSelectionListener(this);
    mHideCompletedCheckBox->setActionEventId("hideCompleted");
    mHideCompletedCheckBox->addActionListener(this);

    auto questListScrollArea = new ScrollArea(mQuestsListBox);
    questListScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mQuestDetails->setLinkHandler(mLinkHandler.get());
    mQuestDetailsScrollArea = new ScrollArea(mQuestDetails);
    mQuestDetailsScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    auto place = getPlacer(0, 0);
    place(0, 0, questListScrollArea, 2, 2).setPadding(2);
    place(2, 0, mQuestDetailsScrollArea, 3, 2).setPadding(2);
    place = getPlacer(0, 1);
    place(0, 0, mHideCompletedCheckBox);

    getLayout().setRowHeight(1, 0); // Don't scale up the bottom row

    listen(Event::QuestsChannel);

    refreshQuestList();
    loadWindowState();
}

QuestsWindow::~QuestsWindow() = default;

void QuestsWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "hideCompleted")
    {
        config.hideCompletedQuests = mHideCompletedCheckBox->isSelected();
        refreshQuestList();
    }
}

void QuestsWindow::valueChanged(const gcn::SelectionEvent &event)
{
    if (mSelectedQuestIndex != mQuestsListBox->getSelected())
        updateQuestDetails();
}

void QuestsWindow::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::QuestsChannel)
    {
        if (event.getType() == Event::QuestVarsChanged)
            refreshQuestList();
    }
}

void QuestsWindow::refreshQuestList()
{
    // Store the currently selected quest state and varId to preserve selection
    const QuestState *selectedQuestState = nullptr;
    int selectedVarId = -1;
    if (mSelectedQuestIndex >= 0 && mSelectedQuestIndex < mQuestsModel->getNumberOfElements())
    {
        const auto &selectedQuest = mQuestsModel->getQuests().at(mSelectedQuestIndex);
        selectedQuestState = selectedQuest.state;
        selectedVarId = selectedQuest.varId;
    }

    auto &questVars = Net::getPlayerHandler()->getQuestVars();
    auto newQuests = QuestDB::getQuestsEntries(questVars, config.hideCompletedQuests);

    // Put completed quests at the top
    std::stable_sort(newQuests.begin(), newQuests.end(), [](const QuestEntry &a, const QuestEntry &b) {
        return a.completed > b.completed;
    });

    mQuestsModel->setQuests(newQuests);

    if (!selectedQuestState)
        return;

    // Try to find and reselect the same quest, preferring exact state match
    int newSelectedIndex = -1;

    for (int i = 0; i < static_cast<int>(newQuests.size()); ++i)
    {
        if (newQuests[i].state == selectedQuestState)
        {
            newSelectedIndex = i;
            break;
        }
        else if (newSelectedIndex == -1 && newQuests[i].varId == selectedVarId)
        {
            newSelectedIndex = i;
            // Don't break here - continue looking for exact state match
        }
    }

    if (mSelectedQuestIndex != newSelectedIndex)
        mQuestsListBox->setSelected(newSelectedIndex);
    else
        updateQuestDetails();
}

void QuestsWindow::updateQuestDetails()
{
    mQuestDetails->clearRows();

    mSelectedQuestIndex = mQuestsListBox->getSelected();
    if (mSelectedQuestIndex < 0 || mSelectedQuestIndex >= mQuestsModel->getNumberOfElements())
        return;

    const QuestEntry &quest = mQuestsModel->getQuests().at(mSelectedQuestIndex);
    for (const auto &row : quest.rows())
    {
        switch (row.type)
        {
        case QuestRowType::Text:
            mQuestDetails->addRow(row.text);
            break;
        case QuestRowType::Name:
            mQuestDetails->addRow("[" + row.text + "]");
            break;
        case QuestRowType::Reward:
            mQuestDetails->addRow(strprintf(_("Reward: %s"), row.text.c_str()));
            break;
        case QuestRowType::Giver:
            mQuestDetails->addRow(strprintf(_("Quest Giver: %s"), row.text.c_str()));
            break;
        case QuestRowType::Coordinates:
            mQuestDetails->addRow(strprintf(_("Coordinates: %s (%d, %d)"),
                                            row.text.c_str(), row.x, row.y));
            break;
        case QuestRowType::NPC:
            mQuestDetails->addRow(strprintf(_("NPC: %s"), row.text.c_str()));
            break;
        }
    }
}
