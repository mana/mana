/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "gui/debugwindow.h"

#include "client.h"
#include "game.h"
#include "particle.h"
#include "map.h"

#include "gui/setup.h"
#include "gui/viewport.h"

#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/radiobutton.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/tabbedarea.h"

#include "resources/image.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"


class DebugInfo : public Container
{
public:
    DebugInfo()
    {
#ifdef USE_OPENGL
        if (Image::getLoadAsOpenGL())
        {
            mFPSText = _("%d FPS (OpenGL)");
        }
        else
#endif
        {
            mFPSText = _("%d FPS");
        }

        mFPSLabel = new Label("");
        mMusicFileLabel = new Label("");
        mMapLabel = new Label("");
        mMinimapLabel = new Label("");
        mTileMouseLabel = new Label("");
        mParticleCountLabel = new Label("");

        LayoutHelper h = (this);
        ContainerPlacer place = h.getPlacer(0, 0);

        place(0, 0, mFPSLabel, 1);
        place(0, 1, mMusicFileLabel, 1);
        place(0, 2, mMapLabel, 1);
        place(0, 3, mMinimapLabel, 1);
        place(0, 4, mTileMouseLabel, 1);
        place(0, 5, mParticleCountLabel, 1);

        h.reflowLayout(0, 0);
    }

    void logic()
    {
        if (!isVisible())
            return;

        mFPSLabel->setCaption(strprintf(mFPSText.c_str(), fps));

        if (const Map *map = Game::instance()->getCurrentMap())
        {
            // Get the current mouse position
            const int mouseTileX = (viewport->getMouseX() +
                             viewport->getCameraX()) / map->getTileWidth();
            const int mouseTileY = (viewport->getMouseY() +
                            viewport->getCameraY()) / map->getTileHeight();
            mTileMouseLabel->setCaption(strprintf(_("Cursor: (%d, %d)"),
                                        mouseTileX, mouseTileY));

            mMusicFileLabel->setCaption(strprintf(
                        _("Music: %s"), map->getProperty("music").c_str()));
            mMinimapLabel->setCaption(strprintf(_("Minimap: %s"),
                                      map->getProperty("minimap").c_str()));
            mMapLabel->setCaption(strprintf(_("Map: %s"),
                                    map->getProperty("_filename").c_str()));
        }

        mParticleCountLabel->setCaption(strprintf(_("Particle count: %d"),
                                        Particle::particleCount));

        mFPSLabel->adjustSize();
        mMusicFileLabel->adjustSize();
        mMapLabel->adjustSize();
        mMinimapLabel->adjustSize();
        mTileMouseLabel->adjustSize();
        mParticleCountLabel->adjustSize();
    }

private:
    std::string mFPSText;
    Label *mFPSLabel;
    Label *mMusicFileLabel;
    Label *mMapLabel;
    Label *mMinimapLabel;
    Label *mTileMouseLabel;
    Label *mParticleCountLabel;
};

class DebugSwitches : public Container, public gcn::ActionListener
{
public:
    DebugSwitches()
    {
        mapNormal = new RadioButton(_("Normal"), "mapdebug");
        mapDebug = new RadioButton(_("Debug"), "mapdebug");
        mapSpecial = new RadioButton(_("Special"), "mapdebug");
        mapSpecial2 = new RadioButton(_("Special 2"), "mapdebug");
        mapSpecial3 = new RadioButton(_("Special 3"), "mapdebug");

        LayoutHelper h = (this);
        ContainerPlacer place = h.getPlacer(0, 0);

        place(0, 0, mapNormal, 1);
        place(0, 1, mapDebug, 1);
        place(0, 2, mapSpecial, 1);
        place(0, 3, mapSpecial2, 1);
        place(0, 4, mapSpecial3, 1);

        h.reflowLayout(0, 0);

        mapNormal->setSelected(true);

        mapNormal->addActionListener(this);
        mapDebug->addActionListener(this);
        mapSpecial->addActionListener(this);
        mapSpecial2->addActionListener(this);
        mapSpecial3->addActionListener(this);
    }

    void action(const gcn::ActionEvent &event)
    {
        if (mapNormal->isSelected())
            viewport->setShowDebugPath(Map::MAP_NORMAL);
        else if (mapDebug->isSelected())
            viewport->setShowDebugPath(Map::MAP_DEBUG);
        else if (mapSpecial->isSelected())
            viewport->setShowDebugPath(Map::MAP_SPECIAL);
        else if (mapSpecial2->isSelected())
            viewport->setShowDebugPath(Map::MAP_SPECIAL2);
        else if (mapSpecial3->isSelected())
            viewport->setShowDebugPath(Map::MAP_SPECIAL3);
    }

private:
    RadioButton *mapNormal;
    RadioButton *mapDebug;
    RadioButton *mapSpecial;
    RadioButton *mapSpecial2;
    RadioButton *mapSpecial3;
};

DebugWindow::DebugWindow():
    Window(_("Debug"))
{
    setupWindow->registerWindowForReset(this);

    setResizable(true);
    setCloseButton(true);

    setMinWidth(100);
    setMinHeight(100);
    setDefaultSize(0, 120, 300, 180);

    loadWindowState();

    TabbedArea *mTabs = new TabbedArea;

    place(0, 0, mTabs, 2, 2);

    widgetResized(NULL);

    Tab *tabInfo = new Tab();
    tabInfo->setCaption("Info");
    mTabs->addTab(tabInfo, new DebugInfo);

    Tab *tabSwitches = new Tab();
    tabSwitches->setCaption("Switches");
    mTabs->addTab(tabSwitches, new DebugSwitches);
}
