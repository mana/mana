The Mana Client
===============

With the Mana client you can play The Mana World and related games. The Mana
World is a free 2D open source MMORPG. This client supports games hosted by
either tmwAthena or manaserv.

The Mana client is written in C++17 and builds upon:

- [SDL2](https://www.libsdl.org/),
  [SDL2\_image](https://github.com/libsdl-org/SDL_image),
  [SDL2\_mixer](https://github.com/libsdl-org/SDL_mixer),
  [SDL2\_ttf](https://github.com/libsdl-org/SDL_ttf),
  [SDL2\_net](https://github.com/libsdl-org/SDL_net) (media and networking)
- [Guichan 0.8](https://github.com/darkbitsorg/guichan) (GUI framework)
- [libxml2](https://gitlab.gnome.org/GNOME/libxml2/-/wikis/home) (XML parsing and writing)
- [PhysicsFS 3](https://icculus.org/physfs/) (data files)
- [ENet 1.3](http://sauerbraten.org/enet/) (UDP networking library)
- [GNU gettext](https://www.gnu.org/software/gettext/) (translations)
- libcurl (HTTP downloads)
- [zlib 1.2](https://zlib.net/) (compression)

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

Apart from the dependencies listed above you will need a compiler and
[CMake](https://cmake.org/) installed.

Installing the dependencies on Ubuntu:

    sudo apt install build-essential cmake \
                     libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev \
                     libsdl2-net-dev libsdl2-ttf-dev \
                     libcurl4-openssl-dev libphysfs-dev libxml2-dev \
                     libguichan-dev libpng-dev gettext

Installing the dependencies on macOS (with Homebrew):

    brew install sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf \
                 physfs curl guichan

Installing the dependencies on Fedora:

    sudo dnf install gcc-c++ cmake physfs-devel libcurl-devel guichan-devel \
                     SDL2_image-devel SDL2_mixer-devel SDL2_net-devel \
                     SDL2_ttf-devel

Installing the dependencies on Windows (using MSYS2 UCRT64):

    pacman -S mingw-w64-ucrt-x86_64-gcc \
              mingw-w64-ucrt-x86_64-cmake \
              mingw-w64-ucrt-x86_64-physfs \
              mingw-w64-ucrt-x86_64-curl-winssl \
              mingw-w64-ucrt-x86_64-SDL2_image \
              mingw-w64-ucrt-x86_64-SDL2_mixer \
              mingw-w64-ucrt-x86_64-SDL2_net \
              mingw-w64-ucrt-x86_64-SDL2_ttf \
              mingw-w64-ucrt-x86_64-libxml2

Once the dependencies are installed, use CMake:

    cmake -B build .
    cmake --build build

This produces an executable in `build/src/mana`. Be sure to run it from the
repository root so that it can find its data files.

Installing the Client
---------------------

On Linux you can run `cmake --install build`. By default all files are
installed to `/usr/local`, but you can pass a different prefix to the first
`cmake` invocation with `-DCMAKE_INSTALL_PREFIX=/path/to/prefix`.

On Windows, you can install the same way as above to put the Mana client in
your Program Files. However, you'll probably want to create an installer
instead. For MSYS2 UCRT64 builds, an NSIS installer can be produced as follows:

    cd build
    cpack

On macOS, you can create a stand-alone application bundle by installing,
similar to the Linux command above. To make it easy for others to install your
build, you can also build a DragNDrop DMG using:

    cd build
    cpack -G DragNDrop

See `README.cmake` for additional information.
