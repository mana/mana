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

#include <guichan/focushandler.hpp>

#include "browserbox.h"
#include "chat.h"
#include "chatinput.h"
#include "itemlinkhandler.h"
#include "recorder.h"
#include "scrollarea.h"
#include "sdlinput.h"
#include "windowcontainer.h"

#include "widgets/layout.h"
#include "widgets/tab.h"
#include "widgets/tabbedarea.h"

#include "../beingmanager.h"
#include "../commandhandler.h"
#include "../channelmanager.h"
#include "../channel.h"
#include "../configuration.h"
#include "../game.h"
#include "../localplayer.h"

#ifdef TMWSERV_SUPPORT
#include "../net/tmwserv/chatserver/chatserver.h"
#include "../net/tmwserv/gameserver/player.h"
#else
#include "../party.h"
#include "../net/messageout.h"
#include "../net/ea/protocol.h"
#endif

#include "../resources/iteminfo.h"
#include "../resources/itemdb.h"

#include "../utils/dtor.h"
#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/stringutils.h"

#ifdef TMWSERV_SUPPORT
ChatWindow::ChatWindow():
    Window("Chat"),
#else
ChatWindow::ChatWindow(Network * network):
    Window(""), mNetwork(network),
#endif
    mTmpVisible(false)
{
    setWindowName("Chat");

    setResizable(true);
    setDefaultSize(0, windowContainer->getHeight() - 123, 600, 123);
    setMinWidth(150);
    setMinHeight(90);

    mItemLinkHandler = new ItemLinkHandler;

    mChatInput = new ChatInput;
    mChatInput->setActionEventId("chatinput");
    mChatInput->addActionListener(this);

    mChatTabs = new TabbedArea();
    createNewChannelTab("General");

    place(0, 0, mChatTabs, 5, 5).setPadding(0);
    place(0, 5, mChatInput, 5).setPadding(1);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);
    layout.setMargin(2);

    loadWindowState();

    // Add key listener to chat input to be able to respond to up/down
    mChatInput->addKeyListener(this);
    mCurHist = mHistory.end();

#ifdef EATHENA_SUPPORT
    // Read the party prefix
    std::string partyPrefix = config.getValue("PartyPrefix", "$");
    mPartyPrefix = (partyPrefix.empty() ? '$' : partyPrefix.at(0));
    mReturnToggles = config.getValue("ReturnToggles", "0") == "1";
    mRecorder = new Recorder(this);

    // If the player had @assert on in the last session, ask the server to
    // run the @assert command for the player again. Convenience for GMs.
    if (config.getValue(player_node->getName() + "GMassert", 0)) {
        std::string cmd = "@assert";
        chatSend(cmd);
    }
#endif
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

void ChatWindow::chatLog(std::string line, int own, std::string channelName,
                         bool ignoreRecord)
{
    if(channelName.empty())
#ifdef TMWSERV_SUPPORT
        channelName = getFocused();
#else
        channelName = "General";
#endif

    ChannelMap::const_iterator chan = mChannels.find(channelName);
    if (chan == mChannels.end())
        return;

    BrowserBox * const output = chan->second.browser;
    ScrollArea * const scroll = chan->second.scroll;

    // Trim whitespace
    trim(line);

    if (line.empty())
        return;

    CHATLOG tmp;
    tmp.own = own;
    tmp.nick = "";
    tmp.text = line;

    std::string::size_type pos = line.find(" : ");
    if (pos != std::string::npos)
    {
        tmp.nick = line.substr(0, pos);
        tmp.text = line.substr(pos + 3);
    }
    else
    {
        // Fix the owner of welcome message.
        if (line.substr(0, 7) == "Welcome")
        {
            own = BY_SERVER;
        }
    }

    // *implements actions in a backwards compatible way*
    if (own == BY_PLAYER &&
        tmp.text.at(0) == '*' &&
        tmp.text.at(tmp.text.length()-1) == '*')
    {
        tmp.text[0] = ' ';
        tmp.text.erase(tmp.text.length() - 1);
        own = ACT_IS;
    }

    std::string lineColor = "##C";
    switch (own)
    {
        case BY_GM:
            if (tmp.nick.empty())
            {
                tmp.nick = std::string(_("Global announcement:"));
                tmp.nick += " ";
                lineColor = "##G";
            }
            else
            {
                tmp.nick = strprintf(_("Global announcement from %s:"),
                                     tmp.nick.c_str());
                tmp.nick += " ";
                lineColor = "##1"; // Equiv. to BrowserBox::RED
            }
            break;
        case BY_PLAYER:
            tmp.nick += CAT_NORMAL;
            lineColor = "##Y";
            break;
        case BY_OTHER:
            tmp.nick += CAT_NORMAL;
            lineColor = "##C";
            break;
        case BY_SERVER:
            tmp.nick = _("Server:");
            tmp.nick += " ";
            tmp.text = line;
            lineColor = "##S";
            break;
        case BY_CHANNEL:
            tmp.nick = "";
            // TODO: Use a predefined color
            lineColor = "##2"; // Equiv. to BrowserBox::GREEN
            break;
#ifdef EATHENA_SUPPORT
        case BY_PARTY:
            tmp.nick += CAT_NORMAL;
            lineColor = "##P";
            break;
#endif
        case ACT_WHISPER:
            tmp.nick = strprintf(_("%s whispers:"), tmp.nick.c_str());
            tmp.nick += " ";
            lineColor = "##W";
            break;
        case ACT_IS:
            tmp.nick += CAT_IS;
            lineColor = "##I";
            break;
        case BY_LOGGER:
            tmp.nick = "";
            tmp.text = line;
            lineColor = "##L";
            break;
    }

    if (tmp.nick == ": ")
    {
        tmp.nick = "";
        lineColor = "##S";
    }

#ifdef EATHENA_SUPPORT
    if (tmp.nick.empty() && tmp.text.substr(0, 17) == "Visible GM status")
    {
        player_node->setGM();
    }
#endif

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
    mRecorder->record(line.substr(3));
}

const std::string &ChatWindow::getFocused() const
{
    return mChatTabs->getSelectedTab()->getCaption();
}

void ChatWindow::clearTab(const std::string &tab)
{
    ChannelMap::const_iterator chan = mChannels.find(tab);
    if (chan != mChannels.end())
    chan->second.browser->clearRows();
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
            chatSend(message);

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

void ChatWindow::requestChatFocus()
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

bool ChatWindow::isInputFocused()
{
    return mChatInput->isFocused();
}

void ChatWindow::removeChannel(short channelId)
{
    removeChannel(channelManager->findById(channelId));
}

void ChatWindow::removeChannel(const std::string &channelName)
{
    removeChannel(channelManager->findByName(channelName));
}

void ChatWindow::removeChannel(Channel *channel)
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

void ChatWindow::createNewChannelTab(const std::string &channelName)
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

void ChatWindow::sendToChannel(short channelId,
                               const std::string &user,
                               const std::string &msg)
{
    Channel *channel = channelManager->findById(channelId);
    if (channel)
    {
        std::string channelName = channel->getName();
        chatLog(user + ": " + msg, user == player_node->getName() ? BY_PLAYER : BY_OTHER, channelName);
        mChatTabs->getTab(channelName)->setHighlighted(true);
    }
}

void ChatWindow::chatSend(std::string &msg)
{
    trim(msg);

    if (msg.empty()) return;

#ifdef EATHENA_SUPPORT
    // Send party message
    if (msg.at(0) == mPartyPrefix)
    {
        msg.erase(0, 1);
        std::size_t length = msg.length() + 1;

        if (length == 0)
        {
            chatLog(_("Trying to send a blank party message."), BY_SERVER);
            return;
        }
        MessageOut outMsg(mNetwork);

        outMsg.writeInt16(CMSG_PARTY_MESSAGE);
        outMsg.writeInt16(length + 4);
        outMsg.writeString(msg, length);
        return;
    }
#endif

    // Check for item link
    std::string::size_type start = msg.find('[');
    while (start != std::string::npos && msg[start+1] != '@')
    {
        std::string::size_type end = msg.find(']', start);
        if (start+1 != end && end != std::string::npos)
        {
            // Catch multiple embeds and ignore them
            // so it doesn't crash the client.
            while ((msg.find('[', start + 1) != std::string::npos) &&
                   (msg.find('[', start + 1) < end))
            {
                start = msg.find('[', start + 1);
            }

            std::string temp = msg.substr(start + 1, end - start - 1);

            toLower(trim(temp));

            const ItemInfo itemInfo = ItemDB::get(temp);
            if (itemInfo.getName() != _("Unknown item"))
            {
                msg.insert(end, "@@");
                msg.insert(start+1, "|");
                msg.insert(start+1, toString(itemInfo.getId()));
                msg.insert(start+1, "@@");
            }
        }
        start =  msg.find('[', start + 1);
    }


    // Prepare ordinary message
    if (msg[0] != '/')
    {
#ifdef TMWSERV_SUPPORT
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
#else
        msg = player_node->getName() + " : " + msg;

        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_CHAT_MESSAGE);
        // Added + 1 in order to let eAthena parse admin commands correctly
        outMsg.writeInt16(msg.length() + 4 + 1);
        outMsg.writeString(msg, msg.length() + 1);
        return;
#endif
    }
    else
    {
        commandHandler->handleCommand(std::string(msg, 1));
    }
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
        chatLog(_("Attendance written to record log."), BY_SERVER, std::string(), true);
    }
    else
    {
        chatLog(_("Present: ") + response, BY_SERVER);
    }
}

void ChatWindow::scroll(int amount)
{
    if (!isVisible())
        return;

    ChannelMap::const_iterator chan = mChannels.find(getFocused());
    if (chan == mChannels.end())
        return;

    BrowserBox *browser = chan->second.browser;
    ScrollArea *scroll = chan->second.scroll;

    int range = scroll->getHeight() / 8 * amount;
    gcn::Rectangle scr;
    scr.y = scroll->getVerticalScrollAmount() + range;
    scr.height = abs(range);
    browser->showPart(scr);
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

bool ChatWindow::tabExists(const std::string &tabName)
{
    Tab *tab = mChatTabs->getTab(tabName);
    return tab != 0;
}

void ChatWindow::setRecordingFile(const std::string &msg)
{
    mRecorder->setRecordingFile(msg);
}
