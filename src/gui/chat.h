/**

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

#ifndef _CHAT_H
#define _CHAT_H

#include "../main.h"
#include <list>
#include <string>
#include <fstream>
#include <string>

using namespace std;

/**
	Simple ChatLog Object v0.5 (i'd say...)

	Bestviewd w/ Bitstream Vera Sans Mono @ 9pt and a tab-width of 2 spaces

	Author: kth5 aka Alexander Baldeck
		pipe your questions, suggestions and flames to: kth5@gawab.com

	NOTE:
		i documented all functions in their implementation. ;-)
*/


#define BY_GM					0		// those should be self-explanatory =)
#define BY_PLAYER			1
#define BY_OTHER			2
#define BY_SERVER			3

#define ACT_WHISPER		4		// getting whispered at
#define ACT_IS				5		// equivalent to "/me" in irc

#define IS_ANNOUNCE					"/announce "
#define IS_ANNOUNCE_LENGTH	10

/**
		gets in between usernick and message text depending on
		message type                   
*/
#define CAT_NORMAL          ": "
#define CAT_IS              ""
#define CAT_WHISPER         " says: "

/** some generic color macros      */
#define COLOR_WHITE				(makecol(255,255,255)) // plain white
#define COLOR_BLUE				(makecol( 97,156,236)) // cold gm blue :P
#define COLOR_YELLOW			(makecol(255,246, 98)) // sexy yellow
#define COLOR_GREEN				(makecol( 39,197, 39)) // cool green
#define COLOR_RED					(makecol(255,  0,  0)) // ack red XD
#define COLOR_LIGHTBLUE		(makecol( 83,223,246)) // bright blue

/** calculate text-width in pixel  */
#define TEXT_GETWIDTH(str)  (alfont_text_length(font, str))

/** job dependend identifiers (?)  */
#define		SKILL_BASIC			0x0001
#define		SKILL_WARP			0x001b
#define		SKILL_STEAL			0x0032
#define		SKILL_ENVENOM		0x0034

/** basic skills identifiers       */
#define		BSKILL_TRADE			0x0000
#define		BSKILL_EMOTE			0x0001
#define		BSKILL_SIT				0x0002
#define		BSKILL_CREATECHAT	0x0003
#define		BSKILL_JOINPARTY	0x0004
#define		BSKILL_SHOUT			0x0005
#define		BSKILL_PK					0x0006 // ??
#define		BSKILL_SETALLIGN	0x0007 // ??

/** reasons why action failed      */
#define		RFAIL_SKILLDEP		0x00
#define		RFAIL_INSUFHP			0x01
#define		RFAIL_INSUFSP			0x02
#define		RFAIL_NOMEMO			0x03
#define		RFAIL_SKILLDELAY	0x04
#define		RFAIL_ZENY				0x05
#define		RFAIL_WEAPON			0x06
#define		RFAIL_REDGEM			0x07
#define		RFAIL_BLUEGEM			0x08
#define		RFAIL_OVERWEIGHT	0x09
#define		RFAIL_GENERIC			0x0a

/** should always be zero if failed */
#define		SKILL_FAILED		0x00

struct CHATSKILL {
	short skill;
	short bskill;
	short unused;
	char  success;
	char  reason;
};

class Chat {
	public :
		Chat(const char *, int);
		void chat_dlgrsize(int);

		void chat_log(string, int, ALFONT_FONT *);
		void chat_log(CHATSKILL, ALFONT_FONT *);	// function overload -> calls original chat_log()
																							//   after processing the packet

		void chat_draw(BITMAP *, int, ALFONT_FONT *);
		char * chat_send(string, string);
		~Chat();
	private :
		ofstream chatlog_file;

		typedef struct CHATLOG {									// list item container object
			string nick;
			string text;
			int own;
			int width;
		};
		
		list<CHATLOG> chatlog;										// list object ready to accept out CHATLOG struct :)
		list<CHATLOG>::iterator iter;

		int items;
		int items_keep;
		
		string const_msg(CHATSKILL);							// contructs action-fail messages
		string const_msg(int);										// constructs normal messages (not implemented yet)
};

#endif
