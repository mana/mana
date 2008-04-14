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
//class TabbedContainer;
//class GCContainer;
class TabbedArea;

enum
{
    BY_GM     = 0,
    BY_PLAYER = 1,
    BY_OTHER  = 2,
    BY_SERVER = 3,
    BY_LOGGER
};

#if 0
/**
 * gets in between usernick and message text depending on
 * message type
 */
#define CAT_NORMAL        ": "
#define CAT_IS            ""
#define CAT_WHISPER       " says: "

/** job dependend identifiers (?)  */
#define SKILL_BASIC       0x0001
#define SKILL_WARP        0x001b
#define SKILL_STEAL       0x0032
#define SKILL_ENVENOM     0x0034

/** basic skills identifiers       */
#define BSKILL_TRADE      0x0000
#define BSKILL_EMOTE      0x0001
#define BSKILL_SIT        0x0002
#define BSKILL_CREATECHAT 0x0003
#define BSKILL_JOINPARTY  0x0004
#define BSKILL_SHOUT      0x0005
#define BSKILL_PK         0x0006 // ??
#define BSKILL_SETALLIGN  0x0007 // ??

/** reasons why action failed      */
#define RFAIL_SKILLDEP    0x00
#define RFAIL_INSUFHP     0x01
#define RFAIL_INSUFSP     0x02
#define RFAIL_NOMEMO      0x03
#define RFAIL_SKILLDELAY  0x04
#define RFAIL_ZENY        0x05
#define RFAIL_WEAPON      0x06
#define RFAIL_REDGEM      0x07
#define RFAIL_BLUEGEM     0x08
#define RFAIL_OVERWEIGHT  0x09
#define RFAIL_GENERIC     0x0a

/** should always be zero if failed */
#define SKILL_FAILED    0x00

struct CHATSKILL
{
    short skill;
    short bskill;
    short unused;
    char success;
    char reason;
};
#endif

/**
 * The chat window.
 *
 * \ingroup Interface
 */
class ChatWindow : public Window, public gcn::ActionListener,
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
         * Logic (updates components' size)
         */
        void logic();

        /*
         * Adds a line of text to our message list. Parameters:
         *
         * @param line Text message.
         * @parem own  Type of message (usually the owner-type).
         */
        void chatLog(std::string line, int own, std::string channelName = "General");

#if 0
        /*
         * Calls original chat_log() after processing the packet.
         */
        void chatLog(CHATSKILL);
#endif

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

        /*
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

        /** Called to add the channel to the channel manager */
        void
        addChannel(short channel, std::string channelName);

        /** Called to remove the channel from the channel manager */
        void
        removeChannel(short channel);

        /** Called to create a new channel tab */
        void
        createNewChannelTab(std::string channelName);

        /** Called to join channel */
        void
        enterChannel(std::string channel, std::string password = "None");

        /** Called to output text to a specific channel */
        void
        sendToChannel(short channel, std::string user, std::string msg);

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

#if 0
        /** Constructs failed messages for actions */
        std::string const_msg(CHATSKILL);
        std::map<std::string, GCContainer*> mTabs;
        TabbedContainer *mContainer; /**< Tabbed container for tabbing between channels */
        GCContainer *mTab; /**< Tabs */
#endif

        TabbedArea *mChatTabs; /** < Chat Tabbed area for holding each channel */
        gcn::TextField *mChatInput; /**< Input box for typing chat messages */
        std::map<std::string, BrowserBox*> mChannelOutput; /**< Map each TextOutput to a tab */
        std::map<std::string, ScrollArea*> mChannelScroll; /**< Map each ScrollArea to a tab */
        BrowserBox *mTextOutput;    /**< Text box for displaying chat history */
        ScrollArea *mScrollArea;    /**< Scroll area around text output */

        typedef std::list<std::string> History;
        typedef History::iterator HistoryIterator;
        History mHistory;           /**< Command history */
        HistoryIterator mCurHist; /**< History iterator */
};

extern ChatWindow *chatWindow;

#endif
