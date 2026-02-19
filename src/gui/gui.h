/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "eventlistener.h"
#include "guichanfwd.h"

#include "gui/dragndrop.h"

#include "resources/theme.h"

#include "utils/time.h"

#include <guichan/gui.hpp>
#include <guichan/keylistener.hpp>

#include <SDL.h>

#include <memory>
#include <optional>
#include <vector>

class TextInput;
class Graphics;
class SDLInput;

/**
 * \defgroup GUI Core GUI related classes (widgets)
 */

/**
 * \defgroup Interface User interface related classes (windows, dialogs)
 */

/**
 * Cursors are in graphic order from left to right.
 * CURSOR_POINTER should be left untouched.
 */
enum class Cursor
{
    Pointer = 0,
    ResizeAcross,
    ResizeDown,
    ResizeDownLeft,
    ResizeDownRight,
    Fight,
    PickUp,
    Talk,
    Action,
    Left,
    Up,
    Right,
    Down,
    Drag,
    Hand,
    Count,
};

/**
 * Main GUI class.
 *
 * \ingroup GUI
 */
class Gui final : public gcn::Gui,
                  public EventListener,
                  public gcn::KeyListener
{
    public:
        Gui(Graphics *screen, const std::string &themePath);

        ~Gui() override;

        /**
         * Performs logic of the GUI. Overridden to track mouse pointer
         * activity.
         */
        void logic() override;
        void draw() override;

        void event(Event::Channel channel, const Event &event) override;

        /**
         * Called when the application window has been resized.
         *
         * Returns whether the top widget changed size.
         */
        bool videoResized(int width, int height);

        gcn::FocusHandler *getFocusHandler() const
        { return mFocusHandler; }

        /**
         * Return game font.
         */
        gcn::Font *getFont() const
        { return mGuiFont; }

        /**
         * Return the Font used for "Info Particles", i.e. ones showing, what
         * you picked up, etc.
         */
        gcn::Font *getInfoParticleFont() const
        { return mInfoParticleFont; }

        /**
         * Sets whether a custom cursor should be rendered.
         */
        void setUseCustomCursor(bool customCursor);

        /**
         * Sets which cursor should be used.
         */
        void setCursorType(Cursor cursor);

        const std::vector<ThemeInfo> &getAvailableThemes() const
        { return mAvailableThemes; }

        /**
         * Sets the global GUI theme.
         */
        void setTheme(const ThemeInfo &theme);

        /**
         * The global GUI theme.
         */
        Theme *getTheme() const
        { return mTheme.get(); }

        const Drag *getActiveDrag() const
        { return mActiveDrag ? &(*mActiveDrag) : nullptr; }

        void startDrag(Drag drag);
        bool cancelActiveDrag();

        DragTarget *getDragTargetUnderMouse() const
        { return mDragTargetUnderMouse; }

        static bool debugDraw;

    protected:
        void keyPressed(gcn::KeyEvent &event) override;
        void keyReleased(gcn::KeyEvent &event) override;
        void handleMousePressed(const gcn::MouseInput &mouseInput) override;
        void handleMouseMoved(const gcn::MouseInput &mouseInput) override;
        void handleMouseReleased(const gcn::MouseInput &mouseInput) override;
        void handleTextInput(const TextInput &textInput);

    private:
        void updateCursor();
        void updateDragTargetFromPosition(int x, int y);

        void loadCustomCursors();
        void loadSystemCursors();

        std::vector<ThemeInfo> mAvailableThemes;
        std::unique_ptr<Theme> mTheme;        /**< The global GUI theme */
        std::optional<Drag> mActiveDrag;      /**< Shared active drag state */
        DragTarget *mDragTargetUnderMouse = nullptr;
        gcn::Font *mGuiFont;                  /**< The global GUI font */
        gcn::Font *mInfoParticleFont;         /**< Font for Info Particles*/
        bool mCustomCursor = false;           /**< Show custom cursor */
        float mCustomCursorScale = 1.0f;
        std::vector<SDL_Cursor *> mSystemMouseCursors;
        std::vector<SDL_Cursor *> mCustomMouseCursors;
        Timer mMouseActivityTimer;
        int mMouseX = 0;
        int mMouseY = 0;
        Cursor mCursorType = Cursor::Pointer;
};

extern Gui *gui;                              /**< The GUI system */
extern SDLInput *guiInput;                    /**< GUI input */

/**
 * Bolded text font
 */
extern gcn::Font *boldFont;

/**
 * Monospaced text font
 */
extern gcn::Font *monoFont;
