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

#include "gui/npcdialog.h"

#include "client.h"
#include "configuration.h"
#include "event.h"
#include "eventlistener.h"
#include "inventory.h"
#include "item.h"
#include "playerinfo.h"

#include "gui/inventorywindow.h"
#include "gui/npcpostdialog.h"

#include "gui/widgets/browserbox.h"
#include "gui/widgets/button.h"
#include "gui/widgets/inttextfield.h"
#include "gui/widgets/itemcontainer.h"
#include "gui/widgets/itemlinkhandler.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textfield.h"

#include "net/net.h"
#include "net/npchandler.h"

#include "resources/iteminfo.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>

#include <algorithm>

#define CAPTION_WAITING _("Waiting for server")
#define CAPTION_NEXT _("Next")
#define CAPTION_CLOSE _("Close")
#define CAPTION_SUBMIT _("Submit")

class NpcEventListener : public EventListener
{
public:
    void event(Event::Channel channel, const Event &event) override;

    NpcDialog *findDialog(int id);
    NpcDialog *getDialog(int id);

    void removeDialog(int id);

private:
    std::map<int, NpcDialog *> mNpcDialogs;
};

static NpcEventListener *npcListener = nullptr;

NpcDialog::DialogList NpcDialog::instances;

NpcDialog::NpcDialog(int npcId)
    : Window(_("NPC"))
    , mNpcId(npcId)
    , mItemLinkHandler(std::make_unique<ItemLinkHandler>(this))
{
    // Basic Window Setup
    setWindowName("NpcText");
    setResizable(true);
    setCloseButton(false);
    setFocusable(true);

    setMinWidth(200);
    setMinHeight(150);

    setDefaultSize(260, 200, WindowAlignment::Center);

    // Setup output text box
    mTextBox = new BrowserBox(BrowserBox::AUTO_WRAP);
    mTextBox->setWrapIndent(15);
    mTextBox->setLinkHandler(mItemLinkHandler.get());
    mTextBox->setEnableKeys(true);

    mScrollArea = new ScrollArea(mTextBox);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);

    // Setup listbox
    mItemList = new ListBox(this);
    mItemList->setWrappingEnabled(true);
    setContentSize(260, 175);

    mListScrollArea = new ScrollArea(mItemList);
    mListScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mItemList->setVisible(true);

    // Setup string input box
    mTextField = new TextField;
    mTextField->setVisible(true);

    // Setup int input box
    mIntField = new IntTextField;
    mIntField->setVisible(true);
    mPlusButton = new Button(_("+"), "inc", this);
    mMinusButton = new Button(_("-"), "dec", this);

    // Setup item input
    mInputItems = std::make_unique<Inventory>(Inventory::NPC, 1);
    mInputItemsContainer = new ItemContainer(mInputItems.get());
    mInputItemsScrollArea = new ScrollArea(mInputItemsContainer);
    mInputItemsScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mAddItemButton = new Button(_("Add Item"), "add", this);

    mClearButton = new Button(_("Clear log"), "clear", this);

    // Setup button
    mNextButton = new Button("", "ok", this);

    int width = std::max(mNextButton->getFont()->getWidth(CAPTION_WAITING),
                         mNextButton->getFont()->getWidth(CAPTION_NEXT));
    width = std::max(width, mNextButton->getFont()->getWidth(CAPTION_CLOSE));
    width = std::max(width, mNextButton->getFont()->getWidth(CAPTION_SUBMIT));

    mNextButton->setWidth(8 + width);

    mResetButton = new Button(_("Reset"), "reset", this);

    // Place widgets
    buildLayout();

    center();
    loadWindowState();

    instances.push_back(this);
    setVisible(true);
    requestFocus();

    PlayerInfo::setNPCInteractionCount(PlayerInfo::getNPCInteractionCount()
                                       + 1);
}

NpcDialog::~NpcDialog()
{
    // These might not actually be in the layout, so lets be safe
    delete mScrollArea;
    delete mListScrollArea;
    delete mTextField;
    delete mIntField;
    delete mResetButton;
    delete mPlusButton;
    delete mMinusButton;
    delete mInputItemsScrollArea;
    delete mAddItemButton;
    delete mNextButton;

    instances.remove(this);

    PlayerInfo::setNPCInteractionCount(PlayerInfo::getNPCInteractionCount()
                                       - 1);

    npcListener->removeDialog(mNpcId);
}

void NpcDialog::setText(const std::vector<std::string> &text)
{
    mTextBox->clearRows();
    for (const std::string &row : text)
        mTextBox->addRow(row);
}

void NpcDialog::addText(const std::string &text, bool save)
{
    if (save || config.logNpcInGui)
    {
        mNewText.push_back(text);
        mTextBox->addRow(text);
    }
    mScrollArea->setVerticalScrollAmount(mScrollArea->getVerticalMaxScroll());
    mActionState = NPC_ACTION_WAIT;
    buildLayout();
}

void NpcDialog::showNextButton()
{
    mActionState = NPC_ACTION_NEXT;
    buildLayout();
}

void NpcDialog::showCloseButton()
{
    mActionState = NPC_ACTION_CLOSE;
    buildLayout();
}

void NpcDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        if (mActionState == NPC_ACTION_NEXT)
        {
            nextDialog();
            addText(std::string(), false);
        }
        else if (mActionState == NPC_ACTION_CLOSE)
        {
            close();
        }
        else if (mActionState == NPC_ACTION_INPUT)
        {
            std::string printText;  // Text that will get printed in the textbox

            switch (mInputState)
            {
            case NPC_INPUT_NONE:
                return;
            case NPC_INPUT_LIST: {
                int selectedIndex = mItemList->getSelected();
                if (selectedIndex >= (int) mItems.size() || selectedIndex < 0)
                    return;

                printText = mItems[selectedIndex];
                Net::getNpcHandler()->menuSelect(mNpcId, selectedIndex + 1);
                break;
            }
            case NPC_INPUT_STRING: {
                printText = mTextField->getText();
                Net::getNpcHandler()->stringInput(mNpcId, printText);
                break;
            }
            case NPC_INPUT_INTEGER: {
                printText = strprintf("%d", mIntField->getValue());
                Net::getNpcHandler()->integerInput(mNpcId, mIntField->getValue());
                break;
            }
            case NPC_INPUT_ITEM: {
                std::string response;
                for (auto &item : mInputItems->getItems())
                {
                    if (item)
                    {
                        if (!printText.empty())
                        {
                            printText += ", ";
                            response += ';';
                        }
                        printText += itemDb->get(item->getId()).name;
                        response += toString(item->getId());
                    }
                }

                Net::getNpcHandler()->stringInput(mNpcId, response);
                break;
            }
            }

            // addText will auto remove the input layout
            addText(strprintf("> \"%s\"", printText.c_str()), false);
            addText(std::string(), false);

            mNewText.clear();
        }

        if (!config.logNpcInGui)
            setText({});
    }
    else if (event.getId() == "reset")
    {
        if (mInputState == NPC_INPUT_STRING)
        {
            mTextField->setText(mDefaultString);
        }
        else if (mInputState == NPC_INPUT_INTEGER)
        {
            mIntField->setValue(mDefaultInt);
        }
    }
    else if (event.getId() == "inc")
    {
        mIntField->setValue(mIntField->getValue() + 1);
    }
    else if (event.getId() == "dec")
    {
        mIntField->setValue(mIntField->getValue() - 1);
    }
    else if (event.getId() == "add" && mInputState == NPC_INPUT_ITEM)
    {
        if (Item *item = inventoryWindow->getSelectedItem())
        {
            // When we only need one item, just replace whatever is there
            if (mInputItems->getSize() == 1)
                mInputItems->clear();

            int freeSlot = mInputItems->getFreeSlot();
            if (freeSlot == -1)
                return;

            mInputItems->setItem(freeSlot, item->getId(), 1);
            mAddItemButton->setEnabled(mInputItems->getSize() == 1 ||
                                       mInputItems->getFreeSlot() != -1);
        }
    }
    else if (event.getId() == "clear")
    {
        setText(mNewText);
    }
}

void NpcDialog::nextDialog() const
{
    Net::getNpcHandler()->nextDialog(mNpcId);
}

void NpcDialog::close()
{
    Net::getNpcHandler()->closeDialog(mNpcId);
    Window::close();
}

int NpcDialog::getNumberOfElements()
{
    return mItems.size();
}

std::string NpcDialog::getElementAt(int i)
{
    return mItems[i];
}

void NpcDialog::choiceRequest()
{
    mItems.clear();
    mActionState = NPC_ACTION_INPUT;
    mInputState = NPC_INPUT_LIST;
    buildLayout();
}

void NpcDialog::addChoice(const std::string &choice)
{
    mItems.push_back(choice);
}

void NpcDialog::initChoiceSelection()
{
    mItemList->setSelected(0);
}

void NpcDialog::textRequest(const std::string &defaultText)
{
    mActionState = NPC_ACTION_INPUT;
    mInputState = NPC_INPUT_STRING;
    mDefaultString = defaultText;
    mTextField->setText(defaultText);
    buildLayout();
}

bool NpcDialog::isTextInputFocused() const
{
    return mTextField->isFocused();
}

bool NpcDialog::isInputFocused() const
{
    return mTextField->isFocused() || mIntField->isFocused();
}

bool NpcDialog::isAnyInputFocused()
{
    return std::any_of(instances.begin(), instances.end(),
                       [](NpcDialog *dialog) {
                           return dialog->isInputFocused();
                       });
}

void NpcDialog::integerRequest(int defaultValue, int min, int max)
{
    mActionState = NPC_ACTION_INPUT;
    mInputState = NPC_INPUT_INTEGER;
    mDefaultInt = defaultValue;
    mIntField->setRange(min, max);
    mIntField->setValue(defaultValue);
    buildLayout();
}

void NpcDialog::itemRequest(int amount)
{
    mActionState = NPC_ACTION_INPUT;
    mInputState = NPC_INPUT_ITEM;
    mInputItems->clear();
    mInputItems->setSize(amount);
    mAddItemButton->setEnabled(true);
    buildLayout();
}

void NpcDialog::move(int amount)
{
    if (mActionState != NPC_ACTION_INPUT)
        return;

    switch (mInputState)
    {
        case NPC_INPUT_INTEGER:
            mIntField->setValue(mIntField->getValue() + amount);
            break;
        case NPC_INPUT_LIST:
            mItemList->setSelected(mItemList->getSelected() - amount);
            break;
        case NPC_INPUT_NONE:
        case NPC_INPUT_STRING:
        case NPC_INPUT_ITEM:
            break;
    }
}

void NpcDialog::setVisible(bool visible)
{
    Window::setVisible(visible);

    if (!visible)
        scheduleDelete();
}

void NpcDialog::mouseClicked(gcn::MouseEvent &mouseEvent)
{
    if (mouseEvent.getSource() == mItemList &&
        isDoubleClick(mItemList->getSelected()))
    {
        action(gcn::ActionEvent(mNextButton, mNextButton->getActionEventId()));
    }
    if (mouseEvent.getSource() == mTextBox && isDoubleClick((int)(intptr_t)mTextBox))
    {
        if (mActionState == NPC_ACTION_NEXT || mActionState == NPC_ACTION_CLOSE)
            action(gcn::ActionEvent(mNextButton,
                                    mNextButton->getActionEventId()));
    }
}

NpcDialog *NpcDialog::getActive()
{
    if (instances.size() == 1)
        return instances.front();

    for (auto dialog : instances)
        if (dialog->isFocused())
            return dialog;

    return nullptr;
}

void NpcDialog::closeAll()
{
    for (auto dialog : instances)
        dialog->close();
}

void NpcDialog::setup()
{
    if (npcListener)
        return;

    npcListener = new NpcEventListener;
    npcListener->listen(Event::NpcChannel);
}

void NpcDialog::buildLayout()
{
    clearLayout();

    if (mActionState != NPC_ACTION_INPUT)
    {
        if (mActionState == NPC_ACTION_WAIT)
        {
            mNextButton->setCaption(CAPTION_WAITING);
        }
        else if (mActionState == NPC_ACTION_NEXT)
        {
            mNextButton->setCaption(CAPTION_NEXT);
        }
        else if (mActionState == NPC_ACTION_CLOSE)
        {
            mNextButton->setCaption(CAPTION_CLOSE);
        }
        place(0, 0, mScrollArea, 5, 3);
        place(3, 3, mClearButton);
        place(4, 3, mNextButton);
    }
    else if (mInputState != NPC_INPUT_NONE)
    {
        if (!config.logNpcInGui)
            setText(mNewText);

        mNextButton->setCaption(CAPTION_SUBMIT);

        if (mInputState == NPC_INPUT_LIST)
        {
            place(0, 0, mScrollArea, 6, 3);
            place(0, 3, mListScrollArea, 6, 3);
            place(2, 6, mClearButton, 2);
            place(4, 6, mNextButton, 2);

            mItemList->setSelected(-1);
        }
        else if (mInputState == NPC_INPUT_STRING)
        {
            place(0, 0, mScrollArea, 6, 3);
            place(0, 3, mTextField, 6);
            place(0, 4, mResetButton, 2);
            place(2, 4, mClearButton, 2);
            place(4, 4, mNextButton, 2);
        }
        else if (mInputState == NPC_INPUT_INTEGER)
        {
            place(0, 0, mScrollArea, 6, 3);
            place(0, 3, mMinusButton, 1);
            place(1, 3, mIntField, 4);
            place(5, 3, mPlusButton, 1);
            place(0, 4, mResetButton, 2);
            place(2, 4, mClearButton, 2);
            place(4, 4, mNextButton, 2);
        }
        else if (mInputState == NPC_INPUT_ITEM)
        {
            place(0, 0, mScrollArea, 6, 3);
            place(0, 3, mInputItemsScrollArea, 6, 2);
            place(0, 5, mAddItemButton, 2);
            place(2, 5, mClearButton, 2);
            place(4, 5, mNextButton, 2);
        }
    }

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    bool waitState = isWaitingForTheServer();
    mNextButton->setEnabled(!waitState);
    setCloseButton(waitState);

    redraw();

    mScrollArea->setVerticalScrollAmount(mScrollArea->getVerticalMaxScroll());
}

void NpcEventListener::event(Event::Channel channel,
                             const Event &event)
{
    if (channel != Event::NpcChannel)
        return;

    if (event.getType() == Event::Message)
    {
        NpcDialog *dialog = getDialog(event.getInt("id"));

        dialog->addText(event.getString("text"));
    }
    else if (event.getType() == Event::Menu)
    {
        NpcDialog *dialog = getDialog(event.getInt("id"));

        dialog->choiceRequest();

        int count = event.getInt("choiceCount");
        for (int i = 1; i <= count; i++)
            dialog->addChoice(event.getString("choice" + toString(i)));
        dialog->initChoiceSelection();
    }
    else if (event.getType() == Event::IntegerInput)
    {
        NpcDialog *dialog = getDialog(event.getInt("id"));

        int defaultValue = event.getInt("default", 0);
        int min = event.getInt("min", 0);
        int max = event.getInt("max", 2147483647);

        dialog->integerRequest(defaultValue, min, max);
    }
    else if (event.getType() == Event::ItemInput)
    {
        NpcDialog *dialog = getDialog(event.getInt("id"));
        dialog->itemRequest(event.getInt("amount", 1));
    }
    else if (event.getType() == Event::StringInput)
    {
        NpcDialog *dialog = getDialog(event.getInt("id"));
        dialog->textRequest(event.getString("default", std::string()));
    }
    else if (event.getType() == Event::Next)
    {
        int id = event.getInt("id");

        if (NpcDialog *dialog = findDialog(id))
            dialog->showNextButton();
        else
            Net::getNpcHandler()->nextDialog(id);
    }
    else if (event.getType() == Event::ClearDialog)
    {
        if (NpcDialog *dialog = findDialog(event.getInt("id")))
            dialog->setText({});
    }
    else if (event.getType() == Event::Close)
    {
        int id = event.getInt("id");

        if (NpcDialog *dialog = findDialog(id))
            dialog->showCloseButton();
        else
            Net::getNpcHandler()->closeDialog(id);
    }
    else if (event.getType() == Event::CloseAll)
    {
        NpcDialog::closeAll();
    }
    else if (event.getType() == Event::CloseDialog)
    {
        if (NpcDialog *dialog = findDialog(event.getInt("id")))
            dialog->close();
    }
    else if (event.getType() == Event::Post)
    {
        new NpcPostDialog(event.getInt("id"));
    }
}

NpcDialog *NpcEventListener::findDialog(int id)
{
    auto it = mNpcDialogs.find(id);
    return it == mNpcDialogs.end() ? nullptr : it->second;
}

NpcDialog *NpcEventListener::getDialog(int id)
{
    auto dialog = findDialog(id);

    if (!dialog)
    {
        dialog = new NpcDialog(id);
        mNpcDialogs[id] = dialog;
    }

    return dialog;
}

void NpcEventListener::removeDialog(int id)
{
    auto it = mNpcDialogs.find(id);
    if (it != mNpcDialogs.end())
        mNpcDialogs.erase(it);
}
