#!/bin/bash

echo "Debian Package Creation...";

if [ $USER != "root" ]; # only root can create deb packages...
then
	echo "Sorry ! this must be run as root because only root can make Debian Packages !";
	echo "Aborting...";
	exit;
fi

make clean;
make;

if [ ! -x ./tmw ]; # if tmw doesn't exist
then
	echo "Compilation failed somewhere. Please try to correct errors given by gcc output...";
	echo "Aborting...";
	exit;
fi

if [ ! -f ./tmw.ini ]; # if tmw.ini doesn't exist
then
	echo "Can't find the tmw.ini file. Maybe this tmw version is uncomplete...";
	echo "Aborting...";
	exit;
fi

if [ ! -d ./data ]; # if the data folder doesn't exist...
then
	echo "Can't find the Mana World datas..."
	echo "Aborting...";
	exit;
fi

echo "The Mana World seems to have compiled successfully.";
echo "Now creating Debian Repository...";

rm -rf Debian;
mkdir Debian 0>/dev/null 1>/dev/null 2>/dev/null;

if [ ! -d ./Debian ]; # if the Debian folder doesn't exist...
then
	echo "Can't create a Debian folder here...";
	echo "Have you got enough rights to do that ?"
	echo "Aborting...";
	exit;
fi

TMW_VERSION=`grep "CORE_VERSION" ./src/main.h | cut -d'"' -f2`;
echo "Version :" $TMW_VERSION;
mkdir `echo 'Debian/manaworld_'$TMW_VERSION'_i386'`;
cd `echo 'Debian/manaworld_'$TMW_VERSION'_i386'`;
mkdir DEBIAN;
mkdir usr;
mkdir usr/share;
mkdir usr/share/manaworld;
#cp ../../tmw.ini ./usr/share/manaworld;
cp ../../tmw ./usr/share/manaworld;
cp ../../keyboard.dat ./usr/share/manaworld;
cp -a ../../data ./usr/share/manaworld;
cp -a ../../docs ./usr/share/manaworld;

# Creating a good link to launch the game
touch ./usr/share/manaworld/manaworld;
echo "#!/bin/sh" >>./usr/share/manaworld/manaworld;
echo "cd /usr/share/manaworld;" >>./usr/share/manaworld/manaworld;
echo "./tmw;" >>./usr/share/manaworld/manaworld;
echo "cd -" >>./usr/share/manaworld/manaworld;

chmod +x ./usr/share/manaworld/manaworld;
# End of the link

echo "Creating Rules for the Debian package..."
#echo "Set the variables value at the begining of this script if you want to change something...";

touch DEBIAN/postinst;
echo "#!/bin/sh" >>DEBIAN/postinst;
echo 'if [ "$1" = "configure" ]; then' >>DEBIAN/postinst;
echo '   chmod -R 777 /usr/share/manaworld/docs;' >>DEBIAN/postinst;
echo '   if [ -d /usr/games ]; then' >>DEBIAN/postinst;
echo '      cp /usr/share/manaworld/manaworld /usr/games/manaworld;' >>DEBIAN/postinst;
echo '      chmod +x /usr/share/manaworld/manaworld;' >>DEBIAN/postinst;
echo '   fi' >>DEBIAN/postinst;
echo 'fi' >>DEBIAN/postinst;

chmod 755 DEBIAN/postinst;
# End of postinst script...

touch DEBIAN/prerm;
echo '#!/bin/sh' >>DEBIAN/prerm;
echo 'if [ \( "$1" = "upgrade" -o "$1" = "remove" \) ]; then' >>DEBIAN/prerm;
echo '   if [ -d /usr/games ]; then' >>DEBIAN/prerm;
echo '      rm -rf /usr/games/manaworld;' >>DEBIAN/prerm;
echo '   fi' >>DEBIAN/prerm;
echo 'fi' >>DEBIAN/prerm;

chmod 755 DEBIAN/prerm;
# End of prerm script...

touch DEBIAN/control
echo 'Package: manaworld' >>DEBIAN/control;
echo 'Version: '$TMW_VERSION >>DEBIAN/control;
echo 'Section: bin' >>DEBIAN/control;
echo 'Priority: optional' >>DEBIAN/control;
echo 'Architecture: i386' >>DEBIAN/control;
echo 'Depends: libsdl1.2debian (>= 1.2.7), libsdl-image1.2 (>= 1.2.3), libsdl-mixer1.2 (>= 1.2.5), libguichan (>= 0.2.0)' >>DEBIAN/control;
echo 'Suggests: Nothing' >>DEBIAN/control;
echo 'Installed-Size: 1200' >>DEBIAN/control;
echo 'Maintainer: Ferreira Yohann <bertram@cegetel.net>' >>DEBIAN/control;
echo 'Description: The Mana World is a Great Online Game based upon the Seiken Densetsu Serie.' >>DEBIAN/control;
echo ' It has its own universe, and and its own character management system, which will' >>DEBIAN/control;
echo ' give you the opportunity to play in a 2D heroic-fantasy world forever.' >>DEBIAN/control;
echo ' .' >>DEBIAN/control;
echo ' You will find the not officially supported dependencies of this package on the website. Install them before trying to install this one.' >>DEBIAN/control;
echo ' You need : libsdldebian1.2-all, libsdl-image1.2, and libsdl-mixer1.2 given by any Debian mirrors...' >>DEBIAN/control;
echo ' You also need : libguichan which can be found on : http://guichan.darkbits.org/downloads.shtml' >>DEBIAN/control
echo ' Look at the website for further informations...' >>DEBIAN/control;
echo ' .' >>DEBIAN/control;
echo ' Authors:' >>DEBIAN/control;
echo ' Elven Programmer <elvenprogrammer@sourceforge.net>' >>DEBIAN/control;
echo ' Kth5 <kth5@gawab.com>' >>DEBIAN/control;
echo ' Ultramichy <celdron15@hotmail.com>' >>DEBIAN/control;
echo ' SimEdw <simon@crossnet.se>' >>DEBIAN/control;
echo ' Rotonen <j_orponen@hotmail.com>' >>DEBIAN/control;
echo ' Chetic <Chetic@gmail.com>' >>DEBIAN/control;
echo ' HammerBear' >>DEBIAN/control
echo ' Bertram' >>DEBIAN/control
echo ' .' >>DEBIAN/control;
echo ' Web Site: http://themanaworld.sourceforge.net/' >>DEBIAN/control;
# End of control file...

echo "Rules created..."
echo "Creating Debian Package..."
cd ..
chown -R root:root `echo 'manaworld_'$TMW_VERSION'_i386'`;
dpkg --build `echo 'manaworld_'$TMW_VERSION'_i386'`;
cp `echo 'manaworld_'$TMW_VERSION'_i386.deb'` ..;
cd ..
echo "Cleaning ...";
rm -rf Debian;
echo "You Debian Package is normally ready :"
echo "His name is : manaworld_"$TMW_VERSION"_i386.deb";
echo "Don't forget you need libsdldebian1.2, lisdl-mixer1.2, libsdl-image1.2 and libguichan to install manaworld.";
echo "You can find libguichan in http://guichan.darkbits.org/downloads.shtml";
echo "End of Debian Creation...";

