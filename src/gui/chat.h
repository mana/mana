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

#include <guichan.hpp>
#include "../resources/image.h"
#include "../net/network.h"
#include "window.h"
#include <SDL.h>
#include <list>
#include <string>
#include <fstream>

#define BY_GM         0   // those should be self-explanatory =)
#define BY_PLAYER     1
#define BY_OTHER      2
#define BY_SERVER     3

#define ACT_WHISPER   4   // getting whispered at
#define ACT_IS        5   // equivalent to "/me" in irc

#define IS_ANNOUNCE         "/announce "
#define IS_ANNOUNCE_LENGTH  10
#define IS_WHERE            "/where "
#define IS_WHERE_LENGTH     7

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

struct CHATSKILL {
    short skill;
    short bskill;
    short unused;
    char success;
    char reason;
};

/**
 * Simple chatlog object.
 */
class ChatWindow : public Window, public gcn::ActionListener {
    public:
        /**
         * Constructor.
         */
        ChatWindow(const char *logfile, int item_num);

        /**
         * Destructor.
         */
        ~ChatWindow();

        /*
         * Adds a line of text to our message list. Parameters:
         *
         * @param line Text message.
         * @parem own  Type of message (usually the owner-type).
         */
        void chat_log(std::string line, int own);

        /*
         * Calls original chat_log() after processing the packet.
         */
        void chat_log(CHATSKILL);

        /*
         * Draws the chat box.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Performs action.
         */
        void action(const std::string &actionId);

        /**
         * Request focus.
         */
        void requestFocus();
        
        /**
         * Checks wether ChatWindow is Focused or not
         */
        bool isFocused();
        
        /*
         * Determines wether to send a command or an ordinary message, then
         * contructs packets & sends them
         *
         * @param nick The character's name to display in front.
         * @param msg  The message text which is to be send.
         *
         * NOTE:
         * the nickname is required by the server, if not specified
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
        char *chat_send(std::string nick, std::string msg);

    private :
        std::ofstream chatlog_file;

        typedef struct CHATLOG {          // list item container object
            std::string nick;
            std::string text;
            int own;
        };

        std::list<CHATLOG> chatlog;       // list object ready to accept out CHATLOG struct :)
        std::list<CHATLOG>::iterator iter;

        int items;
        int items_keep;

        /** Constructs failed messages for actions */
        std::string const_msg(CHATSKILL);

        /**
         * Cuts a string into two on a per word basis
         * @param value The string to be cut, it may be modified
         * in the function.
         * @param maximumLength The length after which the string
         * should be cut.
         * @return The cut off section of the string
         */
        std::string cut_string(std::string& value, unsigned int maximumLength);

        /** Input box for chat messages */
        gcn::TextField *chatInput;
};

#endif

