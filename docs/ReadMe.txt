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
- 4. F.A.Q.
- 5. Todo
- 6. Version history


1. Setup
--------

Before running the client please open tmw.ini and choose your keyboard layout so the GUI properly works.

2. Account
----------

To create an account, add _M after you username when you login for the first time and enter a password of your choice. Remember: the server is only for development or demo purposes (Thanks again to Ultramichy for hosting the server).

3. Commands
-----------

- Use arrow keys to move around.
- Use right mouse button over NPC's feet to talk to them.
- Left Ctrl to attack
- Alt + 0-9 to show emotions
- F1 to take a screenshot
- F11 raise volume
- F12 lower volume

4. F.A.Q.
---------

- CVS

Q: What's CVS?
A: It's a system where is stored the latest development version. It's not assured that it's stable, but surely it has the latest updates. (CVS version is updated quite frequently). If you want to check it out, read this guide http://themanaworld.sourceforge.net/cvs-tutorial.txt 

- Windows

Q: The client crashes frequently in Windows, how can I correctly terminate it?
A: Due to the Allegro library we're using, the application - if not closed correctly - remains running, so you should open up your task-manager (Alt+Ctrl+Canc) and terminate the "The Mana World.exe" process. This will probably be fixed soon.

- Game

Q: I always get "Unregistered ID" error message, why?
A: Is the first time you login? Then add _M at the end of the username you chose the first time you login. (You don't need to add _M the following times). If you already succesfully connected, but now getting this message, please send an e-mail to umperio@users.sourceforge.net.

Q: I always get "Wrong password" but I'm sure I typed it right, why?
A: If is the first time you login be sure you're not adding _M at the end of your username. Else please send an e-mail to umperio@users.sourceforge.net.

- Development

Q: When will the next version be released?
A: Really don't know. Usually a newer version comes out in a month, but that's not for sure. You can try asking the devs on IRC for further infos, you can try the latest CVS version or the latest CVS daily snaphost. These are no stable releases but are updated freuqently or even on a daily basis.

Q: How can I contribute?
A: There are a lot of ways:
- If you're a programer or an artist you can be part of the development team. Send an e-mail to umperio@users.sourceforge.net explaining what you want to do.
- You want to work on the server: server customization, scripts, maps. Send an e-mail to umperio@users.sourceforge.net explaining what you want to do.
- You can donate money. Follow the link in the main web page http://themanaworld.souceforge.net.
- You can be a beta tester. Just play with The Mana World and report every error to the tracker. Read for more infos on the main page http://themanaworld.sourceforge.net.
- Play with The Mana World: more players, more fun! Simple as that! ;-)

5. Todo
-------

- Replacing heyboard movement with a* pathfinding
- Improve ingame GUI look
- Add new characters
  - Hair style
  - Hair color
  - Clothes color
- Bugfixes

6. Version history
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
