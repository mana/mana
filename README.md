THE MANA CLIENT
===============

With the Mana client you can play The Mana World and related games. The Mana
World is a free 2D open source MMORPG. This client supports games hosted by
either tmwAthena or manaserv.

The Mana client is written in C++ and builds upon:

- SDL2, SDL2\_image, SDL2\_mixer, SDL2\_ttf, SDL2\_net (Media framework)
- Guichan (GUI framework)
- libxml2 (XML parsing and writing)
- PhysicsFS (Data files)
- ENet (UDP networking library)
- libcurl (HTTP downloads)
- zlib (Archives)

See the `AUTHORS` file for the list of developers.


Account Creation
----------------

To create an account you can usually press the "Register" button after choosing
your server. When this doesn't work, visit the website of the server you'd like
to register on, since they may be using an online registration form instead.

Keyboard and Mouse Input
------------------------

Use arrow keys to move around. Other keys:

```
- Ctrl               attack
- F1                 toggle the online help
- F2                 toggle profile window
- F3                 toggle inventory window
- F4                 toggle equipment window
- F5                 toggle skills window
- F6                 toggle minimap
- F7                 toggle chat window
- F8                 toggle shortcut window
- F9                 show setup window
- F10                toggle debug window
- Alt + 0-9          show emotions / usuable at skill level 2.
- Alt + S            sit down / stand up / usuable at skill level 3.
- Alt + F            toggle debug pathfinding feature
- Alt + P            take screenshot
- Alt + T            turns on anti-trade function / usuable at skill level 1.
- A                  target nearest monster
- H                  hide all non-sticky windows
- G or Z             pick up item
- Enter              focus chat window / send message
```

Left click to execute default action: walk, pick up an item, attack a monster
and talk to NPCs. Right click to open a context menu. Holding left Shift
prevents walking when attacking.


Chat Commands
-------------

Certain server interaction requires the use of slash-commands in the chat
window. Here's a list of common commands:

```
- /help              Displays the list of commands
- /clear             clears the chat window
- /where             displays the map name your currently on
- /whisper           send a private msg to another player
                        (format: /whisper <charname> <message>)
                     If the <nick> has spaces in it, enclose it in double
                        quotes e.g. /whisper "char name" <message>
- /record            Records the Chat output
                     (format: /record <filename to write to> starts the record
                        session /record again
                      stops the session)
- /party             <command> <params>: Party commands
 - /party new         creates a new party /party new <party name>
 - /party create      creates a new party /party create <party name>
 - /party prefix      This commands sets the party prefix character
                         /party prefix <prefix-char>
                      "/party prefix" reports the current party prefix
                         character
 - /party leave       This command causes the player to leave the party.
Type /help party <option> for further help.
- /present           This command gets a list of players within hearing
- /toggle            make the chatlog lose focus on a blank line or after
                       message. (format: /toggle <option>, where option can be
                       '1', 'y' or 't' to make the chatlog lose focus on a
                       blank line, and '0', 'n' or 'f' to make the chatlog lose
                       focus after every message.  /toggle displays the status)
```

For more information, type `/help <command>`.


Support
-------

If you're having issues with this client, feel free to report them to us.
There is a bug tracker at https://github.com/mana/mana/issues and you can find
us on IRC on irc.libera.chat in the #mana channel.

If you have feedback about a specific game that uses the Mana client, be sure
to contact the developers of the game instead.

Compiling the Client
--------------------

Installing the dependencies on Ubuntu:

    sudo apt install build-essential cmake \
                     libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev \
                     libsdl2-net-dev libsdl2-ttf-dev \
                     libcurl4-openssl-dev libphysfs-dev libxml2-dev \
                     libguichan-dev libpng-dev gettext

Installing the dependencies on macOS (with Homebrew):

    brew install sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf \
                 physfs curl

Installing the dependencies on Fedora:

    sudo dnf install gcc-c++ cmake physfs-devel libcurl-devel guichan-devel \
                     SDL2_image-devel SDL2_mixer-devel SDL2_net-devel \
                     SDL2_ttf-devel

Installing the dependencies on Windows (using MSYS2 UCRT64):

    pacman -S mingw-w64-ucrt-x86_64-gcc \
              mingw-w64-ucrt-x86_64-cmake \
              mingw-w64-ucrt-x86_64-physfs \
              mingw-w64-ucrt-x86_64-curl \
              mingw-w64-ucrt-x86_64-SDL2_image \
              mingw-w64-ucrt-x86_64-SDL2_mixer \
              mingw-w64-ucrt-x86_64-SDL2_net \
              mingw-w64-ucrt-x86_64-SDL2_ttf \
              mingw-w64-ucrt-x86_64-libxml2

Once the dependencies are installed, use CMake:

    cmake -B build .
    cmake --build build

This produces an executable in `build/src/mana`. If running it without
installing, be sure to run it from the repository root so that it can find its
data files.

See `README.cmake` for additional information.
