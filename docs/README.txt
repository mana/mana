Release info:
    - Version: 0.0.9
    - Date: 29/1/2005

  Development team:
    - Elven Programmer (Admin)
    - Hammerbear (Admin)
    - kth5 (Programmer)
    - SimEdw (Programmer)
    - Ultramichy (Game server admin)
    - Rotonen (Background story and music)
    - Bertram (Programmer)

  Special thanks:
    - Sull (Linux Developer)
    - Neorice (Pixel Artist)
    - natsuki (Win32 Developer)
    - Genis (Win32 Developer)
    - RRC (Graphic Designer)
    - necromus (Pixel Artist)
    - LordNev (Artist)
    - jui-feng for helping in server development and php scripts.

  Powered by:
    - SDL
    - SDL_image
    - SDL_mixer
    - Guichan (gui framework)


0. Index
--------

- 1. Account
- 2. Commands
- 3. Version history

1. Account
----------

To create an account, add "_M" after your username when you login for the first time and enter a password of your choice.
Remember: the server is only for development or demo purposes (Thanks again to Ultramichy for hosting the server).

2. Commands
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

3. Version history
------------------

0.0.9
- Replaced Allegro library with SDL, SDL_image and SDL_mixer
- A huge amount of small GUI improvements
- Lots of internal changes including a resource manager

0.0.8a
- Added controlling player with arrow keys
- Rewrote gui with Guichan
- added scripts for easy autotools

0.0.8
- added sound support for macosx
- added primitive inventory
- added action failed messages for all skills and known reasons
  (0x0110 handled)
- added sit capability with the right level
- added "dead" message and restart packet
- added skill dialog
- added skill update (0x0141 handled)
- added colored text for damage
- added monster death animation
- added icon to win32 executable
- added 'enter' key support in login screen
- added RLE sprites for tiles
- added collision detection with beings
- fix for mapserver segfault-bug in chatsystem (broken pipe for all users on 
  screen)
- partially added TMW GUI to inventory
- added soundpool that can preload, play and remove items to/from ram
- added tmw_bar_proc
- added tmw_plus_proc && possibility to increase stats point, at each new level
- added increase skill level dialog
- added 2xSaI engine
- added patch client v0.1 to Win32 version


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