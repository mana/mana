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

#include "gui/npcdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textbox.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/inttextfield.h"

#include "npc.h"

#include "net/net.h"
#include "net/npchandler.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

NpcDialog::NpcDialog()
    : Window(_("NPC")), 
      mActionState(NPC_ACTION_WAIT),
      mInputState(NPC_INPUT_NONE),
      mNpcId(0),
      mText("")
{
    // Basic Window Setup
    setWindowName("NPCText");
    setResizable(true);

    setMinWidth(200);
    setMinHeight(150);

    setDefaultSize(260, 200, ImageRect::CENTER);

    // Setup output text box
    mTextBox = new TextBox;
    mTextBox->setEditable(false);
    mTextBox->setOpaque(false);

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
    mTextField = new TextField("");
    mTextField->setVisible(true);

    // Setup int input box
    mIntField = new IntTextField;
    mIntField->setVisible(true);

    // Setup button
    mButton = new Button(_("Next"), "ok", this);

    // Place widgets
    buildLayout();

    center();
    loadWindowState();
}


void NpcDialog::setText(const std::string &text)
{
    mText = text;
    mTextBox->setTextWrapped(mText, mScrollArea->getWidth() - 15);
}

void NpcDialog::addText(const std::string &text)
{
    setText(mText + text + "\n");
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
            addText("\n> Next\n");
        } 
        else if (mActionState == NPC_ACTION_CLOSE)
        {
            if (current_npc) 
                closeDialog();
            setVisible(false);
            current_npc = 0;
            NPC::isTalking = false;
        } 
        else if (mActionState == NPC_ACTION_INPUT)
        {
            std::string printText = "";  // Text that will get printed in the textbox
            if (mInputState == NPC_INPUT_LIST)
            {
                 int choice = 0;
                 int selectedIndex = mItemList->getSelected();

                 if (selectedIndex > -1)
                 {
                      choice = selectedIndex + 1;
                      printText = mItems[selectedIndex];
                 }

                 Net::getNpcHandler()->listInput(mNpcId, choice);
            }
            else if (mInputState == NPC_INPUT_STRING)
            {
                printText = mTextField->getText();
                Net::getNpcHandler()->stringInput(mNpcId, printText);
            }
            else if (mInputState == NPC_INPUT_INTEGER)
            {
                printText = strprintf("%d", mIntField->getValue());
                Net::getNpcHandler()->integerInput(mNpcId, mIntField->getValue());
            }
            // addText will auto remove the input layout
            addText( strprintf("\n> \"%s\"\n", printText.c_str()) );
        } 
    }
}

void NpcDialog::nextDialog()
{
    Net::getNpcHandler()->nextDialog(mNpcId);
}

void NpcDialog::closeDialog()
{
    Net::getNpcHandler()->closeDialog(mNpcId);
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

void NpcDialog::addChoice(std::string choice)
{
    mItems.push_back(choice);
}

void NpcDialog::parseListItems(const std::string &itemString)
{
    std::istringstream iss(itemString);

    std::string tmp;
    while (getline(iss, tmp, ':'))
        mItems.push_back(tmp);
}

void NpcDialog::textRequest(std::string defaultText)
{
    mActionState = NPC_ACTION_INPUT;
    mInputState = NPC_INPUT_STRING;
    mTextField->setText(defaultText);
    buildLayout();
}

bool NpcDialog::isInputFocused()
{
    return mTextField->isFocused() || mIntField->isFocused();
}

void NpcDialog::integerRequest(int defaultValue, int min, int max)
{
    mActionState = NPC_ACTION_INPUT;
    mInputState = NPC_INPUT_INTEGER;
    mIntField->setRange(min, max);
    mIntField->setValue(defaultValue);
    buildLayout();
}

void NpcDialog::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    setText(mText);
}

void NpcDialog::buildLayout()
{
    clearLayout();

    if (mActionState != NPC_ACTION_INPUT)
    {
        if (mActionState == NPC_ACTION_WAIT)
        {
            mButton->setCaption(_("Waiting for server"));
        }
        else if (mActionState == NPC_ACTION_NEXT)
        {
            mButton->setCaption(_("Next"));
        }
        else if (mActionState == NPC_ACTION_CLOSE)
        {
            mButton->setCaption(_("Close"));
        }
        place(0, 0, mScrollArea, 5, 3);
        place(4, 3, mButton);
    }
    else if (mInputState != NPC_INPUT_NONE)
    {
        mButton->setCaption(_("Submit"));
        if (mInputState == NPC_INPUT_LIST)
        {
            place(0, 0, mScrollArea, 5, 3);
            place(0, 3, mListScrollArea, 5, 3);
            place(3, 6, mButton, 2);
        }
        else if(mInputState == NPC_INPUT_STRING)
        {
            place(0, 0, mScrollArea, 5, 3);
            place(0, 3, mTextField, 3);
            place(3, 4, mButton, 2);
        }
        else if(mInputState == NPC_INPUT_INTEGER)
        {
            place(0, 0, mScrollArea, 5, 3);
            place(0, 3, mIntField, 3);
            place(3, 4, mButton, 2);
        }
    }
    
    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    mButton->setEnabled(mActionState != NPC_ACTION_WAIT);

    redraw();

    mScrollArea->setVerticalScrollAmount(mScrollArea->getVerticalMaxScroll());
}
