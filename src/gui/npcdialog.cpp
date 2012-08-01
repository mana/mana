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
#include "playerinfo.h"

#include "gui/npcpostdialog.h"
#include "gui/setup.h"

#include "gui/widgets/button.h"
#include "gui/widgets/inttextfield.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textbox.h"
#include "gui/widgets/textfield.h"

#include "net/net.h"
#include "net/npchandler.h"

#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>

typedef std::map<int, NpcDialog*> NpcDialogs;

class NpcEventListener : public EventListener
{
public:
    void event(Event::Channel channel, const Event &event);

    NpcDialog *getDialog(int id, bool make = true);

    void removeDialog(int id);

private:
    NpcDialogs mNpcDialogs;
};

static NpcEventListener *npcListener = NULL;

NpcDialog::DialogList NpcDialog::instances;

NpcDialog::NpcDialog(int npcId)
    : Window("", false, NULL, "npcdialog.xml"),
      mNpcId(npcId),
      mDefaultInt(0),
      mText(""),
      mTextPlayTime(tick_time),
      mClearTextOnNextPlay(true),
      mInputState(NPC_INPUT_NONE),
      mActionState(NPC_ACTION_WAIT)
{
    // Basic Window Setup
    setWindowName("NpcText");
    setResizable(false);
    setCloseButton(false);
    setMovable(false);
    setFocusable(true);

    // Setup output text box
    mTextBox = new TextBox;
    mTextBox->setEditable(false);
    mTextBox->setOpaque(false);

    mTextBox->setTextColor(&(Theme::instance()->getColor(Theme::NPC_DIALOG_TEXT)));

    // Place the window
    setContentSize(getParent()->getWidth() / 2, 175);
    setLocationRelativeTo(ImageRect::LOWER_CENTER, 0, -50);

    // Setup listbox
    mItemList = new ListBox(this);
    mItemList->setWrappingEnabled(true);

    mListScrollArea = new ScrollArea(mItemList);
    mListScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mItemList->setVisible(true);

    // Setup string input box
    mTextField = new TextField("");
    mTextField->setVisible(true);

    // Setup int input box
    mIntField = new IntTextField;
    mIntField->setVisible(true);

    mSubmitButton = new Button(_("Submit"), "submit", this);

    // Place widgets
    buildLayout();

    instances.push_back(this);
    setVisible(true);
    requestFocus();

    this->addKeyListener(this);

    PlayerInfo::setNPCInteractionCount(PlayerInfo::getNPCInteractionCount()
                                       + 1);
}

NpcDialog::~NpcDialog()
{
    // These might not actually be in the layout, so lets be safe
    delete mTextBox;
    delete mItemList;
    delete mTextField;
    delete mIntField;
    delete mSubmitButton;

    instances.remove(this);

    this->removeKeyListener(this);

    PlayerInfo::setNPCInteractionCount(PlayerInfo::getNPCInteractionCount()
                                       - 1);

    npcListener->removeDialog(mNpcId);
}

void NpcDialog::logic()
{

    if (get_elapsed_time(mTextPlayTime) > 10)
    {
        mTextPlayTime = tick_time;

        ushort currentLength = mTextBox->getText().length();
        if (currentLength < mText.length())
        {
            setText(mText.substr(0, currentLength + 1));
        }
    }
    Window::logic();
}

void NpcDialog::playText(const std::string &text)
{
    if (mClearTextOnNextPlay)
    {
        mClearTextOnNextPlay = false;
        mText = "";
        setText("");

    }
    mText += text + "\n";
}

void NpcDialog::setText(const std::string &text)
{
    mTextBox->setTextWrapped(text, getWidth() - 15);
}

void NpcDialog::setStateNext()
{
    mActionState = NPC_ACTION_NEXT;
    buildLayout();
}

void NpcDialog::setStateClose()
{
    mActionState = NPC_ACTION_CLOSE;
    buildLayout();
}

void NpcDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    if (!keyEvent.isConsumed())
    {
        if (keyEvent.getKey() == gcn::Key::SPACE)
        {
            keyEvent.consume();
            proceed();
        }
    }
}

void NpcDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "submit")
    {
        proceed();
    }
}

void NpcDialog::proceed()
{
    // If the message isn't done typing out, finish it
    if (mTextBox->getText().length() < mText.length())
    {
        setText(mText);
        return;
    }

    if (mActionState == NPC_ACTION_NEXT)
    {
        mClearTextOnNextPlay = true;
        Net::getNpcHandler()->nextDialog(mNpcId);
    }
    else if (mActionState == NPC_ACTION_CLOSE)
    {
        close();
    }
    else if (mActionState == NPC_ACTION_INPUT)
    {
        if (mInputState == NPC_INPUT_LIST)
        {
            int selectedIndex = mItemList->getSelected();

            if (selectedIndex >= (int) mItems.size() || selectedIndex < 0)
                return;

            Net::getNpcHandler()->menuSelect(mNpcId, selectedIndex + 1);
        }
        else if (mInputState == NPC_INPUT_STRING)
        {
            Net::getNpcHandler()->stringInput(mNpcId, mTextField->getText());
        }
        else if (mInputState == NPC_INPUT_INTEGER)
        {
            Net::getNpcHandler()->integerInput(mNpcId, mIntField->getValue());
        }
        mClearTextOnNextPlay = true;
    }

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
    DialogList::iterator it = instances.begin();
    DialogList::iterator it_end = instances.end();

    for (; it != it_end; it++)
    {
        if ((*it)->isInputFocused())
        {
            return true;
        }
    }

    return false;
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
            break;
    }
}

void NpcDialog::setVisible(bool visible)
{
    Window::setVisible(visible);

    if (!visible)
    {
        scheduleDelete();
    }
}

void NpcDialog::mousePressed(gcn::MouseEvent &mouseEvent)
{
    if (mouseEvent.getSource() == mItemList &&
        isDoubleClick(mItemList->getSelected()))
    {
        proceed();
    }
    if (mActionState == NPC_ACTION_NEXT || mActionState == NPC_ACTION_CLOSE)
    {
        proceed();
    }
    Window::mousePressed(mouseEvent);
}

NpcDialog *NpcDialog::getActive()
{
    if (instances.size() == 1)
        return instances.front();

    DialogList::iterator it = instances.begin();
    DialogList::iterator it_end = instances.end();

    for (; it != it_end; it++)
    {
        if ((*it)->isFocused())
        {
            return (*it);
        }
    }

    return 0;
}

void NpcDialog::closeAll()
{
    DialogList::iterator it = instances.begin();
    DialogList::iterator it_end = instances.end();

    for (; it != it_end; it++)
    {
        (*it)->close();
    }
}

void NpcDialog::setup()
{
    if (npcListener)
        return;

    npcListener = new NpcEventListener();

    npcListener->listen(Event::NpcChannel);
}

void NpcDialog::buildLayout()
{
    clearLayout();

    if (mActionState != NPC_ACTION_INPUT)
    {
        place(0, 0, mTextBox, 5, 3);
    }
    else if (mInputState != NPC_INPUT_NONE)
    {
        if (mInputState == NPC_INPUT_LIST)
        {
            place(0, 0, mTextBox, 6, 3);
            place(0, 3, mListScrollArea, 6, 3);

            mItemList->setSelected(-1);
        }
        else if (mInputState == NPC_INPUT_STRING)
        {
            place(0, 0, mTextBox, 6, 3);
            place(1, 3, mTextField, 3);
            place(4, 3, mSubmitButton, 1);
        }
        else if (mInputState == NPC_INPUT_INTEGER)
        {
            place(0, 0, mTextBox, 6, 3);
            place(1, 3, mIntField, 3);
            place(4, 3, mSubmitButton, 1);
        }
    }

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    redraw();
}

void NpcEventListener::event(Event::Channel channel,
                             const Event &event)
{
    if (channel != Event::NpcChannel)
        return;

    if (event.getType() == Event::Message)
    {
        NpcDialog *dialog = getDialog(event.getInt("id"));
        dialog->playText(event.getString("text"));
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
    else if (event.getType() == Event::StringInput)
    {
        NpcDialog *dialog = getDialog(event.getInt("id"));

        try
        {
            dialog->textRequest(event.getString("default"));
        }
        catch (BadEvent)
        {
            dialog->textRequest("");
        }
    }
    else if (event.getType() == Event::Next)
    {
        int id = event.getInt("id");
        NpcDialog *dialog = getDialog(id, false);

        if (!dialog)
        {
            int mNpcId = id;
            Net::getNpcHandler()->nextDialog(mNpcId);
            return;
        }

        dialog->setStateNext();
    }
    else if (event.getType() == Event::Close)
    {
        int id = event.getInt("id");
        NpcDialog *dialog = getDialog(id, false);

        if (!dialog)
        {
            int mNpcId = id;
            Net::getNpcHandler()->closeDialog(mNpcId);
            return;
        }

        dialog->setStateClose();
    }
    else if (event.getType() == Event::CloseAll)
    {
        NpcDialog::closeAll();
    }
    else if (event.getType() == Event::End)
    {
        int id = event.getInt("id");
        NpcDialog *dialog = getDialog(id, false);

        if (dialog)
            dialog->close();
    }
    else if (event.getType() == Event::Post)
    {
        new NpcPostDialog(event.getInt("id"));
    }
}

NpcDialog *NpcEventListener::getDialog(int id, bool make)
{
    NpcDialogs::iterator diag = mNpcDialogs.find(id);
    NpcDialog *dialog = 0;

    if (diag == mNpcDialogs.end())
    {
        // Empty dialogs don't help
        if (make)
        {
            dialog = new NpcDialog(id);
            mNpcDialogs[id] = dialog;
        }
    }
    else
    {
        dialog = diag->second;
    }

    return dialog;
}

void NpcEventListener::removeDialog(int id)
{
    NpcDialogs::iterator it = mNpcDialogs.find(id);
    if (it != mNpcDialogs.end())
        mNpcDialogs.erase(it);
}
