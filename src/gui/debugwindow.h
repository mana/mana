/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#pragma once

#include "gui/widgets/window.h"

#include <memory>

class Tab;

/**
 * The debug window.
 *
 * \ingroup Interface
 */
class DebugWindow : public Window
{
    public:
        DebugWindow();
        ~DebugWindow() override;

    private:
        std::unique_ptr<Tab> mInfoTab;
        std::unique_ptr<Tab> mSwitchesTab;
        std::unique_ptr<gcn::Widget> mInfoWidget;
        std::unique_ptr<gcn::Widget> mSwitchesWidget;
};

extern DebugWindow *debugWindow;
