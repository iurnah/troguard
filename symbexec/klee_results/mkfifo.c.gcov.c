        -:    0:Source:../../src/mkfifo.c
        -:    0:Graph:mkfifo.gcno
        -:    0:Data:mkfifo.gcda
        -:    0:Runs:1
        -:    0:Programs:1
        -:    1:/* mkfifo -- make fifo's (named pipes)
        -:    2:   Copyright (C) 90, 91, 1995-2008 Free Software Foundation, Inc.
        -:    3:
        -:    4:   This program is free software: you can redistribute it and/or modify
        -:    5:   it under the terms of the GNU General Public License as published by
        -:    6:   the Free Software Foundation, either version 3 of the License, or
        -:    7:   (at your option) any later version.
        -:    8:
        -:    9:   This program is distributed in the hope that it will be useful,
        -:   10:   but WITHOUT ANY WARRANTY; without even the implied warranty of
        -:   11:   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        -:   12:   GNU General Public License for more details.
        -:   13:
        -:   14:   You should have received a copy of the GNU General Public License
        -:   15:   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
        -:   16:
        -:   17:/* David MacKenzie <djm@ai.mit.edu>  */
        -:   18:
        -:   19:#include <config.h>
        -:   20:#include <stdio.h>
        -:   21:#include <getopt.h>
        -:   22:#include <sys/types.h>
        -:   23:#include <selinux/selinux.h>
        -:   24:
        -:   25:#include "system.h"
        -:   26:#include "error.h"
        -:   27:#include "modechange.h"
        -:   28:#include "quote.h"
        -:   29:
        -:   30:/* The official name of this program (e.g., no `g' prefix).  */
        -:   31:#define PROGRAM_NAME "mkfifo"
        -:   32:
        -:   33:#define AUTHORS "David MacKenzie"
        -:   34:
        -:   35:/* The name this program was run with. */
        -:   36:char *program_name;
        -:   37:
        -:   38:static struct option const longopts[] =
        -:   39:{
        -:   40:  {GETOPT_SELINUX_CONTEXT_OPTION_DECL},
        -:   41:  {"mode", required_argument, NULL, 'm'},
        -:   42:  {GETOPT_HELP_OPTION_DECL},
        -:   43:  {GETOPT_VERSION_OPTION_DECL},
        -:   44:  {NULL, 0, NULL, 0}
        -:   45:};
        -:   46:
        -:   47:void
    #####:   48:usage (int status)
        -:   49:{
    #####:   50:  if (status != EXIT_SUCCESS)
    #####:   51:    fprintf (stderr, _("Try `%s --help' for more information.\n"),
        -:   52:	     program_name);
        -:   53:  else
        -:   54:    {
    #####:   55:      printf (_("Usage: %s [OPTION] NAME...\n"), program_name);
    #####:   56:      fputs (_("\
        -:   57:Create named pipes (FIFOs) with the given NAMEs.\n\
        -:   58:\n\
        -:   59:"), stdout);
    #####:   60:      fputs (_("\
        -:   61:  -Z, --context=CTX  set the SELinux security context of each NAME to CTX\n\
        -:   62:"), stdout);
    #####:   63:      fputs (_("\
        -:   64:Mandatory arguments to long options are mandatory for short options too.\n\
        -:   65:"), stdout);
    #####:   66:      fputs (_("\
        -:   67:  -m, --mode=MODE   set file permission bits to MODE, not a=rw - umask\n\
        -:   68:"), stdout);
    #####:   69:      fputs (HELP_OPTION_DESCRIPTION, stdout);
    #####:   70:      fputs (VERSION_OPTION_DESCRIPTION, stdout);
    #####:   71:      emit_bug_reporting_address ();
        -:   72:    }
    #####:   73:  exit (status);
        -:   74:}
        -:   75:
        -:   76:int
        1:   77:main (int argc, char **argv)
        -:   78:{
        -:   79:  mode_t newmode;
        1:   80:  char const *specified_mode = NULL;
        1:   81:  int exit_status = EXIT_SUCCESS;
        -:   82:  int optc;
        1:   83:  security_context_t scontext = NULL;
        -:   84:
        -:   85:  initialize_main (&argc, &argv);
        1:   86:  program_name = argv[0];
        1:   87:  setlocale (LC_ALL, "");
        -:   88:  bindtextdomain (PACKAGE, LOCALEDIR);
        -:   89:  textdomain (PACKAGE);
        -:   90:
        1:   91:  atexit (close_stdout);
        -:   92:
        2:   93:  while ((optc = getopt_long (argc, argv, "m:Z:", longopts, NULL)) != -1)
        -:   94:    {
    #####:   95:      switch (optc)
        -:   96:	{
        -:   97:	case 'm':
    #####:   98:	  specified_mode = optarg;
    #####:   99:	  break;
        -:  100:	case 'Z':
    #####:  101:	  scontext = optarg;
    #####:  102:	  break;
    #####:  103:	case_GETOPT_HELP_CHAR;
    #####:  104:	case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);
        -:  105:	default:
    #####:  106:	  usage (EXIT_FAILURE);
        -:  107:	}
        -:  108:    }
        -:  109:
        1:  110:  if (optind == argc)
        -:  111:    {
    #####:  112:      error (0, 0, _("missing operand"));
    #####:  113:      usage (EXIT_FAILURE);
        -:  114:    }
        -:  115:
        1:  116:  if (scontext && setfscreatecon (scontext) < 0)
    #####:  117:    error (EXIT_FAILURE, errno,
        -:  118:	   _("failed to set default file creation context to %s"),
        -:  119:	   quote (scontext));
        -:  120:
        1:  121:  newmode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        1:  122:  if (specified_mode)
        -:  123:    {
    #####:  124:      struct mode_change *change = mode_compile (specified_mode);
    #####:  125:      if (!change)
    #####:  126:	error (EXIT_FAILURE, 0, _("invalid mode"));
    #####:  127:      newmode = mode_adjust (newmode, false, umask (0), change, NULL);
    #####:  128:      free (change);
    #####:  129:      if (newmode & ~S_IRWXUGO)
    #####:  130:	error (EXIT_FAILURE, 0,
        -:  131:	       _("mode must specify only file permission bits"));
        -:  132:    }
        -:  133:
        2:  134:  for (; optind < argc; ++optind)
        1:  135:    if (mkfifo (argv[optind], newmode) != 0)
        -:  136:      {
    #####:  137:	error (0, errno, _("cannot create fifo %s"), quote (argv[optind]));
    #####:  138:	exit_status = EXIT_FAILURE;
        -:  139:      }
        -:  140:
        1:  141:  exit (exit_status);
        -:  142:}
