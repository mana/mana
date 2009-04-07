/*
 *  A chat recorder
 *  Copyright (C) 2008  Lloyd Bryant <lloyd_bryant@netzero.net>
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

#include "gui/recorder.h"

#include "gui/chat.h"

#include "gui/widgets/button.h"
#include "gui/widgets/chattab.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/windowcontainer.h"

#include "utils/stringutils.h"

#include <physfs.h>

Recorder::Recorder(ChatWindow *chat, const std::string &title,
                   const std::string &buttonTxt) :
    Window(title)
{
    setWindowName("Recorder");
    const int offsetX = 2 * getPadding() + 10;
    const int offsetY = getTitleBarHeight() + getPadding() + 10;

    mChat = chat;
    Button *button = new Button(buttonTxt, "activate", this);

    // 123 is the default chat window height. If you change this in Chat, please
    // change it here as well
    setDefaultSize(button->getWidth() + offsetX, button->getHeight() +
                   offsetY, ImageRect::LOWER_LEFT, 0, 123);

    place(0, 0, button);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    loadWindowState();
}

Recorder::~Recorder()
{
}

void Recorder::record(const std::string &msg)
{
    if (mStream.is_open())
    {
        mStream << msg << std::endl;
    }
}

void Recorder::setRecordingFile(const std::string &msg)
{
    std::string msgCopy = msg;
    trim(msgCopy);

    if (msgCopy.empty())
    {
        if (mStream.is_open())
        {
            mStream.close();
            setVisible(false);

            /*
             * Message should go after mStream is closed so that it isn't
             * recorded.
             */
            localChatTab->chatLog(_("Finishing recording."), BY_SERVER);
        }
        else
        {
            localChatTab->chatLog(_("Not currently recording."), BY_SERVER);
        }
    }
    else if (mStream.is_open())
    {
        localChatTab->chatLog(_("Already recording."), BY_SERVER);
    }
    else
    {
        /*
         * Message should go before mStream is opened so that it isn't
         * recorded.
         */
        localChatTab->chatLog(_("Starting to record..."), BY_SERVER);
        const std::string file =
            std::string(PHYSFS_getUserDir()) + "/.tmw/" + msgCopy;

        mStream.open(file.c_str(), std::ios_base::trunc);

        if (mStream.is_open())
            setVisible(true);
        else
            localChatTab->chatLog(_("Failed to start recording."), BY_SERVER);
    }
}

void Recorder::action(const gcn::ActionEvent &event)
{
    setRecordingFile("");
}
