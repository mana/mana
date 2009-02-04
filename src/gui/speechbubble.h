/*
 *  The Legend of Mazzeroth
 *  Copyright (C) 2008, The Legend of Mazzeroth Development Team
 *
 *  This file is part of The Legend of Mazzeroth based on original code
 *  from The Mana World.
 *
 *  The Legend of Mazzeroth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Legend of Mazzeroth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Legend of Mazzeroth; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SPEECHBUBBLE_H__
#define SPEECHBUBBLE_H__

#include "window.h"

class ScrollArea;
class TextBox;

class SpeechBubble : public Window
{
    public:

	SpeechBubble();

        void setCaption(const std::string &name,
                        const gcn::Color &color = 0x000000);
	void setText(std::string mText);
	void setLocation(int x, int y);
	unsigned int getNumRows();

    private:
        gcn::Label *mCaption;
	TextBox *mSpeechBox;
	ScrollArea *mSpeechArea;
};

#endif
