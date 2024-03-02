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

#include "main.h"

#include "client.h"

#include "utils/gettext.h"
#include "utils/xml.h"

#include <getopt.h>
#include <iostream>
#include <physfs.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

static void printHelp()
{
    using std::endl;

    std::cout
        << _("mana [options] [mana-file]") << endl << endl
        << _("[mana-file] : The mana file is an XML file (.mana)") << endl
        << _("              used to set custom parameters") << endl
        << _("              to the mana client.")
        << endl << endl
        << _("Options:") << endl
        << _("  -v --version        : Display the version") << endl
        << _("  -h --help           : Display this help") << endl
        << _("  -C --config-dir     : Configuration directory to use") << endl
        << _("  -U --username       : Login with this username") << endl
        << _("  -P --password       : Login with this password") << endl
        << _("  -c --character      : Login with this character") << endl
        << _("  -s --server         : Login server name or IP") << endl
        << _("  -p --port           : Login server port") << endl
        << _("  -y --server-type    : Login server type") << endl
        << _("     --update-host    : Use this update host") << endl
        << _("  -D --default        : Choose default character server and "
                                     "character") << endl
        << _("  -u --skip-update    : Skip the update downloads") << endl
        << _("  -d --data           : Directory to load game data from") << endl
        << _("     --localdata-dir  : Directory to use as local data directory") << endl
        << _("     --chat-log-dir   : Chat log dir to use") << endl
        << _("     --screenshot-dir : Directory to store screenshots") << endl
#ifdef USE_OPENGL
        << _("     --no-opengl      : Disable OpenGL for this session") << endl
#endif
        ;
}

static void printVersion()
{
    std::cout << strprintf("Mana client %s", FULL_VERSION) << std::endl;
}

static void parseOptions(int argc, char *argv[], Client::Options &options)
{
    const char *optstring = "hvud:U:P:Dc:s:p:C:y:";

    const struct option long_options[] = {
        { "config-dir",     required_argument, nullptr, 'C' },
        { "data",           required_argument, nullptr, 'd' },
        { "default",        no_argument,       nullptr, 'D' },
        { "password",       required_argument, nullptr, 'P' },
        { "character",      required_argument, nullptr, 'c' },
        { "help",           no_argument,       nullptr, 'h' },
        { "localdata-dir",  required_argument, nullptr, 'L' },
        { "update-host",    required_argument, nullptr, 'H' },
        { "port",           required_argument, nullptr, 'p' },
        { "server",         required_argument, nullptr, 's' },
        { "skip-update",    no_argument,       nullptr, 'u' },
        { "username",       required_argument, nullptr, 'U' },
        { "no-opengl",      no_argument,       nullptr, 'O' },
        { "chat-log-dir",   required_argument, nullptr, 'T' },
        { "version",        no_argument,       nullptr, 'v' },
        { "screenshot-dir", required_argument, nullptr, 'i' },
        { "server-type",    required_argument, nullptr, 'y' },
        { nullptr }
    };

    while (optind < argc)
    {
        int result = getopt_long(argc, argv, optstring, long_options, nullptr);

        if (result == -1)
            break;

        switch (result)
        {
            case 'C':
                options.configDir = optarg;
                break;
            case 'd':
                options.dataPath = optarg;
                break;
            case 'D':
                options.chooseDefault = true;
                break;
            case '?': // Unknown option
            case ':': // Missing argument
                options.exitWithError = true;
                [[fallthrough]];
            case 'h':
                options.printHelp = true;
                break;
            case 'H':
                options.updateHost = optarg;
                break;
            case 'c':
                options.character = optarg;
                break;
            case 'P':
                options.password = optarg;
                break;
            case 's':
                options.serverName = optarg;
                break;
            case 'p':
                options.serverPort = static_cast<uint16_t>(atoi(optarg));
                break;
            case 'u':
                options.skipUpdate = true;
                break;
            case 'U':
                options.username = optarg;
                break;
            case 'v':
                options.printVersion = true;
                break;
            case 'L':
                options.localDataDir = optarg;
                break;
            case 'O':
                options.noOpenGL = true;
                break;
            case 'T':
                options.chatLogDir = optarg;
                break;
            case 'i':
                options.screenshotDir = optarg;
                break;
            case 'y':
                options.serverType = ServerInfo::parseType(optarg);
                if (options.serverType == ServerType::UNKNOWN)
                {
                    std::cerr << _("Invalid server type, expected one of: tmwathena, manaserv") << std::endl;
                    options.exitWithError = true;
                }
                break;
        }
    }

    // when there are still options left use the last
    // one as branding file
    if (optind < argc)
    {
        options.brandingPath = argv[optind];
    }
}

#ifdef _WIN32
extern "C" char const *_nl_locale_name_default(void);
#endif

static void initInternationalization()
{
#if ENABLE_NLS
#ifdef _WIN32
    SetEnvironmentVariable("LANG", _nl_locale_name_default());
    // mingw doesn't like LOCALEDIR to be defined for some reason
    bindtextdomain("mana", "translations/");
#else
    bindtextdomain("mana", LOCALEDIR);
#endif
    setlocale(LC_MESSAGES, "");
    bind_textdomain_codeset("mana", "UTF-8");
    textdomain("mana");
#endif
}


int main(int argc, char *argv[])
{
#if defined(DEBUG) && defined(__MINGW32__)
    // Load mingw crash handler. Won't fail if dll is not present.
    LoadLibrary("exchndl.dll");
#endif

    // Parse command line options
    Client::Options options;
    parseOptions(argc, argv, options);

    if (options.printVersion)
        printVersion();

    if (options.printHelp)
        printHelp();

    if (options.printHelp || options.printVersion || options.exitWithError)
        return options.exitWithError ? 1 : 0;

    initInternationalization();

    // Initialize PhysicsFS
    if (!PHYSFS_init(argv[0])) {
        std::cout << "Error while initializing PhysFS: "
                << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << std::endl;
        return 1;
    }
    atexit((void(*)()) PHYSFS_deinit);

    XML::init();

    Client client(options);
    return client.exec();
}
