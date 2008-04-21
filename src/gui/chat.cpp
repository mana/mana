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

#include "widgets/tabbedarea.h"

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
    Window(""),
    mTmpVisible(false)
{
    setResizable(true);
    setDefaultSize(0, (windowContainer->getHeight() - 105), 400, 100);
    setTitleBarHeight(5);
    loadWindowState("Chat");

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
    scrollArea->setOpaque(false);

    mChatTabs = new TabbedArea();
    mChatTabs->addTab("General", scrollArea);
    mChatTabs->setPosition(mChatTabs->getFrameSize(), mChatTabs->getFrameSize());

    mChannels.insert(
            std::make_pair("General", ChatArea(textOutput, scrollArea)));

    add(mChatTabs);
    add(mChatInput);

    // Add key listener to chat input to be able to respond to up/down
    mChatInput->addKeyListener(this);
    mCurHist = mHistory.end();
}

ChatWindow::~ChatWindow()
{
    delete mChatInput;
    delete mChatTabs;
}

void
ChatWindow::logic()
{
    // todo: only do this when the size changes (updateWidgets?)
    const gcn::Rectangle area = getChildrenArea();

    mChatInput->setPosition(mChatInput->getFrameSize(),
                            area.height - mChatInput->getHeight() -
                                mChatInput->getFrameSize());
    mChatInput->setWidth(area.width - 2 * mChatInput->getFrameSize());

    mChatTabs->setWidth(area.width - 2 * mChatTabs->getFrameSize());
    mChatTabs->setHeight(area.height - 2 * mChatTabs->getFrameSize());

    const std::string &channelName = mChatTabs->getSelectedTab()->getCaption();
    ChannelMap::const_iterator chan = mChannels.find(channelName);
    if (chan != mChannels.end()) {
        ScrollArea *scroll = chan->second.scroll;
        scroll->setWidth(area.width - 2 * scroll->getFrameSize());
        scroll->setHeight(area.height - 2 * scroll->getFrameSize() -
                mChatInput->getHeight() - 5);
        scroll->logic();
    }

    Window::logic();
}

void
ChatWindow::chatLog(std::string line, int own, const std::string &channelName)
{
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
            lineColor = "##5"; // Equiv. to BrowserBox::YELLOW
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
    if (scroll->getVerticalScrollAmount() == scroll->getVerticalMaxScroll())
    {
        output->addRow(line);
        scroll->setVerticalScrollAmount(scroll->getVerticalMaxScroll());
    }
    else
    {
        output->addRow(line);
    }
}

#if 0
void
ChatWindow::chatLog(CHATSKILL act)
{
    chatLog(const_msg(act), BY_SERVER);
}
#endif

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
            gcn::Tab *tab = mChatTabs->getSelectedTab();
            chatSend(player_node->getName(), message, tab->getCaption());

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
ChatWindow::isFocused()
{
    return mChatInput->isFocused();
}

void ChatWindow::chatSend(std::string const &nick, std::string const &msg,
                          std::string const &channelName)
{
    /* Some messages are managed client side, while others
     * require server handling by proper packet. Probably
     * those if elses should be replaced by protocol calls */

    if (msg.empty()) return;

    // Prepare ordinary message
    if (msg[0] != '/') {
        gcn::Tab *tab = mChatTabs->getSelectedTab();
        if (tab->getCaption() == "General")
        {
            Net::GameServer::Player::say(msg);
        }
        else
        {
            Channel *channel = channelManager->findByName(channelName);
            if (channel)
            {
                int channelId = channel->getId();
                Net::ChatServer::chat(channelId, msg);
            }
        }
        return;
    }

    std::string::size_type pos = msg.find(' ', 1);
    std::string command(msg, 1, pos == std::string::npos ? pos : pos - 1);
    std::string arg(msg, pos == std::string::npos ? msg.size() : pos + 1);

    if (command == "announce")
    {
        Net::ChatServer::announce(arg);
    }
    else if (command == "help")
    {
        chatLog("-- Help --", BY_SERVER, channelName);
        chatLog("/help > Display this help.", BY_SERVER, channelName);
        chatLog("/announce > Global announcement (GM only)", BY_SERVER, channelName);
        chatLog("/where > Display map name", BY_SERVER, channelName);
        chatLog("/who > Display number of online users", BY_SERVER, channelName);
        chatLog("/msg > Send a private message to a user", BY_SERVER, channelName);
        chatLog("/list > Display all public channels", BY_SERVER, channelName);
        chatLog("/register > Register a new channel", BY_SERVER, channelName);
        chatLog("/join > Join an already registered channel", BY_SERVER, channelName);
        chatLog("/quit > Leave a channel", BY_SERVER, channelName);
        chatLog("/admin > Send a command to the server (GM only)", BY_SERVER, channelName);
        chatLog("/clear > Clears this window", BY_SERVER);
    }
    else if (command == "where")
    {
        chatLog(map_path, BY_SERVER);
    }
    else if (command == "who")
    {
        // XXX Convert for new server
        /*
        MessageOut outMsg(0x00c1);
        */
    }
    else if (command == "msg")
    {
        std::string::size_type pos = arg.find(' ', 1);
        std::string recipient(arg, 0, pos-1);
        std::string text(arg, pos+1);
        chatLog("* " + text, BY_SERVER);
        Net::ChatServer::privMsg(recipient, text);
    }
    else if (command == "register")
    {
        // TODO: Parse the announcement and password
        chatLog("Requesting to register channel " + arg, BY_SERVER);
        Net::ChatServer::registerChannel(arg, "", "");
    }
    else if (command == "join")
    {
        //TODO: have passwords too
        chatLog("Requesting to join channel " + arg, BY_SERVER);
        enterChannel(arg, "None");
    }
    else if (command == "list")
    {
        Net::ChatServer::getChannelList();
    }
    else if (command == "quit")
    {
        if (Channel *channel = channelManager->findByName(channelName))
        {
            Net::ChatServer::quitChannel(channel->getId());
        }
        else
        {
            chatLog("Unable to quit this channel", BY_SERVER);
        }
    }
    else if (command == "admin")
    {
        Net::GameServer::Player::say("/" + arg);
    }
    else if (command == "clear")
    {
        ChannelMap::const_iterator chan = mChannels.find(channelName);
        if (chan != mChannels.end())
            chan->second.browser->clearRows();
    }
    else
    {
        chatLog("Unknown command", BY_SERVER, channelName);
    }
}

#if 0
std::string
ChatWindow::const_msg(CHATSKILL act)
{
    std::string msg;
    if (act.success == SKILL_FAILED && act.skill == SKILL_BASIC) {
        switch (act.bskill) {
            case BSKILL_TRADE :
                msg = "Trade failed!";
                break;
            case BSKILL_EMOTE :
                msg = "Emote failed!";
                break;
            case BSKILL_SIT :
                msg = "Sit failed!";
                break;
            case BSKILL_CREATECHAT :
                msg = "Chat creating failed!";
                break;
            case BSKILL_JOINPARTY :
                msg = "Could not join party!";
                break;
            case BSKILL_SHOUT :
                msg = "Cannot shout!";
                break;
        }

        switch (act.reason) {
            case RFAIL_SKILLDEP :
                msg += " You have not yet reached a high enough lvl!";
                break;
            case RFAIL_INSUFHP :
                msg += " Insufficient HP!";
                break;
            case RFAIL_INSUFSP :
                msg += " Insufficient SP!";
                break;
            case RFAIL_NOMEMO :
                msg += " You have no memos!";
                break;
            case RFAIL_SKILLDELAY :
                msg += " You cannot do that right now!";
                break;
            case RFAIL_ZENY :
                msg += " Seems you need more Zeny... ;-)";
                break;
            case RFAIL_WEAPON :
                msg += " You cannot use this skill with that kind of weapon!";
                break;
            case RFAIL_REDGEM :
                msg += " You need another red gem!";
                break;
            case RFAIL_BLUEGEM :
                msg += " You need another blue gem!";
                break;
            case RFAIL_OVERWEIGHT :
                msg += " You're carrying to much to do this!";
                break;
            default :
                msg += " Huh? What's that?";
                break;
        }
    } else {
        switch(act.skill) {
            case SKILL_WARP :
                msg = "Warp failed...";
                break;
            case SKILL_STEAL :
                msg = "Could not steal anything...";
                break;
            case SKILL_ENVENOM :
                msg = "Poison had no effect...";
                break;
        }
    }

    return msg;
}
#endif

void
ChatWindow::addChannel(short channelId, const std::string &channelName)
{
    Channel *channel = new Channel(channelId);
    channel->setName(channelName);
    channelManager->addChannel(channel);
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
        gcn::Tab *tab = mChatTabs->getTab(channel->getName());
        if (!tab)
            return;
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
    ScrollArea *scrollArea = new ScrollArea(textOutput);
    scrollArea->setPosition(scrollArea->getFrameSize(), scrollArea->getFrameSize());
    scrollArea->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER, gcn::ScrollArea::SHOW_ALWAYS);
    scrollArea->setOpaque(false);

    // Add channel to the tabbed area
    mChatTabs->addTab(channelName, scrollArea);
    mChannels.insert(
            std::make_pair(channelName, ChatArea(textOutput, scrollArea)));

    // Ask for channel users
    Net::ChatServer::getUserList(channelName);

    // Update UI
    logic();
}

void
ChatWindow::enterChannel(const std::string &channel, const std::string &password)
{
    Net::ChatServer::enterChannel(channel, password);
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
    gcn::Tab *tab = mChatTabs->getTab(tabName);
    if (tab)
        return true;
    return false;
}
