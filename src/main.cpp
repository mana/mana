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

#include "main.h"

#include "utils/gettext.h"

#include "client.h"

#include <libxml/parser.h>

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
        << _("Options:") << endl
        << _("  -v --version        : Display the version") << endl
        << _("  -h --help           : Display this help") << endl
        << _("  -C --config-dir     : Configuration directory to use") << endl
        << _("  -U --username       : Login with this username") << endl
        << _("  -P --password       : Login with this password") << endl
        << _("  -c --character      : Login with this character") << endl
        << _("  -s --server         : Login server name or IP") << endl
        << _("  -p --port           : Login server port") << endl
        << _("     --update-host    : Use this update host") << endl
        << _("  -D --default        : Choose default character server and "
                                     "character") << endl
        << _("  -u --skip-update    : Skip the update downloads") << endl
        << _("  -d --data           : Directory to load game data from") << endl
        << _("  -L --localdata-dir  : Directory to use as local data directory") << endl
        << _("     --screenshot-dir : Directory to store screenshots") << endl
#ifdef USE_OPENGL
        << _("     --no-opengl      : Disable OpenGL for this session") << endl;
#endif
}

static void printVersion()
{
    std::cout << strprintf("Mana client %s", FULL_VERSION) << std::endl;
}

static void parseOptions(int argc, char *argv[], Client::Options &options)
{
    const char *optstring = "hvud:U:P:Dc:p:C:L:";

    const struct option long_options[] = {
        { "config-dir",     required_argument, 0, 'C' },
        { "data",           required_argument, 0, 'd' },
        { "default",        no_argument,       0, 'D' },
        { "password",       required_argument, 0, 'P' },
        { "character",      required_argument, 0, 'c' },
        { "help",           no_argument,       0, 'h' },
        { "localdata-dir",  required_argument, 0, 'L' },
        { "update-host",    required_argument, 0, 'H' },
        { "port",           required_argument, 0, 'p' },
        { "server",         required_argument, 0, 's' },
        { "skip-update",    no_argument,       0, 'u' },
        { "username",       required_argument, 0, 'U' },
        { "no-opengl",      no_argument,       0, 'O' },
        { "version",        no_argument,       0, 'v' },
        { "screenshot-dir", required_argument, 0, 'i' },
        { 0 }
    };

    while (optind < argc)
    {
        int result = getopt_long(argc, argv, optstring, long_options, NULL);

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
                options.serverPort = (short) atoi(optarg);
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
            case 'S':
                options.localDataDir = optarg;
                break;
            case 'O':
                options.noOpenGL = true;
                break;
            case 'i':
                options.screenshotDir = optarg;
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

#ifdef WIN32
extern "C" char const *_nl_locale_name_default(void);
#endif

static void initInternationalization()
{
#if ENABLE_NLS
#ifdef WIN32
    putenv(("LANG=" + std::string(_nl_locale_name_default())).c_str());
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

static void xmlNullLogger(void *ctx, const char *msg, ...)
{
    // Does nothing, that's the whole point of it
}

// Initialize libxml2 and check for potential ABI mismatches between
// compiled version and the shared library actually used.
static void initXML()
{
    xmlInitParser();
    LIBXML_TEST_VERSION;

    // Suppress libxml2 error messages
    xmlSetGenericErrorFunc(NULL, xmlNullLogger);
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

    if (options.printHelp)
    {
        printHelp();
        return 0;
    }
    else if (options.printVersion)
    {
        printVersion();
        return 0;
    }

    initInternationalization();

    // Initialize PhysicsFS
    if (!PHYSFS_init(argv[0])) {
        std::cout << "Error while initializing PhysFS: "
                << PHYSFS_getLastError() << std::endl;
        return 1;
    }
    atexit((void(*)()) PHYSFS_deinit);

    initXML();

    Client client(options);
    return client.exec();
}
