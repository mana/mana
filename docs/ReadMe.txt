  The Mana World (C) 2004

  http://themanaworld.sourceforge.net
  irc://irc.freenode.net/manaworld
  umperio@users.sourceforge.net

  Release info:
    - Version: 0.0.8 pre release
    - Date: ?/?/2004

  Development team:
    - Elven Programmer (Admin)
    - kth5 (Programmer)
    - Ultramichy (Game server admin)
    - Rotonen (Background story and music)

  Special thanks:
    - Sull (Linux Developer)
    - natsuki (Win32 Developer)
    - Genis (Win32 Developer)
    - RRC (Graphic Designer)
    - necromus (Pixel Artist)
    - LordNev (Artist)
    - jui-feng for helping in server development and php scripts.
    - SimEdw for porting TMW to MacOS X

  Powered by:
    - Allegro, a game developing library
    - AllegroFont
    - JMod


0. Index
--------

- 1. Setup
- 2. Account
- 3. Commands
- 4. Version history


1. Setup
--------

Before running the client please open tmw.ini and choose your keyboard layout so the GUI works properly.

2. Account
----------

To create an account, add "_M" after your username when you login for the first time and enter a password of your choice.
Remember: the server is only for development or demo purposes (Thanks again to Ultramichy for hosting the server).

3. Commands
-----------

- Use arrow keys to move around.
- Use right mouse button over NPC's feet to talk to them.
- Left Ctrl to attack
- Alt + 0-9 to show emotions
- F1 to take a screenshot
- F11 raise volume
- F12 lower volume
- Alt + K Show skills
- Alt + S Show stats
- Alt + I Show inventory
- F5 to sit

4. Version history
------------------

0.0.8
- Add application icon for Win32 binaries

0.0.7e

- Tons of bugfixes & improvements (again)
- MacOS X support
- confirmed support for GNU/Linux on ppc
- Added soundsystem (plays a MOD as BGM)
- Added a 2nd Map
- Added NPCs with simple dialogs
- Added warps
- Changed makefile for GCC to optimize more

0.0.7d:

- Tons of bugfixes
- Added a bit of prediction to player walking
- Added Ultramichy's server to client
- Added a walking monster
- Added a* system (still not applied to player, but only to monster walking)
- Started rewriting of chat system

0.0.7c:

- Started rewriting of graphic engine
- Fixed walking problems
- Server: features a custom map

0.0.7b:

- Completely rewrited network functions
  - Removed listening thread
  - Replaced blocking calls to socket
  - Removed #pragma pack directives
- Replaced MASkinG with own made GUI system
- Finite state machine dialog system
- Greatly reduced memory usage
- Added a lot of comments to the code
- Decreased the number of files in source code
- The code is now a little more organized and easy to understand

0.0.7a:

- Win32 binaries come with an installer
- Renew login GUI:
  - Change from dialogs to windows
  - Add option to remember last login infos
  - Add player creation dialog
- Add NPC GUI

0.0.7:

- Some kind of multiplayer
- Cross-compiler source code
- Added setup utility
- Minor changes
- Bugfixes
