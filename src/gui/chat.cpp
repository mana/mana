/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include <algorithm>
#include <sstream>

#include <guichan/focushandler.hpp>

#include "chat.h"

#include "browserbox.h"
#include "chatinput.h"
#include "scrollarea.h"
#include "sdlinput.h"
#include "windowcontainer.h"

#include "widgets/tab.h"
#include "widgets/tabbedarea.h"

#include "../commandhandler.h"
#include "../channelmanager.h"
#include "../channel.h"
#include "../configuration.h"
#include "../game.h"
#include "../localplayer.h"

#include "../net/chatserver/chatserver.h"
#include "../net/gameserver/player.h"

#include "../utils/dtor.h"
#include "../utils/trim.h"

ChatWindow::ChatWindow():
    Window("Chat"),
    mTmpVisible(false)
{
    setResizable(true);
    setDefaultSize(0, (windowContainer->getHeight() - 105), 400, 100);
    setOpaque(false);

    mChatInput = new ChatInput();
    mChatInput->setActionEventId("chatinput");
    mChatInput->addActionListener(this);

    BrowserBox *textOutput = new BrowserBox(BrowserBox::AUTO_WRAP);
    textOutput->setOpaque(false);
    textOutput->disableLinksAndUserColors();
    textOutput->setMaxRow((int) config.getValue("ChatLogLength", 0));

    ScrollArea *scrollArea = new ScrollArea(textOutput);
    scrollArea->setPosition(
            scrollArea->getFrameSize(), scrollArea->getFrameSize());
    scrollArea->setScrollPolicy(
            gcn::ScrollArea::SHOW_NEVER, gcn::ScrollArea::SHOW_ALWAYS);
    scrollArea->setScrollAmount(0, 1);
    scrollArea->setOpaque(false);

    mChatTabs = new TabbedArea();
    mChatTabs->addTab("General", scrollArea);
    mChatTabs->setPosition(mChatTabs->getFrameSize(),
                           mChatTabs->getFrameSize());

    mChannels.insert(
            std::make_pair("General", ChatArea(textOutput, scrollArea)));

    add(mChatTabs);
    add(mChatInput);

    // Add key listener to chat input to be able to respond to up/down
    mChatInput->addKeyListener(this);
    mCurHist = mHistory.end();

    loadWindowState("Chat");
}

ChatWindow::~ChatWindow()
{
    delete mChatInput;
    delete mChatTabs;
}

const std::string& ChatWindow::getFocused() const
{
    return mChatTabs->getSelectedTab()->getCaption();
}

void ChatWindow::clearTab(const std::string &tab)
{
    ChannelMap::const_iterator chan = mChannels.find(tab);
    if (chan != mChannels.end())
    chan->second.browser->clearRows();
}

void ChatWindow::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    const gcn::Rectangle area = getChildrenArea();

    mChatInput->setPosition(mChatInput->getFrameSize(),
                            area.height - mChatInput->getHeight() -
                                mChatInput->getFrameSize());
    mChatInput->setWidth(area.width - 2 * mChatInput->getFrameSize());

    mChatTabs->setWidth(area.width - 2 * mChatTabs->getFrameSize());
    mChatTabs->setHeight(area.height - 2 * mChatTabs->getFrameSize());

    const std::string &channelName = getFocused();
    ChannelMap::const_iterator chan = mChannels.find(channelName);
    if (chan != mChannels.end()) {
        ScrollArea *scroll = chan->second.scroll;
        scroll->setWidth(area.width - 2 * scroll->getFrameSize());
        scroll->setHeight(area.height - 2 * scroll->getFrameSize() -
                mChatInput->getHeight() - 5);
        scroll->logic();
    }
}

void ChatWindow::logic()
{
    Window::logic();

    const gcn::Rectangle area = getChildrenArea();

    const std::string &channelName = getFocused();
    ChannelMap::const_iterator chan = mChannels.find(channelName);
    if (chan != mChannels.end()) {
        ScrollArea *scroll = chan->second.scroll;
        scroll->setWidth(area.width - 2 * scroll->getFrameSize());
        scroll->setHeight(area.height - 2 * scroll->getFrameSize() -
                mChatInput->getHeight() - 5);
        scroll->logic();
    }
}

void
ChatWindow::chatLog(std::string line, int own, std::string channelName)
{
    if(channelName.empty())
        channelName = getFocused();

    ChannelMap::const_iterator chan = mChannels.find(channelName);
    if (chan == mChannels.end())
        return;

    BrowserBox * const output = chan->second.browser;
    ScrollArea * const scroll = chan->second.scroll;

    // Trim whitespace
    trim(line);

    CHATLOG tmp;
    tmp.own  = own;
    tmp.nick = "";
    tmp.text = line;

    std::string::size_type pos = line.find(" : ");
    if (pos != std::string::npos) {
        tmp.nick = line.substr(0, pos);
        tmp.text = line.substr(pos + 3);
    }

    std::string lineColor = "##0"; // Equiv. to BrowserBox::BLACK
    switch (own) {
        case BY_GM:
            tmp.nick += "Global announcement: ";
            lineColor = "##1"; // Equiv. to BrowserBox::RED
            break;
        case BY_PLAYER:
            tmp.nick += ": ";
            lineColor = "##3"; // Equiv. to BrowserBox::BLUE
            break;
        case BY_OTHER:
            tmp.nick += ": ";
            lineColor = "##0"; // Equiv. to BrowserBox::BLACK
            break;
        case BY_SERVER:
            tmp.nick = "Server: ";
            tmp.text = line;
            lineColor = "##7"; // Equiv. to BrowserBox::PINK
            break;
        case BY_CHANNEL:
            tmp.nick = "";
            lineColor = "##2"; // Equiv. to BrowserBox::GREEN
            break;
        case BY_LOGGER:
            tmp.nick = "";
            tmp.text = line;
            lineColor = "##8"; // Equiv. to BrowserBox::GREY
            break;
    }

    // Get the current system time
    time_t t;
    time(&t);

    // Format the time string properly
    std::stringstream timeStr;
    timeStr << "["
            << ((((t / 60) / 60) % 24 < 10) ? "0" : "")
            << (int)(((t / 60) / 60) % 24)
            << ":"
            << (((t / 60) % 60 < 10) ? "0" : "")
            << (int)((t / 60) % 60)
            << "] ";

    line = lineColor + timeStr.str() + tmp.nick + tmp.text;

    // We look if the Vertical Scroll Bar is set at the max before
    // adding a row, otherwise the max will always be a row higher
    // at comparison.
    if (scroll->getVerticalScrollAmount() >= scroll->getVerticalMaxScroll())
    {
        output->addRow(line);
        scroll->setVerticalScrollAmount(scroll->getVerticalMaxScroll());
    }
    else
    {
        output->addRow(line);
    }

    scroll->logic();
}

void
ChatWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "chatinput")
    {
        std::string message = mChatInput->getText();

        if (!message.empty()) {
            // If message different from previous, put it in the history
            if (mHistory.empty() || message != mHistory.back()) {
                mHistory.push_back(message);
            }

            // Reset history iterator
            mCurHist = mHistory.end();

            // Send the message to the server
            chatSend(message);

            // Clear the text from the chat input
            mChatInput->setText("");
        }

        // Remove focus and hide input
        mFocusHandler->focusNone();

        // If the chatWindow is shown up because you want to send a message
        // It should hide now
        if (mTmpVisible) {
            setVisible(false);
        }
    }
}

void
ChatWindow::requestChatFocus()
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

    // Give focus to the chat input
    mChatInput->setVisible(true);
    mChatInput->requestFocus();
}

bool
ChatWindow::isInputFocused()
{
    return mChatInput->isFocused();
}

void ChatWindow::chatSend(std::string const &msg)
{
    if (msg.empty()) return;

    // Prepare ordinary message
    if (msg[0] != '/')
    {
        if (getFocused() == "General")
        {
            Net::GameServer::Player::say(msg);
        }
        else
        {
            Channel *channel = channelManager->findByName(getFocused());
            if (channel)
            {
                Net::ChatServer::chat(channel->getId(), msg);
            }
        }
    }
    else
    {
        commandHandler->handleCommand(std::string(msg, 1));
    }
}

void
ChatWindow::removeChannel(short channelId)
{
    removeChannel(channelManager->findById(channelId));
}

void
ChatWindow::removeChannel(const std::string &channelName)
{
    removeChannel(channelManager->findByName(channelName));
}

void
ChatWindow::removeChannel(Channel *channel)
{
    if (channel)
    {
        Tab *tab = mChatTabs->getTab(channel->getName());
        if (!tab)
            return;
        clearTab(channel->getName());
        mChatTabs->removeTab(tab);
        mChannels.erase(channel->getName());
        channelManager->removeChannel(channel);

        logic();
    }
}

void
ChatWindow::createNewChannelTab(const std::string &channelName)
{
    // Create new channel
    BrowserBox *textOutput = new BrowserBox(BrowserBox::AUTO_WRAP);
    textOutput->setOpaque(false);
    textOutput->disableLinksAndUserColors();
    textOutput->setMaxRow((int) config.getValue("ChatLogLength", 0));
    ScrollArea *scrollArea = new ScrollArea(textOutput);
    scrollArea->setPosition(scrollArea->getFrameSize(), scrollArea->getFrameSize());
    scrollArea->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER, gcn::ScrollArea::SHOW_ALWAYS);
    scrollArea->setOpaque(false);
    scrollArea->setWidth(getChildrenArea().width - 2 * scrollArea->getFrameSize());
    scrollArea->setHeight(getChildrenArea().height - 2 * scrollArea->getFrameSize() -
                mChatInput->getHeight() - 5);
    scrollArea->logic();
    textOutput->setWidth(scrollArea->getChildrenArea().width);
    textOutput->setHeight(scrollArea->getChildrenArea().height);

    // Add channel to the tabbed area
    mChatTabs->addTab(channelName, scrollArea);
    mChannels.insert(
            std::make_pair(channelName, ChatArea(textOutput, scrollArea)));

    // Update UI
    logic();
}

void
ChatWindow::sendToChannel(short channelId, const std::string &user, const std::string &msg)
{
    Channel *channel = channelManager->findById(channelId);
    if (channel)
    {
        std::string channelName = channel->getName();
        chatLog(user + ": " + msg, user == player_node->getName() ? BY_PLAYER : BY_OTHER, channelName);
    }
}

void
ChatWindow::keyPressed(gcn::KeyEvent &event)
{
    if (event.getKey().getValue() == Key::DOWN &&
            mCurHist != mHistory.end())
    {
        // Move forward through the history
        HistoryIterator prevHist = mCurHist++;
        if (mCurHist != mHistory.end()) {
            mChatInput->setText(*mCurHist);
            mChatInput->setCaretPosition(mChatInput->getText().length());
        }
        else {
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

void
ChatWindow::setInputText(std::string input_str)
{
     mChatInput->setText(input_str + " ");
     requestChatFocus();
}

void
ChatWindow::setVisible(bool isVisible)
{
     Window::setVisible(isVisible);

     /*
      * For whatever reason, if setVisible is called, the mTmpVisible effect
      * should be disabled.
      */
     mTmpVisible = false;
}

bool
ChatWindow::tabExists(const std::string &tabName)
{
    Tab *tab = mChatTabs->getTab(tabName);
    if (tab)
        return true;
    return false;
}
