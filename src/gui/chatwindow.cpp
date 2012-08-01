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

#include "chatwindow.h"

#include "actorspritemanager.h"
#include "channel.h"
#include "channelmanager.h"
#include "configuration.h"
#include "localplayer.h"
#include "party.h"
#include "playerrelations.h"

#include "gui/recorder.h"
#include "gui/setup.h"
#include "gui/sdlinput.h"

#include "gui/widgets/channeltab.h"
#include "gui/widgets/chattab.h"
#include "gui/widgets/itemlinkhandler.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/tabbedarea.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/whispertab.h"

#include "net/chathandler.h"
#include "net/net.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/focushandler.hpp>
#include <guichan/focuslistener.hpp>

#include <sstream>

/**
 * The chat input hides when it loses focus. It is also invisible by default.
 */
class ChatInput : public TextField, public gcn::FocusListener
{
    public:
        ChatInput():
            TextField("", false)
        {
            setVisible(false);
            addFocusListener(this);
        }

        /**
         * Called if the chat input loses focus. It will set itself to
         * invisible as result.
         */
        void focusLost(const gcn::Event &event)
        {
            setVisible(false);
        }
};

class ChatAutoComplete : public AutoCompleteLister
{
    void getAutoCompleteList(std::vector<std::string> &list) const
    {
        ChatTab *tab = static_cast<ChatTab*>(chatWindow->mChatTabs
                                             ->getSelectedTab());

        return tab->getAutoCompleteList(list);
    }
};

ChatWindow::ChatWindow():
    Window(_("Chat")),
    mHistory(new TextHistory()),
    mAutoComplete(new ChatAutoComplete),
    mTmpVisible(false)
{
    listen(Event::ChatChannel);
    listen(Event::NoticesChannel);

    setWindowName("Chat");

    setupWindow->registerWindowForReset(this);

    // no title presented, title bar is padding so window can be moved.
    setTitleBarHeight(getPadding() + 4);
    setShowTitle(false);
    setResizable(true);
    setDefaultVisible(true);
    setSaveVisible(true);
    setDefaultSize(600, 123, ImageRect::LOWER_LEFT);
    setMinWidth(150);
    setMinHeight(90);

    mItemLinkHandler = new ItemLinkHandler;

    mChatInput = new ChatInput;
    mChatInput->setActionEventId("chatinput");
    mChatInput->addActionListener(this);

    mChatTabs = new TabbedArea;

    getLayout().setPadding(3);
    place(0, 0, mChatTabs, 3, 3);
    place(0, 3, mChatInput, 3);

    loadWindowState();

    mChatInput->setHistory(mHistory);
    mChatInput->setAutoComplete(mAutoComplete);

    mReturnToggles = config.getBoolValue("ReturnToggles");

    mRecorder = new Recorder(this);
}

ChatWindow::~ChatWindow()
{
    config.setValue("ReturnToggles", mReturnToggles);
    delete mRecorder;
    removeAllWhispers();
    delete mItemLinkHandler;
    delete mHistory;
    delete mAutoComplete;
}

void ChatWindow::resetToDefaultSize()
{
    mRecorder->resetToDefaultSize();
    Window::resetToDefaultSize();
}

ChatTab *ChatWindow::getFocused() const
{
    return static_cast<ChatTab*>(mChatTabs->getSelectedTab());
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

void ChatWindow::prevTab()
{
    int tab = mChatTabs->getSelectedTabIndex();

    if (tab == 0)
        tab = mChatTabs->getNumberOfTabs();
    tab--;

    mChatTabs->setSelectedTab(tab);
}

void ChatWindow::nextTab()
{
    int tab = mChatTabs->getSelectedTabIndex();

    tab++;
    if (tab == mChatTabs->getNumberOfTabs())
        tab = 0;

    mChatTabs->setSelectedTab(tab);
}

void ChatWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "chatinput")
    {
        std::string message = mChatInput->getText();

        if (!message.empty())
        {
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

bool ChatWindow::isInputFocused() const
{
    return mChatInput->isFocused();
}

void ChatWindow::removeTab(ChatTab *tab)
{
    mChatTabs->removeTab(tab);
}

void ChatWindow::addTab(ChatTab *tab)
{
    // Make sure we don't end up with duplicates in the gui
    // TODO

    mChatTabs->addTab(tab, tab->mScrollArea);

    // Update UI
    logic();
}

void ChatWindow::removeWhisper(const std::string &nick)
{
    std::string tempNick = nick;
    toLower(tempNick);
    mWhispers.erase(tempNick);
}

void ChatWindow::removeAllWhispers()
{
    TabMap::iterator iter;
    std::list<ChatTab*> tabs;

    for (iter = mWhispers.begin(); iter != mWhispers.end(); ++iter)
    {
        tabs.push_back(iter->second);
    }

    for (std::list<ChatTab*>::iterator it = tabs.begin();
         it != tabs.end(); ++it)
    {
        delete *it;
    }

    mWhispers.clear();
}

void ChatWindow::chatInput(const std::string &msg)
{
    ChatTab *tab = getFocused();
    tab->chatInput(msg);
}

void ChatWindow::doPresent()
{
    const ActorSprites &actors = actorSpriteManager->getAll();
    std::string response = "";
    int playercount = 0;

    for (ActorSpritesConstIterator it = actors.begin(), it_end = actors.end();
         it != it_end; it++)
    {
        if ((*it)->getType() == ActorSprite::PLAYER)
        {
            if (!response.empty())
            {
                response += ", ";
            }
            response += static_cast<Being*>(*it)->getName();
            ++playercount;
        }
    }

    std::string log = strprintf(_("Present: %s; %d players are present."),
                                response.c_str(), playercount);

    if (mRecorder->isRecording())
    {
        // Get the current system time
        time_t t;
        time(&t);

        // Format the time string properly
        std::ostringstream timeStr;
        timeStr << "[" << ((((t / 60) / 60) % 24 < 10) ? "0" : "")
            << (int) (((t / 60) / 60) % 24)
            << ":" << (((t / 60) % 60 < 10) ? "0" : "")
            << (int) ((t / 60) % 60)
            << "] ";

        mRecorder->record(timeStr.str() + log);
        getFocused()->chatLog(_("Attendance written to record log."),
                              BY_SERVER, true);
    }
    else
    {
        getFocused()->chatLog(log, BY_SERVER);
    }
}

void ChatWindow::scroll(int amount)
{
    if (!isVisible())
        return;

    ChatTab *tab = getFocused();
    if (tab)
        tab->scroll(amount);
}

void ChatWindow::mousePressed(gcn::MouseEvent &event)
{
    Window::mousePressed(event);

    if (event.isConsumed())
        return;

    mMoved = event.getY() <= getFocused()->getHeight();
    mDragOffsetX = event.getX();
    mDragOffsetY = event.getY();

}

void ChatWindow::mouseDragged(gcn::MouseEvent &event)
{
    Window::mouseDragged(event);

    if (event.isConsumed())
        return;

    if (isMovable() && mMoved)
    {
        int newX = std::max(0, getX() + event.getX() - mDragOffsetX);
        int newY = std::max(0, getY() + event.getY() - mDragOffsetY);
        newX = std::min(graphics->getWidth() - getWidth(), newX);
        newY = std::min(graphics->getHeight() - getHeight(), newY);
        setPosition(newX, newY);
    }
}

void ChatWindow::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::NoticesChannel)
    {
        if (event.getType() == Event::ServerNotice)
            localChatTab->chatLog(event.getString("message"), BY_SERVER);
    }
    else if (channel == Event::ChatChannel)
    {
        if (event.getType() == Event::Whisper)
        {
            whisper(event.getString("nick"), event.getString("message"));
        }
        else if (event.getType() == Event::WhisperError)
        {
            whisper(event.getString("nick"),
                    event.getString("error"), BY_SERVER);
        }
        else if (event.getType() == Event::Player)
        {
            localChatTab->chatLog(event.getString("message"), BY_PLAYER);
        }
        else if (event.getType() == Event::Announcement)
        {
            // Show on local tab
            localChatTab->chatLog(event.getString("message"), BY_GM);
            // Show on selected tab if it is not the global one
            ChatTab *selected = getFocused();
            if (selected && selected != localChatTab)
                selected->chatLog(event.getString("message"), BY_GM);
        }
        else if (event.getType() == Event::Being)
        {
            if (event.getInt("permissions") & PlayerRelation::SPEECH_LOG)
                localChatTab->chatLog(event.getString("message"), BY_OTHER);
        }
    }
}

void ChatWindow::addInputText(const std::string &text)
{
    const int caretPos = mChatInput->getCaretPosition();
    const std::string inputText = mChatInput->getText();

    std::ostringstream ss;
    ss << inputText.substr(0, caretPos) << text << " ";
    ss << inputText.substr(caretPos);

    mChatInput->setText(ss.str());
    mChatInput->setCaretPosition(caretPos + text.length() + 1);
    requestChatFocus();
}

void ChatWindow::addItemText(const std::string &item)
{
    std::ostringstream text;
    text << "[" << item << "]";
    addInputText(text.str());
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

void ChatWindow::whisper(const std::string &nick,
                         const std::string &mes, Own own)
{
    if (mes.empty())
        return;

    std::string playerName = local_player->getName();
    std::string tempNick = nick;

    toLower(playerName);
    toLower(tempNick);

    if (tempNick.compare(playerName) == 0)
        return;

    ChatTab *tab = 0;
    TabMap::const_iterator i = mWhispers.find(tempNick);

    if (i != mWhispers.end())
        tab = i->second;
    else if (config.getBoolValue("whispertab"))
        tab = addWhisperTab(nick);

    if (tab)
    {
        if (own == BY_PLAYER)
        {
            tab->chatInput(mes);
        }
        else if (own == BY_SERVER)
        {
            tab->chatLog(mes);
        }
        else
        {
            tab->chatLog(nick, mes);
            local_player->afkRespond(tab, nick);
        }
    }
    else
    {
        if (own == BY_PLAYER)
        {
            Net::getChatHandler()->privateMessage(nick, mes);

            localChatTab->chatLog(strprintf(_("Whispering to %s: %s"),
                            nick.c_str(), mes.c_str()), BY_PLAYER);
        }
        else
        {
            localChatTab->chatLog(nick + " : " + mes, ACT_WHISPER, false);
        }
    }
}

ChatTab *ChatWindow::addWhisperTab(const std::string &nick, bool switchTo)
{
    std::string playerName = local_player->getName();
    std::string tempNick = nick;

    toLower(playerName);
    toLower(tempNick);

    if (mWhispers.find(tempNick) != mWhispers.end()
        || tempNick.compare(playerName) == 0)
        return NULL;

    ChatTab *ret = new WhisperTab(nick);
    mWhispers[tempNick] = ret;

    if (switchTo)
        mChatTabs->setSelectedTab(ret);

    return ret;
}
