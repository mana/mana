/*
 *  Desktop widget
 *  Copyright (c) 2009-2010  The Mana World Development Team
 *  Copyright (C) 2010-2012  The Mana Developers
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

#include "gui/widgets/desktop.h"

#include "configuration.h"
#include "graphics.h"
#include "log.h"
#include "main.h"

#include "gui/palette.h"

#include "gui/widgets/label.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"
#include "resources/wallpaper.h"

#include "utils/stringutils.h"

Desktop::Desktop()
    : mWallpaper(0)
{
    addWidgetListener(this);

    Wallpaper::loadWallpapers();

    std::string appName = branding.getValue("appName", "");

    if (appName.empty())
        mVersionLabel = new Label(FULL_VERSION);
    else
        mVersionLabel = new Label(strprintf("%s (Mana %s)", appName.c_str(),
                                            FULL_VERSION));

    mVersionLabel->setBackgroundColor(gcn::Color(255, 255, 255, 128));
    add(mVersionLabel, 25, 2);
}

Desktop::~Desktop()
{
    if (mWallpaper)
        mWallpaper->decRef();
}

void Desktop::reloadWallpaper()
{
    Wallpaper::loadWallpapers();
    setBestFittingWallpaper();
}

void Desktop::widgetResized(const gcn::Event &)
{
    setBestFittingWallpaper();
}

void Desktop::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics *>(graphics);

    if (!mWallpaper || (getWidth() > mWallpaper->getWidth() ||
                        getHeight() > mWallpaper->getHeight()))
    {
        // TODO: Color from palette
        g->setColor(gcn::Color(64, 64, 64));
        g->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    }

    if (mWallpaper)
    {
        if (!mWallpaper->useOpenGL())
            g->drawImage(mWallpaper,
                (getWidth() - mWallpaper->getWidth()) / 2,
                (getHeight() - mWallpaper->getHeight()) / 2);
        else
            g->drawRescaledImage(mWallpaper, 0, 0, 0, 0,
                mWallpaper->getWidth(), mWallpaper->getHeight(),
                getWidth(), getHeight(), false);
    }

    // Draw a thin border under the application version...
    g->setColor(gcn::Color(255, 255, 255, 128));
    g->fillRectangle(gcn::Rectangle(mVersionLabel->getDimension()));

    Container::draw(graphics);
}

void Desktop::setBestFittingWallpaper()
{
    const std::string wallpaperName =
            Wallpaper::getWallpaper(getWidth(), getHeight());

    Image *nWallPaper = ResourceManager::getInstance()->getImage(wallpaperName);

    if (nWallPaper)
    {
        if (mWallpaper)
            mWallpaper->decRef();

        if (!nWallPaper->useOpenGL() && (nWallPaper->getWidth() != getWidth()
            || nWallPaper->getHeight() != getHeight()))
        {
            // We rescale to obtain a fullscreen wallpaper...
            Image *newRsclWlPpr = nWallPaper->SDLgetScaledImage(getWidth(), getHeight());
            std::string idPath = nWallPaper->getIdPath();

            // We replace the resource in the resource manager
            nWallPaper->decRef();
            ResourceManager::getInstance()->addResource(idPath, newRsclWlPpr);
            mWallpaper = newRsclWlPpr;
        }
            else
                mWallpaper = nWallPaper;
    }
    else
    {
        logger->log("Couldn't load %s as wallpaper", wallpaperName.c_str());
    }
}
