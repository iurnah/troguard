#ifndef _COMMON_H
#define _COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

#define APPNAME "bash"
//#define APPNAME "pingus"
//#define APPNAME "sdl-ball"
//#define APPNAME "kpat"
//#define APPNAME "abraca"
//#define APPNAME ""
//#define APPNAME ""
//#define APPNAME ""
//#define APPNAME ""
//#define APPNAME ""

//#define APPNAME "openshot"
//#define APPNAME "lives"
//#define APPNAME "iriverter"
//#define APPNAME "kino"
//#define APPNAME "pitivi"
//#define APPNAME "videocut"
//#define APPNAME "winff"
//#define APPNAME "arista-gtk"
//#define APPNAME "kdenlive"
//#define APPNAME "curlew"

//#define APPNAME "grpn"
//#define APPNAME "gcalctool"
//#define APPNAME "EdenMath"
//#define APPNAME "speedcrunch"
//#define APPNAME "kcalc"
//#define APPNAME "keurocalc"
//#define APPNAME "extcalc"
//#define APPNAME "gip"
//#define APPNAME "galculator"
//#define APPNAME "gnome-genius"
//#define APPNAME "qalculate-gtk"


//#define APPNAME "audacity"
//#define APPNAME "avidemux"
//#define APPNAME "dvbcut"
//#define APPNAME "oggconvert"
//#define APPNAME "kwave"
//#define APPNAME "wavbreaker"
//#define APPNAME "mp3splt-gtk"
//#define APPNAME "mhwaveedit"
//#define APPNAME "fillmore" 
//#define APPNAME "soundconverter"

//#define APPNAME "smplayer"
//#define APPNAME "vlc"
//#define APPNAME "audacious"
//#define APPNAME "quodlibet"
//#define APPNAME "gmusicbrowser"
//#define APPNAME "qmmp"
//#define APPNAME "abraca"
//#define APPNAME "amarok"
//#define APPNAME "guayadeque"
//#define APPNAME "aqualung"

//#define APPNAME "gimp"
//#define APPNAME "pinta"
//#define APPNAME "imagej"
//#define APPNAME "inkscape"
//#define APPNAME "kolourpaint"
//#define APPNAME "rawtherapee"
//#define APPNAME "mypaint"
//#define APPNAME "gpaint"
//#define APPNAME "gnome-paint"
//#define APPNAME "pencil"

//#define APPNAME "anjuta"
//#define APPNAME "codelite"
//#define APPNAME "codeblocks"
//#define APPNAME "netbeans"
//#define APPNAME "monodevelop"
//#define APPNAME "kdevelop"
//#define APPNAME "spyder"
//#define APPNAME "monkeystudio"
//#define APPNAME "drracket"
//#define APPNAME "idle"
//#define APPNAME "eclipse"

//seperation::
//#define APPNAME "skype"
//#define APPNAME "kmess"
//#define APPNAME "emesene"
//#define APPNAME "kopete"
//#define APPNAME "pidgin"
//#define APPNAME "psi"
//#define APPNAME "gajim"
//#define APPNAME "empathy"
//#define APPNAME "qutim"
//#define APPNAME "amsn"  //:: this guy really weired
//#define APPNAME "mercury"
//saperation::
//#define APPNAME "xmoto"
//#define APPNAME "bovo"
//#define APPNAME "palapeli"
//#define APPNAME "blobby"
//#define APPNAME "hex-a-hop"
//#define APPNAME "gweled"
//#define APPNAME "mines"
//#define APPNAME "supertux"
//#define APPNAME "sdl-ball"
//#define APPNAME "eboard"
//#define APPNAME "frozen-bubble"
//#define APPNAME "hedgewars"
//#define APPNAME "supertuxkart"
//#define APPNAME "gedit"
//#define APPNAME "scite"
//#define APPNAME "efte"
//#define APPNAME "editra"
//#define APPNAME "pico"
//#define APPNAME "aoeui"
//#define APPNAME "abiword"
//#define APPNAME "texmacs"
//#define APPNAME "xemacs"
//#define APPNAME "emacs"
//#define APPNAME "vi"
//#define APPNAME "tea"
//#define APPNAME "jed"
//#define APPNAME "lyx"

//#define APPNAME "seamonkey"
//#define APPNAME "Dooble"
//#define APPNAME "surf"
//#define APPNAME "konqueror"
//#define APPNAME "links"
//#define APPNAME "links2"
//#define APPNAME "netrik"
//#define APPNAME "chimera2"
//#define APPNAME "hbro"
//#define APPNAME "elinks"
//#define APPNAME "rekonq"
//#define APPNAME "xxxterm"
//#define APPNAME "arora"
//#define APPNAME "netsurf"
//#define APPNAME "chromium"
//#define APPNAME "zdclient"
//#define APPNAME "wesnoth"
//#define APPNAME "thunderbird"
//#define APPNAME "texmaker"
//#define APPNAME "sylpheed"
//#define APPNAME "sol"
//#define APPNAME "soffice.bin"
//#define APPNAME "opera"
//#define APPNAME "neverball"
//#define APPNAME "midori"
//#define APPNAME "kmail"
//#define APPNAME "kmahjongg"
//#define APPNAME "kile"
//#define APPNAME "glchess"
//#define APPNAME "geany"
//#define APPNAME "firefox"
//#define APPNAME "evolution"
//#define APPNAME "epiphany"
//#define APPNAME "chrome"
//#define APPNAME "calligrawords"
//#define APPNAME "okular"
//#define APPNAME "epdfview"
//#define APPNAME "evince"
//define APPNAME "FoxitReader"
//#define APPNAME "acroread"



#define APP "procname = \"chromium-browser\"" //this is to decide which application we are going to profile.
#define TRACE_PERIOD 10
#define EXIT_SYSCALL "exit_syscall:"
#define COMM "comm = \""
#define COMM_LEN 8
#define FILENAME "filename = \""
#define FILENAME_LEN 12 //length of the string "filename = \""
#define SYS_NEWSTAT "stat: "//exclude the sys_newstat sys_newlstat, sys_newfstat.

#define WRITE "Write"
#define READ "Read"
#define PROCESS "PROC"
#define LIBRARY "LIB"
#define IPv4_SOCK "IPv4_SOCK"


#define REG "REG"
#define SOCK_AF_UNIX "SOCK_AF_UNIX"
#define SOCK_AF_INET "SOCK_AF_INET"
#define SOCK_AF_INET6 "SOCK_AF_INET6"
#define FIFO "FIFO"
#define DIR "DIR"
#define CHR "CHR"
#define IPCMSG "IPCMSG"

using namespace std;
using std::vector;

#endif

