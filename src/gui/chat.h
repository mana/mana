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

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include "window.h"

#include "../guichanfwd.h"

class BrowserBox;
class Network;
class ScrollArea;

#define BY_GM         0   // those should be self-explanatory =)
#define BY_PLAYER     1
#define BY_OTHER      2
#define BY_SERVER     3

#define ACT_WHISPER   4   // getting whispered at
#define ACT_IS        5   // equivalent to "/me" in irc

#define BY_LOGGER     6

#define IS_ANNOUNCE         "/announce "
#define IS_ANNOUNCE_LENGTH  10
#define IS_HELP             "/help"
#define IS_HELP_LENGTH      5
#define IS_WHERE            "/where"
#define IS_WHERE_LENGTH     6
#define IS_WHO              "/who"
#define IS_WHO_LENGTH       4
#define IS_CLEAR            "/clear"
#define IS_CLEAR_LENGTH     6

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
        ChatWindow(Network *network);

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
        void chatLog(std::string line, int own);

        /*
         * Calls original chat_log() after processing the packet.
         */
        void chatLog(CHATSKILL);

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
        void
        chatSend(const std::string &nick, std::string msg);

        /** Called when key is pressed */
        void
        keyPressed(gcn::KeyEvent &event);

        /** Called to set current text */
        void
        setInputText(std::string input_str);

        /** Override to reset mTmpVisible */
        void
        setVisible(bool visible);

    private:
        Network *mNetwork;
        bool mTmpVisible;

        /** One item in the chat log */
        struct CHATLOG
        {
            std::string nick;
            std::string text;
            int own;
        };

        /** Constructs failed messages for actions */
        std::string const_msg(CHATSKILL);

        gcn::TextField *mChatInput; /**< Input box for typing chat messages */
        BrowserBox *mTextOutput;    /**< Text box for displaying chat history */
        ScrollArea *mScrollArea;    /**< Scroll area around text output */

        typedef std::list<std::string> History;
        typedef History::iterator HistoryIterator;
        History mHistory;           /**< Command history */
        HistoryIterator mCurHist; /**< History iterator */
};

extern ChatWindow *chatWindow;

#endif

