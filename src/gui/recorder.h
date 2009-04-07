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

#ifndef RECORD_H
#define RECORD_H

#include "gui/widgets/window.h"

#include "utils/gettext.h"

#include <guichan/actionlistener.hpp>

#include <fstream>
#include <string>

class ChatWindow;

class Recorder : public Window, public gcn::ActionListener
{
    public:
        Recorder(ChatWindow *chat, const std::string &title = _("Recording..."),
                 const std::string &buttonTxt = _("Stop recording"));

        virtual ~Recorder();

        /**
         * Outputs the message to the recorder file
         *
         * @param msg the line to write to the recorded file.
         */
        void record(const std::string &msg);

        /**
         * Sets the file being recorded to
         *
         * @param msg The file to write out to. If null, then stop recording.
         */
        void setRecordingFile(const std::string &msg);

        /**
         * Whether or not the recorder is in use.
         */
        bool isRecording() { return (bool) mStream.is_open(); }

        /**
         * called when the button is pressed
         *
         * @param event is the event that is generated
         */
        void action(const gcn::ActionEvent &event);

    private:
        ChatWindow *mChat;

        std::ofstream mStream;
};

#endif
