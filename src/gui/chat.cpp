/*

  The Mana World
  Copyright 2004 The Mana World Development Team

    This file is part of The Mana World.

    The Mana World is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    The Mana World is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Mana World; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "../main.h"
#include "chat.h"
#include <list>
#include <string>
#include <fstream>

using namespace std;

/**
	Simple ChatLog Object v0.5 (i'd say...)

	Bestviewd w/ Bitstream Vera Sans Mono @ 9pt and a tab-width of 2 spaces

	Author: kth5 aka Alexander Baldeck
		pipe your questions, suggestions and flames to: kth5@gawab.com
*/

/**
*/
Chat::Chat(const char * logfile, int item_num) {
	chatlog_file.open(logfile, ios::out | ios::app);
	items = 0;
	items_keep = item_num;
}

/**
*/
Chat::~Chat() {
	chatlog_file.flush();
	chatlog_file.close();
}

/**
*/
void Chat::chat_dlgrsize(int) {
}

/** adds a line of text to our message list

		string line					-> message text
		int own							-> type of message (usually the owner-type)
		ALFONT_FONT * font	-> font that'll be used to draw the text later

		NOTE:
			to all of you who wonder why the font needs to be passed, simple.
			i already store the width in pixel in the list rather than
			calculating it again and again on every draw event. ;-)
*/
void Chat::chat_log(string line, int own, ALFONT_FONT * font) {
	int pos;
	CHATLOG tmp;

	if(items<=items_keep)
		items++;     // delete overhead from the end of the list
	else
		chatlog.pop_back();

	pos = 0;
	pos = (int)line.find(" : ", 0);
	if(pos > 0) {
		tmp.nick = line.substr(0,pos);
		switch(own) {
			case ACT_IS :
				tmp.nick += CAT_IS;
				break;
			case ACT_WHISPER :
				tmp.nick += CAT_WHISPER;
				break;
			default :
				tmp.nick += CAT_NORMAL;
		}
		tmp.width = TEXT_GETWIDTH(tmp.nick.c_str())+2;
		line.erase(0,pos+3);
	}else {
			tmp.nick = "";
			tmp.width = 1;
	}
	tmp.own  = own;
	tmp.text = line;

	chatlog_file << tmp.nick << tmp.text << "\n";
	chatlog_file.flush();

	chatlog.push_front(tmp);
}

/** function overload -> calls original chat_log()
    after processing the packet
*/
void Chat::chat_log(CHATSKILL action, ALFONT_FONT * font) {
	chat_log(const_msg(action), BY_SERVER, font);
}


/** draw first n lines of the list onto a Allegro type bitmap buffer
		using Alfont

		BITMAP * bmp				-> Allegro type bitmap buffer to draw onto
		int n								-> number of lines to be drawn
		ALFONT_FONT * font	-> font to use

		NOTE:
			take great care using this, make sure the buffer passed is
			empty! ;-) anyway, line wrapping is not supported yet.
*/
void Chat::chat_draw(BITMAP * bmp, int n, ALFONT_FONT * font) {
	int y = 600-35, i = 0;
	CHATLOG line;
	n -= 1;

	for(iter = chatlog.begin(); iter != chatlog.end(); iter++) {
		line = *iter;
		y -=11;

		switch(line.own) {
			case BY_GM :
				alfont_textprintf_aa(bmp, font, 1, y, COLOR_BLUE, "Global announcement: ");
				alfont_textprintf_aa(bmp, font, TEXT_GETWIDTH("Global announcement: "), y, COLOR_GREEN, line.text.c_str());
				break;
			case BY_PLAYER :
				alfont_textprintf_aa(bmp, font, 1, y, COLOR_YELLOW, line.nick.c_str());
				alfont_textprintf_aa(bmp, font, line.width, y, COLOR_WHITE, line.text.c_str());
				break;
			case BY_OTHER :
				alfont_textprintf_aa(bmp, font, 1, y, COLOR_GREEN, line.nick.c_str());
				alfont_textprintf_aa(bmp, font, line.width, y, COLOR_WHITE, line.text.c_str());
				break;
			default :
				alfont_textprintf_aa(bmp, font, 1, y, COLOR_LIGHTBLUE, line.text.c_str());
		}

		if(i>=n)
			return;
		i++;
	}
}

/** determines wether to send a command or an ordinary message, then
		contructs packets & sends them

		string nick -> the character's name to display infront
		string msg  -> the message's text which is to be send.

		NOTE:
			the nickname is required by the server, if not specified
			the message may not be sent unless a command was intended
			which requires another packet to be constructed! you can
			achieve this by putting a slash ("/") infront of the
			message followed by the command name and the message.
			of course all slash-commands need implemented handler-
			routines. ;-)
			remember, a line starting w/ "@" is not a command that needs
			to be parsed rather is sent using the normal chat-packet.

		EXAMPLE:
			// for an global announcement   /- command
			chatlog.chat_send("", "/announce Hello to all logged in users!");
			// for simple message by a user /- message
			chatlog.chat_send("Zaeiru", "Hello to all users on the screen!");
*/
char * Chat::chat_send(string nick, string msg) {
	short packid;

	// prepare command
	if(msg.substr(0,1)=="/") {
		// global announcement
		if(msg.substr(0,IS_ANNOUNCE_LENGTH) == IS_ANNOUNCE) {
			msg.erase(0,IS_ANNOUNCE_LENGTH);
			packid = 0x0099;
		} else {
			packid = 0x008c;
		}
	// prepare ordinary message
	} else {
		// temporary hack to make messed-up-keyboard-ppl able to send GM commands
		if(msg.substr(0,1)=="#")
			msg.replace(0,1,"@");
		// end temp. hack XD
		nick += " : ";
		nick += msg;
		msg = nick;
		packid = 0x008c;
	}

	msg += "\0";

	// send processed message
	WFIFOW(0) = net_w_value(packid);
	WFIFOW(2) = net_w_value((unsigned short)(msg.length()+4));
	memcpy(WFIFOP(4), msg.c_str(), msg.length());
	WFIFOSET((int)msg.length()+4);
	nick = msg = "";
	return "";
}

/** PRIVATE :
	NOTE:
		these usually will be left undocumented coz u can't call them
		directly anyway. ;-)
*/

/** constructs failed messages for actions */
string Chat::const_msg(CHATSKILL action) {
	string msg;
	if(action.success == SKILL_FAILED && action.skill == SKILL_BASIC) {
		switch(action.bskill) {
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

		switch(action.reason) {
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
	}else{
		switch(action.skill) {
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

string const_msg(int own) {
  string msg;
	return msg;
}
