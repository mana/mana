/**

	The Mana World
	Copyright 2004 The Mana World Development Team

    This file is part of The Mana World.

    The Mana World is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    The Mana World is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Mana World; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "log.h"

int warning_n; // Keep warning number

#define LOG_FILE "./docs/tmw.log"
#define LOG_VERSION "0.3"

void init_log() {
#ifdef WIN32
  remove(LOG_FILE);
#else 
	unlink(LOG_FILE);
#endif

  push_config_state();
  set_config_file(LOG_FILE);
#ifdef WIN32 // for the moment i cant find a valuable unix function for _strdate // Sull
	char date[9];
  set_config_string("Core", "date", _strdate(date));
#endif
	set_config_string("Core", "log_version", LOG_VERSION);
	set_config_string("Core", "Allegro_ID", allegro_id);
	switch(os_type) {
		case OSTYPE_UNKNOWN:
			set_config_string("Core", "Os", "unknown, or regular MSDOS");
			break;
		case OSTYPE_WIN3:
			set_config_string("Core", "Os", "Windows 3.1 or earlier");
			break;
		case OSTYPE_WIN95:
			set_config_string("Core", "Os", "Windows 95");
			break;
		case OSTYPE_WIN98:
			set_config_string("Core", "Os", "Windows 98");
			break;
		case OSTYPE_WINME:
			set_config_string("Core", "Os", "Windows ME");
			break;
		case OSTYPE_WINNT:
			set_config_string("Core", "Os", "Windows NT");
			break;
		case OSTYPE_WIN2000:
			set_config_string("Core", "Os", "Windows 2000");
			break;
		case OSTYPE_WINXP:
			set_config_string("Core", "Os", "Windows XP");
			break;
		case OSTYPE_OS2:
			set_config_string("Core", "Os", "OS/2");
			break;
		case OSTYPE_WARP:
			set_config_string("Core", "Os", "OS/2 Warp 3");
			break;
		case OSTYPE_DOSEMU:
			set_config_string("Core", "Os", "Linux DOSEMU");
			break;
		case OSTYPE_OPENDOS:
			set_config_string("Core", "Os", "Caldera OpenDOS");
			break;
		case OSTYPE_LINUX:
			set_config_string("Core", "Os", "Linux");
			break;
		case OSTYPE_SUNOS:
			set_config_string("Core", "Os", "SunOS/Solaris");
			break;
		case OSTYPE_FREEBSD:
			set_config_string("Core", "Os", "FreeBSD");
			break;
		case OSTYPE_NETBSD:
			set_config_string("Core", "Os", "NetBSD");
			break;
		case OSTYPE_IRIX:
			set_config_string("Core", "Os", "IRIX");
			break;
		case OSTYPE_QNX:
			set_config_string("Core", "Os", "QNX");
			break;
		case OSTYPE_UNIX:
			set_config_string("Core", "Os", "Unknown Unix variant");
			break;
		case OSTYPE_BEOS:
			set_config_string("Core", "Os", "BeOS");
			break;
		case OSTYPE_MACOS:
			set_config_string("Core", "Os", "MacOS");
			break;
		default:
			set_config_string("Core", "Os", "Unknown");
			break;
	}
	set_config_int("Core", "Os_version", os_version);
	set_config_int("Core", "Os_revision", os_revision);
	if(os_multitasking)set_config_string("Core", "Multitasking", "TRUE");
	else set_config_string("Core", "Multitasking", "FALSE");

	set_config_string("Core", "CPU_Vendor", cpu_vendor);
	switch(cpu_family) {
		case 3:
			set_config_string("Core", "CPU_Family", "386");
			break;
		case 4:
            set_config_string("Core", "CPU_Family", "486");
			break;
		case 5:
			set_config_string("Core", "CPU_Family", "Pentium");
			break;
		case 6:
			set_config_string("Core", "CPU_Family", "Pentium II/III/PRO/Athlon");
			break;
		case 15:
			set_config_string("Core", "CPU_Family", "Pentium IV");
			break;
		default:
			set_config_string("Core", "CPU_Family", "Unknown");
			set_config_int("Core", "CPU_Family_ID", cpu_family);
			break;
	}

    set_config_int("Core", "CPU_model", cpu_model);	

	set_config_int("Core", "CPU_capabilities", cpu_capabilities);
	pop_config_state();

	warning_n = 0;
}

void log(const char *log_section, const char *log_name, const char *log_text) {
    push_config_state();
    set_config_file(LOG_FILE);
	set_config_string(log_section, log_name, log_text);
	pop_config_state();
}

void log_hex(const char *log_section, const char *log_name, const short log_value) {
	push_config_state();
  set_config_file(LOG_FILE);
	set_config_hex(log_section, log_name, log_value);
	pop_config_state();
}

void log_int(const char *log_section, const char *log_name, const int log_value) {
	push_config_state();
    set_config_file(LOG_FILE);
	set_config_int(log_section, log_name, log_value);
	pop_config_state();
}

void error(const char *error_text) {
	log("Error", "Last_error", error_text);
#ifdef WIN32
	MessageBox(NULL, error_text, "Error", MB_ICONERROR|MB_OK);
#else
	printf("Error: %s", error_text);
#endif
	exit(1);
}

void warning(const char *warning_text) {
	char warning_name[40];
	sprintf(warning_name, "warning_%i", warning_n);
	log("Error", warning_name, warning_text);
}

void status(const char *status_text) {
#ifdef DEBUG
	log("Status", "last_function", status_text);
#endif
}
