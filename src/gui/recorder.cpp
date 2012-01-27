/*
 *  A chat recorder
 *  Copyright (C) 2008  Lloyd Bryant <lloyd_bryant@netzero.net>
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

#include "gui/recorder.h"

#include "client.h"
#include "event.h"

#include "gui/chatwindow.h"

#include "gui/widgets/button.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/windowcontainer.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

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
            SERVER_NOTICE(_("Finishing recording."))
        }
        else
        {
            SERVER_NOTICE(_("Not currently recording."))
        }
    }
    else if (mStream.is_open())
    {
        SERVER_NOTICE(_("Already recording."))
    }
    else
    {
        /*
         * Message should go before mStream is opened so that it isn't
         * recorded.
         */
        SERVER_NOTICE(_("Starting to record..."))
        const std::string file = Client::getLocalDataDirectory() + "/" + msgCopy;

        mStream.open(file.c_str(), std::ios_base::trunc);

        if (mStream.is_open())
            setVisible(true);
        else
            SERVER_NOTICE(_("Failed to start recording."))
    }
}

void Recorder::action(const gcn::ActionEvent &event)
{
    setRecordingFile("");
}
