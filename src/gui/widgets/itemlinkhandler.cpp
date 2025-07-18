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

#include <sstream>
#include <string>

#include <SDL.h>

#if SDL_VERSION_ATLEAST(2, 0, 14)
#include "gui/confirmdialog.h"
#endif
#include "gui/itempopup.h"
#include "gui/okdialog.h"
#include "gui/viewport.h"

#include "gui/widgets/itemlinkhandler.h"

#include "client.h"
#include "resources/iteminfo.h"
#include "resources/itemdb.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

ItemLinkHandler::ItemLinkHandler(Window *parent)
    : mParent(parent)
{
    mItemPopup = std::make_unique<ItemPopup>();
    mItemPopup->addDeathListener(this);
}

ItemLinkHandler::~ItemLinkHandler() = default;

static bool isUrl(const std::string &link)
{
    return startsWith(link, "https://") ||
            startsWith(link, "http://") ||
            startsWith(link, "ftp://");
}

void ItemLinkHandler::handleLink(const std::string &link)
{
#if SDL_VERSION_ATLEAST(2, 0, 14)
    // Handle screenshots by constructing full file path
    if (startsWith(link, "screenshot:"))
    {
        std::string filename = link.substr(11); // Remove "screenshot:" prefix

        // Prevent directory traversal attacks or opening malicious files
        if (filename.find("..") == std::string::npos && endsWith(filename, ".png"))
        {
            std::string fileUrl = "file://" + Client::getScreenshotDirectory() + "/" + filename;
            if (SDL_OpenURL(fileUrl.c_str()) == -1)
                new OkDialog(_("Open URL Failed"), SDL_GetError(), true, mParent);
        }

        return;
    }
#endif

    if (isUrl(link))
    {
        mLink = link;

#if SDL_VERSION_ATLEAST(2, 0, 14)
        auto confirmDialog = new ConfirmDialog(_("Open URL?"), link, mParent);
        confirmDialog->addActionListener(this);
#else
        new OkDialog(_("Open URL Failed"),
                     _("Opening of URLs requires SDL 2.0.14."), true, mParent);
#endif
        return;
    }

    int id = 0;
    std::istringstream stream(link);
    stream >> id;

    if (id > 0)
    {
        const ItemInfo &itemInfo = itemDb->get(id);
        mItemPopup->setItem(itemInfo, true);

        if (mItemPopup->isVisible())
            mItemPopup->setVisible(false);
        else
            mItemPopup->position(viewport->getMouseX(), viewport->getMouseY());
    }
}

void ItemLinkHandler::action(const gcn::ActionEvent &actionEvent)
{
    if (actionEvent.getId() == "yes")
    {
#if SDL_VERSION_ATLEAST(2, 0, 14)
        if (SDL_OpenURL(mLink.c_str()) == -1)
        {
            new OkDialog(_("Open URL Failed"), SDL_GetError(), true, mParent);
        }
#endif
    }
}

void ItemLinkHandler::death(const gcn::Event &event)
{
    // If somebody else killed the PopupUp, make sure we don't also try to delete it
    if (event.getSource() == mItemPopup.get())
        mItemPopup.release();
}
