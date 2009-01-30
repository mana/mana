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
 */

#include <physfs.h>

#include "button.h"
#include "chat.h"
#include "recorder.h"

#include "../utils/trim.h"

Recorder::Recorder(ChatWindow *chat, const std::string &title,
                   const std::string &buttonTxt) :
    Window(title)
{
    mChat = chat;
    Button *button = new Button(buttonTxt, "activate", this);
    setContentSize(button->getWidth() + 10, button->getHeight() + 10);
    button->setPosition(5, 5);
    add(button);
}

void Recorder::record(const std::string &msg)
{
    if (mStream.is_open())
    {
        mStream << msg << std::endl;
    }
}

void Recorder::changeRecordingStatus(const std::string &msg)
{
    std::string msgCopy = msg;
    trim(msgCopy);

    if (msgCopy == "")
    {
        if (mStream.is_open())
        {
            mStream.close();
            setVisible(false);

            /*
             * Message should go after mStream is closed so that it isn't
             * recorded.
             */
            mChat->chatLog("Finishing recording.", BY_SERVER);
        }
        else
        {
            mChat->chatLog("Not currently recording.", BY_SERVER);
        }
    }
    else if (mStream.is_open())
    {
        mChat->chatLog("Already recording.", BY_SERVER);
    }
    else
    {
        /*
         * Message should go before mStream is opened so that it isn't
         * recorded.
         */
        mChat->chatLog("Starting to record...", BY_SERVER);
        std::string file = std::string(PHYSFS_getUserDir()) + "/.aethyra/" + msgCopy;
        
        mStream.open(file.c_str(), std::ios_base::trunc);

        if (mStream.is_open())
            setVisible(true);
        else
            mChat->chatLog("Failed to start recording.", BY_SERVER);
    }
}

void Recorder::action(const gcn::ActionEvent &event)
{
    changeRecordingStatus("");
}

Recorder::~Recorder()
{
}
