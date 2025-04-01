/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "gui/setup_players.h"

#include "configuration.h"

#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/dropdown.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/table.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

#include <string>
#include <vector>

#define COLUMNS_NR 2 // name plus listbox
#define NAME_COLUMN 0
#define RELATION_CHOICE_COLUMN 1

#define ROW_HEIGHT 12
// The following column widths really shouldn't be hardcoded but should scale with the size of the widget... except
// that, right now, the widget doesn't exactly scale either.
#define NAME_COLUMN_WIDTH 230
#define RELATION_CHOICE_COLUMN_WIDTH 80

#define WIDGET_AT(row, column) (((row) * COLUMNS_NR) + column)

static const char *table_titles[COLUMNS_NR] =
{
    N_("Name"),
    N_("Relation")
};

static const char *RELATION_NAMES[RELATIONS_NR] =
{
    N_("Neutral"),
    N_("Friend"),
    N_("Disregarded"),
    N_("Ignored")
};

class PlayerRelationListModel : public gcn::ListModel
{
public:
    ~PlayerRelationListModel() override { }

    int getNumberOfElements() override
    {
        return RELATIONS_NR;
    }

    std::string getElementAt(int i) override
    {
        if (i >= getNumberOfElements() || i < 0)
            return "";
        return gettext(RELATION_NAMES[i]);
    }
};

class PlayerTableModel : public TableModel
{
public:
    PlayerTableModel() :
        mListModel(new PlayerRelationListModel)
    {
        playerRelationsUpdated();
    }

    ~PlayerTableModel() override
    {
        freeWidgets();
        delete mListModel;
    }

    int getRows() const override
    {
        return mPlayers.size();
    }

    int getColumns() const override
    {
        return COLUMNS_NR;
    }

    int getRowHeight() const override
    {
        return ROW_HEIGHT;
    }

    int getColumnWidth(int index) const override
    {
        if (index == NAME_COLUMN)
            return NAME_COLUMN_WIDTH;
        else
            return RELATION_CHOICE_COLUMN_WIDTH;
    }

    void playerRelationsUpdated()
    {
        signalBeforeUpdate();

        freeWidgets();

        mPlayers = player_relations.getPlayers();

        // set up widgets
        for (const auto &name : mPlayers)
        {
            gcn::Widget *widget = new Label(name);
            mWidgets.push_back(widget);

            gcn::DropDown *choicebox = new DropDown(mListModel);
            choicebox->setSelected(static_cast<int>(player_relations.getRelation(name)));
            mWidgets.push_back(choicebox);
        }

        signalAfterUpdate();
    }

    virtual void updateModelInRow(int row)
    {
        auto *choicebox = static_cast<gcn::DropDown *>(
                                   getElementAt(row, RELATION_CHOICE_COLUMN));
        player_relations.setRelation(getPlayerAt(row),
                                     static_cast<PlayerRelation>(
                                         choicebox->getSelected()));
    }


    gcn::Widget *getElementAt(int row, int column) const override
    {
        return mWidgets[WIDGET_AT(row, column)];
    }

    virtual void freeWidgets()
    {
        mPlayers.clear();

        delete_all(mWidgets);
        mWidgets.clear();
    }

    const std::string &getPlayerAt(int index) const
    {
        return mPlayers[index];
    }

protected:
    std::vector<std::string> mPlayers;
    std::vector<gcn::Widget *> mWidgets;
    PlayerRelationListModel *mListModel;
};

/**
 * Class for choosing one of the various `what to do when ignoring a player' options
 */
class IgnoreChoicesListModel : public gcn::ListModel
{
public:
    ~IgnoreChoicesListModel() override { }

    int getNumberOfElements() override
    {
        return player_relations.getPlayerIgnoreStrategies().size();
    }

    std::string getElementAt(int i) override
    {
        if (i >= getNumberOfElements())
            return _("???");

        return player_relations.getPlayerIgnoreStrategies()[i]->mDescription;
    }
};

#define ACTION_DELETE "delete"
#define ACTION_TABLE "table"
#define ACTION_STRATEGY "strategy"

Setup_Players::Setup_Players():
    mShowGender(config.showGender),
    mPlayerTableTitleModel(new StaticTableModel(1, COLUMNS_NR)),
    mPlayerTableModel(new PlayerTableModel),
    mPlayerTable(new GuiTable(mPlayerTableModel)),
    mPlayerTitleTable(new GuiTable(mPlayerTableTitleModel)),
    mPlayerScrollArea(new ScrollArea(mPlayerTable)),
    mDefaultTrading(new CheckBox(_("Allow trading"),
                player_relations.getDefault() & PlayerPermissions::TRADE)),
    mDefaultWhisper(new CheckBox(_("Allow whispers"),
                player_relations.getDefault() & PlayerPermissions::WHISPER)),
    mDeleteButton(new Button(_("Delete"), ACTION_DELETE, this)),
    mWhisperTabCheckBox(new CheckBox(_("Put all whispers in tabs"), config.whisperTab)),
    mShowGenderCheckBox(new CheckBox(_("Show gender"), config.showGender)),
    mEnableChatLogCheckBox(new CheckBox(_("Enable Chat log"), config.enableChatLog))
{
    setName(_("Players"));

    mPlayerTable->setOpaque(false);

    mPlayerTableTitleModel->fixColumnWidth(NAME_COLUMN, NAME_COLUMN_WIDTH);
    mPlayerTableTitleModel->fixColumnWidth(RELATION_CHOICE_COLUMN,
                                           RELATION_CHOICE_COLUMN_WIDTH);
    mPlayerTitleTable->setBackgroundColor(gcn::Color(0xbf, 0xbf, 0xbf));

    mIgnoreActionChoicesModel = new IgnoreChoicesListModel;
    mIgnoreActionChoicesBox = new DropDown(mIgnoreActionChoicesModel);

    for (int i = 0; i < COLUMNS_NR; i++)
    {
        mPlayerTableTitleModel->set(0, i,
                new Label(gettext(table_titles[i])));
    }

    mPlayerTitleTable->setLinewiseSelection(true);

    mPlayerScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mPlayerTable->setActionEventId(ACTION_TABLE);
    mPlayerTable->setLinewiseSelection(true);
    mPlayerTable->addActionListener(this);

    gcn::Label *ignore_action_label = new Label(_("When ignoring:"));

    mShowGenderCheckBox->setActionEventId("showgender");
    mShowGenderCheckBox->addActionListener(this);
    mIgnoreActionChoicesBox->setActionEventId(ACTION_STRATEGY);
    mIgnoreActionChoicesBox->addActionListener(this);

    int ignoreStrategyIndex = 0; // safe default

    if (auto ignoreStrategy = player_relations.getPlayerIgnoreStrategy())
    {
        ignoreStrategyIndex = player_relations.getPlayerIgnoreStrategyIndex(
            ignoreStrategy->mShortName);
        if (ignoreStrategyIndex < 0)
            ignoreStrategyIndex = 0;
    }
    mIgnoreActionChoicesBox->setSelected(ignoreStrategyIndex);
    mIgnoreActionChoicesBox->adjustHeight();

    reset();

    // Do the layout
    place(0, 0, mPlayerTitleTable, 4);
    place(0, 1, mPlayerScrollArea, 4, 4).setPadding(2);
    place(0, 5, mDeleteButton);
    place(0, 6, mShowGenderCheckBox, 2);
    place(0, 7, mEnableChatLogCheckBox, 2);
    place(2, 5, ignore_action_label);
    place(2, 6, mIgnoreActionChoicesBox, 2);
    place(0, 8, mDefaultTrading);
    place(0, 9, mDefaultWhisper);
    place(0, 10, mWhisperTabCheckBox, 4);

    player_relations.addListener(this);
}

Setup_Players::~Setup_Players()
{
    player_relations.removeListener(this);
    delete mIgnoreActionChoicesModel;
}


void Setup_Players::reset()
{
    // We now have to search through the list of ignore choices to find the
    // current selection. We could use an index into the table of config
    // options in player_relations instead of strategies to sidestep this.
    const auto &strategies = player_relations.getPlayerIgnoreStrategies();
    auto i = std::find(strategies.begin(), strategies.end(),
                       player_relations.getPlayerIgnoreStrategy());

    int selection = i == strategies.end() ? 0 : i - strategies.begin();
    mIgnoreActionChoicesBox->setSelected(selection);
}

void Setup_Players::apply()
{
    player_relations.store();

    unsigned int old_default_relations = player_relations.getDefault() &
                                         ~(PlayerPermissions::TRADE |
                                           PlayerPermissions::WHISPER);
    player_relations.setDefault(old_default_relations
                                | (mDefaultTrading->isSelected() ?
                                       PlayerPermissions::TRADE : 0)
                                | (mDefaultWhisper->isSelected() ?
                                       PlayerPermissions::WHISPER : 0));

    config.whisperTab = mWhisperTabCheckBox->isSelected();
    config.enableChatLog = mEnableChatLogCheckBox->isSelected();

    mShowGender = config.showGender;
}

void Setup_Players::cancel()
{
    mWhisperTabCheckBox->setSelected(config.whisperTab);
    mShowGenderCheckBox->setSelected(mShowGender);
    mEnableChatLogCheckBox->setSelected(config.enableChatLog);

    setConfigValue(&Config::showGender, mShowGender);
}

void Setup_Players::action(const gcn::ActionEvent &event)
{
    if (event.getId() == ACTION_TABLE)
    {
        // temporarily eliminate ourselves: we are fully aware of this change,
        // so there is no need for asynchronous updates.  (In fact, thouse
        // might destroy the widet that triggered them, which would be rather
        // embarrassing.)
        player_relations.removeListener(this);

        int row = mPlayerTable->getSelectedRow();
        if (row >= 0)
            mPlayerTableModel->updateModelInRow(row);

        player_relations.addListener(this);
    }
    else if (event.getId() == ACTION_DELETE)
    {
        int player_index = mPlayerTable->getSelectedRow();
        if (player_index < 0)
            return;

        const std::string &name = mPlayerTableModel->getPlayerAt(player_index);
        player_relations.removePlayer(name);
    }
    else if (event.getId() == ACTION_STRATEGY)
    {
        PlayerIgnoreStrategy *s =
            player_relations.getPlayerIgnoreStrategies()[
                mIgnoreActionChoicesBox->getSelected()];

        player_relations.setPlayerIgnoreStrategy(s);
    }
    else if (event.getId() == "showgender")
    {
        setConfigValue(&Config::showGender, mShowGenderCheckBox->isSelected());
    }
}

void Setup_Players::playerRelationsUpdated()
{
    mPlayerTableModel->playerRelationsUpdated();
    mDefaultTrading->setSelected(
            player_relations.getDefault() & PlayerPermissions::TRADE);
    mDefaultWhisper->setSelected(
            player_relations.getDefault() & PlayerPermissions::WHISPER);
}
