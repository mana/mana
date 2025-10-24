/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "gui/widgets/linkhandler.h"

#include <guichan/actionlistener.hpp>
#include <guichan/deathlistener.hpp>

#include <memory>

class ItemPopup;
class MonsterPopup;
class Window;

class ItemLinkHandler : public LinkHandler, gcn::ActionListener, public gcn::DeathListener
{
    public:
        ItemLinkHandler(Window *parent = nullptr);
        ~ItemLinkHandler() override;

        // LinkHandler interface
        std::string captionForLink(const std::string &link) override;
        void handleLink(const std::string &link) override;

        // ActionListener interface
        void action(const gcn::ActionEvent &actionEvent) override;

        // DeathListener interface
        void death(const gcn::Event &event) override;

    private:
        std::unique_ptr<ItemPopup> mItemPopup;
        std::unique_ptr<MonsterPopup> mMonsterPopup;

        Window *mParent = nullptr;
        std::string mLink;
};
