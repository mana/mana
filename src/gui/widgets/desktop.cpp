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
#include "resources/theme.h"
#include "resources/wallpaper.h"

#include "utils/stringutils.h"

Desktop::Desktop()
    : mWallpaper(0)
    , mBackground(ResourceManager::getInstance()->getImage("graphics/images/bg_image.png"))
    , mOverlay(ResourceManager::getInstance()->getImage("graphics/images/bg_overlay.png"))
    , mBgSkin(Theme::instance()->load("bg.xml"))
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

    mBgSkin->instances--;
    mBackground->decRef();
    mOverlay->decRef();
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
        if (!Image::useOpenGL())
            g->drawImage(mWallpaper,
                (getWidth() - mWallpaper->getWidth()) / 2,
                (getHeight() - mWallpaper->getHeight()) / 2);
        else
            g->drawRescaledImage(mWallpaper, 0, 0, 0, 0,
                mWallpaper->getWidth(), mWallpaper->getHeight(),
                getWidth(), getHeight(), false);
    }

    mBgSkin->setAlpha(1.0f);
    g->drawImageRect(gcn::Rectangle(5, 5, getWidth() - 10, getHeight() - 10),
                     mBgSkin->getBorder());

    gcn::Rectangle innerArea(5 + 64,
                             5 + 64,
                             getWidth() - 10 - 64 * 2,
                             getHeight() - 10 - 64 * 2);

    if (innerArea.width > 0 && innerArea.height > 0)
    {
        g->pushClipArea(innerArea);

        float scale = std::max((float) innerArea.width / mBackground->getWidth(),
                               (float) innerArea.height / mBackground->getHeight());

        int width = scale * mBackground->getWidth();
        int height = scale * mBackground->getHeight();

        g->drawRescaledImage(mBackground, 0, 0,
                             (innerArea.width - width) / 2,
                             (innerArea.height - height) / 2,
                             mBackground->getWidth(),
                             mBackground->getHeight(),
                             width, height, false);

        g->drawRescaledImage(mOverlay, 0, 0, -1, -1,
                             mOverlay->getWidth(),
                             mOverlay->getHeight(),
                             innerArea.width + 2,
                             innerArea.height + 2, false);

        g->popClipArea();
    }

    // Draw a thin border under the application version...
    g->setColor(gcn::Color(255, 255, 255, 128));
    g->fillRectangle(gcn::Rectangle(mVersionLabel->getDimension()));

    Container::draw(graphics);
}

void Desktop::setBestFittingWallpaper()
{
    const int width = getWidth();
    const int height = getHeight();

    if (width == 0 || height == 0)
        return;

    const std::string wallpaperName = Wallpaper::getWallpaper(width, height);

    if (wallpaperName.empty())
        return;

    ResourceManager *resman = ResourceManager::getInstance();
    Image *wallpaper = resman->getImage(wallpaperName);

    if (wallpaper)
    {
        if (mWallpaper)
            mWallpaper->decRef(Resource::DeleteImmediately);

        mWallpaper = wallpaper;

        // In software mode we try to prescale the image for performance
        if (!wallpaper->useOpenGL() &&
                (wallpaper->getWidth() != width ||
                 wallpaper->getHeight() != height))
        {
            if (Image *prescaled = wallpaper->SDLgetScaledImage(width, height))
            {
                // Make sure the original can be freed
                wallpaper->decRef();
                mWallpaper = prescaled;
            }
        }
    }
    else
    {
        logger->log("Couldn't load %s as wallpaper", wallpaperName.c_str());
    }
}
