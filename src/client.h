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

#ifndef CLIENT_H
#define CLIENT_H

#include "net/serverinfo.h"

#include "configlistener.h"

#include <guichan/actionlistener.hpp>

#include <string>

#include <SDL.h>
#include <SDL_framerate.h>

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

class ErrorListener : public gcn::ActionListener
{
    public:
        void action(const gcn::ActionEvent &event);
};

extern std::string errorMessage;
extern ErrorListener errorListener;
extern LoginData loginData;

/**
 * Returns elapsed time. (Warning: supposes the delay is always < 100 seconds)
 */
int get_elapsed_time(int start_time);

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
class Client : public ConfigListener, public gcn::ActionListener
{
public:
    /**
     * A structure holding the values of various options that can be passed
     * from the command line.
     */
    struct Options
    {
        Options():
            printHelp(false),
            printVersion(false),
            skipUpdate(false),
            chooseDefault(false),
            noOpenGL(false),
            serverPort(0)
        {}

        bool printHelp;
        bool printVersion;
        bool skipUpdate;
        bool chooseDefault;
        bool noOpenGL;
        std::string username;
        std::string password;
        std::string character;
        std::string brandingPath;
        std::string updateHost;
        std::string dataPath;
        std::string configDir;
        std::string localDataDir;
        std::string screenshotDir;

        std::string serverName;
        short serverPort;
    };

    Client(const Options &options);
    ~Client();

    /**
     * Provides access to the client instance.
     */
    static Client *instance() { return mInstance; }

    int exec();

    static void setState(State state)
    { instance()->state = state; }

    static State getState()
    { return instance()->state; }

    static const std::string &getConfigDirectory()
    { return instance()->configDir; }

    static const std::string &getLocalDataDirectory()
    { return instance()->localDataDir; }

    static const std::string &getScreenshotDirectory()
    { return instance()->screenshotDir; }

    void optionChanged(const std::string &name);
    void action(const gcn::ActionEvent &event);

private:
    void initHomeDir(const Options &options);
    void initConfiguration(const Options &options);
    void initUpdatesDir();
    void initScreenshotDir(const std::string &dir);

    void accountLogin(LoginData *loginData);

    static Client *mInstance;

    Options options;

    std::string configDir;
    std::string localDataDir;
    std::string updateHost;
    std::string updatesDir;
    std::string screenshotDir;

    ServerInfo currentServer;

    Window *currentDialog;
    QuitDialog *quitDialog;
    Desktop *desktop;
    Button *setupButton;

    State state;
    State oldstate;

    SDL_Surface *icon;

    SDL_TimerID mLogicCounterId;
    SDL_TimerID mSecondsCounterId;

    bool mLimitFps;
    FPSmanager mFpsManager;
};

#endif // CLIENT_H
