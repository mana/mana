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

#ifndef _TMW_CHAT_H
#define _TMW_CHAT_H

#include <list>
#include <string>
#include <map>

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include "window.h"

#include "../guichanfwd.h"

class BrowserBox;
class ScrollArea;
class TabbedArea;
class Channel;

enum
{
    BY_GM     = 0,
    BY_PLAYER = 1,
    BY_OTHER  = 2,
    BY_SERVER = 3,
    BY_LOGGER
};

/**
 * The chat window.
 *
 * \ingroup Interface
 */
class ChatWindow : public Window,
                   public gcn::ActionListener,
                   public gcn::KeyListener
{
    public:
        /**
         * Constructor.
         */
        ChatWindow();

        /**
         *
         */
        ~ChatWindow();

        /**
         * Called when the widget changes size. Used for adapting the size of
         * the tabbed area.
         */
        void widgetResized(const gcn::Event &event);

        /**
         * Adds a line of text to our message list. Parameters:
         *
         * @param line Text message.
         * @parem own  Type of message (usually the owner-type).
         */
        void chatLog(std::string line, int own, const std::string &channelName = "General");

        /**
         * Performs action.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Request focus for typing chat message.
         */
        void requestChatFocus();

        /**
         * Checks whether ChatWindow is Focused or not.
         */
        bool isFocused();

        /**
         * Determines whether to send a command or an ordinary message, then
         * contructs packets & sends them.
         *
         * @param nick The character's name to display in front.
         * @param msg  The message text which is to be send.
         *
         * NOTE:
         * The nickname is required by the server, if not specified
         * the message may not be sent unless a command was intended
         * which requires another packet to be constructed! you can
         * achieve this by putting a slash ("/") infront of the
         * message followed by the command name and the message.
         * of course all slash-commands need implemented handler-
         * routines. ;-)
         * remember, a line starting with "@" is not a command that needs
         * to be parsed rather is sent using the normal chat-packet.
         *
         * EXAMPLE:
         * // for an global announcement   /- command
         * chatlog.chat_send("", "/announce Hello to all logged in users!");
         * // for simple message by a user /- message
         * chatlog.chat_send("Zaeiru", "Hello to all users on the screen!");
         */
        void chatSend(std::string const &nick, std::string const &msg,
                      std::string const &channelName);

        /** Called to remove the channel from the channel manager */
        void
        removeChannel(short channelId);

        void
        removeChannel(const std::string &channelName);

        void
        removeChannel(Channel *channel);

        /** Called to create a new channel tab */
        void
        createNewChannelTab(const std::string &channelName);

        /** Called to join channel */
        void
        enterChannel(const std::string &channel, const std::string &password = "None");

        /** Called to output text to a specific channel */
        void
        sendToChannel(short channel, const std::string &user, const std::string &msg);

        /** Called when key is pressed */
        void
        keyPressed(gcn::KeyEvent &event);

        /** Called to set current text */
        void
        setInputText(std::string input_str);

        /** Override to reset mTmpVisible */
        void
        setVisible(bool visible);

        /** Check if tab with that name already exists */
        bool
        tabExists(const std::string &tabName);

    private:
        bool mTmpVisible;

        int mItems;
        int mItemsKeep;

        struct CHATLOG
        {
            std::string nick;
            std::string text;
            int own;
        };

        /**
         * A structure combining a BrowserBox with its ScrollArea.
         */
        struct ChatArea
        {
            ChatArea(BrowserBox *b, ScrollArea *s):
                browser(b), scroll(s)
            {}

            BrowserBox *browser;
            ScrollArea *scroll;
        };

        /** Tabbed area for holding each channel. */
        TabbedArea *mChatTabs;

        /** Input box for typing chat messages. */
        gcn::TextField *mChatInput;

        typedef std::map<const std::string, ChatArea> ChannelMap;
        /** Map each tab to its browser and scroll area. */
        ChannelMap mChannels;

        typedef std::list<std::string> History;
        typedef History::iterator HistoryIterator;
        History mHistory;           /**< Command history. */
        HistoryIterator mCurHist;   /**< History iterator. */
};

extern ChatWindow *chatWindow;

#endif
