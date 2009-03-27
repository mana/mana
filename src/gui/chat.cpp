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

#include "chat.h"

#include "gui/chatinput.h"
#include "gui/itemlinkhandler.h"
#include "gui/recorder.h"
#include "gui/scrollarea.h"
#include "gui/sdlinput.h"

#include "gui/widgets/chattab.h"
#include "gui/widgets/tabbedarea.h"
#include "gui/widgets/whispertab.h"

#include "beingmanager.h"
#include "configuration.h"
#include "localplayer.h"

#include "utils/dtor.h"
#include "utils/stringutils.h"

#include <guichan/focushandler.hpp>

ChatWindow::ChatWindow():
    Window(_("Chat")),
    mTmpVisible(false)
{
    setWindowName("Chat");

    setResizable(true);
    setDefaultSize(600, 123, ImageRect::LOWER_LEFT);
    setMinWidth(150);
    setMinHeight(90);

    mItemLinkHandler = new ItemLinkHandler;

    mChatInput = new ChatInput;
    mChatInput->setActionEventId("chatinput");
    mChatInput->addActionListener(this);

    mChatTabs = new TabbedArea;

    add(mChatTabs);
    add(mChatInput);

    loadWindowState();

    // Add key listener to chat input to be able to respond to up/down
    mChatInput->addKeyListener(this);
    mCurHist = mHistory.end();

#ifdef EATHENA_SUPPORT
    // Read the party prefix
    std::string partyPrefix = config.getValue("PartyPrefix", "$");
    mPartyPrefix = (partyPrefix.empty() ? '$' : partyPrefix.at(0));
    mReturnToggles = config.getValue("ReturnToggles", "0") == "1";

    // If the player had @assert on in the last session, ask the server to
    // run the @assert command for the player again. Convenience for GMs.
    if (config.getValue(player_node->getName() + "GMassert", 0)) {
        std::string cmd = "@assert";
        chatInput(cmd);
    }
#endif
    mRecorder = new Recorder(this);
}

ChatWindow::~ChatWindow()
{
#ifdef EATHENA_SUPPORT
    char partyPrefix[2] = ".";
    *partyPrefix = mPartyPrefix;
    config.setValue("PartyPrefix", partyPrefix);
    config.setValue("ReturnToggles", mReturnToggles ? "1" : "0");
    delete mRecorder;
#endif
    delete_all(mWhispers);
    delete mItemLinkHandler;
}

void ChatWindow::resetToDefaultSize()
{
    mRecorder->resetToDefaultSize();
    Window::resetToDefaultSize();
}

void ChatWindow::adjustTabSize()
{
    const gcn::Rectangle area = getChildrenArea();

    mChatInput->setPosition(mChatInput->getFrameSize(),
                            area.height - mChatInput->getHeight() -
                                mChatInput->getFrameSize());
    mChatInput->setWidth(area.width - 2 * mChatInput->getFrameSize());

    mChatTabs->setWidth(area.width - 2 * mChatTabs->getFrameSize());
    mChatTabs->setHeight(area.height - 2 * mChatTabs->getFrameSize() -
                         (mChatInput->getHeight() + mChatInput->getFrameSize() * 2));

    ChatTab *tab = getFocused();
    if (tab) {
        gcn::Widget *content = tab->mScrollArea;
        content->setSize(mChatTabs->getWidth() - 2 * content->getFrameSize(),
                         mChatTabs->getContainerHeight() - 2 * content->getFrameSize());
        content->logic();
    }
}

void ChatWindow::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    adjustTabSize();
}

void ChatWindow::logic()
{
    Window::logic();

    Tab *tab = getFocused();
    if (tab != mCurrentTab) {
        mCurrentTab = tab;
        adjustTabSize();
    }
}

ChatTab *ChatWindow::getFocused() const
{
    return dynamic_cast<ChatTab*>(mChatTabs->getSelectedTab());
}

void ChatWindow::clearTab(ChatTab *tab)
{
    if (tab)
        tab->clearText();
}

void ChatWindow::clearTab()
{
    clearTab(getFocused());
}

void ChatWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "chatinput")
    {
        std::string message = mChatInput->getText();

        if (!message.empty())
        {
            // If message different from previous, put it in the history
            if (mHistory.empty() || message != mHistory.back())
            {
                mHistory.push_back(message);
            }
            // Reset history iterator
            mCurHist = mHistory.end();

            // Send the message to the server
            chatInput(message);

            // Clear the text from the chat input
            mChatInput->setText("");
        }

        if (message.empty() || !mReturnToggles)
        {
            // Remove focus and hide input
            mFocusHandler->focusNone();

            // If the chatWindow is shown up because you want to send a message
            // It should hide now
            if (mTmpVisible)
                setVisible(false);
        }
    }
}

bool ChatWindow::requestChatFocus()
{
    // Make sure chatWindow is visible
    if (!isVisible())
    {
        setVisible(true);

        /*
         * This is used to hide chatWindow after sending the message. There is
         * a trick here, because setVisible will set mTmpVisible to false, you
         * have to put this sentence *after* setVisible, not before it
         */
        mTmpVisible = true;
    }

    // Don't do anything else if the input is already visible and has focus
    if (mChatInput->isVisible() && mChatInput->isFocused())
        return false;

    // Give focus to the chat input
    mChatInput->setVisible(true);
    mChatInput->requestFocus();
    return true;
}

bool ChatWindow::isInputFocused()
{
    return mChatInput->isFocused();
}

void ChatWindow::removeTab(ChatTab *tab)
{
    // Prevent removal of the local chat tab
    if (tab == localChatTab)
        return;

    mChatTabs->removeTab(tab);
}

void ChatWindow::addTab(ChatTab *tab)
{
    // Make sure we don't end up with duplicates in the gui
    // TODO

    mChatTabs->addTab(tab, tab->mScrollArea);

    // Fix for layout issues when adding the first tab
    if (tab == localChatTab)
        adjustTabSize();

    // Update UI
    logic();
}

void ChatWindow::removeWhisper(std::string nick)
{
    mWhispers.erase(nick);
}

void ChatWindow::chatInput(std::string &msg)
{
    ChatTab *tab = getFocused();
    tab->chatInput(msg);
}

void ChatWindow::doPresent()
{
    Beings & beings = beingManager->getAll();
    std::string response = "";

    for (BeingIterator bi = beings.begin(), be = beings.end();
            bi != be; ++bi)
    {
        if ((*bi)->getType() == Being::PLAYER)
        {
            if (!response.empty())
            {
                response += ", ";
            }
            response += (*bi)->getName();
        }
    }

    if (mRecorder->isRecording())
    {
        // Get the current system time
        time_t t;
        time(&t);

        // Format the time string properly
        std::stringstream timeStr;
        timeStr << "[" << ((((t / 60) / 60) % 24 < 10) ? "0" : "")
            << (int) (((t / 60) / 60) % 24)
            << ":" << (((t / 60) % 60 < 10) ? "0" : "")
            << (int) ((t / 60) % 60)
            << "] ";


        mRecorder->record(timeStr.str() + _("Present: ") + response + ".");
        localChatTab->chatLog(_("Attendance written to record log."),
                              BY_SERVER, true);
    }
    else
    {
        localChatTab->chatLog(_("Present: ") + response, BY_SERVER);
    }
}

void ChatWindow::scroll(int amount)
{
    if (!isVisible())
        return;

    ChatTab *tab = getFocused();
    if (tab) tab->scroll(amount);
}

void ChatWindow::keyPressed(gcn::KeyEvent &event)
{
    if (event.getKey().getValue() == Key::DOWN &&
            mCurHist != mHistory.end())
    {
        // Move forward through the history
        HistoryIterator prevHist = mCurHist++;

        if (mCurHist != mHistory.end())
        {
            mChatInput->setText(*mCurHist);
            mChatInput->setCaretPosition(mChatInput->getText().length());
        }
        else
        {
            mCurHist = prevHist;
        }
    }
    else if (event.getKey().getValue() == Key::UP &&
            mCurHist != mHistory.begin() && mHistory.size() > 0)
    {
        // Move backward through the history
        mCurHist--;
        mChatInput->setText(*mCurHist);
        mChatInput->setCaretPosition(mChatInput->getText().length());
    }
}

void ChatWindow::setInputText(std::string input_str)
{
     mChatInput->setText(mChatInput->getText() + input_str + " ");
     requestChatFocus();
}

void ChatWindow::addItemText(const std::string &item)
{
    std::ostringstream text;
    text << "[" << item << "] ";
    mChatInput->setText(mChatInput->getText() + text.str());
    requestChatFocus();
}

void ChatWindow::setVisible(bool isVisible)
{
    Window::setVisible(isVisible);

    /*
     * For whatever reason, if setVisible is called, the mTmpVisible effect
     * should be disabled.
     */
    mTmpVisible = false;
}

void ChatWindow::setRecordingFile(const std::string &msg)
{
    mRecorder->setRecordingFile(msg);
}

void ChatWindow::whisper(std::string nick, std::string mes, bool own)
{
    if (mes.length() == 0) return;

    std::string playerName = player_node->getName();
    std::string tempNick = nick;

    toLower(playerName);
    toLower(tempNick);

    if (!own && tempNick.compare(playerName) == 0)
        return;

    ChatTab *tab = mWhispers[tempNick];

    if (!tab)
    {
        tab = new WhisperTab(tempNick);
        mWhispers[tempNick] = tab;
    }

    if (own)
        tab->chatInput(mes);
    else
        tab->chatLog(nick, mes);
}
