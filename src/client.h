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

#ifndef CLIENT_H
#define CLIENT_H

#include "eventlistener.h"
#include "video.h"

#include "net/serverinfo.h"

#include <guichan/actionlistener.hpp>

#include <SDL.h>
#include <SDL2_framerate.h>

#include <string>

class Game;
class Button;
class Desktop;
class LoginData;
class Window;
class QuitDialog;

/**
 * Set the milliseconds value of a tick time.
 */
static const int MILLISECONDS_IN_A_TICK = 10;

//manaserv uses 9601
//static const short DEFAULT_PORT = 9601;
static const short DEFAULT_PORT = 6901;

extern volatile int fps;
extern volatile int tick_time;
extern volatile int cur_time;

extern std::string errorMessage;
extern LoginData loginData;

/**
 * @param startTime The value to check in client ticks.
 *
 * @return the elapsed time in milliseconds.
 * between startTime and the current client tick value.
 *
 * @warning This function can't handle delays > 100 seconds.
 * @see MILLISECONDS_IN_A_TICK
 * @see tick_time
 */
int get_elapsed_time(int startTime);

/**
 * Returns whether this call and the last call were done for the same
 * selected index and within a short time.
 */
bool isDoubleClick(int selected);

/**
 * All client states.
 */
enum State {
    STATE_ERROR = -1,
    STATE_START = 0,
    STATE_CHOOSE_SERVER,
    STATE_CONNECT_SERVER,
    STATE_LOGIN,
    STATE_LOGIN_ATTEMPT,
    STATE_WORLD_SELECT,           // 5
    STATE_WORLD_SELECT_ATTEMPT,
    STATE_UPDATE,
    STATE_LOAD_DATA,
    STATE_GET_CHARACTERS,
    STATE_CHAR_SELECT,            // 10
    STATE_CONNECT_GAME,
    STATE_GAME,
    STATE_CHANGE_MAP,             // Switch map-server/gameserver
    STATE_LOGIN_ERROR,
    STATE_ACCOUNTCHANGE_ERROR,    // 15
    STATE_REGISTER_PREP,
    STATE_REGISTER,
    STATE_REGISTER_ATTEMPT,
    STATE_CHANGEPASSWORD,
    STATE_CHANGEPASSWORD_ATTEMPT, // 20
    STATE_CHANGEPASSWORD_SUCCESS,
    STATE_CHANGEEMAIL,
    STATE_CHANGEEMAIL_ATTEMPT,
    STATE_CHANGEEMAIL_SUCCESS,
    STATE_UNREGISTER,             // 25
    STATE_UNREGISTER_ATTEMPT,
    STATE_UNREGISTER_SUCCESS,
    STATE_SWITCH_SERVER,
    STATE_SWITCH_LOGIN,
    STATE_SWITCH_CHARACTER,       // 30
    STATE_LOGOUT_ATTEMPT,
    STATE_WAIT,
    STATE_EXIT,
    STATE_FORCE_QUIT
};

/**
 * The core part of the client. This class initializes all subsystems, runs
 * the event loop, and shuts everything down again.
 */
class Client final : public EventListener, public gcn::ActionListener
{
public:
    /**
     * A structure holding the values of various options that can be passed
     * from the command line.
     */
    struct Options
    {
        bool printHelp = false;
        bool printVersion = false;
        bool skipUpdate = false;
        bool chooseDefault = false;
        bool noOpenGL = false;
        bool exitWithError = false;
        std::string username;
        std::string password;
        std::string character;
        std::string brandingPath;
        std::string updateHost;
        std::string dataPath;
        std::string chatLogDir;
        std::string configDir;
        std::string localDataDir;
        std::string screenshotDir;
        ServerType serverType = ServerType::UNKNOWN;

        std::string serverName;
        uint16_t serverPort = 0;
    };

    Client(const Options &options);
    ~Client() override;

    /**
     * Provides access to the client instance.
     */
    static Client *instance() { return mInstance; }

    int exec();

    /**
     * Pops up an OkDialog with the given \a title and \a message, and
     * switches to the given \a state when Ok is pressed.
     */
    void showOkDialog(const std::string &title,
                      const std::string &message,
                      State state);

    /**
     * Pops up an error dialog with the given \a message, and switches to the
     * given \a state when Ok is pressed.
     */
    void showErrorDialog(const std::string &message, State state);

    static void setState(State state)
    { instance()->mState = state; }

    static State getState()
    { return instance()->mState; }

    static const std::string &getPackageDirectory()
    { return instance()->mPackageDir; }

    static const std::string &getConfigDirectory()
    { return instance()->mConfigDir; }

    static const std::string &getLocalDataDirectory()
    { return instance()->mLocalDataDir; }

    static const std::string &getScreenshotDirectory()
    { return instance()->mScreenshotDir; }

    static Video &getVideo()
    { return instance()->mVideo; }

    void event(Event::Channel channel, const Event &event) override;
    void action(const gcn::ActionEvent &event) override;

    /**
     * Should be called after the window has been resized. Makes sure the GUI
     * and game adapt to the new size.
     */
    void videoResized(int width, int height);

    static bool isActive();
    static bool hasInputFocus();
    static bool hasMouseFocus();

private:
    void initRootDir();
    void initHomeDir();
    void initConfiguration();
    void initUpdatesDir();
    void initScreenshotDir();

    void accountLogin(LoginData *loginData);

    static Client *mInstance;

    Options mOptions;

    std::string mPackageDir;
    std::string mConfigDir;
    std::string mLocalDataDir;
    std::string mUpdateHost;
    std::string mUpdatesDir;
    std::string mScreenshotDir;
    std::string mRootDir;

    ServerInfo mCurrentServer;
    Video mVideo;

    Game *mGame = nullptr;
    Window *mCurrentDialog = nullptr;
    QuitDialog *mQuitDialog = nullptr;
    Desktop *mDesktop = nullptr;
    Button *mSetupButton = nullptr;

    State mState = STATE_CHOOSE_SERVER;
    State mOldState = STATE_START;
    State mStateAfterOkDialog;

    SDL_Surface *mIcon = nullptr;

    SDL_TimerID mLogicCounterId = 0;
    SDL_TimerID mSecondsCounterId = 0;

    bool mLimitFps = false;
    FPSmanager mFpsManager;
};

#endif // CLIENT_H
