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

#include "gui/debugwindow.h"

#include "client.h"
#include "game.h"
#include "particle.h"
#include "map.h"

#include "gui/setup.h"
#include "gui/viewport.h"

#include "gui/widgets/checkbox.h"
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

        LayoutHelper h(this);
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
        Label *showLabel = new Label(_("Show:"));
        mGrid = new CheckBox(_("Grid"));
        mCollisionTiles = new CheckBox(_("Collision tiles"));
        mBeingCollisionRadius = new CheckBox(_("Being collision radius"));
        mBeingPosition = new CheckBox(_("Being positions"));
        mBeingPath = new CheckBox(_("Being path"));
        mMousePath = new CheckBox(_("Mouse path"));
        mBeingIds = new CheckBox(_("Being Ids"));

        Label *specialsLabel = new Label(_("Specials:"));
        mSpecialNormal = new RadioButton(_("Normal"), "mapdebug");
        mSpecial1 = new RadioButton(_("Special 1"), "mapdebug");
        mSpecial2 = new RadioButton(_("Special 2"), "mapdebug");
        mSpecial3 = new RadioButton(_("Special 3"), "mapdebug");

        LayoutHelper h(this);
        ContainerPlacer place = h.getPlacer(0, 0);

        place(0, 0, showLabel, 1);
        place(0, 1, mGrid, 1);
        place(0, 2, mCollisionTiles, 1);
        place(0, 3, mBeingCollisionRadius, 1);
        place(0, 4, mBeingPosition, 1);
        place(0, 5, mBeingPath, 1);
        place(0, 6, mMousePath, 1);
        place(0, 7, mBeingIds, 1);
        place(1, 0, specialsLabel, 1);
        place(1, 1, mSpecialNormal, 1);
        place(1, 2, mSpecial1, 1);
        place(1, 3, mSpecial2, 1);
        place(1, 4, mSpecial3, 1);

        h.reflowLayout(0, 0);

        mSpecialNormal->setSelected(true);

        mGrid->addActionListener(this);
        mCollisionTiles->addActionListener(this);
        mBeingCollisionRadius->addActionListener(this);
        mBeingPosition->addActionListener(this);
        mBeingPath->addActionListener(this);
        mMousePath->addActionListener(this);
        mBeingIds->addActionListener(this);
        mSpecialNormal->addActionListener(this);
        mSpecial1->addActionListener(this);
        mSpecial2->addActionListener(this);
        mSpecial3->addActionListener(this);
    }

    ~DebugSwitches()
    {
        delete mGrid;
        delete mCollisionTiles;
        delete mBeingCollisionRadius;
        delete mBeingPosition;
        delete mBeingPath;
        delete mMousePath;
        delete mBeingIds;
        delete mSpecialNormal;
        delete mSpecial1;
        delete mSpecial2;
        delete mSpecial3;
    }

    void action(const gcn::ActionEvent &event)
    {
        int flags = 0;

        if (mGrid->isSelected())
            flags |= Map::DEBUG_GRID;
        if (mCollisionTiles->isSelected())
            flags |= Map::DEBUG_COLLISION_TILES;
        if (mBeingCollisionRadius->isSelected())
            flags |= Map::DEBUG_BEING_COLLISION_RADIUS;
        if (mBeingPosition->isSelected())
            flags |= Map::DEBUG_BEING_POSITION;
        if (mBeingPath->isSelected())
            flags |= Map::DEBUG_BEING_PATH;
        if (mMousePath->isSelected())
            flags |= Map::DEBUG_MOUSE_PATH;
        if (mBeingIds->isSelected())
            flags |= Map::DEBUG_BEING_IDS;
        if (mSpecial1->isSelected())
            flags |= Map::DEBUG_SPECIAL1;
        if (mSpecial2->isSelected())
            flags |= Map::DEBUG_SPECIAL2;
        if (mSpecial3->isSelected())
            flags |= Map::DEBUG_SPECIAL3;

        viewport->setShowDebugPath(flags);
    }

private:
    CheckBox *mGrid;
    CheckBox *mCollisionTiles;
    CheckBox *mBeingCollisionRadius;
    CheckBox *mBeingPosition;
    CheckBox *mBeingPath;
    CheckBox *mMousePath;
    CheckBox *mBeingIds;
    RadioButton *mSpecialNormal;
    RadioButton *mSpecial1;
    RadioButton *mSpecial2;
    RadioButton *mSpecial3;
};

DebugWindow::DebugWindow()
    : Window(_("Debug"))
{
    setupWindow->registerWindowForReset(this);

    setWindowName("Debug");
    setResizable(true);
    setCloseButton(true);
    setMinWidth(100);
    setMinHeight(100);
    setDefaultSize(0, 120, 300, 190);

    TabbedArea *tabs = new TabbedArea;
    place(0, 0, tabs, 2, 2);
    loadWindowState();

    Tab *tabInfo = new Tab;
    tabInfo->setCaption(_("Info"));
    tabs->addTab(tabInfo, new DebugInfo);

    Tab *tabSwitches = new Tab;
    tabSwitches->setCaption(_("Switches"));
    tabs->addTab(tabSwitches, new DebugSwitches);
}
