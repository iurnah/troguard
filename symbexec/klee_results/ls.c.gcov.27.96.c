        -:    0:Source:../../src/ls.c
        -:    0:Graph:ls.gcno
        -:    0:Data:ls.gcda
        -:    0:Runs:1
        -:    0:Programs:1
        -:    1:/* `dir', `vdir' and `ls' directory listing programs for GNU.
        -:    2:   Copyright (C) 85, 88, 90, 91, 1995-2008 Free Software Foundation, Inc.
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
        -:   17:/* If ls_mode is LS_MULTI_COL,
        -:   18:   the multi-column format is the default regardless
        -:   19:   of the type of output device.
        -:   20:   This is for the `dir' program.
        -:   21:
        -:   22:   If ls_mode is LS_LONG_FORMAT,
        -:   23:   the long format is the default regardless of the
        -:   24:   type of output device.
        -:   25:   This is for the `vdir' program.
        -:   26:
        -:   27:   If ls_mode is LS_LS,
        -:   28:   the output format depends on whether the output
        -:   29:   device is a terminal.
        -:   30:   This is for the `ls' program.  */
        -:   31:
        -:   32:/* Written by Richard Stallman and David MacKenzie.  */
        -:   33:
        -:   34:/* Color support by Peter Anvin <Peter.Anvin@linux.org> and Dennis
        -:   35:   Flaherty <dennisf@denix.elk.miles.com> based on original patches by
        -:   36:   Greg Lee <lee@uhunix.uhcc.hawaii.edu>.  */
        -:   37:
        -:   38:#include <config.h>
        -:   39:#include <sys/types.h>
        -:   40:
        -:   41:#if HAVE_TERMIOS_H
        -:   42:# include <termios.h>
        -:   43:#endif
        -:   44:#if HAVE_STROPTS_H
        -:   45:# include <stropts.h>
        -:   46:#endif
        -:   47:#if HAVE_SYS_IOCTL_H
        -:   48:# include <sys/ioctl.h>
        -:   49:#endif
        -:   50:
        -:   51:#ifdef WINSIZE_IN_PTEM
        -:   52:# include <sys/stream.h>
        -:   53:# include <sys/ptem.h>
        -:   54:#endif
        -:   55:
        -:   56:#include <stdio.h>
        -:   57:#include <assert.h>
        -:   58:#include <setjmp.h>
        -:   59:#include <grp.h>
        -:   60:#include <pwd.h>
        -:   61:#include <getopt.h>
        -:   62:#include <signal.h>
        -:   63:#include <selinux/selinux.h>
        -:   64:#include <wchar.h>
        -:   65:
        -:   66:/* Use SA_NOCLDSTOP as a proxy for whether the sigaction machinery is
        -:   67:   present.  */
        -:   68:#ifndef SA_NOCLDSTOP
        -:   69:# define SA_NOCLDSTOP 0
        -:   70:# define sigprocmask(How, Set, Oset) /* empty */
        -:   71:# define sigset_t int
        -:   72:# if ! HAVE_SIGINTERRUPT
        -:   73:#  define siginterrupt(sig, flag) /* empty */
        -:   74:# endif
        -:   75:#endif
        -:   76:#ifndef SA_RESTART
        -:   77:# define SA_RESTART 0
        -:   78:#endif
        -:   79:
        -:   80:#include "system.h"
        -:   81:#include <fnmatch.h>
        -:   82:
        -:   83:#include "acl.h"
        -:   84:#include "argmatch.h"
        -:   85:#include "dev-ino.h"
        -:   86:#include "dirfd.h"
        -:   87:#include "error.h"
        -:   88:#include "filenamecat.h"
        -:   89:#include "hard-locale.h"
        -:   90:#include "hash.h"
        -:   91:#include "human.h"
        -:   92:#include "filemode.h"
        -:   93:#include "idcache.h"
        -:   94:#include "inttostr.h"
        -:   95:#include "ls.h"
        -:   96:#include "lstat.h"
        -:   97:#include "mbswidth.h"
        -:   98:#include "mpsort.h"
        -:   99:#include "obstack.h"
        -:  100:#include "quote.h"
        -:  101:#include "quotearg.h"
        -:  102:#include "same.h"
        -:  103:#include "stat-time.h"
        -:  104:#include "strftime.h"
        -:  105:#include "strverscmp.h"
        -:  106:#include "xstrtol.h"
        -:  107:#include "areadlink.h"
        -:  108:
        -:  109:#define PROGRAM_NAME (ls_mode == LS_LS ? "ls" \
        -:  110:		      : (ls_mode == LS_MULTI_COL \
        -:  111:			 ? "dir" : "vdir"))
        -:  112:
        -:  113:#define AUTHORS "Richard Stallman", "David MacKenzie"
        -:  114:
        -:  115:#define obstack_chunk_alloc malloc
        -:  116:#define obstack_chunk_free free
        -:  117:
        -:  118:/* Return an int indicating the result of comparing two integers.
        -:  119:   Subtracting doesn't always work, due to overflow.  */
        -:  120:#define longdiff(a, b) ((a) < (b) ? -1 : (a) > (b))
        -:  121:
        -:  122:#if ! HAVE_STRUCT_STAT_ST_AUTHOR
        -:  123:# define st_author st_uid
        -:  124:#endif
        -:  125:
        -:  126:enum filetype
        -:  127:  {
        -:  128:    unknown,
        -:  129:    fifo,
        -:  130:    chardev,
        -:  131:    directory,
        -:  132:    blockdev,
        -:  133:    normal,
        -:  134:    symbolic_link,
        -:  135:    sock,
        -:  136:    whiteout,
        -:  137:    arg_directory
        -:  138:  };
        -:  139:
        -:  140:/* Display letters and indicators for each filetype.
        -:  141:   Keep these in sync with enum filetype.  */
        -:  142:static char const filetype_letter[] = "?pcdb-lswd";
        -:  143:
        -:  144:/* Ensure that filetype and filetype_letter have the same
        -:  145:   number of elements.  */
        -:  146:verify (sizeof filetype_letter - 1 == arg_directory + 1);
        -:  147:
        -:  148:#define FILETYPE_INDICATORS				\
        -:  149:  {							\
        -:  150:    C_ORPHAN, C_FIFO, C_CHR, C_DIR, C_BLK, C_FILE,	\
        -:  151:    C_LINK, C_SOCK, C_FILE, C_DIR			\
        -:  152:  }
        -:  153:
        -:  154:
        -:  155:struct fileinfo
        -:  156:  {
        -:  157:    /* The file name.  */
        -:  158:    char *name;
        -:  159:
        -:  160:    /* For symbolic link, name of the file linked to, otherwise zero.  */
        -:  161:    char *linkname;
        -:  162:
        -:  163:    struct stat stat;
        -:  164:
        -:  165:    enum filetype filetype;
        -:  166:
        -:  167:    /* For symbolic link and long listing, st_mode of file linked to, otherwise
        -:  168:       zero.  */
        -:  169:    mode_t linkmode;
        -:  170:
        -:  171:    /* SELinux security context.  */
        -:  172:    security_context_t scontext;
        -:  173:
        -:  174:    bool stat_ok;
        -:  175:
        -:  176:    /* For symbolic link and color printing, true if linked-to file
        -:  177:       exists, otherwise false.  */
        -:  178:    bool linkok;
        -:  179:
        -:  180:    /* For long listings, true if the file has an access control list,
        -:  181:       or an SELinux security context.  */
        -:  182:    bool have_acl;
        -:  183:  };
        -:  184:
        -:  185:#define LEN_STR_PAIR(s) sizeof (s) - 1, s
        -:  186:
        -:  187:/* Null is a valid character in a color indicator (think about Epson
        -:  188:   printers, for example) so we have to use a length/buffer string
        -:  189:   type.  */
        -:  190:
        -:  191:struct bin_str
        -:  192:  {
        -:  193:    size_t len;			/* Number of bytes */
        -:  194:    const char *string;		/* Pointer to the same */
        -:  195:  };
        -:  196:
        -:  197:#if ! HAVE_TCGETPGRP
        -:  198:# define tcgetpgrp(Fd) 0
        -:  199:#endif
        -:  200:
        -:  201:static size_t quote_name (FILE *out, const char *name,
        -:  202:			  struct quoting_options const *options,
        -:  203:			  size_t *width);
        -:  204:static char *make_link_name (char const *name, char const *linkname);
        -:  205:static int decode_switches (int argc, char **argv);
        -:  206:static bool file_ignored (char const *name);
        -:  207:static uintmax_t gobble_file (char const *name, enum filetype type,
        -:  208:			      ino_t inode, bool command_line_arg,
        -:  209:			      char const *dirname);
        -:  210:static bool print_color_indicator (const char *name, mode_t mode, int linkok,
        -:  211:				   bool stat_ok, enum filetype type);
        -:  212:static void put_indicator (const struct bin_str *ind);
        -:  213:static void add_ignore_pattern (const char *pattern);
        -:  214:static void attach (char *dest, const char *dirname, const char *name);
        -:  215:static void clear_files (void);
        -:  216:static void extract_dirs_from_files (char const *dirname,
        -:  217:				     bool command_line_arg);
        -:  218:static void get_link_name (char const *filename, struct fileinfo *f,
        -:  219:			   bool command_line_arg);
        -:  220:static void indent (size_t from, size_t to);
        -:  221:static size_t calculate_columns (bool by_columns);
        -:  222:static void print_current_files (void);
        -:  223:static void print_dir (char const *name, char const *realname,
        -:  224:		       bool command_line_arg);
        -:  225:static void print_file_name_and_frills (const struct fileinfo *f);
        -:  226:static void print_horizontal (void);
        -:  227:static int format_user_width (uid_t u);
        -:  228:static int format_group_width (gid_t g);
        -:  229:static void print_long_format (const struct fileinfo *f);
        -:  230:static void print_many_per_line (void);
        -:  231:static void print_name_with_quoting (const char *p, mode_t mode,
        -:  232:				     int linkok, bool stat_ok,
        -:  233:				     enum filetype type,
        -:  234:				     struct obstack *stack);
        -:  235:static void prep_non_filename_text (void);
        -:  236:static void print_type_indicator (bool stat_ok, mode_t mode,
        -:  237:				  enum filetype type);
        -:  238:static void print_with_commas (void);
        -:  239:static void queue_directory (char const *name, char const *realname,
        -:  240:			     bool command_line_arg);
        -:  241:static void sort_files (void);
        -:  242:static void parse_ls_color (void);
        -:  243:void usage (int status);
        -:  244:
        -:  245:/* The name this program was run with.  */
        -:  246:char *program_name;
        -:  247:
        -:  248:/* Initial size of hash table.
        -:  249:   Most hierarchies are likely to be shallower than this.  */
        -:  250:#define INITIAL_TABLE_SIZE 30
        -:  251:
        -:  252:/* The set of `active' directories, from the current command-line argument
        -:  253:   to the level in the hierarchy at which files are being listed.
        -:  254:   A directory is represented by its device and inode numbers (struct dev_ino).
        -:  255:   A directory is added to this set when ls begins listing it or its
        -:  256:   entries, and it is removed from the set just after ls has finished
        -:  257:   processing it.  This set is used solely to detect loops, e.g., with
        -:  258:   mkdir loop; cd loop; ln -s ../loop sub; ls -RL  */
        -:  259:static Hash_table *active_dir_set;
        -:  260:
        -:  261:#define LOOP_DETECT (!!active_dir_set)
        -:  262:
        -:  263:/* The table of files in the current directory:
        -:  264:
        -:  265:   `cwd_file' points to a vector of `struct fileinfo', one per file.
        -:  266:   `cwd_n_alloc' is the number of elements space has been allocated for.
        -:  267:   `cwd_n_used' is the number actually in use.  */
        -:  268:
        -:  269:/* Address of block containing the files that are described.  */
        -:  270:static struct fileinfo *cwd_file;
        -:  271:
        -:  272:/* Length of block that `cwd_file' points to, measured in files.  */
        -:  273:static size_t cwd_n_alloc;
        -:  274:
        -:  275:/* Index of first unused slot in `cwd_file'.  */
        -:  276:static size_t cwd_n_used;
        -:  277:
        -:  278:/* Vector of pointers to files, in proper sorted order, and the number
        -:  279:   of entries allocated for it.  */
        -:  280:static void **sorted_file;
        -:  281:static size_t sorted_file_alloc;
        -:  282:
        -:  283:/* When true, in a color listing, color each symlink name according to the
        -:  284:   type of file it points to.  Otherwise, color them according to the `ln'
        -:  285:   directive in LS_COLORS.  Dangling (orphan) symlinks are treated specially,
        -:  286:   regardless.  This is set when `ln=target' appears in LS_COLORS.  */
        -:  287:
        -:  288:static bool color_symlink_as_referent;
        -:  289:
        -:  290:/* mode of appropriate file for colorization */
        -:  291:#define FILE_OR_LINK_MODE(File) \
        -:  292:    ((color_symlink_as_referent & (File)->linkok) \
        -:  293:     ? (File)->linkmode : (File)->stat.st_mode)
        -:  294:
        -:  295:
        -:  296:/* Record of one pending directory waiting to be listed.  */
        -:  297:
        -:  298:struct pending
        -:  299:  {
        -:  300:    char *name;
        -:  301:    /* If the directory is actually the file pointed to by a symbolic link we
        -:  302:       were told to list, `realname' will contain the name of the symbolic
        -:  303:       link, otherwise zero.  */
        -:  304:    char *realname;
        -:  305:    bool command_line_arg;
        -:  306:    struct pending *next;
        -:  307:  };
        -:  308:
        -:  309:static struct pending *pending_dirs;
        -:  310:
        -:  311:/* Current time in seconds and nanoseconds since 1970, updated as
        -:  312:   needed when deciding whether a file is recent.  */
        -:  313:
        -:  314:static struct timespec current_time;
        -:  315:
        -:  316:static bool print_scontext;
        -:  317:static char UNKNOWN_SECURITY_CONTEXT[] = "?";
        -:  318:
        -:  319:/* Whether any of the files has an ACL.  This affects the width of the
        -:  320:   mode column.  */
        -:  321:
        -:  322:static bool any_has_acl;
        -:  323:
        -:  324:/* The number of columns to use for columns containing inode numbers,
        -:  325:   block sizes, link counts, owners, groups, authors, major device
        -:  326:   numbers, minor device numbers, and file sizes, respectively.  */
        -:  327:
        -:  328:static int inode_number_width;
        -:  329:static int block_size_width;
        -:  330:static int nlink_width;
        -:  331:static int scontext_width;
        -:  332:static int owner_width;
        -:  333:static int group_width;
        -:  334:static int author_width;
        -:  335:static int major_device_number_width;
        -:  336:static int minor_device_number_width;
        -:  337:static int file_size_width;
        -:  338:
        -:  339:/* Option flags */
        -:  340:
        -:  341:/* long_format for lots of info, one per line.
        -:  342:   one_per_line for just names, one per line.
        -:  343:   many_per_line for just names, many per line, sorted vertically.
        -:  344:   horizontal for just names, many per line, sorted horizontally.
        -:  345:   with_commas for just names, many per line, separated by commas.
        -:  346:
        -:  347:   -l (and other options that imply -l), -1, -C, -x and -m control
        -:  348:   this parameter.  */
        -:  349:
        -:  350:enum format
        -:  351:  {
        -:  352:    long_format,		/* -l and other options that imply -l */
        -:  353:    one_per_line,		/* -1 */
        -:  354:    many_per_line,		/* -C */
        -:  355:    horizontal,			/* -x */
        -:  356:    with_commas			/* -m */
        -:  357:  };
        -:  358:
        -:  359:static enum format format;
        -:  360:
        -:  361:/* `full-iso' uses full ISO-style dates and times.  `long-iso' uses longer
        -:  362:   ISO-style time stamps, though shorter than `full-iso'.  `iso' uses shorter
        -:  363:   ISO-style time stamps.  `locale' uses locale-dependent time stamps.  */
        -:  364:enum time_style
        -:  365:  {
        -:  366:    full_iso_time_style,	/* --time-style=full-iso */
        -:  367:    long_iso_time_style,	/* --time-style=long-iso */
        -:  368:    iso_time_style,		/* --time-style=iso */
        -:  369:    locale_time_style		/* --time-style=locale */
        -:  370:  };
        -:  371:
        -:  372:static char const *const time_style_args[] =
        -:  373:{
        -:  374:  "full-iso", "long-iso", "iso", "locale", NULL
        -:  375:};
        -:  376:static enum time_style const time_style_types[] =
        -:  377:{
        -:  378:  full_iso_time_style, long_iso_time_style, iso_time_style,
        -:  379:  locale_time_style
        -:  380:};
        -:  381:ARGMATCH_VERIFY (time_style_args, time_style_types);
        -:  382:
        -:  383:/* Type of time to print or sort by.  Controlled by -c and -u.
        -:  384:   The values of each item of this enum are important since they are
        -:  385:   used as indices in the sort functions array (see sort_files()).  */
        -:  386:
        -:  387:enum time_type
        -:  388:  {
        -:  389:    time_mtime,			/* default */
        -:  390:    time_ctime,			/* -c */
        -:  391:    time_atime,			/* -u */
        -:  392:    time_numtypes		/* the number of elements of this enum */
        -:  393:  };
        -:  394:
        -:  395:static enum time_type time_type;
        -:  396:
        -:  397:/* The file characteristic to sort by.  Controlled by -t, -S, -U, -X, -v.
        -:  398:   The values of each item of this enum are important since they are
        -:  399:   used as indices in the sort functions array (see sort_files()).  */
        -:  400:
        -:  401:enum sort_type
        -:  402:  {
        -:  403:    sort_none = -1,		/* -U */
        -:  404:    sort_name,			/* default */
        -:  405:    sort_extension,		/* -X */
        -:  406:    sort_size,			/* -S */
        -:  407:    sort_version,		/* -v */
        -:  408:    sort_time,			/* -t */
        -:  409:    sort_numtypes		/* the number of elements of this enum */
        -:  410:  };
        -:  411:
        -:  412:static enum sort_type sort_type;
        -:  413:
        -:  414:/* Direction of sort.
        -:  415:   false means highest first if numeric,
        -:  416:   lowest first if alphabetic;
        -:  417:   these are the defaults.
        -:  418:   true means the opposite order in each case.  -r  */
        -:  419:
        -:  420:static bool sort_reverse;
        -:  421:
        -:  422:/* True means to display owner information.  -g turns this off.  */
        -:  423:
        -:  424:static bool print_owner = true;
        -:  425:
        -:  426:/* True means to display author information.  */
        -:  427:
        -:  428:static bool print_author;
        -:  429:
        -:  430:/* True means to display group information.  -G and -o turn this off.  */
        -:  431:
        -:  432:static bool print_group = true;
        -:  433:
        -:  434:/* True means print the user and group id's as numbers rather
        -:  435:   than as names.  -n  */
        -:  436:
        -:  437:static bool numeric_ids;
        -:  438:
        -:  439:/* True means mention the size in blocks of each file.  -s  */
        -:  440:
        -:  441:static bool print_block_size;
        -:  442:
        -:  443:/* Human-readable options for output.  */
        -:  444:static int human_output_opts;
        -:  445:
        -:  446:/* The units to use when printing sizes other than file sizes.  */
        -:  447:static uintmax_t output_block_size;
        -:  448:
        -:  449:/* Likewise, but for file sizes.  */
        -:  450:static uintmax_t file_output_block_size = 1;
        -:  451:
        -:  452:/* Follow the output with a special string.  Using this format,
        -:  453:   Emacs' dired mode starts up twice as fast, and can handle all
        -:  454:   strange characters in file names.  */
        -:  455:static bool dired;
        -:  456:
        -:  457:/* `none' means don't mention the type of files.
        -:  458:   `slash' means mention directories only, with a '/'.
        -:  459:   `file_type' means mention file types.
        -:  460:   `classify' means mention file types and mark executables.
        -:  461:
        -:  462:   Controlled by -F, -p, and --indicator-style.  */
        -:  463:
        -:  464:enum indicator_style
        -:  465:  {
        -:  466:    none,	/*     --indicator-style=none */
        -:  467:    slash,	/* -p, --indicator-style=slash */
        -:  468:    file_type,	/*     --indicator-style=file-type */
        -:  469:    classify	/* -F, --indicator-style=classify */
        -:  470:  };
        -:  471:
        -:  472:static enum indicator_style indicator_style;
        -:  473:
        -:  474:/* Names of indicator styles.  */
        -:  475:static char const *const indicator_style_args[] =
        -:  476:{
        -:  477:  "none", "slash", "file-type", "classify", NULL
        -:  478:};
        -:  479:static enum indicator_style const indicator_style_types[] =
        -:  480:{
        -:  481:  none, slash, file_type, classify
        -:  482:};
        -:  483:ARGMATCH_VERIFY (indicator_style_args, indicator_style_types);
        -:  484:
        -:  485:/* True means use colors to mark types.  Also define the different
        -:  486:   colors as well as the stuff for the LS_COLORS environment variable.
        -:  487:   The LS_COLORS variable is now in a termcap-like format.  */
        -:  488:
        -:  489:static bool print_with_color;
        -:  490:
        -:  491:/* Whether we used any colors in the output so far.  If so, we will
        -:  492:   need to restore the default color later.  If not, we will need to
        -:  493:   call prep_non_filename_text before using color for the first time. */
        -:  494:
        -:  495:static bool used_color = false;
        -:  496:
        -:  497:enum color_type
        -:  498:  {
        -:  499:    color_never,		/* 0: default or --color=never */
        -:  500:    color_always,		/* 1: --color=always */
        -:  501:    color_if_tty		/* 2: --color=tty */
        -:  502:  };
        -:  503:
        -:  504:enum Dereference_symlink
        -:  505:  {
        -:  506:    DEREF_UNDEFINED = 1,
        -:  507:    DEREF_NEVER,
        -:  508:    DEREF_COMMAND_LINE_ARGUMENTS,	/* -H */
        -:  509:    DEREF_COMMAND_LINE_SYMLINK_TO_DIR,	/* the default, in certain cases */
        -:  510:    DEREF_ALWAYS			/* -L */
        -:  511:  };
        -:  512:
        -:  513:enum indicator_no
        -:  514:  {
        -:  515:    C_LEFT, C_RIGHT, C_END, C_RESET, C_NORM, C_FILE, C_DIR, C_LINK,
        -:  516:    C_FIFO, C_SOCK,
        -:  517:    C_BLK, C_CHR, C_MISSING, C_ORPHAN, C_EXEC, C_DOOR, C_SETUID, C_SETGID,
        -:  518:    C_STICKY, C_OTHER_WRITABLE, C_STICKY_OTHER_WRITABLE
        -:  519:  };
        -:  520:
        -:  521:static const char *const indicator_name[]=
        -:  522:  {
        -:  523:    "lc", "rc", "ec", "rs", "no", "fi", "di", "ln", "pi", "so",
        -:  524:    "bd", "cd", "mi", "or", "ex", "do", "su", "sg", "st",
        -:  525:    "ow", "tw", NULL
        -:  526:  };
        -:  527:
        -:  528:struct color_ext_type
        -:  529:  {
        -:  530:    struct bin_str ext;		/* The extension we're looking for */
        -:  531:    struct bin_str seq;		/* The sequence to output when we do */
        -:  532:    struct color_ext_type *next;	/* Next in list */
        -:  533:  };
        -:  534:
        -:  535:static struct bin_str color_indicator[] =
        -:  536:  {
        -:  537:    { LEN_STR_PAIR ("\033[") },		/* lc: Left of color sequence */
        -:  538:    { LEN_STR_PAIR ("m") },		/* rc: Right of color sequence */
        -:  539:    { 0, NULL },			/* ec: End color (replaces lc+no+rc) */
        -:  540:    { LEN_STR_PAIR ("0") },		/* rs: Reset to ordinary colors */
        -:  541:    { 0, NULL },			/* no: Normal */
        -:  542:    { 0, NULL },			/* fi: File: default */
        -:  543:    { LEN_STR_PAIR ("01;34") },		/* di: Directory: bright blue */
        -:  544:    { LEN_STR_PAIR ("01;36") },		/* ln: Symlink: bright cyan */
        -:  545:    { LEN_STR_PAIR ("33") },		/* pi: Pipe: yellow/brown */
        -:  546:    { LEN_STR_PAIR ("01;35") },		/* so: Socket: bright magenta */
        -:  547:    { LEN_STR_PAIR ("01;33") },		/* bd: Block device: bright yellow */
        -:  548:    { LEN_STR_PAIR ("01;33") },		/* cd: Char device: bright yellow */
        -:  549:    { 0, NULL },			/* mi: Missing file: undefined */
        -:  550:    { 0, NULL },			/* or: Orphaned symlink: undefined */
        -:  551:    { LEN_STR_PAIR ("01;32") },		/* ex: Executable: bright green */
        -:  552:    { LEN_STR_PAIR ("01;35") },		/* do: Door: bright magenta */
        -:  553:    { LEN_STR_PAIR ("37;41") },		/* su: setuid: white on red */
        -:  554:    { LEN_STR_PAIR ("30;43") },		/* sg: setgid: black on yellow */
        -:  555:    { LEN_STR_PAIR ("37;44") },		/* st: sticky: black on blue */
        -:  556:    { LEN_STR_PAIR ("34;42") },		/* ow: other-writable: blue on green */
        -:  557:    { LEN_STR_PAIR ("30;42") },		/* tw: ow w/ sticky: black on green */
        -:  558:  };
        -:  559:
        -:  560:/* FIXME: comment  */
        -:  561:static struct color_ext_type *color_ext_list = NULL;
        -:  562:
        -:  563:/* Buffer for color sequences */
        -:  564:static char *color_buf;
        -:  565:
        -:  566:/* True means to check for orphaned symbolic link, for displaying
        -:  567:   colors.  */
        -:  568:
        -:  569:static bool check_symlink_color;
        -:  570:
        -:  571:/* True means mention the inode number of each file.  -i  */
        -:  572:
        -:  573:static bool print_inode;
        -:  574:
        -:  575:/* What to do with symbolic links.  Affected by -d, -F, -H, -l (and
        -:  576:   other options that imply -l), and -L.  */
        -:  577:
        -:  578:static enum Dereference_symlink dereference;
        -:  579:
        -:  580:/* True means when a directory is found, display info on its
        -:  581:   contents.  -R  */
        -:  582:
        -:  583:static bool recursive;
        -:  584:
        -:  585:/* True means when an argument is a directory name, display info
        -:  586:   on it itself.  -d  */
        -:  587:
        -:  588:static bool immediate_dirs;
        -:  589:
        -:  590:/* True means that directories are grouped before files. */
        -:  591:
        -:  592:static bool directories_first;
        -:  593:
        -:  594:/* Which files to ignore.  */
        -:  595:
        -:  596:static enum
        -:  597:{
        -:  598:  /* Ignore files whose names start with `.', and files specified by
        -:  599:     --hide and --ignore.  */
        -:  600:  IGNORE_DEFAULT,
        -:  601:
        -:  602:  /* Ignore `.', `..', and files specified by --ignore.  */
        -:  603:  IGNORE_DOT_AND_DOTDOT,
        -:  604:
        -:  605:  /* Ignore only files specified by --ignore.  */
        -:  606:  IGNORE_MINIMAL
        -:  607:} ignore_mode;
        -:  608:
        -:  609:/* A linked list of shell-style globbing patterns.  If a non-argument
        -:  610:   file name matches any of these patterns, it is ignored.
        -:  611:   Controlled by -I.  Multiple -I options accumulate.
        -:  612:   The -B option adds `*~' and `.*~' to this list.  */
        -:  613:
        -:  614:struct ignore_pattern
        -:  615:  {
        -:  616:    const char *pattern;
        -:  617:    struct ignore_pattern *next;
        -:  618:  };
        -:  619:
        -:  620:static struct ignore_pattern *ignore_patterns;
        -:  621:
        -:  622:/* Similar to IGNORE_PATTERNS, except that -a or -A causes this
        -:  623:   variable itself to be ignored.  */
        -:  624:static struct ignore_pattern *hide_patterns;
        -:  625:
        -:  626:/* True means output nongraphic chars in file names as `?'.
        -:  627:   (-q, --hide-control-chars)
        -:  628:   qmark_funny_chars and the quoting style (-Q, --quoting-style=WORD) are
        -:  629:   independent.  The algorithm is: first, obey the quoting style to get a
        -:  630:   string representing the file name;  then, if qmark_funny_chars is set,
        -:  631:   replace all nonprintable chars in that string with `?'.  It's necessary
        -:  632:   to replace nonprintable chars even in quoted strings, because we don't
        -:  633:   want to mess up the terminal if control chars get sent to it, and some
        -:  634:   quoting methods pass through control chars as-is.  */
        -:  635:static bool qmark_funny_chars;
        -:  636:
        -:  637:/* Quoting options for file and dir name output.  */
        -:  638:
        -:  639:static struct quoting_options *filename_quoting_options;
        -:  640:static struct quoting_options *dirname_quoting_options;
        -:  641:
        -:  642:/* The number of chars per hardware tab stop.  Setting this to zero
        -:  643:   inhibits the use of TAB characters for separating columns.  -T */
        -:  644:static size_t tabsize;
        -:  645:
        -:  646:/* True means print each directory name before listing it.  */
        -:  647:
        -:  648:static bool print_dir_name;
        -:  649:
        -:  650:/* The line length to use for breaking lines in many-per-line format.
        -:  651:   Can be set with -w.  */
        -:  652:
        -:  653:static size_t line_length;
        -:  654:
        -:  655:/* If true, the file listing format requires that stat be called on
        -:  656:   each file.  */
        -:  657:
        -:  658:static bool format_needs_stat;
        -:  659:
        -:  660:/* Similar to `format_needs_stat', but set if only the file type is
        -:  661:   needed.  */
        -:  662:
        -:  663:static bool format_needs_type;
        -:  664:
        -:  665:/* An arbitrary limit on the number of bytes in a printed time stamp.
        -:  666:   This is set to a relatively small value to avoid the need to worry
        -:  667:   about denial-of-service attacks on servers that run "ls" on behalf
        -:  668:   of remote clients.  1000 bytes should be enough for any practical
        -:  669:   time stamp format.  */
        -:  670:
        -:  671:enum { TIME_STAMP_LEN_MAXIMUM = MAX (1000, INT_STRLEN_BOUND (time_t)) };
        -:  672:
        -:  673:/* strftime formats for non-recent and recent files, respectively, in
        -:  674:   -l output.  */
        -:  675:
        -:  676:static char const *long_time_format[2] =
        -:  677:  {
        -:  678:    /* strftime format for non-recent files (older than 6 months), in
        -:  679:       -l output.  This should contain the year, month and day (at
        -:  680:       least), in an order that is understood by people in your
        -:  681:       locale's territory.  Please try to keep the number of used
        -:  682:       screen columns small, because many people work in windows with
        -:  683:       only 80 columns.  But make this as wide as the other string
        -:  684:       below, for recent files.  */
        -:  685:    N_("%b %e  %Y"),
        -:  686:    /* strftime format for recent files (younger than 6 months), in -l
        -:  687:       output.  This should contain the month, day and time (at
        -:  688:       least), in an order that is understood by people in your
        -:  689:       locale's territory.  Please try to keep the number of used
        -:  690:       screen columns small, because many people work in windows with
        -:  691:       only 80 columns.  But make this as wide as the other string
        -:  692:       above, for non-recent files.  */
        -:  693:    N_("%b %e %H:%M")
        -:  694:  };
        -:  695:
        -:  696:/* The set of signals that are caught.  */
        -:  697:
        -:  698:static sigset_t caught_signals;
        -:  699:
        -:  700:/* If nonzero, the value of the pending fatal signal.  */
        -:  701:
        -:  702:static sig_atomic_t volatile interrupt_signal;
        -:  703:
        -:  704:/* A count of the number of pending stop signals that have been received.  */
        -:  705:
        -:  706:static sig_atomic_t volatile stop_signal_count;
        -:  707:
        -:  708:/* Desired exit status.  */
        -:  709:
        -:  710:static int exit_status;
        -:  711:
        -:  712:/* Exit statuses.  */
        -:  713:enum
        -:  714:  {
        -:  715:    /* "ls" had a minor problem (e.g., it could not stat a directory
        -:  716:       entry).  */
        -:  717:    LS_MINOR_PROBLEM = 1,
        -:  718:
        -:  719:    /* "ls" had more serious trouble.  */
        -:  720:    LS_FAILURE = 2
        -:  721:  };
        -:  722:
        -:  723:/* For long options that have no equivalent short option, use a
        -:  724:   non-character as a pseudo short option, starting with CHAR_MAX + 1.  */
        -:  725:enum
        -:  726:{
        -:  727:  AUTHOR_OPTION = CHAR_MAX + 1,
        -:  728:  BLOCK_SIZE_OPTION,
        -:  729:  COLOR_OPTION,
        -:  730:  DEREFERENCE_COMMAND_LINE_SYMLINK_TO_DIR_OPTION,
        -:  731:  FILE_TYPE_INDICATOR_OPTION,
        -:  732:  FORMAT_OPTION,
        -:  733:  FULL_TIME_OPTION,
        -:  734:  GROUP_DIRECTORIES_FIRST_OPTION,
        -:  735:  HIDE_OPTION,
        -:  736:  INDICATOR_STYLE_OPTION,
        -:  737:  QUOTING_STYLE_OPTION,
        -:  738:  SHOW_CONTROL_CHARS_OPTION,
        -:  739:  SI_OPTION,
        -:  740:  SORT_OPTION,
        -:  741:  TIME_OPTION,
        -:  742:  TIME_STYLE_OPTION
        -:  743:};
        -:  744:
        -:  745:static struct option const long_options[] =
        -:  746:{
        -:  747:  {"all", no_argument, NULL, 'a'},
        -:  748:  {"escape", no_argument, NULL, 'b'},
        -:  749:  {"directory", no_argument, NULL, 'd'},
        -:  750:  {"dired", no_argument, NULL, 'D'},
        -:  751:  {"full-time", no_argument, NULL, FULL_TIME_OPTION},
        -:  752:  {"group-directories-first", no_argument, NULL,
        -:  753:   GROUP_DIRECTORIES_FIRST_OPTION},
        -:  754:  {"human-readable", no_argument, NULL, 'h'},
        -:  755:  {"inode", no_argument, NULL, 'i'},
        -:  756:  {"numeric-uid-gid", no_argument, NULL, 'n'},
        -:  757:  {"no-group", no_argument, NULL, 'G'},
        -:  758:  {"hide-control-chars", no_argument, NULL, 'q'},
        -:  759:  {"reverse", no_argument, NULL, 'r'},
        -:  760:  {"size", no_argument, NULL, 's'},
        -:  761:  {"width", required_argument, NULL, 'w'},
        -:  762:  {"almost-all", no_argument, NULL, 'A'},
        -:  763:  {"ignore-backups", no_argument, NULL, 'B'},
        -:  764:  {"classify", no_argument, NULL, 'F'},
        -:  765:  {"file-type", no_argument, NULL, FILE_TYPE_INDICATOR_OPTION},
        -:  766:  {"si", no_argument, NULL, SI_OPTION},
        -:  767:  {"dereference-command-line", no_argument, NULL, 'H'},
        -:  768:  {"dereference-command-line-symlink-to-dir", no_argument, NULL,
        -:  769:   DEREFERENCE_COMMAND_LINE_SYMLINK_TO_DIR_OPTION},
        -:  770:  {"hide", required_argument, NULL, HIDE_OPTION},
        -:  771:  {"ignore", required_argument, NULL, 'I'},
        -:  772:  {"indicator-style", required_argument, NULL, INDICATOR_STYLE_OPTION},
        -:  773:  {"dereference", no_argument, NULL, 'L'},
        -:  774:  {"literal", no_argument, NULL, 'N'},
        -:  775:  {"quote-name", no_argument, NULL, 'Q'},
        -:  776:  {"quoting-style", required_argument, NULL, QUOTING_STYLE_OPTION},
        -:  777:  {"recursive", no_argument, NULL, 'R'},
        -:  778:  {"format", required_argument, NULL, FORMAT_OPTION},
        -:  779:  {"show-control-chars", no_argument, NULL, SHOW_CONTROL_CHARS_OPTION},
        -:  780:  {"sort", required_argument, NULL, SORT_OPTION},
        -:  781:  {"tabsize", required_argument, NULL, 'T'},
        -:  782:  {"time", required_argument, NULL, TIME_OPTION},
        -:  783:  {"time-style", required_argument, NULL, TIME_STYLE_OPTION},
        -:  784:  {"color", optional_argument, NULL, COLOR_OPTION},
        -:  785:  {"block-size", required_argument, NULL, BLOCK_SIZE_OPTION},
        -:  786:  {"context", no_argument, 0, 'Z'},
        -:  787:  {"author", no_argument, NULL, AUTHOR_OPTION},
        -:  788:  {GETOPT_HELP_OPTION_DECL},
        -:  789:  {GETOPT_VERSION_OPTION_DECL},
        -:  790:  {NULL, 0, NULL, 0}
        -:  791:};
        -:  792:
        -:  793:static char const *const format_args[] =
        -:  794:{
        -:  795:  "verbose", "long", "commas", "horizontal", "across",
        -:  796:  "vertical", "single-column", NULL
        -:  797:};
        -:  798:static enum format const format_types[] =
        -:  799:{
        -:  800:  long_format, long_format, with_commas, horizontal, horizontal,
        -:  801:  many_per_line, one_per_line
        -:  802:};
        -:  803:ARGMATCH_VERIFY (format_args, format_types);
        -:  804:
        -:  805:static char const *const sort_args[] =
        -:  806:{
        -:  807:  "none", "time", "size", "extension", "version", NULL
        -:  808:};
        -:  809:static enum sort_type const sort_types[] =
        -:  810:{
        -:  811:  sort_none, sort_time, sort_size, sort_extension, sort_version
        -:  812:};
        -:  813:ARGMATCH_VERIFY (sort_args, sort_types);
        -:  814:
        -:  815:static char const *const time_args[] =
        -:  816:{
        -:  817:  "atime", "access", "use", "ctime", "status", NULL
        -:  818:};
        -:  819:static enum time_type const time_types[] =
        -:  820:{
        -:  821:  time_atime, time_atime, time_atime, time_ctime, time_ctime
        -:  822:};
        -:  823:ARGMATCH_VERIFY (time_args, time_types);
        -:  824:
        -:  825:static char const *const color_args[] =
        -:  826:{
        -:  827:  /* force and none are for compatibility with another color-ls version */
        -:  828:  "always", "yes", "force",
        -:  829:  "never", "no", "none",
        -:  830:  "auto", "tty", "if-tty", NULL
        -:  831:};
        -:  832:static enum color_type const color_types[] =
        -:  833:{
        -:  834:  color_always, color_always, color_always,
        -:  835:  color_never, color_never, color_never,
        -:  836:  color_if_tty, color_if_tty, color_if_tty
        -:  837:};
        -:  838:ARGMATCH_VERIFY (color_args, color_types);
        -:  839:
        -:  840:/* Information about filling a column.  */
        -:  841:struct column_info
        -:  842:{
        -:  843:  bool valid_len;
        -:  844:  size_t line_len;
        -:  845:  size_t *col_arr;
        -:  846:};
        -:  847:
        -:  848:/* Array with information about column filledness.  */
        -:  849:static struct column_info *column_info;
        -:  850:
        -:  851:/* Maximum number of columns ever possible for this display.  */
        -:  852:static size_t max_idx;
        -:  853:
        -:  854:/* The minimum width of a column is 3: 1 character for the name and 2
        -:  855:   for the separating white space.  */
        -:  856:#define MIN_COLUMN_WIDTH	3
        -:  857:
        -:  858:
        -:  859:/* This zero-based index is used solely with the --dired option.
        -:  860:   When that option is in effect, this counter is incremented for each
        -:  861:   byte of output generated by this program so that the beginning
        -:  862:   and ending indices (in that output) of every file name can be recorded
        -:  863:   and later output themselves.  */
        -:  864:static size_t dired_pos;
        -:  865:
        -:  866:#define DIRED_PUTCHAR(c) do {putchar ((c)); ++dired_pos;} while (0)
        -:  867:
        -:  868:/* Write S to STREAM and increment DIRED_POS by S_LEN.  */
        -:  869:#define DIRED_FPUTS(s, stream, s_len) \
        -:  870:    do {fputs (s, stream); dired_pos += s_len;} while (0)
        -:  871:
        -:  872:/* Like DIRED_FPUTS, but for use when S is a literal string.  */
        -:  873:#define DIRED_FPUTS_LITERAL(s, stream) \
        -:  874:    do {fputs (s, stream); dired_pos += sizeof (s) - 1;} while (0)
        -:  875:
        -:  876:#define DIRED_INDENT()							\
        -:  877:    do									\
        -:  878:      {									\
        -:  879:	if (dired)							\
        -:  880:	  DIRED_FPUTS_LITERAL ("  ", stdout);				\
        -:  881:      }									\
        -:  882:    while (0)
        -:  883:
        -:  884:/* With --dired, store pairs of beginning and ending indices of filenames.  */
        -:  885:static struct obstack dired_obstack;
        -:  886:
        -:  887:/* With --dired, store pairs of beginning and ending indices of any
        -:  888:   directory names that appear as headers (just before `total' line)
        -:  889:   for lists of directory entries.  Such directory names are seen when
        -:  890:   listing hierarchies using -R and when a directory is listed with at
        -:  891:   least one other command line argument.  */
        -:  892:static struct obstack subdired_obstack;
        -:  893:
        -:  894:/* Save the current index on the specified obstack, OBS.  */
        -:  895:#define PUSH_CURRENT_DIRED_POS(obs)					\
        -:  896:  do									\
        -:  897:    {									\
        -:  898:      if (dired)							\
        -:  899:	obstack_grow (obs, &dired_pos, sizeof (dired_pos));		\
        -:  900:    }									\
        -:  901:  while (0)
        -:  902:
        -:  903:/* With -R, this stack is used to help detect directory cycles.
        -:  904:   The device/inode pairs on this stack mirror the pairs in the
        -:  905:   active_dir_set hash table.  */
        -:  906:static struct obstack dev_ino_obstack;
        -:  907:
        -:  908:/* Push a pair onto the device/inode stack.  */
        -:  909:#define DEV_INO_PUSH(Dev, Ino)						\
        -:  910:  do									\
        -:  911:    {									\
        -:  912:      struct dev_ino *di;						\
        -:  913:      obstack_blank (&dev_ino_obstack, sizeof (struct dev_ino));	\
        -:  914:      di = -1 + (struct dev_ino *) obstack_next_free (&dev_ino_obstack); \
        -:  915:      di->st_dev = (Dev);						\
        -:  916:      di->st_ino = (Ino);						\
        -:  917:    }									\
        -:  918:  while (0)
        -:  919:
        -:  920:/* Pop a dev/ino struct off the global dev_ino_obstack
        -:  921:   and return that struct.  */
        -:  922:static struct dev_ino
    #####:  923:dev_ino_pop (void)
        -:  924:{
    #####:  925:  assert (sizeof (struct dev_ino) <= obstack_object_size (&dev_ino_obstack));
    #####:  926:  obstack_blank (&dev_ino_obstack, -(int) (sizeof (struct dev_ino)));
    #####:  927:  return *(struct dev_ino *) obstack_next_free (&dev_ino_obstack);
        -:  928:}
        -:  929:
        -:  930:#define ASSERT_MATCHING_DEV_INO(Name, Di)	\
        -:  931:  do						\
        -:  932:    {						\
        -:  933:      struct stat sb;				\
        -:  934:      assert (Name);				\
        -:  935:      assert (0 <= stat (Name, &sb));		\
        -:  936:      assert (sb.st_dev == Di.st_dev);		\
        -:  937:      assert (sb.st_ino == Di.st_ino);		\
        -:  938:    }						\
        -:  939:  while (0)
        -:  940:
        -:  941:
        -:  942:/* Write to standard output PREFIX, followed by the quoting style and
        -:  943:   a space-separated list of the integers stored in OS all on one line.  */
        -:  944:
        -:  945:static void
    #####:  946:dired_dump_obstack (const char *prefix, struct obstack *os)
        -:  947:{
        -:  948:  size_t n_pos;
        -:  949:
    #####:  950:  n_pos = obstack_object_size (os) / sizeof (dired_pos);
    #####:  951:  if (n_pos > 0)
        -:  952:    {
        -:  953:      size_t i;
        -:  954:      size_t *pos;
        -:  955:
    #####:  956:      pos = (size_t *) obstack_finish (os);
    #####:  957:      fputs (prefix, stdout);
    #####:  958:      for (i = 0; i < n_pos; i++)
    #####:  959:	printf (" %lu", (unsigned long int) pos[i]);
    #####:  960:      putchar ('\n');
        -:  961:    }
    #####:  962:}
        -:  963:
        -:  964:static size_t
    #####:  965:dev_ino_hash (void const *x, size_t table_size)
        -:  966:{
    #####:  967:  struct dev_ino const *p = x;
    #####:  968:  return (uintmax_t) p->st_ino % table_size;
        -:  969:}
        -:  970:
        -:  971:static bool
    #####:  972:dev_ino_compare (void const *x, void const *y)
        -:  973:{
    #####:  974:  struct dev_ino const *a = x;
    #####:  975:  struct dev_ino const *b = y;
    #####:  976:  return SAME_INODE (*a, *b) ? true : false;
        -:  977:}
        -:  978:
        -:  979:static void
    #####:  980:dev_ino_free (void *x)
        -:  981:{
    #####:  982:  free (x);
    #####:  983:}
        -:  984:
        -:  985:/* Add the device/inode pair (P->st_dev/P->st_ino) to the set of
        -:  986:   active directories.  Return true if there is already a matching
        -:  987:   entry in the table.  */
        -:  988:
        -:  989:static bool
    #####:  990:visit_dir (dev_t dev, ino_t ino)
        -:  991:{
        -:  992:  struct dev_ino *ent;
        -:  993:  struct dev_ino *ent_from_table;
        -:  994:  bool found_match;
        -:  995:
    #####:  996:  ent = xmalloc (sizeof *ent);
    #####:  997:  ent->st_ino = ino;
    #####:  998:  ent->st_dev = dev;
        -:  999:
        -: 1000:  /* Attempt to insert this entry into the table.  */
    #####: 1001:  ent_from_table = hash_insert (active_dir_set, ent);
        -: 1002:
    #####: 1003:  if (ent_from_table == NULL)
        -: 1004:    {
        -: 1005:      /* Insertion failed due to lack of memory.  */
    #####: 1006:      xalloc_die ();
        -: 1007:    }
        -: 1008:
    #####: 1009:  found_match = (ent_from_table != ent);
        -: 1010:
    #####: 1011:  if (found_match)
        -: 1012:    {
        -: 1013:      /* ent was not inserted, so free it.  */
    #####: 1014:      free (ent);
        -: 1015:    }
        -: 1016:
    #####: 1017:  return found_match;
        -: 1018:}
        -: 1019:
        -: 1020:static void
        1: 1021:free_pending_ent (struct pending *p)
        -: 1022:{
        1: 1023:  free (p->name);
        1: 1024:  free (p->realname);
        1: 1025:  free (p);
        1: 1026:}
        -: 1027:
        -: 1028:static bool
    #####: 1029:is_colored (enum indicator_no type)
        -: 1030:{
    #####: 1031:  size_t len = color_indicator[type].len;
    #####: 1032:  char const *s = color_indicator[type].string;
    #####: 1033:  return ! (len == 0
    #####: 1034:	    || (len == 1 && strncmp (s, "0", 1) == 0)
    #####: 1035:	    || (len == 2 && strncmp (s, "00", 2) == 0));
        -: 1036:}
        -: 1037:
        -: 1038:static void
    #####: 1039:restore_default_color (void)
        -: 1040:{
    #####: 1041:  put_indicator (&color_indicator[C_LEFT]);
    #####: 1042:  put_indicator (&color_indicator[C_RIGHT]);
    #####: 1043:}
        -: 1044:
        -: 1045:/* An ordinary signal was received; arrange for the program to exit.  */
        -: 1046:
        -: 1047:static void
    #####: 1048:sighandler (int sig)
        -: 1049:{
        -: 1050:  if (! SA_NOCLDSTOP)
        -: 1051:    signal (sig, SIG_IGN);
    #####: 1052:  if (! interrupt_signal)
    #####: 1053:    interrupt_signal = sig;
    #####: 1054:}
        -: 1055:
        -: 1056:/* A SIGTSTP was received; arrange for the program to suspend itself.  */
        -: 1057:
        -: 1058:static void
    #####: 1059:stophandler (int sig)
        -: 1060:{
        -: 1061:  if (! SA_NOCLDSTOP)
        -: 1062:    signal (sig, stophandler);
    #####: 1063:  if (! interrupt_signal)
    #####: 1064:    stop_signal_count++;
    #####: 1065:}
        -: 1066:
        -: 1067:/* Process any pending signals.  If signals are caught, this function
        -: 1068:   should be called periodically.  Ideally there should never be an
        -: 1069:   unbounded amount of time when signals are not being processed.
        -: 1070:   Signal handling can restore the default colors, so callers must
        -: 1071:   immediately change colors after invoking this function.  */
        -: 1072:
        -: 1073:static void
    #####: 1074:process_signals (void)
        -: 1075:{
    #####: 1076:  while (interrupt_signal | stop_signal_count)
        -: 1077:    {
        -: 1078:      int sig;
        -: 1079:      int stops;
        -: 1080:      sigset_t oldset;
        -: 1081:
    #####: 1082:      if (used_color)
    #####: 1083:	restore_default_color ();
    #####: 1084:      fflush (stdout);
        -: 1085:
    #####: 1086:      sigprocmask (SIG_BLOCK, &caught_signals, &oldset);
        -: 1087:
        -: 1088:      /* Reload interrupt_signal and stop_signal_count, in case a new
        -: 1089:	 signal was handled before sigprocmask took effect.  */
    #####: 1090:      sig = interrupt_signal;
    #####: 1091:      stops = stop_signal_count;
        -: 1092:
        -: 1093:      /* SIGTSTP is special, since the application can receive that signal
        -: 1094:	 more than once.  In this case, don't set the signal handler to the
        -: 1095:	 default.  Instead, just raise the uncatchable SIGSTOP.  */
    #####: 1096:      if (stops)
        -: 1097:	{
    #####: 1098:	  stop_signal_count = stops - 1;
    #####: 1099:	  sig = SIGSTOP;
        -: 1100:	}
        -: 1101:      else
    #####: 1102:	signal (sig, SIG_DFL);
        -: 1103:
        -: 1104:      /* Exit or suspend the program.  */
    #####: 1105:      raise (sig);
    #####: 1106:      sigprocmask (SIG_SETMASK, &oldset, NULL);
        -: 1107:
        -: 1108:      /* If execution reaches here, then the program has been
        -: 1109:	 continued (after being suspended).  */
        -: 1110:    }
    #####: 1111:}
        -: 1112:
        -: 1113:int
        1: 1114:main (int argc, char **argv)
        -: 1115:{
        -: 1116:  int i;
        -: 1117:  struct pending *thispend;
        -: 1118:  int n_files;
        -: 1119:
        -: 1120:  /* The signals that are trapped, and the number of such signals.  */
        -: 1121:  static int const sig[] =
        -: 1122:    {
        -: 1123:      /* This one is handled specially.  */
        -: 1124:      SIGTSTP,
        -: 1125:
        -: 1126:      /* The usual suspects.  */
        -: 1127:      SIGALRM, SIGHUP, SIGINT, SIGPIPE, SIGQUIT, SIGTERM,
        -: 1128:#ifdef SIGPOLL
        -: 1129:      SIGPOLL,
        -: 1130:#endif
        -: 1131:#ifdef SIGPROF
        -: 1132:      SIGPROF,
        -: 1133:#endif
        -: 1134:#ifdef SIGVTALRM
        -: 1135:      SIGVTALRM,
        -: 1136:#endif
        -: 1137:#ifdef SIGXCPU
        -: 1138:      SIGXCPU,
        -: 1139:#endif
        -: 1140:#ifdef SIGXFSZ
        -: 1141:      SIGXFSZ,
        -: 1142:#endif
        -: 1143:    };
        -: 1144:  enum { nsigs = sizeof sig / sizeof sig[0] };
        -: 1145:
        -: 1146:#if ! SA_NOCLDSTOP
        -: 1147:  bool caught_sig[nsigs];
        -: 1148:#endif
        -: 1149:
        -: 1150:  initialize_main (&argc, &argv);
        1: 1151:  program_name = argv[0];
        1: 1152:  setlocale (LC_ALL, "");
        -: 1153:  bindtextdomain (PACKAGE, LOCALEDIR);
        -: 1154:  textdomain (PACKAGE);
        -: 1155:
        1: 1156:  initialize_exit_failure (LS_FAILURE);
        1: 1157:  atexit (close_stdout);
        -: 1158:
        -: 1159:#define N_ENTRIES(Array) (sizeof Array / sizeof *(Array))
        -: 1160:  assert (N_ENTRIES (color_indicator) + 1 == N_ENTRIES (indicator_name));
        -: 1161:
        1: 1162:  exit_status = EXIT_SUCCESS;
        1: 1163:  print_dir_name = true;
        1: 1164:  pending_dirs = NULL;
        -: 1165:
        1: 1166:  current_time.tv_sec = TYPE_MINIMUM (time_t);
        1: 1167:  current_time.tv_nsec = -1;
        -: 1168:
        1: 1169:  i = decode_switches (argc, argv);
        -: 1170:
        1: 1171:  if (print_with_color)
    #####: 1172:    parse_ls_color ();
        -: 1173:
        -: 1174:  /* Test print_with_color again, because the call to parse_ls_color
        -: 1175:     may have just reset it -- e.g., if LS_COLORS is invalid.  */
        1: 1176:  if (print_with_color)
        -: 1177:    {
        -: 1178:      /* Avoid following symbolic links when possible.  */
    #####: 1179:      if (is_colored (C_ORPHAN)
    #####: 1180:	  || (is_colored (C_EXEC) && color_symlink_as_referent)
    #####: 1181:	  || (is_colored (C_MISSING) && format == long_format))
    #####: 1182:	check_symlink_color = true;
        -: 1183:
        -: 1184:      /* If the standard output is a controlling terminal, watch out
        -: 1185:         for signals, so that the colors can be restored to the
        -: 1186:         default state if "ls" is suspended or interrupted.  */
        -: 1187:
    #####: 1188:      if (0 <= tcgetpgrp (STDOUT_FILENO))
        -: 1189:	{
        -: 1190:	  int j;
        -: 1191:#if SA_NOCLDSTOP
        -: 1192:	  struct sigaction act;
        -: 1193:
    #####: 1194:	  sigemptyset (&caught_signals);
    #####: 1195:	  for (j = 0; j < nsigs; j++)
        -: 1196:	    {
    #####: 1197:	      sigaction (sig[j], NULL, &act);
    #####: 1198:	      if (act.sa_handler != SIG_IGN)
    #####: 1199:		sigaddset (&caught_signals, sig[j]);
        -: 1200:	    }
        -: 1201:
    #####: 1202:	  act.sa_mask = caught_signals;
    #####: 1203:	  act.sa_flags = SA_RESTART;
        -: 1204:
    #####: 1205:	  for (j = 0; j < nsigs; j++)
    #####: 1206:	    if (sigismember (&caught_signals, sig[j]))
        -: 1207:	      {
    #####: 1208:		act.sa_handler = sig[j] == SIGTSTP ? stophandler : sighandler;
    #####: 1209:		sigaction (sig[j], &act, NULL);
        -: 1210:	      }
        -: 1211:#else
        -: 1212:	  for (j = 0; j < nsigs; j++)
        -: 1213:	    {
        -: 1214:	      caught_sig[j] = (signal (sig[j], SIG_IGN) != SIG_IGN);
        -: 1215:	      if (caught_sig[j])
        -: 1216:		{
        -: 1217:		  signal (sig[j], sig[j] == SIGTSTP ? stophandler : sighandler);
        -: 1218:		  siginterrupt (sig[j], 0);
        -: 1219:		}
        -: 1220:	    }
        -: 1221:#endif
        -: 1222:	}
        -: 1223:    }
        -: 1224:
        1: 1225:  if (dereference == DEREF_UNDEFINED)
        2: 1226:    dereference = ((immediate_dirs
        1: 1227:		    || indicator_style == classify
        1: 1228:		    || format == long_format)
        -: 1229:		   ? DEREF_NEVER
        -: 1230:		   : DEREF_COMMAND_LINE_SYMLINK_TO_DIR);
        -: 1231:
        -: 1232:  /* When using -R, initialize a data structure we'll use to
        -: 1233:     detect any directory cycles.  */
        1: 1234:  if (recursive)
        -: 1235:    {
    #####: 1236:      active_dir_set = hash_initialize (INITIAL_TABLE_SIZE, NULL,
        -: 1237:					dev_ino_hash,
        -: 1238:					dev_ino_compare,
        -: 1239:					dev_ino_free);
    #####: 1240:      if (active_dir_set == NULL)
    #####: 1241:	xalloc_die ();
        -: 1242:
    #####: 1243:      obstack_init (&dev_ino_obstack);
        -: 1244:    }
        -: 1245:
        3: 1246:  format_needs_stat = sort_type == sort_time || sort_type == sort_size
        1: 1247:    || format == long_format
    #####: 1248:    || print_scontext
        1: 1249:    || print_block_size;
        2: 1250:  format_needs_type = (! format_needs_stat
        1: 1251:		       && (recursive
    #####: 1252:			   || print_with_color
    #####: 1253:			   || indicator_style != none
    #####: 1254:			   || directories_first));
        -: 1255:
        1: 1256:  if (dired)
        -: 1257:    {
    #####: 1258:      obstack_init (&dired_obstack);
    #####: 1259:      obstack_init (&subdired_obstack);
        -: 1260:    }
        -: 1261:
        1: 1262:  cwd_n_alloc = 100;
        1: 1263:  cwd_file = xnmalloc (cwd_n_alloc, sizeof *cwd_file);
        1: 1264:  cwd_n_used = 0;
        -: 1265:
        1: 1266:  clear_files ();
        -: 1267:
        1: 1268:  n_files = argc - i;
        -: 1269:
        1: 1270:  if (n_files <= 0)
        -: 1271:    {
        1: 1272:      if (immediate_dirs)
    #####: 1273:	gobble_file (".", directory, NOT_AN_INODE_NUMBER, true, "");
        -: 1274:      else
        1: 1275:	queue_directory (".", NULL, true);
        -: 1276:    }
        -: 1277:  else
        -: 1278:    do
    #####: 1279:      gobble_file (argv[i++], unknown, NOT_AN_INODE_NUMBER, true, "");
    #####: 1280:    while (i < argc);
        -: 1281:
        1: 1282:  if (cwd_n_used)
        -: 1283:    {
    #####: 1284:      sort_files ();
    #####: 1285:      if (!immediate_dirs)
    #####: 1286:	extract_dirs_from_files (NULL, true);
        -: 1287:      /* `cwd_n_used' might be zero now.  */
        -: 1288:    }
        -: 1289:
        -: 1290:  /* In the following if/else blocks, it is sufficient to test `pending_dirs'
        -: 1291:     (and not pending_dirs->name) because there may be no markers in the queue
        -: 1292:     at this point.  A marker may be enqueued when extract_dirs_from_files is
        -: 1293:     called with a non-empty string or via print_dir.  */
        1: 1294:  if (cwd_n_used)
        -: 1295:    {
    #####: 1296:      print_current_files ();
    #####: 1297:      if (pending_dirs)
    #####: 1298:	DIRED_PUTCHAR ('\n');
        -: 1299:    }
        1: 1300:  else if (n_files <= 1 && pending_dirs && pending_dirs->next == 0)
        1: 1301:    print_dir_name = false;
        -: 1302:
        3: 1303:  while (pending_dirs)
        -: 1304:    {
        1: 1305:      thispend = pending_dirs;
        1: 1306:      pending_dirs = pending_dirs->next;
        -: 1307:
        1: 1308:      if (LOOP_DETECT)
        -: 1309:	{
    #####: 1310:	  if (thispend->name == NULL)
        -: 1311:	    {
        -: 1312:	      /* thispend->name == NULL means this is a marker entry
        -: 1313:		 indicating we've finished processing the directory.
        -: 1314:		 Use its dev/ino numbers to remove the corresponding
        -: 1315:		 entry from the active_dir_set hash table.  */
    #####: 1316:	      struct dev_ino di = dev_ino_pop ();
    #####: 1317:	      struct dev_ino *found = hash_delete (active_dir_set, &di);
        -: 1318:	      /* ASSERT_MATCHING_DEV_INO (thispend->realname, di); */
    #####: 1319:	      assert (found);
    #####: 1320:	      dev_ino_free (found);
    #####: 1321:	      free_pending_ent (thispend);
    #####: 1322:	      continue;
        -: 1323:	    }
        -: 1324:	}
        -: 1325:
        1: 1326:      print_dir (thispend->name, thispend->realname,
        1: 1327:		 thispend->command_line_arg);
        -: 1328:
        1: 1329:      free_pending_ent (thispend);
        1: 1330:      print_dir_name = true;
        -: 1331:    }
        -: 1332:
        1: 1333:  if (print_with_color)
        -: 1334:    {
        -: 1335:      int j;
        -: 1336:
    #####: 1337:      if (used_color)
    #####: 1338:	restore_default_color ();
    #####: 1339:      fflush (stdout);
        -: 1340:
        -: 1341:      /* Restore the default signal handling.  */
        -: 1342:#if SA_NOCLDSTOP
    #####: 1343:      for (j = 0; j < nsigs; j++)
    #####: 1344:	if (sigismember (&caught_signals, sig[j]))
    #####: 1345:	  signal (sig[j], SIG_DFL);
        -: 1346:#else
        -: 1347:      for (j = 0; j < nsigs; j++)
        -: 1348:	if (caught_sig[j])
        -: 1349:	  signal (sig[j], SIG_DFL);
        -: 1350:#endif
        -: 1351:
        -: 1352:      /* Act on any signals that arrived before the default was restored.
        -: 1353:	 This can process signals out of order, but there doesn't seem to
        -: 1354:	 be an easy way to do them in order, and the order isn't that
        -: 1355:	 important anyway.  */
    #####: 1356:      for (j = stop_signal_count; j; j--)
    #####: 1357:	raise (SIGSTOP);
    #####: 1358:      j = interrupt_signal;
    #####: 1359:      if (j)
    #####: 1360:	raise (j);
        -: 1361:    }
        -: 1362:
        1: 1363:  if (dired)
        -: 1364:    {
        -: 1365:      /* No need to free these since we're about to exit.  */
    #####: 1366:      dired_dump_obstack ("//DIRED//", &dired_obstack);
    #####: 1367:      dired_dump_obstack ("//SUBDIRED//", &subdired_obstack);
    #####: 1368:      printf ("//DIRED-OPTIONS// --quoting-style=%s\n",
    #####: 1369:	      quoting_style_args[get_quoting_style (filename_quoting_options)]);
        -: 1370:    }
        -: 1371:
        1: 1372:  if (LOOP_DETECT)
        -: 1373:    {
    #####: 1374:      assert (hash_get_n_entries (active_dir_set) == 0);
    #####: 1375:      hash_free (active_dir_set);
        -: 1376:    }
        -: 1377:
        1: 1378:  exit (exit_status);
        -: 1379:}
        -: 1380:
        -: 1381:/* Set all the option flags according to the switches specified.
        -: 1382:   Return the index of the first non-option argument.  */
        -: 1383:
        -: 1384:static int
        1: 1385:decode_switches (int argc, char **argv)
        -: 1386:{
        1: 1387:  char *time_style_option = NULL;
        -: 1388:
        -: 1389:  /* Record whether there is an option specifying sort type.  */
        1: 1390:  bool sort_type_specified = false;
        -: 1391:
        1: 1392:  qmark_funny_chars = false;
        -: 1393:
        -: 1394:  /* initialize all switches to default settings */
        -: 1395:
        1: 1396:  switch (ls_mode)
        -: 1397:    {
        -: 1398:    case LS_MULTI_COL:
        -: 1399:      /* This is for the `dir' program.  */
    #####: 1400:      format = many_per_line;
    #####: 1401:      set_quoting_style (NULL, escape_quoting_style);
    #####: 1402:      break;
        -: 1403:
        -: 1404:    case LS_LONG_FORMAT:
        -: 1405:      /* This is for the `vdir' program.  */
    #####: 1406:      format = long_format;
    #####: 1407:      set_quoting_style (NULL, escape_quoting_style);
    #####: 1408:      break;
        -: 1409:
        -: 1410:    case LS_LS:
        -: 1411:      /* This is for the `ls' program.  */
        1: 1412:      if (isatty (STDOUT_FILENO))
        -: 1413:	{
        1: 1414:	  format = many_per_line;
        -: 1415:	  /* See description of qmark_funny_chars, above.  */
        1: 1416:	  qmark_funny_chars = true;
        -: 1417:	}
        -: 1418:      else
        -: 1419:	{
    #####: 1420:	  format = one_per_line;
    #####: 1421:	  qmark_funny_chars = false;
        -: 1422:	}
        1: 1423:      break;
        -: 1424:
        -: 1425:    default:
    #####: 1426:      abort ();
        -: 1427:    }
        -: 1428:
        1: 1429:  time_type = time_mtime;
        1: 1430:  sort_type = sort_name;
        1: 1431:  sort_reverse = false;
        1: 1432:  numeric_ids = false;
        1: 1433:  print_block_size = false;
        1: 1434:  indicator_style = none;
        1: 1435:  print_inode = false;
        1: 1436:  dereference = DEREF_UNDEFINED;
        1: 1437:  recursive = false;
        1: 1438:  immediate_dirs = false;
        1: 1439:  ignore_mode = IGNORE_DEFAULT;
        1: 1440:  ignore_patterns = NULL;
        1: 1441:  hide_patterns = NULL;
        1: 1442:  print_scontext = false;
        -: 1443:
        -: 1444:  /* FIXME: put this in a function.  */
        -: 1445:  {
        1: 1446:    char const *q_style = getenv ("QUOTING_STYLE");
        1: 1447:    if (q_style)
        -: 1448:      {
    #####: 1449:	int i = ARGMATCH (q_style, quoting_style_args, quoting_style_vals);
    #####: 1450:	if (0 <= i)
    #####: 1451:	  set_quoting_style (NULL, quoting_style_vals[i]);
        -: 1452:	else
    #####: 1453:	  error (0, 0,
        -: 1454:	 _("ignoring invalid value of environment variable QUOTING_STYLE: %s"),
        -: 1455:		 quotearg (q_style));
        -: 1456:      }
        -: 1457:  }
        -: 1458:
        -: 1459:  {
        1: 1460:    char const *ls_block_size = getenv ("LS_BLOCK_SIZE");
        1: 1461:    human_options (ls_block_size,
        -: 1462:		   &human_output_opts, &output_block_size);
        1: 1463:    if (ls_block_size || getenv ("BLOCK_SIZE"))
    #####: 1464:      file_output_block_size = output_block_size;
        -: 1465:  }
        -: 1466:
        1: 1467:  line_length = 80;
        -: 1468:  {
        1: 1469:    char const *p = getenv ("COLUMNS");
        1: 1470:    if (p && *p)
        -: 1471:      {
        -: 1472:	unsigned long int tmp_ulong;
    #####: 1473:	if (xstrtoul (p, NULL, 0, &tmp_ulong, NULL) == LONGINT_OK
    #####: 1474:	    && 0 < tmp_ulong && tmp_ulong <= SIZE_MAX)
        -: 1475:	  {
    #####: 1476:	    line_length = tmp_ulong;
        -: 1477:	  }
        -: 1478:	else
        -: 1479:	  {
    #####: 1480:	    error (0, 0,
        -: 1481:	       _("ignoring invalid width in environment variable COLUMNS: %s"),
        -: 1482:		   quotearg (p));
        -: 1483:	  }
        -: 1484:      }
        -: 1485:  }
        -: 1486:
        -: 1487:#ifdef TIOCGWINSZ
        -: 1488:  {
        -: 1489:    struct winsize ws;
        -: 1490:
        1: 1491:    if (ioctl (STDOUT_FILENO, TIOCGWINSZ, &ws) != -1
        1: 1492:	&& 0 < ws.ws_col && ws.ws_col == (size_t) ws.ws_col)
        1: 1493:      line_length = ws.ws_col;
        -: 1494:  }
        -: 1495:#endif
        -: 1496:
        -: 1497:  {
        1: 1498:    char const *p = getenv ("TABSIZE");
        1: 1499:    tabsize = 8;
        1: 1500:    if (p)
        -: 1501:      {
        -: 1502:	unsigned long int tmp_ulong;
    #####: 1503:	if (xstrtoul (p, NULL, 0, &tmp_ulong, NULL) == LONGINT_OK
        -: 1504:	    && tmp_ulong <= SIZE_MAX)
        -: 1505:	  {
    #####: 1506:	    tabsize = tmp_ulong;
        -: 1507:	  }
        -: 1508:	else
        -: 1509:	  {
    #####: 1510:	    error (0, 0,
        -: 1511:	     _("ignoring invalid tab size in environment variable TABSIZE: %s"),
        -: 1512:		   quotearg (p));
        -: 1513:	  }
        -: 1514:      }
        -: 1515:  }
        -: 1516:
        -: 1517:  for (;;)
        -: 1518:    {
        3: 1519:      int oi = -1;
        3: 1520:      int c = getopt_long (argc, argv,
        -: 1521:			   "abcdfghiklmnopqrstuvw:xABCDFGHI:LNQRST:UXZ1",
        -: 1522:			   long_options, &oi);
        3: 1523:      if (c == -1)
        1: 1524:	break;
        -: 1525:
        2: 1526:      switch (c)
        -: 1527:	{
        -: 1528:	case 'a':
        1: 1529:	  ignore_mode = IGNORE_MINIMAL;
        1: 1530:	  break;
        -: 1531:
        -: 1532:	case 'b':
    #####: 1533:	  set_quoting_style (NULL, escape_quoting_style);
    #####: 1534:	  break;
        -: 1535:
        -: 1536:	case 'c':
    #####: 1537:	  time_type = time_ctime;
    #####: 1538:	  break;
        -: 1539:
        -: 1540:	case 'd':
    #####: 1541:	  immediate_dirs = true;
    #####: 1542:	  break;
        -: 1543:
        -: 1544:	case 'f':
        -: 1545:	  /* Same as enabling -a -U and disabling -l -s.  */
    #####: 1546:	  ignore_mode = IGNORE_MINIMAL;
    #####: 1547:	  sort_type = sort_none;
    #####: 1548:	  sort_type_specified = true;
        -: 1549:	  /* disable -l */
    #####: 1550:	  if (format == long_format)
    #####: 1551:	    format = (isatty (STDOUT_FILENO) ? many_per_line : one_per_line);
    #####: 1552:	  print_block_size = false;	/* disable -s */
    #####: 1553:	  print_with_color = false;	/* disable --color */
    #####: 1554:	  break;
        -: 1555:
        -: 1556:	case FILE_TYPE_INDICATOR_OPTION: /* --file-type */
    #####: 1557:	  indicator_style = file_type;
    #####: 1558:	  break;
        -: 1559:
        -: 1560:	case 'g':
    #####: 1561:	  format = long_format;
    #####: 1562:	  print_owner = false;
    #####: 1563:	  break;
        -: 1564:
        -: 1565:	case 'h':
    #####: 1566:	  human_output_opts = human_autoscale | human_SI | human_base_1024;
    #####: 1567:	  file_output_block_size = output_block_size = 1;
    #####: 1568:	  break;
        -: 1569:
        -: 1570:	case 'i':
    #####: 1571:	  print_inode = true;
    #####: 1572:	  break;
        -: 1573:
        -: 1574:	case 'k':
    #####: 1575:	  human_output_opts = 0;
    #####: 1576:	  file_output_block_size = output_block_size = 1024;
    #####: 1577:	  break;
        -: 1578:
        -: 1579:	case 'l':
        1: 1580:	  format = long_format;
        1: 1581:	  break;
        -: 1582:
        -: 1583:	case 'm':
    #####: 1584:	  format = with_commas;
    #####: 1585:	  break;
        -: 1586:
        -: 1587:	case 'n':
    #####: 1588:	  numeric_ids = true;
    #####: 1589:	  format = long_format;
    #####: 1590:	  break;
        -: 1591:
        -: 1592:	case 'o':  /* Just like -l, but don't display group info.  */
    #####: 1593:	  format = long_format;
    #####: 1594:	  print_group = false;
    #####: 1595:	  break;
        -: 1596:
        -: 1597:	case 'p':
    #####: 1598:	  indicator_style = slash;
    #####: 1599:	  break;
        -: 1600:
        -: 1601:	case 'q':
    #####: 1602:	  qmark_funny_chars = true;
    #####: 1603:	  break;
        -: 1604:
        -: 1605:	case 'r':
    #####: 1606:	  sort_reverse = true;
    #####: 1607:	  break;
        -: 1608:
        -: 1609:	case 's':
    #####: 1610:	  print_block_size = true;
    #####: 1611:	  break;
        -: 1612:
        -: 1613:	case 't':
    #####: 1614:	  sort_type = sort_time;
    #####: 1615:	  sort_type_specified = true;
    #####: 1616:	  break;
        -: 1617:
        -: 1618:	case 'u':
    #####: 1619:	  time_type = time_atime;
    #####: 1620:	  break;
        -: 1621:
        -: 1622:	case 'v':
    #####: 1623:	  sort_type = sort_version;
    #####: 1624:	  sort_type_specified = true;
    #####: 1625:	  break;
        -: 1626:
        -: 1627:	case 'w':
        -: 1628:	  {
        -: 1629:	    unsigned long int tmp_ulong;
    #####: 1630:	    if (xstrtoul (optarg, NULL, 0, &tmp_ulong, NULL) != LONGINT_OK
    #####: 1631:		|| ! (0 < tmp_ulong && tmp_ulong <= SIZE_MAX))
    #####: 1632:	      error (LS_FAILURE, 0, _("invalid line width: %s"),
        -: 1633:		     quotearg (optarg));
    #####: 1634:	    line_length = tmp_ulong;
    #####: 1635:	    break;
        -: 1636:	  }
        -: 1637:
        -: 1638:	case 'x':
    #####: 1639:	  format = horizontal;
    #####: 1640:	  break;
        -: 1641:
        -: 1642:	case 'A':
    #####: 1643:	  if (ignore_mode == IGNORE_DEFAULT)
    #####: 1644:	    ignore_mode = IGNORE_DOT_AND_DOTDOT;
    #####: 1645:	  break;
        -: 1646:
        -: 1647:	case 'B':
    #####: 1648:	  add_ignore_pattern ("*~");
    #####: 1649:	  add_ignore_pattern (".*~");
    #####: 1650:	  break;
        -: 1651:
        -: 1652:	case 'C':
    #####: 1653:	  format = many_per_line;
    #####: 1654:	  break;
        -: 1655:
        -: 1656:	case 'D':
    #####: 1657:	  dired = true;
    #####: 1658:	  break;
        -: 1659:
        -: 1660:	case 'F':
    #####: 1661:	  indicator_style = classify;
    #####: 1662:	  break;
        -: 1663:
        -: 1664:	case 'G':		/* inhibit display of group info */
    #####: 1665:	  print_group = false;
    #####: 1666:	  break;
        -: 1667:
        -: 1668:	case 'H':
    #####: 1669:	  dereference = DEREF_COMMAND_LINE_ARGUMENTS;
    #####: 1670:	  break;
        -: 1671:
        -: 1672:	case DEREFERENCE_COMMAND_LINE_SYMLINK_TO_DIR_OPTION:
    #####: 1673:	  dereference = DEREF_COMMAND_LINE_SYMLINK_TO_DIR;
    #####: 1674:	  break;
        -: 1675:
        -: 1676:	case 'I':
    #####: 1677:	  add_ignore_pattern (optarg);
    #####: 1678:	  break;
        -: 1679:
        -: 1680:	case 'L':
    #####: 1681:	  dereference = DEREF_ALWAYS;
    #####: 1682:	  break;
        -: 1683:
        -: 1684:	case 'N':
    #####: 1685:	  set_quoting_style (NULL, literal_quoting_style);
    #####: 1686:	  break;
        -: 1687:
        -: 1688:	case 'Q':
    #####: 1689:	  set_quoting_style (NULL, c_quoting_style);
    #####: 1690:	  break;
        -: 1691:
        -: 1692:	case 'R':
    #####: 1693:	  recursive = true;
    #####: 1694:	  break;
        -: 1695:
        -: 1696:	case 'S':
    #####: 1697:	  sort_type = sort_size;
    #####: 1698:	  sort_type_specified = true;
    #####: 1699:	  break;
        -: 1700:
        -: 1701:	case 'T':
        -: 1702:	  {
        -: 1703:	    unsigned long int tmp_ulong;
    #####: 1704:	    if (xstrtoul (optarg, NULL, 0, &tmp_ulong, NULL) != LONGINT_OK
        -: 1705:		|| SIZE_MAX < tmp_ulong)
    #####: 1706:	      error (LS_FAILURE, 0, _("invalid tab size: %s"),
        -: 1707:		     quotearg (optarg));
    #####: 1708:	    tabsize = tmp_ulong;
    #####: 1709:	    break;
        -: 1710:	  }
        -: 1711:
        -: 1712:	case 'U':
    #####: 1713:	  sort_type = sort_none;
    #####: 1714:	  sort_type_specified = true;
    #####: 1715:	  break;
        -: 1716:
        -: 1717:	case 'X':
    #####: 1718:	  sort_type = sort_extension;
    #####: 1719:	  sort_type_specified = true;
    #####: 1720:	  break;
        -: 1721:
        -: 1722:	case '1':
        -: 1723:	  /* -1 has no effect after -l.  */
    #####: 1724:	  if (format != long_format)
    #####: 1725:	    format = one_per_line;
    #####: 1726:	  break;
        -: 1727:
        -: 1728:        case AUTHOR_OPTION:
    #####: 1729:          print_author = true;
    #####: 1730:          break;
        -: 1731:
        -: 1732:	case HIDE_OPTION:
        -: 1733:	  {
    #####: 1734:	    struct ignore_pattern *hide = xmalloc (sizeof *hide);
    #####: 1735:	    hide->pattern = optarg;
    #####: 1736:	    hide->next = hide_patterns;
    #####: 1737:	    hide_patterns = hide;
        -: 1738:	  }
    #####: 1739:	  break;
        -: 1740:
        -: 1741:	case SORT_OPTION:
    #####: 1742:	  sort_type = XARGMATCH ("--sort", optarg, sort_args, sort_types);
    #####: 1743:	  sort_type_specified = true;
    #####: 1744:	  break;
        -: 1745:
        -: 1746:	case GROUP_DIRECTORIES_FIRST_OPTION:
    #####: 1747:	  directories_first = true;
    #####: 1748:	  break;
        -: 1749:
        -: 1750:	case TIME_OPTION:
    #####: 1751:	  time_type = XARGMATCH ("--time", optarg, time_args, time_types);
    #####: 1752:	  break;
        -: 1753:
        -: 1754:	case FORMAT_OPTION:
    #####: 1755:	  format = XARGMATCH ("--format", optarg, format_args, format_types);
    #####: 1756:	  break;
        -: 1757:
        -: 1758:	case FULL_TIME_OPTION:
    #####: 1759:	  format = long_format;
    #####: 1760:	  time_style_option = "full-iso";
    #####: 1761:	  break;
        -: 1762:
        -: 1763:	case COLOR_OPTION:
        -: 1764:	  {
        -: 1765:	    int i;
    #####: 1766:	    if (optarg)
    #####: 1767:	      i = XARGMATCH ("--color", optarg, color_args, color_types);
        -: 1768:	    else
        -: 1769:	      /* Using --color with no argument is equivalent to using
        -: 1770:		 --color=always.  */
    #####: 1771:	      i = color_always;
        -: 1772:
    #####: 1773:	    print_with_color = (i == color_always
    #####: 1774:				|| (i == color_if_tty
    #####: 1775:				    && isatty (STDOUT_FILENO)));
        -: 1776:
    #####: 1777:	    if (print_with_color)
        -: 1778:	      {
        -: 1779:		/* Don't use TAB characters in output.  Some terminal
        -: 1780:		   emulators can't handle the combination of tabs and
        -: 1781:		   color codes on the same line.  */
    #####: 1782:		tabsize = 0;
        -: 1783:	      }
    #####: 1784:	    break;
        -: 1785:	  }
        -: 1786:
        -: 1787:	case INDICATOR_STYLE_OPTION:
    #####: 1788:	  indicator_style = XARGMATCH ("--indicator-style", optarg,
        -: 1789:				       indicator_style_args,
        -: 1790:				       indicator_style_types);
    #####: 1791:	  break;
        -: 1792:
        -: 1793:	case QUOTING_STYLE_OPTION:
    #####: 1794:	  set_quoting_style (NULL,
    #####: 1795:			     XARGMATCH ("--quoting-style", optarg,
        -: 1796:					quoting_style_args,
        -: 1797:					quoting_style_vals));
    #####: 1798:	  break;
        -: 1799:
        -: 1800:	case TIME_STYLE_OPTION:
    #####: 1801:	  time_style_option = optarg;
    #####: 1802:	  break;
        -: 1803:
        -: 1804:	case SHOW_CONTROL_CHARS_OPTION:
    #####: 1805:	  qmark_funny_chars = false;
    #####: 1806:	  break;
        -: 1807:
        -: 1808:	case BLOCK_SIZE_OPTION:
        -: 1809:	  {
    #####: 1810:	    enum strtol_error e = human_options (optarg, &human_output_opts,
        -: 1811:						 &output_block_size);
    #####: 1812:	    if (e != LONGINT_OK)
    #####: 1813:	      xstrtol_fatal (e, oi, 0, long_options, optarg);
    #####: 1814:	    file_output_block_size = output_block_size;
        -: 1815:	  }
    #####: 1816:	  break;
        -: 1817:
        -: 1818:	case SI_OPTION:
    #####: 1819:	  human_output_opts = human_autoscale | human_SI;
    #####: 1820:	  file_output_block_size = output_block_size = 1;
    #####: 1821:	  break;
        -: 1822:
        -: 1823:	case 'Z':
    #####: 1824:	  print_scontext = true;
    #####: 1825:	  break;
        -: 1826:
    #####: 1827:	case_GETOPT_HELP_CHAR;
        -: 1828:
    #####: 1829:	case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);
        -: 1830:
        -: 1831:	default:
    #####: 1832:	  usage (LS_FAILURE);
        -: 1833:	}
        2: 1834:    }
        -: 1835:
        1: 1836:  max_idx = MAX (1, line_length / MIN_COLUMN_WIDTH);
        -: 1837:
        1: 1838:  filename_quoting_options = clone_quoting_options (NULL);
        1: 1839:  if (get_quoting_style (filename_quoting_options) == escape_quoting_style)
    #####: 1840:    set_char_quoting (filename_quoting_options, ' ', 1);
        1: 1841:  if (file_type <= indicator_style)
        -: 1842:    {
        -: 1843:      char const *p;
    #####: 1844:      for (p = "*=>@|" + indicator_style - file_type; *p; p++)
    #####: 1845:	set_char_quoting (filename_quoting_options, *p, 1);
        -: 1846:    }
        -: 1847:
        1: 1848:  dirname_quoting_options = clone_quoting_options (NULL);
        1: 1849:  set_char_quoting (dirname_quoting_options, ':', 1);
        -: 1850:
        -: 1851:  /* --dired is meaningful only with --format=long (-l).
        -: 1852:     Otherwise, ignore it.  FIXME: warn about this?
        -: 1853:     Alternatively, make --dired imply --format=long?  */
        1: 1854:  if (dired && format != long_format)
    #####: 1855:    dired = false;
        -: 1856:
        -: 1857:  /* If -c or -u is specified and not -l (or any other option that implies -l),
        -: 1858:     and no sort-type was specified, then sort by the ctime (-c) or atime (-u).
        -: 1859:     The behavior of ls when using either -c or -u but with neither -l nor -t
        -: 1860:     appears to be unspecified by POSIX.  So, with GNU ls, `-u' alone means
        -: 1861:     sort by atime (this is the one that's not specified by the POSIX spec),
        -: 1862:     -lu means show atime and sort by name, -lut means show atime and sort
        -: 1863:     by atime.  */
        -: 1864:
        1: 1865:  if ((time_type == time_ctime || time_type == time_atime)
    #####: 1866:      && !sort_type_specified && format != long_format)
        -: 1867:    {
    #####: 1868:      sort_type = sort_time;
        -: 1869:    }
        -: 1870:
        1: 1871:  if (format == long_format)
        -: 1872:    {
        1: 1873:      char *style = time_style_option;
        -: 1874:      static char const posix_prefix[] = "posix-";
        -: 1875:
        1: 1876:      if (! style)
        1: 1877:	if (! (style = getenv ("TIME_STYLE")))
        1: 1878:	  style = "locale";
        -: 1879:
        2: 1880:      while (strncmp (style, posix_prefix, sizeof posix_prefix - 1) == 0)
        -: 1881:	{
    #####: 1882:	  if (! hard_locale (LC_TIME))
    #####: 1883:	    return optind;
    #####: 1884:	  style += sizeof posix_prefix - 1;
        -: 1885:	}
        -: 1886:
        1: 1887:      if (*style == '+')
        -: 1888:	{
    #####: 1889:	  char *p0 = style + 1;
    #####: 1890:	  char *p1 = strchr (p0, '\n');
    #####: 1891:	  if (! p1)
    #####: 1892:	    p1 = p0;
        -: 1893:	  else
        -: 1894:	    {
    #####: 1895:	      if (strchr (p1 + 1, '\n'))
    #####: 1896:		error (LS_FAILURE, 0, _("invalid time style format %s"),
        -: 1897:		       quote (p0));
    #####: 1898:	      *p1++ = '\0';
        -: 1899:	    }
    #####: 1900:	  long_time_format[0] = p0;
    #####: 1901:	  long_time_format[1] = p1;
        -: 1902:	}
        -: 1903:      else
        1: 1904:	switch (XARGMATCH ("time style", style,
        -: 1905:			   time_style_args,
        -: 1906:			   time_style_types))
        -: 1907:	  {
        -: 1908:	  case full_iso_time_style:
    #####: 1909:	    long_time_format[0] = long_time_format[1] =
        -: 1910:	      "%Y-%m-%d %H:%M:%S.%N %z";
    #####: 1911:	    break;
        -: 1912:
        -: 1913:	  case long_iso_time_style:
        -: 1914:	  case_long_iso_time_style:
        1: 1915:	    long_time_format[0] = long_time_format[1] = "%Y-%m-%d %H:%M";
        1: 1916:	    break;
        -: 1917:
        -: 1918:	  case iso_time_style:
    #####: 1919:	    long_time_format[0] = "%Y-%m-%d ";
    #####: 1920:	    long_time_format[1] = "%m-%d %H:%M";
    #####: 1921:	    break;
        -: 1922:
        -: 1923:	  case locale_time_style:
        1: 1924:	    if (hard_locale (LC_TIME))
        -: 1925:	      {
        -: 1926:		/* Ensure that the locale has translations for both
        -: 1927:		   formats.  If not, fall back on long-iso format.  */
        -: 1928:		int i;
        1: 1929:		for (i = 0; i < 2; i++)
        -: 1930:		  {
        1: 1931:		    char const *locale_format =
        -: 1932:		      dcgettext (NULL, long_time_format[i], LC_TIME);
        1: 1933:		    if (locale_format == long_time_format[i])
        1: 1934:		      goto case_long_iso_time_style;
    #####: 1935:		    long_time_format[i] = locale_format;
        -: 1936:		  }
        -: 1937:	      }
        -: 1938:	  }
        -: 1939:    }
        -: 1940:
        1: 1941:  return optind;
        -: 1942:}
        -: 1943:
        -: 1944:/* Parse a string as part of the LS_COLORS variable; this may involve
        -: 1945:   decoding all kinds of escape characters.  If equals_end is set an
        -: 1946:   unescaped equal sign ends the string, otherwise only a : or \0
        -: 1947:   does.  Set *OUTPUT_COUNT to the number of bytes output.  Return
        -: 1948:   true if successful.
        -: 1949:
        -: 1950:   The resulting string is *not* null-terminated, but may contain
        -: 1951:   embedded nulls.
        -: 1952:
        -: 1953:   Note that both dest and src are char **; on return they point to
        -: 1954:   the first free byte after the array and the character that ended
        -: 1955:   the input string, respectively.  */
        -: 1956:
        -: 1957:static bool
    #####: 1958:get_funky_string (char **dest, const char **src, bool equals_end,
        -: 1959:		  size_t *output_count)
        -: 1960:{
        -: 1961:  char num;			/* For numerical codes */
        -: 1962:  size_t count;			/* Something to count with */
        -: 1963:  enum {
        -: 1964:    ST_GND, ST_BACKSLASH, ST_OCTAL, ST_HEX, ST_CARET, ST_END, ST_ERROR
        -: 1965:  } state;
        -: 1966:  const char *p;
        -: 1967:  char *q;
        -: 1968:
    #####: 1969:  p = *src;			/* We don't want to double-indirect */
    #####: 1970:  q = *dest;			/* the whole darn time.  */
        -: 1971:
    #####: 1972:  count = 0;			/* No characters counted in yet.  */
    #####: 1973:  num = 0;
        -: 1974:
    #####: 1975:  state = ST_GND;		/* Start in ground state.  */
    #####: 1976:  while (state < ST_END)
        -: 1977:    {
    #####: 1978:      switch (state)
        -: 1979:	{
        -: 1980:	case ST_GND:		/* Ground state (no escapes) */
    #####: 1981:	  switch (*p)
        -: 1982:	    {
        -: 1983:	    case ':':
        -: 1984:	    case '\0':
    #####: 1985:	      state = ST_END;	/* End of string */
    #####: 1986:	      break;
        -: 1987:	    case '\\':
    #####: 1988:	      state = ST_BACKSLASH; /* Backslash scape sequence */
    #####: 1989:	      ++p;
    #####: 1990:	      break;
        -: 1991:	    case '^':
    #####: 1992:	      state = ST_CARET; /* Caret escape */
    #####: 1993:	      ++p;
    #####: 1994:	      break;
        -: 1995:	    case '=':
    #####: 1996:	      if (equals_end)
        -: 1997:		{
    #####: 1998:		  state = ST_END; /* End */
    #####: 1999:		  break;
        -: 2000:		}
        -: 2001:	      /* else fall through */
        -: 2002:	    default:
    #####: 2003:	      *(q++) = *(p++);
    #####: 2004:	      ++count;
    #####: 2005:	      break;
        -: 2006:	    }
    #####: 2007:	  break;
        -: 2008:
        -: 2009:	case ST_BACKSLASH:	/* Backslash escaped character */
    #####: 2010:	  switch (*p)
        -: 2011:	    {
        -: 2012:	    case '0':
        -: 2013:	    case '1':
        -: 2014:	    case '2':
        -: 2015:	    case '3':
        -: 2016:	    case '4':
        -: 2017:	    case '5':
        -: 2018:	    case '6':
        -: 2019:	    case '7':
    #####: 2020:	      state = ST_OCTAL;	/* Octal sequence */
    #####: 2021:	      num = *p - '0';
    #####: 2022:	      break;
        -: 2023:	    case 'x':
        -: 2024:	    case 'X':
    #####: 2025:	      state = ST_HEX;	/* Hex sequence */
    #####: 2026:	      num = 0;
    #####: 2027:	      break;
        -: 2028:	    case 'a':		/* Bell */
    #####: 2029:	      num = '\a';
    #####: 2030:	      break;
        -: 2031:	    case 'b':		/* Backspace */
    #####: 2032:	      num = '\b';
    #####: 2033:	      break;
        -: 2034:	    case 'e':		/* Escape */
    #####: 2035:	      num = 27;
    #####: 2036:	      break;
        -: 2037:	    case 'f':		/* Form feed */
    #####: 2038:	      num = '\f';
    #####: 2039:	      break;
        -: 2040:	    case 'n':		/* Newline */
    #####: 2041:	      num = '\n';
    #####: 2042:	      break;
        -: 2043:	    case 'r':		/* Carriage return */
    #####: 2044:	      num = '\r';
    #####: 2045:	      break;
        -: 2046:	    case 't':		/* Tab */
    #####: 2047:	      num = '\t';
    #####: 2048:	      break;
        -: 2049:	    case 'v':		/* Vtab */
    #####: 2050:	      num = '\v';
    #####: 2051:	      break;
        -: 2052:	    case '?':		/* Delete */
    #####: 2053:              num = 127;
    #####: 2054:	      break;
        -: 2055:	    case '_':		/* Space */
    #####: 2056:	      num = ' ';
    #####: 2057:	      break;
        -: 2058:	    case '\0':		/* End of string */
    #####: 2059:	      state = ST_ERROR;	/* Error! */
    #####: 2060:	      break;
        -: 2061:	    default:		/* Escaped character like \ ^ : = */
    #####: 2062:	      num = *p;
    #####: 2063:	      break;
        -: 2064:	    }
    #####: 2065:	  if (state == ST_BACKSLASH)
        -: 2066:	    {
    #####: 2067:	      *(q++) = num;
    #####: 2068:	      ++count;
    #####: 2069:	      state = ST_GND;
        -: 2070:	    }
    #####: 2071:	  ++p;
    #####: 2072:	  break;
        -: 2073:
        -: 2074:	case ST_OCTAL:		/* Octal sequence */
    #####: 2075:	  if (*p < '0' || *p > '7')
        -: 2076:	    {
    #####: 2077:	      *(q++) = num;
    #####: 2078:	      ++count;
    #####: 2079:	      state = ST_GND;
        -: 2080:	    }
        -: 2081:	  else
    #####: 2082:	    num = (num << 3) + (*(p++) - '0');
    #####: 2083:	  break;
        -: 2084:
        -: 2085:	case ST_HEX:		/* Hex sequence */
    #####: 2086:	  switch (*p)
        -: 2087:	    {
        -: 2088:	    case '0':
        -: 2089:	    case '1':
        -: 2090:	    case '2':
        -: 2091:	    case '3':
        -: 2092:	    case '4':
        -: 2093:	    case '5':
        -: 2094:	    case '6':
        -: 2095:	    case '7':
        -: 2096:	    case '8':
        -: 2097:	    case '9':
    #####: 2098:	      num = (num << 4) + (*(p++) - '0');
    #####: 2099:	      break;
        -: 2100:	    case 'a':
        -: 2101:	    case 'b':
        -: 2102:	    case 'c':
        -: 2103:	    case 'd':
        -: 2104:	    case 'e':
        -: 2105:	    case 'f':
    #####: 2106:	      num = (num << 4) + (*(p++) - 'a') + 10;
    #####: 2107:	      break;
        -: 2108:	    case 'A':
        -: 2109:	    case 'B':
        -: 2110:	    case 'C':
        -: 2111:	    case 'D':
        -: 2112:	    case 'E':
        -: 2113:	    case 'F':
    #####: 2114:	      num = (num << 4) + (*(p++) - 'A') + 10;
    #####: 2115:	      break;
        -: 2116:	    default:
    #####: 2117:	      *(q++) = num;
    #####: 2118:	      ++count;
    #####: 2119:	      state = ST_GND;
    #####: 2120:	      break;
        -: 2121:	    }
    #####: 2122:	  break;
        -: 2123:
        -: 2124:	case ST_CARET:		/* Caret escape */
    #####: 2125:	  state = ST_GND;	/* Should be the next state... */
    #####: 2126:	  if (*p >= '@' && *p <= '~')
        -: 2127:	    {
    #####: 2128:	      *(q++) = *(p++) & 037;
    #####: 2129:	      ++count;
        -: 2130:	    }
    #####: 2131:	  else if (*p == '?')
        -: 2132:	    {
    #####: 2133:	      *(q++) = 127;
    #####: 2134:	      ++count;
        -: 2135:	    }
        -: 2136:	  else
    #####: 2137:	    state = ST_ERROR;
    #####: 2138:	  break;
        -: 2139:
        -: 2140:	default:
    #####: 2141:	  abort ();
        -: 2142:	}
        -: 2143:    }
        -: 2144:
    #####: 2145:  *dest = q;
    #####: 2146:  *src = p;
    #####: 2147:  *output_count = count;
        -: 2148:
    #####: 2149:  return state != ST_ERROR;
        -: 2150:}
        -: 2151:
        -: 2152:static void
    #####: 2153:parse_ls_color (void)
        -: 2154:{
        -: 2155:  const char *p;		/* Pointer to character being parsed */
        -: 2156:  char *buf;			/* color_buf buffer pointer */
        -: 2157:  int state;			/* State of parser */
        -: 2158:  int ind_no;			/* Indicator number */
        -: 2159:  char label[3];		/* Indicator label */
        -: 2160:  struct color_ext_type *ext;	/* Extension we are working on */
        -: 2161:
    #####: 2162:  if ((p = getenv ("LS_COLORS")) == NULL || *p == '\0')
    #####: 2163:    return;
        -: 2164:
    #####: 2165:  ext = NULL;
    #####: 2166:  strcpy (label, "??");
        -: 2167:
        -: 2168:  /* This is an overly conservative estimate, but any possible
        -: 2169:     LS_COLORS string will *not* generate a color_buf longer than
        -: 2170:     itself, so it is a safe way of allocating a buffer in
        -: 2171:     advance.  */
    #####: 2172:  buf = color_buf = xstrdup (p);
        -: 2173:
    #####: 2174:  state = 1;
    #####: 2175:  while (state > 0)
        -: 2176:    {
    #####: 2177:      switch (state)
        -: 2178:	{
        -: 2179:	case 1:		/* First label character */
    #####: 2180:	  switch (*p)
        -: 2181:	    {
        -: 2182:	    case ':':
    #####: 2183:	      ++p;
    #####: 2184:	      break;
        -: 2185:
        -: 2186:	    case '*':
        -: 2187:	      /* Allocate new extension block and add to head of
        -: 2188:		 linked list (this way a later definition will
        -: 2189:		 override an earlier one, which can be useful for
        -: 2190:		 having terminal-specific defs override global).  */
        -: 2191:
    #####: 2192:	      ext = xmalloc (sizeof *ext);
    #####: 2193:	      ext->next = color_ext_list;
    #####: 2194:	      color_ext_list = ext;
        -: 2195:
    #####: 2196:	      ++p;
    #####: 2197:	      ext->ext.string = buf;
        -: 2198:
    #####: 2199:	      state = (get_funky_string (&buf, &p, true, &ext->ext.len)
    #####: 2200:		       ? 4 : -1);
    #####: 2201:	      break;
        -: 2202:
        -: 2203:	    case '\0':
    #####: 2204:	      state = 0;	/* Done! */
    #####: 2205:	      break;
        -: 2206:
        -: 2207:	    default:	/* Assume it is file type label */
    #####: 2208:	      label[0] = *(p++);
    #####: 2209:	      state = 2;
    #####: 2210:	      break;
        -: 2211:	    }
    #####: 2212:	  break;
        -: 2213:
        -: 2214:	case 2:		/* Second label character */
    #####: 2215:	  if (*p)
        -: 2216:	    {
    #####: 2217:	      label[1] = *(p++);
    #####: 2218:	      state = 3;
        -: 2219:	    }
        -: 2220:	  else
    #####: 2221:	    state = -1;	/* Error */
    #####: 2222:	  break;
        -: 2223:
        -: 2224:	case 3:		/* Equal sign after indicator label */
    #####: 2225:	  state = -1;	/* Assume failure...  */
    #####: 2226:	  if (*(p++) == '=')/* It *should* be...  */
        -: 2227:	    {
    #####: 2228:	      for (ind_no = 0; indicator_name[ind_no] != NULL; ++ind_no)
        -: 2229:		{
    #####: 2230:		  if (STREQ (label, indicator_name[ind_no]))
        -: 2231:		    {
    #####: 2232:		      color_indicator[ind_no].string = buf;
    #####: 2233:		      state = (get_funky_string (&buf, &p, false,
        -: 2234:						 &color_indicator[ind_no].len)
    #####: 2235:			       ? 1 : -1);
    #####: 2236:		      break;
        -: 2237:		    }
        -: 2238:		}
    #####: 2239:	      if (state == -1)
    #####: 2240:		error (0, 0, _("unrecognized prefix: %s"), quotearg (label));
        -: 2241:	    }
    #####: 2242:	  break;
        -: 2243:
        -: 2244:	case 4:		/* Equal sign after *.ext */
    #####: 2245:	  if (*(p++) == '=')
        -: 2246:	    {
    #####: 2247:	      ext->seq.string = buf;
    #####: 2248:	      state = (get_funky_string (&buf, &p, false, &ext->seq.len)
    #####: 2249:		       ? 1 : -1);
        -: 2250:	    }
        -: 2251:	  else
    #####: 2252:	    state = -1;
    #####: 2253:	  break;
        -: 2254:	}
        -: 2255:    }
        -: 2256:
    #####: 2257:  if (state < 0)
        -: 2258:    {
        -: 2259:      struct color_ext_type *e;
        -: 2260:      struct color_ext_type *e2;
        -: 2261:
    #####: 2262:      error (0, 0,
        -: 2263:	     _("unparsable value for LS_COLORS environment variable"));
    #####: 2264:      free (color_buf);
    #####: 2265:      for (e = color_ext_list; e != NULL; /* empty */)
        -: 2266:	{
    #####: 2267:	  e2 = e;
    #####: 2268:	  e = e->next;
    #####: 2269:	  free (e2);
        -: 2270:	}
    #####: 2271:      print_with_color = false;
        -: 2272:    }
        -: 2273:
    #####: 2274:  if (color_indicator[C_LINK].len == 6
    #####: 2275:      && !strncmp (color_indicator[C_LINK].string, "target", 6))
    #####: 2276:    color_symlink_as_referent = true;
        -: 2277:}
        -: 2278:
        -: 2279:/* Set the exit status to report a failure.  If SERIOUS, it is a
        -: 2280:   serious failure; otherwise, it is merely a minor problem.  */
        -: 2281:
        -: 2282:static void
    #####: 2283:set_exit_status (bool serious)
        -: 2284:{
    #####: 2285:  if (serious)
    #####: 2286:    exit_status = LS_FAILURE;
    #####: 2287:  else if (exit_status == EXIT_SUCCESS)
    #####: 2288:    exit_status = LS_MINOR_PROBLEM;
    #####: 2289:}
        -: 2290:
        -: 2291:/* Assuming a failure is serious if SERIOUS, use the printf-style
        -: 2292:   MESSAGE to report the failure to access a file named FILE.  Assume
        -: 2293:   errno is set appropriately for the failure.  */
        -: 2294:
        -: 2295:static void
    #####: 2296:file_failure (bool serious, char const *message, char const *file)
        -: 2297:{
    #####: 2298:  error (0, errno, message, quotearg_colon (file));
    #####: 2299:  set_exit_status (serious);
    #####: 2300:}
        -: 2301:
        -: 2302:/* Request that the directory named NAME have its contents listed later.
        -: 2303:   If REALNAME is nonzero, it will be used instead of NAME when the
        -: 2304:   directory name is printed.  This allows symbolic links to directories
        -: 2305:   to be treated as regular directories but still be listed under their
        -: 2306:   real names.  NAME == NULL is used to insert a marker entry for the
        -: 2307:   directory named in REALNAME.
        -: 2308:   If NAME is non-NULL, we use its dev/ino information to save
        -: 2309:   a call to stat -- when doing a recursive (-R) traversal.
        -: 2310:   COMMAND_LINE_ARG means this directory was mentioned on the command line.  */
        -: 2311:
        -: 2312:static void
        1: 2313:queue_directory (char const *name, char const *realname, bool command_line_arg)
        -: 2314:{
        1: 2315:  struct pending *new = xmalloc (sizeof *new);
        1: 2316:  new->realname = realname ? xstrdup (realname) : NULL;
        1: 2317:  new->name = name ? xstrdup (name) : NULL;
        1: 2318:  new->command_line_arg = command_line_arg;
        1: 2319:  new->next = pending_dirs;
        1: 2320:  pending_dirs = new;
        1: 2321:}
        -: 2322:
        -: 2323:/* Read directory NAME, and list the files in it.
        -: 2324:   If REALNAME is nonzero, print its name instead of NAME;
        -: 2325:   this is used for symbolic links to directories.
        -: 2326:   COMMAND_LINE_ARG means this directory was mentioned on the command line.  */
        -: 2327:
        -: 2328:static void
        1: 2329:print_dir (char const *name, char const *realname, bool command_line_arg)
        -: 2330:{
        -: 2331:  DIR *dirp;
        -: 2332:  struct dirent *next;
        1: 2333:  uintmax_t total_blocks = 0;
        -: 2334:  static bool first = true;
        -: 2335:
        1: 2336:  errno = 0;
        1: 2337:  dirp = opendir (name);
        1: 2338:  if (!dirp)
        -: 2339:    {
    #####: 2340:      file_failure (command_line_arg, _("cannot open directory %s"), name);
    #####: 2341:      return;
        -: 2342:    }
        -: 2343:
        1: 2344:  if (LOOP_DETECT)
        -: 2345:    {
        -: 2346:      struct stat dir_stat;
    #####: 2347:      int fd = dirfd (dirp);
        -: 2348:
        -: 2349:      /* If dirfd failed, endure the overhead of using stat.  */
    #####: 2350:      if ((0 <= fd
    #####: 2351:	   ? fstat (fd, &dir_stat)
    #####: 2352:	   : stat (name, &dir_stat)) < 0)
        -: 2353:	{
    #####: 2354:	  file_failure (command_line_arg,
        -: 2355:			_("cannot determine device and inode of %s"), name);
    #####: 2356:	  closedir (dirp);
    #####: 2357:	  return;
        -: 2358:	}
        -: 2359:
        -: 2360:      /* If we've already visited this dev/inode pair, warn that
        -: 2361:	 we've found a loop, and do not process this directory.  */
    #####: 2362:      if (visit_dir (dir_stat.st_dev, dir_stat.st_ino))
        -: 2363:	{
    #####: 2364:	  error (0, 0, _("%s: not listing already-listed directory"),
        -: 2365:		 quotearg_colon (name));
    #####: 2366:	  closedir (dirp);
    #####: 2367:	  return;
        -: 2368:	}
        -: 2369:
    #####: 2370:      DEV_INO_PUSH (dir_stat.st_dev, dir_stat.st_ino);
        -: 2371:    }
        -: 2372:
        -: 2373:  /* Read the directory entries, and insert the subfiles into the `cwd_file'
        -: 2374:     table.  */
        -: 2375:
        1: 2376:  clear_files ();
        -: 2377:
        -: 2378:  while (1)
        -: 2379:    {
        -: 2380:      /* Set errno to zero so we can distinguish between a readdir failure
        -: 2381:	 and when readdir simply finds that there are no more entries.  */
      322: 2382:      errno = 0;
      322: 2383:      next = readdir (dirp);
      322: 2384:      if (next)
        -: 2385:	{
      321: 2386:	  if (! file_ignored (next->d_name))
        -: 2387:	    {
      321: 2388:	      enum filetype type = unknown;
        -: 2389:
        -: 2390:#if HAVE_STRUCT_DIRENT_D_TYPE
      321: 2391:	      switch (next->d_type)
        -: 2392:		{
    #####: 2393:		case DT_BLK:  type = blockdev;		break;
    #####: 2394:		case DT_CHR:  type = chardev;		break;
        3: 2395:		case DT_DIR:  type = directory;		break;
    #####: 2396:		case DT_FIFO: type = fifo;		break;
    #####: 2397:		case DT_LNK:  type = symbolic_link;	break;
      318: 2398:		case DT_REG:  type = normal;		break;
    #####: 2399:		case DT_SOCK: type = sock;		break;
        -: 2400:# ifdef DT_WHT
    #####: 2401:		case DT_WHT:  type = whiteout;		break;
        -: 2402:# endif
        -: 2403:		}
        -: 2404:#endif
      321: 2405:	      total_blocks += gobble_file (next->d_name, type, D_INO (next),
        -: 2406:					   false, name);
        -: 2407:	    }
        -: 2408:	}
        1: 2409:      else if (errno != 0)
        -: 2410:	{
    #####: 2411:	  file_failure (command_line_arg, _("reading directory %s"), name);
    #####: 2412:	  if (errno != EOVERFLOW)
    #####: 2413:	    break;
        -: 2414:	}
        -: 2415:      else
        1: 2416:	break;
      321: 2417:    }
        -: 2418:
        1: 2419:  if (closedir (dirp) != 0)
        -: 2420:    {
    #####: 2421:      file_failure (command_line_arg, _("closing directory %s"), name);
        -: 2422:      /* Don't return; print whatever we got.  */
        -: 2423:    }
        -: 2424:
        -: 2425:  /* Sort the directory contents.  */
        1: 2426:  sort_files ();
        -: 2427:
        -: 2428:  /* If any member files are subdirectories, perhaps they should have their
        -: 2429:     contents listed rather than being mentioned here as files.  */
        -: 2430:
        1: 2431:  if (recursive)
    #####: 2432:    extract_dirs_from_files (name, command_line_arg);
        -: 2433:
        1: 2434:  if (recursive | print_dir_name)
        -: 2435:    {
    #####: 2436:      if (!first)
    #####: 2437:	DIRED_PUTCHAR ('\n');
    #####: 2438:      first = false;
    #####: 2439:      DIRED_INDENT ();
    #####: 2440:      PUSH_CURRENT_DIRED_POS (&subdired_obstack);
    #####: 2441:      dired_pos += quote_name (stdout, realname ? realname : name,
        -: 2442:			       dirname_quoting_options, NULL);
    #####: 2443:      PUSH_CURRENT_DIRED_POS (&subdired_obstack);
    #####: 2444:      DIRED_FPUTS_LITERAL (":\n", stdout);
        -: 2445:    }
        -: 2446:
        1: 2447:  if (format == long_format || print_block_size)
        -: 2448:    {
        -: 2449:      const char *p;
        -: 2450:      char buf[LONGEST_HUMAN_READABLE + 1];
        -: 2451:
        1: 2452:      DIRED_INDENT ();
        1: 2453:      p = _("total");
        1: 2454:      DIRED_FPUTS (p, stdout, strlen (p));
        1: 2455:      DIRED_PUTCHAR (' ');
        1: 2456:      p = human_readable (total_blocks, buf, human_output_opts,
        -: 2457:			  ST_NBLOCKSIZE, output_block_size);
        1: 2458:      DIRED_FPUTS (p, stdout, strlen (p));
        1: 2459:      DIRED_PUTCHAR ('\n');
        -: 2460:    }
        -: 2461:
        1: 2462:  if (cwd_n_used)
        1: 2463:    print_current_files ();
        -: 2464:}
        -: 2465:
        -: 2466:/* Add `pattern' to the list of patterns for which files that match are
        -: 2467:   not listed.  */
        -: 2468:
        -: 2469:static void
    #####: 2470:add_ignore_pattern (const char *pattern)
        -: 2471:{
        -: 2472:  struct ignore_pattern *ignore;
        -: 2473:
    #####: 2474:  ignore = xmalloc (sizeof *ignore);
    #####: 2475:  ignore->pattern = pattern;
        -: 2476:  /* Add it to the head of the linked list.  */
    #####: 2477:  ignore->next = ignore_patterns;
    #####: 2478:  ignore_patterns = ignore;
    #####: 2479:}
        -: 2480:
        -: 2481:/* Return true if one of the PATTERNS matches FILE.  */
        -: 2482:
        -: 2483:static bool
      321: 2484:patterns_match (struct ignore_pattern const *patterns, char const *file)
        -: 2485:{
        -: 2486:  struct ignore_pattern const *p;
      321: 2487:  for (p = patterns; p; p = p->next)
    #####: 2488:    if (fnmatch (p->pattern, file, FNM_PERIOD) == 0)
    #####: 2489:      return true;
      321: 2490:  return false;
        -: 2491:}
        -: 2492:
        -: 2493:/* Return true if FILE should be ignored.  */
        -: 2494:
        -: 2495:static bool
      321: 2496:file_ignored (char const *name)
        -: 2497:{
      642: 2498:  return ((ignore_mode != IGNORE_MINIMAL
    #####: 2499:	   && name[0] == '.'
    #####: 2500:	   && (ignore_mode == IGNORE_DEFAULT || ! name[1 + (name[1] == '.')]))
      321: 2501:	  || (ignore_mode == IGNORE_DEFAULT
    #####: 2502:	      && patterns_match (hide_patterns, name))
      642: 2503:	  || patterns_match (ignore_patterns, name));
        -: 2504:}
        -: 2505:
        -: 2506:/* POSIX requires that a file size be printed without a sign, even
        -: 2507:   when negative.  Assume the typical case where negative sizes are
        -: 2508:   actually positive values that have wrapped around.  */
        -: 2509:
        -: 2510:static uintmax_t
      642: 2511:unsigned_file_size (off_t size)
        -: 2512:{
      642: 2513:  return size + (size < 0) * ((uintmax_t) OFF_T_MAX - OFF_T_MIN + 1);
        -: 2514:}
        -: 2515:
        -: 2516:/* Enter and remove entries in the table `cwd_file'.  */
        -: 2517:
        -: 2518:/* Empty the table of files.  */
        -: 2519:
        -: 2520:static void
        2: 2521:clear_files (void)
        -: 2522:{
        -: 2523:  size_t i;
        -: 2524:
        2: 2525:  for (i = 0; i < cwd_n_used; i++)
        -: 2526:    {
    #####: 2527:      struct fileinfo *f = sorted_file[i];
    #####: 2528:      free (f->name);
    #####: 2529:      free (f->linkname);
    #####: 2530:      if (f->scontext != UNKNOWN_SECURITY_CONTEXT)
    #####: 2531:	freecon (f->scontext);
        -: 2532:    }
        -: 2533:
        2: 2534:  cwd_n_used = 0;
        2: 2535:  any_has_acl = false;
        2: 2536:  inode_number_width = 0;
        2: 2537:  block_size_width = 0;
        2: 2538:  nlink_width = 0;
        2: 2539:  owner_width = 0;
        2: 2540:  group_width = 0;
        2: 2541:  author_width = 0;
        2: 2542:  scontext_width = 0;
        2: 2543:  major_device_number_width = 0;
        2: 2544:  minor_device_number_width = 0;
        2: 2545:  file_size_width = 0;
        2: 2546:}
        -: 2547:
        -: 2548:/* Add a file to the current table of files.
        -: 2549:   Verify that the file exists, and print an error message if it does not.
        -: 2550:   Return the number of blocks that the file occupies.  */
        -: 2551:
        -: 2552:static uintmax_t
      321: 2553:gobble_file (char const *name, enum filetype type, ino_t inode,
        -: 2554:	     bool command_line_arg, char const *dirname)
        -: 2555:{
      321: 2556:  uintmax_t blocks = 0;
        -: 2557:  struct fileinfo *f;
        -: 2558:
        -: 2559:  /* An inode value prior to gobble_file necessarily came from readdir,
        -: 2560:     which is not used for command line arguments.  */
      321: 2561:  assert (! command_line_arg || inode == NOT_AN_INODE_NUMBER);
        -: 2562:
      321: 2563:  if (cwd_n_used == cwd_n_alloc)
        -: 2564:    {
        2: 2565:      cwd_file = xnrealloc (cwd_file, cwd_n_alloc, 2 * sizeof *cwd_file);
        2: 2566:      cwd_n_alloc *= 2;
        -: 2567:    }
        -: 2568:
      321: 2569:  f = &cwd_file[cwd_n_used];
      321: 2570:  memset (f, '\0', sizeof *f);
      321: 2571:  f->stat.st_ino = inode;
      321: 2572:  f->filetype = type;
        -: 2573:
      321: 2574:  if (command_line_arg
      321: 2575:      || format_needs_stat
        -: 2576:      /* When coloring a directory (we may know the type from
        -: 2577:	 direct.d_type), we have to stat it in order to indicate
        -: 2578:	 sticky and/or other-writable attributes.  */
    #####: 2579:      || (type == directory && print_with_color)
        -: 2580:      /* When dereferencing symlinks, the inode and type must come from
        -: 2581:	 stat, but readdir provides the inode and type of lstat.  */
    #####: 2582:      || ((print_inode || format_needs_type)
    #####: 2583:	  && (type == symbolic_link || type == unknown)
    #####: 2584:	  && (dereference == DEREF_ALWAYS
    #####: 2585:	      || (command_line_arg && dereference != DEREF_NEVER)
    #####: 2586:	      || color_symlink_as_referent || check_symlink_color))
        -: 2587:      /* Command line dereferences are already taken care of by the above
        -: 2588:	 assertion that the inode number is not yet known.  */
    #####: 2589:      || (print_inode && inode == NOT_AN_INODE_NUMBER)
    #####: 2590:      || (format_needs_type
    #####: 2591:	  && (type == unknown || command_line_arg
        -: 2592:	      /* --indicator-style=classify (aka -F)
        -: 2593:		 requires that we stat each regular file
        -: 2594:		 to see if it's executable.  */
    #####: 2595:	      || (type == normal && (indicator_style == classify
        -: 2596:				     /* This is so that --color ends up
        -: 2597:					highlighting files with the executable
        -: 2598:					bit set even when options like -F are
        -: 2599:					not specified.  */
    #####: 2600:				     || (print_with_color
    #####: 2601:					 && is_colored (C_EXEC))
        -: 2602:				     )))))
        -: 2603:
        -: 2604:    {
        -: 2605:      /* Absolute name of this file.  */
        -: 2606:      char *absolute_name;
        -: 2607:      bool do_deref;
        -: 2608:      int err;
        -: 2609:
      321: 2610:      if (name[0] == '/' || dirname[0] == 0)
    #####: 2611:	absolute_name = (char *) name;
        -: 2612:      else
        -: 2613:	{
      321: 2614:	  absolute_name = alloca (strlen (name) + strlen (dirname) + 2);
      321: 2615:	  attach (absolute_name, dirname, name);
        -: 2616:	}
        -: 2617:
      321: 2618:      switch (dereference)
        -: 2619:	{
        -: 2620:	case DEREF_ALWAYS:
    #####: 2621:	  err = stat (absolute_name, &f->stat);
    #####: 2622:	  do_deref = true;
    #####: 2623:	  break;
        -: 2624:
        -: 2625:	case DEREF_COMMAND_LINE_ARGUMENTS:
        -: 2626:	case DEREF_COMMAND_LINE_SYMLINK_TO_DIR:
    #####: 2627:	  if (command_line_arg)
        -: 2628:	    {
        -: 2629:	      bool need_lstat;
    #####: 2630:	      err = stat (absolute_name, &f->stat);
    #####: 2631:	      do_deref = true;
        -: 2632:
    #####: 2633:	      if (dereference == DEREF_COMMAND_LINE_ARGUMENTS)
    #####: 2634:		break;
        -: 2635:
    #####: 2636:	      need_lstat = (err < 0
    #####: 2637:			    ? errno == ENOENT
    #####: 2638:			    : ! S_ISDIR (f->stat.st_mode));
    #####: 2639:	      if (!need_lstat)
    #####: 2640:		break;
        -: 2641:
        -: 2642:	      /* stat failed because of ENOENT, maybe indicating a dangling
        -: 2643:		 symlink.  Or stat succeeded, ABSOLUTE_NAME does not refer to a
        -: 2644:		 directory, and --dereference-command-line-symlink-to-dir is
        -: 2645:		 in effect.  Fall through so that we call lstat instead.  */
        -: 2646:	    }
        -: 2647:
        -: 2648:	default: /* DEREF_NEVER */
      321: 2649:	  err = lstat (absolute_name, &f->stat);
      321: 2650:	  do_deref = false;
      321: 2651:	  break;
        -: 2652:	}
        -: 2653:
      321: 2654:      if (err != 0)
        -: 2655:	{
        -: 2656:	  /* Failure to stat a command line argument leads to
        -: 2657:	     an exit status of 2.  For other files, stat failure
        -: 2658:	     provokes an exit status of 1.  */
    #####: 2659:	  file_failure (command_line_arg,
        -: 2660:			_("cannot access %s"), absolute_name);
    #####: 2661:	  if (command_line_arg)
    #####: 2662:	    return 0;
        -: 2663:
    #####: 2664:	  f->name = xstrdup (name);
    #####: 2665:	  cwd_n_used++;
        -: 2666:
    #####: 2667:	  return 0;
        -: 2668:	}
        -: 2669:
      321: 2670:      f->stat_ok = true;
        -: 2671:
      321: 2672:      if (format == long_format || print_scontext)
        -: 2673:	{
      321: 2674:	  bool have_acl = false;
      321: 2675:	  int attr_len = (do_deref
    #####: 2676:			  ?  getfilecon (absolute_name, &f->scontext)
      321: 2677:			  : lgetfilecon (absolute_name, &f->scontext));
      321: 2678:	  err = (attr_len < 0);
        -: 2679:
        -: 2680:	  /* Contrary to its documented API, getfilecon may return 0,
        -: 2681:	     yet set f->scontext to NULL (on at least Debian's libselinux1
        -: 2682:	     2.0.15-2+b1), so work around that bug.
        -: 2683:	     FIXME: remove this work-around in 2011, or whenever affected
        -: 2684:	     versions of libselinux are long gone.  */
      321: 2685:	  if (attr_len == 0)
        -: 2686:	    {
    #####: 2687:	      err = 0;
    #####: 2688:	      f->scontext = xstrdup ("unlabeled");
        -: 2689:	    }
        -: 2690:
      321: 2691:	  if (err == 0)
    #####: 2692:	    have_acl = ! STREQ ("unlabeled", f->scontext);
        -: 2693:	  else
        -: 2694:	    {
      321: 2695:	      f->scontext = UNKNOWN_SECURITY_CONTEXT;
        -: 2696:
        -: 2697:	      /* When requesting security context information, don't make
        -: 2698:		 ls fail just because the file (even a command line argument)
        -: 2699:		 isn't on the right type of file system.  I.e., a getfilecon
        -: 2700:		 failure isn't in the same class as a stat failure.  */
      321: 2701:	      if (errno == ENOTSUP || errno == ENODATA)
      321: 2702:		err = 0;
        -: 2703:	    }
        -: 2704:
      321: 2705:	  if (err == 0 && ! have_acl && format == long_format)
        -: 2706:	    {
      321: 2707:	      int n = file_has_acl (absolute_name, &f->stat);
      321: 2708:	      err = (n < 0);
      321: 2709:	      have_acl = (0 < n);
        -: 2710:	    }
        -: 2711:
      321: 2712:	  f->have_acl = have_acl;
      321: 2713:	  any_has_acl |= have_acl;
        -: 2714:
      321: 2715:	  if (err)
    #####: 2716:	    error (0, errno, "%s", quotearg_colon (absolute_name));
        -: 2717:	}
        -: 2718:
      321: 2719:      if (S_ISLNK (f->stat.st_mode)
    #####: 2720:	  && (format == long_format || check_symlink_color))
        -: 2721:	{
        -: 2722:	  char *linkname;
        -: 2723:	  struct stat linkstats;
        -: 2724:
    #####: 2725:	  get_link_name (absolute_name, f, command_line_arg);
    #####: 2726:	  linkname = make_link_name (absolute_name, f->linkname);
        -: 2727:
        -: 2728:	  /* Avoid following symbolic links when possible, ie, when
        -: 2729:	     they won't be traced and when no indicator is needed.  */
    #####: 2730:	  if (linkname
    #####: 2731:	      && (file_type <= indicator_style || check_symlink_color)
    #####: 2732:	      && stat (linkname, &linkstats) == 0)
        -: 2733:	    {
    #####: 2734:	      f->linkok = true;
        -: 2735:
        -: 2736:	      /* Symbolic links to directories that are mentioned on the
        -: 2737:		 command line are automatically traced if not being
        -: 2738:		 listed as files.  */
    #####: 2739:	      if (!command_line_arg || format == long_format
    #####: 2740:		  || !S_ISDIR (linkstats.st_mode))
        -: 2741:		{
        -: 2742:		  /* Get the linked-to file's mode for the filetype indicator
        -: 2743:		     in long listings.  */
    #####: 2744:		  f->linkmode = linkstats.st_mode;
        -: 2745:		}
        -: 2746:	    }
    #####: 2747:	  free (linkname);
        -: 2748:	}
        -: 2749:
        -: 2750:      /* When not distinguishing types of symlinks, pretend we know that
        -: 2751:	 it is stat'able, so that it will be colored as a regular symlink,
        -: 2752:	 and not as an orphan.  */
      321: 2753:      if (S_ISLNK (f->stat.st_mode) && !check_symlink_color)
    #####: 2754:	f->linkok = true;
        -: 2755:
      321: 2756:      if (S_ISLNK (f->stat.st_mode))
    #####: 2757:	f->filetype = symbolic_link;
      321: 2758:      else if (S_ISDIR (f->stat.st_mode))
        -: 2759:	{
        3: 2760:	  if (command_line_arg & !immediate_dirs)
    #####: 2761:	    f->filetype = arg_directory;
        -: 2762:	  else
        3: 2763:	    f->filetype = directory;
        -: 2764:	}
        -: 2765:      else
      318: 2766:	f->filetype = normal;
        -: 2767:
      321: 2768:      blocks = ST_NBLOCKS (f->stat);
      321: 2769:      if (format == long_format || print_block_size)
        -: 2770:	{
        -: 2771:	  char buf[LONGEST_HUMAN_READABLE + 1];
      321: 2772:	  int len = mbswidth (human_readable (blocks, buf, human_output_opts,
        -: 2773:					      ST_NBLOCKSIZE, output_block_size),
        -: 2774:			      0);
      321: 2775:	  if (block_size_width < len)
        2: 2776:	    block_size_width = len;
        -: 2777:	}
        -: 2778:
      321: 2779:      if (format == long_format)
        -: 2780:	{
      321: 2781:	  if (print_owner)
        -: 2782:	    {
      321: 2783:	      int len = format_user_width (f->stat.st_uid);
      321: 2784:	      if (owner_width < len)
        1: 2785:		owner_width = len;
        -: 2786:	    }
        -: 2787:
      321: 2788:	  if (print_group)
        -: 2789:	    {
      321: 2790:	      int len = format_group_width (f->stat.st_gid);
      321: 2791:	      if (group_width < len)
        1: 2792:		group_width = len;
        -: 2793:	    }
        -: 2794:
      321: 2795:	  if (print_author)
        -: 2796:	    {
    #####: 2797:	      int len = format_user_width (f->stat.st_author);
    #####: 2798:	      if (author_width < len)
    #####: 2799:		author_width = len;
        -: 2800:	    }
        -: 2801:	}
        -: 2802:
      321: 2803:      if (print_scontext)
        -: 2804:	{
    #####: 2805:	  int len = strlen (f->scontext);
    #####: 2806:	  if (scontext_width < len)
    #####: 2807:	    scontext_width = len;
        -: 2808:	}
        -: 2809:
      321: 2810:      if (format == long_format)
        -: 2811:	{
        -: 2812:	  char b[INT_BUFSIZE_BOUND (uintmax_t)];
      321: 2813:	  int b_len = strlen (umaxtostr (f->stat.st_nlink, b));
      321: 2814:	  if (nlink_width < b_len)
        2: 2815:	    nlink_width = b_len;
        -: 2816:
      321: 2817:	  if (S_ISCHR (f->stat.st_mode) || S_ISBLK (f->stat.st_mode))
    #####: 2818:	    {
        -: 2819:	      char buf[INT_BUFSIZE_BOUND (uintmax_t)];
    #####: 2820:	      int len = strlen (umaxtostr (major (f->stat.st_rdev), buf));
    #####: 2821:	      if (major_device_number_width < len)
    #####: 2822:		major_device_number_width = len;
    #####: 2823:	      len = strlen (umaxtostr (minor (f->stat.st_rdev), buf));
    #####: 2824:	      if (minor_device_number_width < len)
    #####: 2825:		minor_device_number_width = len;
    #####: 2826:	      len = major_device_number_width + 2 + minor_device_number_width;
    #####: 2827:	      if (file_size_width < len)
    #####: 2828:		file_size_width = len;
        -: 2829:	    }
        -: 2830:	  else
        -: 2831:	    {
        -: 2832:	      char buf[LONGEST_HUMAN_READABLE + 1];
      321: 2833:	      uintmax_t size = unsigned_file_size (f->stat.st_size);
      321: 2834:	      int len = mbswidth (human_readable (size, buf, human_output_opts,
        -: 2835:						  1, file_output_block_size),
        -: 2836:				  0);
      321: 2837:	      if (file_size_width < len)
        2: 2838:		file_size_width = len;
        -: 2839:	    }
        -: 2840:	}
        -: 2841:    }
        -: 2842:
      321: 2843:  if (print_inode)
        -: 2844:    {
        -: 2845:      char buf[INT_BUFSIZE_BOUND (uintmax_t)];
    #####: 2846:      int len = strlen (umaxtostr (f->stat.st_ino, buf));
    #####: 2847:      if (inode_number_width < len)
    #####: 2848:	inode_number_width = len;
        -: 2849:    }
        -: 2850:
      321: 2851:  f->name = xstrdup (name);
      321: 2852:  cwd_n_used++;
        -: 2853:
      321: 2854:  return blocks;
        -: 2855:}
        -: 2856:
        -: 2857:/* Return true if F refers to a directory.  */
        -: 2858:static bool
    #####: 2859:is_directory (const struct fileinfo *f)
        -: 2860:{
    #####: 2861:  return f->filetype == directory || f->filetype == arg_directory;
        -: 2862:}
        -: 2863:
        -: 2864:/* Put the name of the file that FILENAME is a symbolic link to
        -: 2865:   into the LINKNAME field of `f'.  COMMAND_LINE_ARG indicates whether
        -: 2866:   FILENAME is a command-line argument.  */
        -: 2867:
        -: 2868:static void
    #####: 2869:get_link_name (char const *filename, struct fileinfo *f, bool command_line_arg)
        -: 2870:{
    #####: 2871:  f->linkname = areadlink_with_size (filename, f->stat.st_size);
    #####: 2872:  if (f->linkname == NULL)
    #####: 2873:    file_failure (command_line_arg, _("cannot read symbolic link %s"),
        -: 2874:		  filename);
    #####: 2875:}
        -: 2876:
        -: 2877:/* If `linkname' is a relative name and `name' contains one or more
        -: 2878:   leading directories, return `linkname' with those directories
        -: 2879:   prepended; otherwise, return a copy of `linkname'.
        -: 2880:   If `linkname' is zero, return zero.  */
        -: 2881:
        -: 2882:static char *
    #####: 2883:make_link_name (char const *name, char const *linkname)
        -: 2884:{
        -: 2885:  char *linkbuf;
        -: 2886:  size_t bufsiz;
        -: 2887:
    #####: 2888:  if (!linkname)
    #####: 2889:    return NULL;
        -: 2890:
    #####: 2891:  if (*linkname == '/')
    #####: 2892:    return xstrdup (linkname);
        -: 2893:
        -: 2894:  /* The link is to a relative name.  Prepend any leading directory
        -: 2895:     in `name' to the link name.  */
    #####: 2896:  linkbuf = strrchr (name, '/');
    #####: 2897:  if (linkbuf == 0)
    #####: 2898:    return xstrdup (linkname);
        -: 2899:
    #####: 2900:  bufsiz = linkbuf - name + 1;
    #####: 2901:  linkbuf = xmalloc (bufsiz + strlen (linkname) + 1);
    #####: 2902:  strncpy (linkbuf, name, bufsiz);
    #####: 2903:  strcpy (linkbuf + bufsiz, linkname);
    #####: 2904:  return linkbuf;
        -: 2905:}
        -: 2906:
        -: 2907:/* Return true if the last component of NAME is `.' or `..'
        -: 2908:   This is so we don't try to recurse on `././././. ...' */
        -: 2909:
        -: 2910:static bool
    #####: 2911:basename_is_dot_or_dotdot (const char *name)
        -: 2912:{
    #####: 2913:  char const *base = last_component (name);
    #####: 2914:  return dot_or_dotdot (base);
        -: 2915:}
        -: 2916:
        -: 2917:/* Remove any entries from CWD_FILE that are for directories,
        -: 2918:   and queue them to be listed as directories instead.
        -: 2919:   DIRNAME is the prefix to prepend to each dirname
        -: 2920:   to make it correct relative to ls's working dir;
        -: 2921:   if it is null, no prefix is needed and "." and ".." should not be ignored.
        -: 2922:   If COMMAND_LINE_ARG is true, this directory was mentioned at the top level,
        -: 2923:   This is desirable when processing directories recursively.  */
        -: 2924:
        -: 2925:static void
    #####: 2926:extract_dirs_from_files (char const *dirname, bool command_line_arg)
        -: 2927:{
        -: 2928:  size_t i;
        -: 2929:  size_t j;
    #####: 2930:  bool ignore_dot_and_dot_dot = (dirname != NULL);
        -: 2931:
    #####: 2932:  if (dirname && LOOP_DETECT)
        -: 2933:    {
        -: 2934:      /* Insert a marker entry first.  When we dequeue this marker entry,
        -: 2935:	 we'll know that DIRNAME has been processed and may be removed
        -: 2936:	 from the set of active directories.  */
    #####: 2937:      queue_directory (NULL, dirname, false);
        -: 2938:    }
        -: 2939:
        -: 2940:  /* Queue the directories last one first, because queueing reverses the
        -: 2941:     order.  */
    #####: 2942:  for (i = cwd_n_used; i-- != 0; )
        -: 2943:    {
    #####: 2944:      struct fileinfo *f = sorted_file[i];
        -: 2945:
    #####: 2946:      if (is_directory (f)
    #####: 2947:	  && (! ignore_dot_and_dot_dot
    #####: 2948:	      || ! basename_is_dot_or_dotdot (f->name)))
        -: 2949:	{
    #####: 2950:	  if (!dirname || f->name[0] == '/')
    #####: 2951:	    queue_directory (f->name, f->linkname, command_line_arg);
        -: 2952:	  else
        -: 2953:	    {
    #####: 2954:	      char *name = file_name_concat (dirname, f->name, NULL);
    #####: 2955:	      queue_directory (name, f->linkname, command_line_arg);
    #####: 2956:	      free (name);
        -: 2957:	    }
    #####: 2958:	  if (f->filetype == arg_directory)
    #####: 2959:	    free (f->name);
        -: 2960:	}
        -: 2961:    }
        -: 2962:
        -: 2963:  /* Now delete the directories from the table, compacting all the remaining
        -: 2964:     entries.  */
        -: 2965:
    #####: 2966:  for (i = 0, j = 0; i < cwd_n_used; i++)
        -: 2967:    {
    #####: 2968:      struct fileinfo *f = sorted_file[i];
    #####: 2969:      sorted_file[j] = f;
    #####: 2970:      j += (f->filetype != arg_directory);
        -: 2971:    }
    #####: 2972:  cwd_n_used = j;
    #####: 2973:}
        -: 2974:
        -: 2975:/* Use strcoll to compare strings in this locale.  If an error occurs,
        -: 2976:   report an error and longjmp to failed_strcoll.  */
        -: 2977:
        -: 2978:static jmp_buf failed_strcoll;
        -: 2979:
        -: 2980:static int
     2285: 2981:xstrcoll (char const *a, char const *b)
        -: 2982:{
        -: 2983:  int diff;
     2285: 2984:  errno = 0;
     2285: 2985:  diff = strcoll (a, b);
     2285: 2986:  if (errno)
        -: 2987:    {
    #####: 2988:      error (0, errno, _("cannot compare file names %s and %s"),
        -: 2989:	     quote_n (0, a), quote_n (1, b));
    #####: 2990:      set_exit_status (false);
    #####: 2991:      longjmp (failed_strcoll, 1);
        -: 2992:    }
     2285: 2993:  return diff;
        -: 2994:}
        -: 2995:
        -: 2996:/* Comparison routines for sorting the files.  */
        -: 2997:
        -: 2998:typedef void const *V;
        -: 2999:typedef int (*qsortFunc)(V a, V b);
        -: 3000:
        -: 3001:/* Used below in DEFINE_SORT_FUNCTIONS for _df_ sort function variants.
        -: 3002:   The do { ... } while(0) makes it possible to use the macro more like
        -: 3003:   a statement, without violating C89 rules: */
        -: 3004:#define DIRFIRST_CHECK(a, b)						\
        -: 3005:  do									\
        -: 3006:    {									\
        -: 3007:      bool a_is_dir = is_directory ((struct fileinfo const *) a);	\
        -: 3008:      bool b_is_dir = is_directory ((struct fileinfo const *) b);	\
        -: 3009:      if (a_is_dir && !b_is_dir)					\
        -: 3010:	return -1;         /* a goes before b */			\
        -: 3011:      if (!a_is_dir && b_is_dir)					\
        -: 3012:	return 1;          /* b goes before a */			\
        -: 3013:    }									\
        -: 3014:  while (0)
        -: 3015:
        -: 3016:/* Define the 8 different sort function variants required for each sortkey.
        -: 3017:   KEY_NAME is a token describing the sort key, e.g., ctime, atime, size.
        -: 3018:   KEY_CMP_FUNC is a function to compare records based on that key, e.g.,
        -: 3019:   ctime_cmp, atime_cmp, size_cmp.  Append KEY_NAME to the string,
        -: 3020:   '[rev_][x]str{cmp|coll}[_df]_', to create each function name.  */
        -: 3021:#define DEFINE_SORT_FUNCTIONS(key_name, key_cmp_func)			\
        -: 3022:  /* direct, non-dirfirst versions */					\
        -: 3023:  static int xstrcoll_##key_name (V a, V b)				\
        -: 3024:  { return key_cmp_func (a, b, xstrcoll); }				\
        -: 3025:  static int strcmp_##key_name (V a, V b)				\
        -: 3026:  { return key_cmp_func (a, b, strcmp); }				\
        -: 3027:									\
        -: 3028:  /* reverse, non-dirfirst versions */					\
        -: 3029:  static int rev_xstrcoll_##key_name (V a, V b)				\
        -: 3030:  { return key_cmp_func (b, a, xstrcoll); }				\
        -: 3031:  static int rev_strcmp_##key_name (V a, V b)				\
        -: 3032:  { return key_cmp_func (b, a, strcmp); }				\
        -: 3033:									\
        -: 3034:  /* direct, dirfirst versions */					\
        -: 3035:  static int xstrcoll_df_##key_name (V a, V b)				\
        -: 3036:  { DIRFIRST_CHECK (a, b); return key_cmp_func (a, b, xstrcoll); }	\
        -: 3037:  static int strcmp_df_##key_name (V a, V b)				\
        -: 3038:  { DIRFIRST_CHECK (a, b); return key_cmp_func (a, b, strcmp); }	\
        -: 3039:									\
        -: 3040:  /* reverse, dirfirst versions */					\
        -: 3041:  static int rev_xstrcoll_df_##key_name (V a, V b)			\
        -: 3042:  { DIRFIRST_CHECK (a, b); return key_cmp_func (b, a, xstrcoll); }	\
        -: 3043:  static int rev_strcmp_df_##key_name (V a, V b)			\
        -: 3044:  { DIRFIRST_CHECK (a, b); return key_cmp_func (b, a, strcmp); }
        -: 3045:
        -: 3046:static inline int
    #####: 3047:cmp_ctime (struct fileinfo const *a, struct fileinfo const *b,
        -: 3048:	   int (*cmp) (char const *, char const *))
        -: 3049:{
    #####: 3050:  int diff = timespec_cmp (get_stat_ctime (&b->stat),
        -: 3051:			   get_stat_ctime (&a->stat));
    #####: 3052:  return diff ? diff : cmp (a->name, b->name);
        -: 3053:}
        -: 3054:
        -: 3055:static inline int
    #####: 3056:cmp_mtime (struct fileinfo const *a, struct fileinfo const *b,
        -: 3057:	   int (*cmp) (char const *, char const *))
        -: 3058:{
    #####: 3059:  int diff = timespec_cmp (get_stat_mtime (&b->stat),
        -: 3060:			   get_stat_mtime (&a->stat));
    #####: 3061:  return diff ? diff : cmp (a->name, b->name);
        -: 3062:}
        -: 3063:
        -: 3064:static inline int
    #####: 3065:cmp_atime (struct fileinfo const *a, struct fileinfo const *b,
        -: 3066:	   int (*cmp) (char const *, char const *))
        -: 3067:{
    #####: 3068:  int diff = timespec_cmp (get_stat_atime (&b->stat),
        -: 3069:			   get_stat_atime (&a->stat));
    #####: 3070:  return diff ? diff : cmp (a->name, b->name);
        -: 3071:}
        -: 3072:
        -: 3073:static inline int
    #####: 3074:cmp_size (struct fileinfo const *a, struct fileinfo const *b,
        -: 3075:	  int (*cmp) (char const *, char const *))
        -: 3076:{
    #####: 3077:  int diff = longdiff (b->stat.st_size, a->stat.st_size);
    #####: 3078:  return diff ? diff : cmp (a->name, b->name);
        -: 3079:}
        -: 3080:
        -: 3081:static inline int
     2285: 3082:cmp_name (struct fileinfo const *a, struct fileinfo const *b,
        -: 3083:	  int (*cmp) (char const *, char const *))
        -: 3084:{
     2285: 3085:  return cmp (a->name, b->name);
        -: 3086:}
        -: 3087:
        -: 3088:/* Compare file extensions.  Files with no extension are `smallest'.
        -: 3089:   If extensions are the same, compare by filenames instead.  */
        -: 3090:
        -: 3091:static inline int
    #####: 3092:cmp_extension (struct fileinfo const *a, struct fileinfo const *b,
        -: 3093:	       int (*cmp) (char const *, char const *))
        -: 3094:{
    #####: 3095:  char const *base1 = strrchr (a->name, '.');
    #####: 3096:  char const *base2 = strrchr (b->name, '.');
    #####: 3097:  int diff = cmp (base1 ? base1 : "", base2 ? base2 : "");
    #####: 3098:  return diff ? diff : cmp (a->name, b->name);
        -: 3099:}
        -: 3100:
    #####: 3101:DEFINE_SORT_FUNCTIONS (ctime, cmp_ctime)
    #####: 3102:DEFINE_SORT_FUNCTIONS (mtime, cmp_mtime)
    #####: 3103:DEFINE_SORT_FUNCTIONS (atime, cmp_atime)
    #####: 3104:DEFINE_SORT_FUNCTIONS (size, cmp_size)
     2285: 3105:DEFINE_SORT_FUNCTIONS (name, cmp_name)
    #####: 3106:DEFINE_SORT_FUNCTIONS (extension, cmp_extension)
        -: 3107:
        -: 3108:/* Compare file versions.
        -: 3109:   Unlike all other compare functions above, cmp_version depends only
        -: 3110:   on strverscmp, which does not fail (even for locale reasons), and does not
        -: 3111:   need a secondary sort key.
        -: 3112:   All the other sort options, in fact, need xstrcoll and strcmp variants,
        -: 3113:   because they all use a string comparison (either as the primary or secondary
        -: 3114:   sort key), and xstrcoll has the ability to do a longjmp if strcoll fails for
        -: 3115:   locale reasons.  Last, strverscmp is ALWAYS available in coreutils,
        -: 3116:   thanks to the gnulib library. */
        -: 3117:static inline int
    #####: 3118:cmp_version (struct fileinfo const *a, struct fileinfo const *b)
        -: 3119:{
    #####: 3120:  return strverscmp (a->name, b->name);
        -: 3121:}
        -: 3122:
    #####: 3123:static int xstrcoll_version (V a, V b)
    #####: 3124:{ return cmp_version (a, b); }
    #####: 3125:static int rev_xstrcoll_version (V a, V b)
    #####: 3126:{ return cmp_version (b, a); }
    #####: 3127:static int xstrcoll_df_version (V a, V b)
    #####: 3128:{ DIRFIRST_CHECK (a, b); return cmp_version (a, b); }
    #####: 3129:static int rev_xstrcoll_df_version (V a, V b)
    #####: 3130:{ DIRFIRST_CHECK (a, b); return cmp_version (b, a); }
        -: 3131:
        -: 3132:
        -: 3133:/* We have 2^3 different variants for each sortkey function
        -: 3134:   (for 3 independent sort modes).
        -: 3135:   The function pointers stored in this array must be dereferenced as:
        -: 3136:
        -: 3137:    sort_variants[sort_key][use_strcmp][reverse][dirs_first]
        -: 3138:
        -: 3139:   Note that the order in which sortkeys are listed in the function pointer
        -: 3140:   array below is defined by the order of the elements in the time_type and
        -: 3141:   sort_type enums!  */
        -: 3142:
        -: 3143:#define LIST_SORTFUNCTION_VARIANTS(key_name)                        \
        -: 3144:  {                                                                 \
        -: 3145:    {                                                               \
        -: 3146:      { xstrcoll_##key_name, xstrcoll_df_##key_name },              \
        -: 3147:      { rev_xstrcoll_##key_name, rev_xstrcoll_df_##key_name },      \
        -: 3148:    },                                                              \
        -: 3149:    {                                                               \
        -: 3150:      { strcmp_##key_name, strcmp_df_##key_name },                  \
        -: 3151:      { rev_strcmp_##key_name, rev_strcmp_df_##key_name },          \
        -: 3152:    }                                                               \
        -: 3153:  }
        -: 3154:
        -: 3155:static qsortFunc sort_functions[][2][2][2] =
        -: 3156:  {
        -: 3157:    LIST_SORTFUNCTION_VARIANTS (name),
        -: 3158:    LIST_SORTFUNCTION_VARIANTS (extension),
        -: 3159:    LIST_SORTFUNCTION_VARIANTS (size),
        -: 3160:
        -: 3161:    {
        -: 3162:      {
        -: 3163:        { xstrcoll_version, xstrcoll_df_version },
        -: 3164:        { rev_xstrcoll_version, rev_xstrcoll_df_version },
        -: 3165:      },
        -: 3166:
        -: 3167:      /* We use NULL for the strcmp variants of version comparison
        -: 3168:         since as explained in cmp_version definition, version comparison
        -: 3169:         does not rely on xstrcoll, so it will never longjmp, and never
        -: 3170:         need to try the strcmp fallback. */
        -: 3171:      {
        -: 3172:        { NULL, NULL },
        -: 3173:        { NULL, NULL },
        -: 3174:      }
        -: 3175:    },
        -: 3176:
        -: 3177:    /* last are time sort functions */
        -: 3178:    LIST_SORTFUNCTION_VARIANTS (mtime),
        -: 3179:    LIST_SORTFUNCTION_VARIANTS (ctime),
        -: 3180:    LIST_SORTFUNCTION_VARIANTS (atime)
        -: 3181:  };
        -: 3182:
        -: 3183:/* The number of sortkeys is calculated as
        -: 3184:     the number of elements in the sort_type enum (i.e. sort_numtypes) +
        -: 3185:     the number of elements in the time_type enum (i.e. time_numtypes) - 1
        -: 3186:   This is because when sort_type==sort_time, we have up to
        -: 3187:   time_numtypes possible sortkeys.
        -: 3188:
        -: 3189:   This line verifies at compile-time that the array of sort functions has been
        -: 3190:   initialized for all possible sortkeys. */
        -: 3191:verify (ARRAY_CARDINALITY (sort_functions)
        -: 3192:	== sort_numtypes + time_numtypes - 1 );
        -: 3193:
        -: 3194:/* Set up SORTED_FILE to point to the in-use entries in CWD_FILE, in order.  */
        -: 3195:
        -: 3196:static void
        1: 3197:initialize_ordering_vector (void)
        -: 3198:{
        -: 3199:  size_t i;
      322: 3200:  for (i = 0; i < cwd_n_used; i++)
      321: 3201:    sorted_file[i] = &cwd_file[i];
        1: 3202:}
        -: 3203:
        -: 3204:/* Sort the files now in the table.  */
        -: 3205:
        -: 3206:static void
        1: 3207:sort_files (void)
        -: 3208:{
        -: 3209:  bool use_strcmp;
        -: 3210:
        1: 3211:  if (sorted_file_alloc < cwd_n_used + cwd_n_used / 2)
        -: 3212:    {
        1: 3213:      free (sorted_file);
        1: 3214:      sorted_file = xnmalloc (cwd_n_used, 3 * sizeof *sorted_file);
        1: 3215:      sorted_file_alloc = 3 * cwd_n_used;
        -: 3216:    }
        -: 3217:
        1: 3218:  initialize_ordering_vector ();
        -: 3219:
        1: 3220:  if (sort_type == sort_none)
        1: 3221:    return;
        -: 3222:
        -: 3223:  /* Try strcoll.  If it fails, fall back on strcmp.  We can't safely
        -: 3224:     ignore strcoll failures, as a failing strcoll might be a
        -: 3225:     comparison function that is not a total order, and if we ignored
        -: 3226:     the failure this might cause qsort to dump core.  */
        -: 3227:
        1: 3228:  if (! setjmp (failed_strcoll))
        1: 3229:    use_strcmp = false;      /* strcoll() succeeded */
        -: 3230:  else
        -: 3231:    {
    #####: 3232:      use_strcmp = true;
    #####: 3233:      assert (sort_type != sort_version);
    #####: 3234:      initialize_ordering_vector ();
        -: 3235:    }
        -: 3236:
        -: 3237:  /* When sort_type == sort_time, use time_type as subindex.  */
        2: 3238:  mpsort ((void const **) sorted_file, cwd_n_used,
        1: 3239:	  sort_functions[sort_type + (sort_type == sort_time ? time_type : 0)]
        -: 3240:			[use_strcmp][sort_reverse]
        1: 3241:			[directories_first]);
        -: 3242:}
        -: 3243:
        -: 3244:/* List all the files now in the table.  */
        -: 3245:
        -: 3246:static void
        1: 3247:print_current_files (void)
        -: 3248:{
        -: 3249:  size_t i;
        -: 3250:
        1: 3251:  switch (format)
        -: 3252:    {
        -: 3253:    case one_per_line:
    #####: 3254:      for (i = 0; i < cwd_n_used; i++)
        -: 3255:	{
    #####: 3256:	  print_file_name_and_frills (sorted_file[i]);
    #####: 3257:	  putchar ('\n');
        -: 3258:	}
    #####: 3259:      break;
        -: 3260:
        -: 3261:    case many_per_line:
    #####: 3262:      print_many_per_line ();
    #####: 3263:      break;
        -: 3264:
        -: 3265:    case horizontal:
    #####: 3266:      print_horizontal ();
    #####: 3267:      break;
        -: 3268:
        -: 3269:    case with_commas:
    #####: 3270:      print_with_commas ();
    #####: 3271:      break;
        -: 3272:
        -: 3273:    case long_format:
      322: 3274:      for (i = 0; i < cwd_n_used; i++)
        -: 3275:	{
      321: 3276:	  print_long_format (sorted_file[i]);
      321: 3277:	  DIRED_PUTCHAR ('\n');
        -: 3278:	}
        1: 3279:      break;
        -: 3280:    }
        1: 3281:}
        -: 3282:
        -: 3283:/* Return the expected number of columns in a long-format time stamp,
        -: 3284:   or zero if it cannot be calculated.  */
        -: 3285:
        -: 3286:static int
    #####: 3287:long_time_expected_width (void)
        -: 3288:{
        -: 3289:  static int width = -1;
        -: 3290:
    #####: 3291:  if (width < 0)
        -: 3292:    {
    #####: 3293:      time_t epoch = 0;
    #####: 3294:      struct tm const *tm = localtime (&epoch);
        -: 3295:      char buf[TIME_STAMP_LEN_MAXIMUM + 1];
        -: 3296:
        -: 3297:      /* In case you're wondering if localtime can fail with an input time_t
        -: 3298:	 value of 0, let's just say it's very unlikely, but not inconceivable.
        -: 3299:	 The TZ environment variable would have to specify a time zone that
        -: 3300:	 is 2**31-1900 years or more ahead of UTC.  This could happen only on
        -: 3301:	 a 64-bit system that blindly accepts e.g., TZ=UTC+20000000000000.
        -: 3302:	 However, this is not possible with Solaris 10 or glibc-2.3.5, since
        -: 3303:	 their implementations limit the offset to 167:59 and 24:00, resp.  */
    #####: 3304:      if (tm)
        -: 3305:	{
    #####: 3306:	  size_t len =
    #####: 3307:	    nstrftime (buf, sizeof buf, long_time_format[0], tm, 0, 0);
    #####: 3308:	  if (len != 0)
    #####: 3309:	    width = mbsnwidth (buf, len, 0);
        -: 3310:	}
        -: 3311:
    #####: 3312:      if (width < 0)
    #####: 3313:	width = 0;
        -: 3314:    }
        -: 3315:
    #####: 3316:  return width;
        -: 3317:}
        -: 3318:
        -: 3319:/* Print the user or group name NAME, with numeric id ID, using a
        -: 3320:   print width of WIDTH columns.  */
        -: 3321:
        -: 3322:static void
      642: 3323:format_user_or_group (char const *name, unsigned long int id, int width)
        -: 3324:{
        -: 3325:  size_t len;
        -: 3326:
      642: 3327:  if (name)
        -: 3328:    {
      642: 3329:      int width_gap = width - mbswidth (name, 0);
      642: 3330:      int pad = MAX (0, width_gap);
      642: 3331:      fputs (name, stdout);
      642: 3332:      len = strlen (name) + pad;
        -: 3333:
        -: 3334:      do
      642: 3335:	putchar (' ');
      642: 3336:      while (pad--);
        -: 3337:    }
        -: 3338:  else
        -: 3339:    {
    #####: 3340:      printf ("%*lu ", width, id);
    #####: 3341:      len = width;
        -: 3342:    }
        -: 3343:
      642: 3344:  dired_pos += len + 1;
      642: 3345:}
        -: 3346:
        -: 3347:/* Print the name or id of the user with id U, using a print width of
        -: 3348:   WIDTH.  */
        -: 3349:
        -: 3350:static void
      321: 3351:format_user (uid_t u, int width, bool stat_ok)
        -: 3352:{
      321: 3353:  format_user_or_group (! stat_ok ? "?" :
        -: 3354:			(numeric_ids ? NULL : getuser (u)), u, width);
      321: 3355:}
        -: 3356:
        -: 3357:/* Likewise, for groups.  */
        -: 3358:
        -: 3359:static void
      321: 3360:format_group (gid_t g, int width, bool stat_ok)
        -: 3361:{
      321: 3362:  format_user_or_group (! stat_ok ? "?" :
        -: 3363:			(numeric_ids ? NULL : getgroup (g)), g, width);
      321: 3364:}
        -: 3365:
        -: 3366:/* Return the number of columns that format_user_or_group will print.  */
        -: 3367:
        -: 3368:static int
      642: 3369:format_user_or_group_width (char const *name, unsigned long int id)
        -: 3370:{
      642: 3371:  if (name)
        -: 3372:    {
      642: 3373:      int len = mbswidth (name, 0);
      642: 3374:      return MAX (0, len);
        -: 3375:    }
        -: 3376:  else
        -: 3377:    {
        -: 3378:      char buf[INT_BUFSIZE_BOUND (unsigned long int)];
    #####: 3379:      sprintf (buf, "%lu", id);
    #####: 3380:      return strlen (buf);
        -: 3381:    }
        -: 3382:}
        -: 3383:
        -: 3384:/* Return the number of columns that format_user will print.  */
        -: 3385:
        -: 3386:static int
      321: 3387:format_user_width (uid_t u)
        -: 3388:{
      321: 3389:  return format_user_or_group_width (numeric_ids ? NULL : getuser (u), u);
        -: 3390:}
        -: 3391:
        -: 3392:/* Likewise, for groups.  */
        -: 3393:
        -: 3394:static int
      321: 3395:format_group_width (gid_t g)
        -: 3396:{
      321: 3397:  return format_user_or_group_width (numeric_ids ? NULL : getgroup (g), g);
        -: 3398:}
        -: 3399:
        -: 3400:
        -: 3401:/* Print information about F in long format.  */
        -: 3402:
        -: 3403:static void
      321: 3404:print_long_format (const struct fileinfo *f)
        -: 3405:{
        -: 3406:  char modebuf[12];
        -: 3407:  char buf
        -: 3408:    [LONGEST_HUMAN_READABLE + 1		/* inode */
        -: 3409:     + LONGEST_HUMAN_READABLE + 1	/* size in blocks */
        -: 3410:     + sizeof (modebuf) - 1 + 1		/* mode string */
        -: 3411:     + INT_BUFSIZE_BOUND (uintmax_t)	/* st_nlink */
        -: 3412:     + LONGEST_HUMAN_READABLE + 2	/* major device number */
        -: 3413:     + LONGEST_HUMAN_READABLE + 1	/* minor device number */
        -: 3414:     + TIME_STAMP_LEN_MAXIMUM + 1	/* max length of time/date */
        -: 3415:     ];
        -: 3416:  size_t s;
        -: 3417:  char *p;
        -: 3418:  struct timespec when_timespec;
        -: 3419:  struct tm *when_local;
        -: 3420:
        -: 3421:  /* Compute the mode string, except remove the trailing space if no
        -: 3422:     file in this directory has an ACL or SELinux security context.  */
      321: 3423:  if (f->stat_ok)
      321: 3424:    filemodestring (&f->stat, modebuf);
        -: 3425:  else
        -: 3426:    {
    #####: 3427:      modebuf[0] = filetype_letter[f->filetype];
    #####: 3428:      memset (modebuf + 1, '?', 10);
    #####: 3429:      modebuf[11] = '\0';
        -: 3430:    }
      321: 3431:  if (! any_has_acl)
      321: 3432:    modebuf[10] = '\0';
    #####: 3433:  else if (f->have_acl)
    #####: 3434:    modebuf[10] = '+';
        -: 3435:
      321: 3436:  switch (time_type)
        -: 3437:    {
        -: 3438:    case time_ctime:
    #####: 3439:      when_timespec = get_stat_ctime (&f->stat);
    #####: 3440:      break;
        -: 3441:    case time_mtime:
      321: 3442:      when_timespec = get_stat_mtime (&f->stat);
      321: 3443:      break;
        -: 3444:    case time_atime:
    #####: 3445:      when_timespec = get_stat_atime (&f->stat);
    #####: 3446:      break;
        -: 3447:    default:
    #####: 3448:      abort ();
        -: 3449:    }
        -: 3450:
      321: 3451:  p = buf;
        -: 3452:
      321: 3453:  if (print_inode)
        -: 3454:    {
        -: 3455:      char hbuf[INT_BUFSIZE_BOUND (uintmax_t)];
    #####: 3456:      sprintf (p, "%*s ", inode_number_width,
    #####: 3457:	       (f->stat.st_ino == NOT_AN_INODE_NUMBER
        -: 3458:		? "?"
    #####: 3459:		: umaxtostr (f->stat.st_ino, hbuf)));
        -: 3460:      /* Increment by strlen (p) here, rather than by inode_number_width + 1.
        -: 3461:	 The latter is wrong when inode_number_width is zero.  */
    #####: 3462:      p += strlen (p);
        -: 3463:    }
        -: 3464:
      321: 3465:  if (print_block_size)
        -: 3466:    {
        -: 3467:      char hbuf[LONGEST_HUMAN_READABLE + 1];
    #####: 3468:      char const *blocks =
    #####: 3469:	(! f->stat_ok
        -: 3470:	 ? "?"
    #####: 3471:	 : human_readable (ST_NBLOCKS (f->stat), hbuf, human_output_opts,
        -: 3472:			   ST_NBLOCKSIZE, output_block_size));
        -: 3473:      int pad;
    #####: 3474:      for (pad = block_size_width - mbswidth (blocks, 0); 0 < pad; pad--)
    #####: 3475:	*p++ = ' ';
    #####: 3476:      while ((*p++ = *blocks++))
    #####: 3477:	continue;
    #####: 3478:      p[-1] = ' ';
        -: 3479:    }
        -: 3480:
        -: 3481:  /* The last byte of the mode string is the POSIX
        -: 3482:     "optional alternate access method flag".  */
        -: 3483:  {
        -: 3484:    char hbuf[INT_BUFSIZE_BOUND (uintmax_t)];
      642: 3485:    sprintf (p, "%s %*s ", modebuf, nlink_width,
      642: 3486:	     ! f->stat_ok ? "?" : umaxtostr (f->stat.st_nlink, hbuf));
        -: 3487:  }
        -: 3488:  /* Increment by strlen (p) here, rather than by, e.g.,
        -: 3489:     sizeof modebuf - 2 + any_has_acl + 1 + nlink_width + 1.
        -: 3490:     The latter is wrong when nlink_width is zero.  */
      321: 3491:  p += strlen (p);
        -: 3492:
      321: 3493:  DIRED_INDENT ();
        -: 3494:
      321: 3495:  if (print_owner | print_group | print_author | print_scontext)
        -: 3496:    {
      321: 3497:      DIRED_FPUTS (buf, stdout, p - buf);
        -: 3498:
      321: 3499:      if (print_owner)
      321: 3500:	format_user (f->stat.st_uid, owner_width, f->stat_ok);
        -: 3501:
      321: 3502:      if (print_group)
      321: 3503:	format_group (f->stat.st_gid, group_width, f->stat_ok);
        -: 3504:
      321: 3505:      if (print_author)
    #####: 3506:	format_user (f->stat.st_author, author_width, f->stat_ok);
        -: 3507:
      321: 3508:      if (print_scontext)
    #####: 3509:	format_user_or_group (f->scontext, 0, scontext_width);
        -: 3510:
      321: 3511:      p = buf;
        -: 3512:    }
        -: 3513:
      321: 3514:  if (f->stat_ok
      321: 3515:      && (S_ISCHR (f->stat.st_mode) || S_ISBLK (f->stat.st_mode)))
    #####: 3516:    {
        -: 3517:      char majorbuf[INT_BUFSIZE_BOUND (uintmax_t)];
        -: 3518:      char minorbuf[INT_BUFSIZE_BOUND (uintmax_t)];
    #####: 3519:      int blanks_width = (file_size_width
    #####: 3520:			  - (major_device_number_width + 2
    #####: 3521:			     + minor_device_number_width));
    #####: 3522:      sprintf (p, "%*s, %*s ",
    #####: 3523:	       major_device_number_width + MAX (0, blanks_width),
    #####: 3524:	       umaxtostr (major (f->stat.st_rdev), majorbuf),
        -: 3525:	       minor_device_number_width,
    #####: 3526:	       umaxtostr (minor (f->stat.st_rdev), minorbuf));
    #####: 3527:      p += file_size_width + 1;
        -: 3528:    }
        -: 3529:  else
        -: 3530:    {
        -: 3531:      char hbuf[LONGEST_HUMAN_READABLE + 1];
      321: 3532:      char const *size =
      321: 3533:	(! f->stat_ok
        -: 3534:	 ? "?"
      321: 3535:	 : human_readable (unsigned_file_size (f->stat.st_size),
        -: 3536:			   hbuf, human_output_opts, 1, file_output_block_size));
        -: 3537:      int pad;
      588: 3538:      for (pad = file_size_width - mbswidth (size, 0); 0 < pad; pad--)
      267: 3539:	*p++ = ' ';
     2301: 3540:      while ((*p++ = *size++))
     1659: 3541:	continue;
      321: 3542:      p[-1] = ' ';
        -: 3543:    }
        -: 3544:
      321: 3545:  when_local = localtime (&when_timespec.tv_sec);
      321: 3546:  s = 0;
      321: 3547:  *p = '\1';
        -: 3548:
      321: 3549:  if (f->stat_ok && when_local)
        -: 3550:    {
        -: 3551:      struct timespec six_months_ago;
        -: 3552:      bool recent;
        -: 3553:      char const *fmt;
        -: 3554:
        -: 3555:      /* If the file appears to be in the future, update the current
        -: 3556:	 time, in case the file happens to have been modified since
        -: 3557:	 the last time we checked the clock.  */
      321: 3558:      if (timespec_cmp (current_time, when_timespec) < 0)
        -: 3559:	{
        -: 3560:	  /* Note that gettime may call gettimeofday which, on some non-
        -: 3561:	     compliant systems, clobbers the buffer used for localtime's result.
        -: 3562:	     But it's ok here, because we use a gettimeofday wrapper that
        -: 3563:	     saves and restores the buffer around the gettimeofday call.  */
        1: 3564:	  gettime (&current_time);
        -: 3565:	}
        -: 3566:
        -: 3567:      /* Consider a time to be recent if it is within the past six
        -: 3568:	 months.  A Gregorian year has 365.2425 * 24 * 60 * 60 ==
        -: 3569:	 31556952 seconds on the average.  Write this value as an
        -: 3570:	 integer constant to avoid floating point hassles.  */
      321: 3571:      six_months_ago.tv_sec = current_time.tv_sec - 31556952 / 2;
      321: 3572:      six_months_ago.tv_nsec = current_time.tv_nsec;
        -: 3573:
      642: 3574:      recent = (timespec_cmp (six_months_ago, when_timespec) < 0
      321: 3575:		&& (timespec_cmp (when_timespec, current_time) < 0));
      321: 3576:      fmt = long_time_format[recent];
        -: 3577:
        -: 3578:      /* We assume here that all time zones are offset from UTC by a
        -: 3579:	 whole number of seconds.  */
      321: 3580:      s = nstrftime (p, TIME_STAMP_LEN_MAXIMUM + 1, fmt,
      321: 3581:		     when_local, 0, when_timespec.tv_nsec);
        -: 3582:    }
        -: 3583:
      321: 3584:  if (s || !*p)
        -: 3585:    {
      321: 3586:      p += s;
      321: 3587:      *p++ = ' ';
        -: 3588:
        -: 3589:      /* NUL-terminate the string -- fputs (via DIRED_FPUTS) requires it.  */
      321: 3590:      *p = '\0';
        -: 3591:    }
        -: 3592:  else
        -: 3593:    {
        -: 3594:      /* The time cannot be converted using the desired format, so
        -: 3595:	 print it as a huge integer number of seconds.  */
        -: 3596:      char hbuf[INT_BUFSIZE_BOUND (intmax_t)];
    #####: 3597:      sprintf (p, "%*s ", long_time_expected_width (),
    #####: 3598:	       (! f->stat_ok
        -: 3599:		? "?"
        -: 3600:		: (TYPE_SIGNED (time_t)
        -: 3601:		   ? imaxtostr (when_timespec.tv_sec, hbuf)
    #####: 3602:		   : umaxtostr (when_timespec.tv_sec, hbuf))));
        -: 3603:      /* FIXME: (maybe) We discarded when_timespec.tv_nsec. */
    #####: 3604:      p += strlen (p);
        -: 3605:    }
        -: 3606:
      321: 3607:  DIRED_FPUTS (buf, stdout, p - buf);
      642: 3608:  print_name_with_quoting (f->name, FILE_OR_LINK_MODE (f), f->linkok,
      321: 3609:			   f->stat_ok, f->filetype, &dired_obstack);
        -: 3610:
      321: 3611:  if (f->filetype == symbolic_link)
        -: 3612:    {
    #####: 3613:      if (f->linkname)
        -: 3614:	{
    #####: 3615:	  DIRED_FPUTS_LITERAL (" -> ", stdout);
    #####: 3616:	  print_name_with_quoting (f->linkname, f->linkmode, f->linkok - 1,
    #####: 3617:				   f->stat_ok, f->filetype, NULL);
    #####: 3618:	  if (indicator_style != none)
    #####: 3619:	    print_type_indicator (true, f->linkmode, unknown);
        -: 3620:	}
        -: 3621:    }
      321: 3622:  else if (indicator_style != none)
    #####: 3623:    print_type_indicator (f->stat_ok, f->stat.st_mode, f->filetype);
      321: 3624:}
        -: 3625:
        -: 3626:/* Output to OUT a quoted representation of the file name NAME,
        -: 3627:   using OPTIONS to control quoting.  Produce no output if OUT is NULL.
        -: 3628:   Store the number of screen columns occupied by NAME's quoted
        -: 3629:   representation into WIDTH, if non-NULL.  Return the number of bytes
        -: 3630:   produced.  */
        -: 3631:
        -: 3632:static size_t
      321: 3633:quote_name (FILE *out, const char *name, struct quoting_options const *options,
        -: 3634:	    size_t *width)
        -: 3635:{
        -: 3636:  char smallbuf[BUFSIZ];
      321: 3637:  size_t len = quotearg_buffer (smallbuf, sizeof smallbuf, name, -1, options);
        -: 3638:  char *buf;
        -: 3639:  size_t displayed_width IF_LINT (= 0);
        -: 3640:
      321: 3641:  if (len < sizeof smallbuf)
      321: 3642:    buf = smallbuf;
        -: 3643:  else
        -: 3644:    {
    #####: 3645:      buf = alloca (len + 1);
    #####: 3646:      quotearg_buffer (buf, len + 1, name, -1, options);
        -: 3647:    }
        -: 3648:
      321: 3649:  if (qmark_funny_chars)
        -: 3650:    {
        -: 3651:#if HAVE_MBRTOWC
      321: 3652:      if (MB_CUR_MAX > 1)
        -: 3653:	{
      321: 3654:	  char const *p = buf;
      321: 3655:	  char const *plimit = buf + len;
      321: 3656:	  char *q = buf;
      321: 3657:	  displayed_width = 0;
        -: 3658:
     3056: 3659:	  while (p < plimit)
     2414: 3660:	    switch (*p)
        -: 3661:	      {
        -: 3662:		case ' ': case '!': case '"': case '#': case '%':
        -: 3663:		case '&': case '\'': case '(': case ')': case '*':
        -: 3664:		case '+': case ',': case '-': case '.': case '/':
        -: 3665:		case '0': case '1': case '2': case '3': case '4':
        -: 3666:		case '5': case '6': case '7': case '8': case '9':
        -: 3667:		case ':': case ';': case '<': case '=': case '>':
        -: 3668:		case '?':
        -: 3669:		case 'A': case 'B': case 'C': case 'D': case 'E':
        -: 3670:		case 'F': case 'G': case 'H': case 'I': case 'J':
        -: 3671:		case 'K': case 'L': case 'M': case 'N': case 'O':
        -: 3672:		case 'P': case 'Q': case 'R': case 'S': case 'T':
        -: 3673:		case 'U': case 'V': case 'W': case 'X': case 'Y':
        -: 3674:		case 'Z':
        -: 3675:		case '[': case '\\': case ']': case '^': case '_':
        -: 3676:		case 'a': case 'b': case 'c': case 'd': case 'e':
        -: 3677:		case 'f': case 'g': case 'h': case 'i': case 'j':
        -: 3678:		case 'k': case 'l': case 'm': case 'n': case 'o':
        -: 3679:		case 'p': case 'q': case 'r': case 's': case 't':
        -: 3680:		case 'u': case 'v': case 'w': case 'x': case 'y':
        -: 3681:		case 'z': case '{': case '|': case '}': case '~':
        -: 3682:		  /* These characters are printable ASCII characters.  */
     2414: 3683:		  *q++ = *p++;
     2414: 3684:		  displayed_width += 1;
     2414: 3685:		  break;
        -: 3686:		default:
        -: 3687:		  /* If we have a multibyte sequence, copy it until we
        -: 3688:		     reach its end, replacing each non-printable multibyte
        -: 3689:		     character with a single question mark.  */
        -: 3690:		  {
    #####: 3691:		    mbstate_t mbstate = { 0, };
        -: 3692:		    do
        -: 3693:		      {
        -: 3694:			wchar_t wc;
        -: 3695:			size_t bytes;
        -: 3696:			int w;
        -: 3697:
    #####: 3698:			bytes = mbrtowc (&wc, p, plimit - p, &mbstate);
        -: 3699:
    #####: 3700:			if (bytes == (size_t) -1)
        -: 3701:			  {
        -: 3702:			    /* An invalid multibyte sequence was
        -: 3703:			       encountered.  Skip one input byte, and
        -: 3704:			       put a question mark.  */
    #####: 3705:			    p++;
    #####: 3706:			    *q++ = '?';
    #####: 3707:			    displayed_width += 1;
    #####: 3708:			    break;
        -: 3709:			  }
        -: 3710:
    #####: 3711:			if (bytes == (size_t) -2)
        -: 3712:			  {
        -: 3713:			    /* An incomplete multibyte character
        -: 3714:			       at the end.  Replace it entirely with
        -: 3715:			       a question mark.  */
    #####: 3716:			    p = plimit;
    #####: 3717:			    *q++ = '?';
    #####: 3718:			    displayed_width += 1;
    #####: 3719:			    break;
        -: 3720:			  }
        -: 3721:
    #####: 3722:			if (bytes == 0)
        -: 3723:			  /* A null wide character was encountered.  */
    #####: 3724:			  bytes = 1;
        -: 3725:
    #####: 3726:			w = wcwidth (wc);
    #####: 3727:			if (w >= 0)
        -: 3728:			  {
        -: 3729:			    /* A printable multibyte character.
        -: 3730:			       Keep it.  */
    #####: 3731:			    for (; bytes > 0; --bytes)
    #####: 3732:			      *q++ = *p++;
    #####: 3733:			    displayed_width += w;
        -: 3734:			  }
        -: 3735:			else
        -: 3736:			  {
        -: 3737:			    /* An unprintable multibyte character.
        -: 3738:			       Replace it entirely with a question
        -: 3739:			       mark.  */
    #####: 3740:			    p += bytes;
    #####: 3741:			    *q++ = '?';
    #####: 3742:			    displayed_width += 1;
        -: 3743:			  }
        -: 3744:		      }
    #####: 3745:		    while (! mbsinit (&mbstate));
        -: 3746:		  }
    #####: 3747:		  break;
        -: 3748:	      }
        -: 3749:
        -: 3750:	  /* The buffer may have shrunk.  */
      321: 3751:	  len = q - buf;
        -: 3752:	}
        -: 3753:      else
        -: 3754:#endif
        -: 3755:	{
    #####: 3756:	  char *p = buf;
    #####: 3757:	  char const *plimit = buf + len;
        -: 3758:
    #####: 3759:	  while (p < plimit)
        -: 3760:	    {
    #####: 3761:	      if (! isprint (to_uchar (*p)))
    #####: 3762:		*p = '?';
    #####: 3763:	      p++;
        -: 3764:	    }
    #####: 3765:	  displayed_width = len;
        -: 3766:	}
        -: 3767:    }
    #####: 3768:  else if (width != NULL)
        -: 3769:    {
        -: 3770:#if HAVE_MBRTOWC
    #####: 3771:      if (MB_CUR_MAX > 1)
    #####: 3772:	displayed_width = mbsnwidth (buf, len, 0);
        -: 3773:      else
        -: 3774:#endif
        -: 3775:	{
    #####: 3776:	  char const *p = buf;
    #####: 3777:	  char const *plimit = buf + len;
        -: 3778:
    #####: 3779:	  displayed_width = 0;
    #####: 3780:	  while (p < plimit)
        -: 3781:	    {
    #####: 3782:	      if (isprint (to_uchar (*p)))
    #####: 3783:		displayed_width++;
    #####: 3784:	      p++;
        -: 3785:	    }
        -: 3786:	}
        -: 3787:    }
        -: 3788:
      321: 3789:  if (out != NULL)
      321: 3790:    fwrite (buf, 1, len, out);
      321: 3791:  if (width != NULL)
    #####: 3792:    *width = displayed_width;
      321: 3793:  return len;
        -: 3794:}
        -: 3795:
        -: 3796:static void
      321: 3797:print_name_with_quoting (const char *p, mode_t mode, int linkok,
        -: 3798:			 bool stat_ok, enum filetype type,
        -: 3799:			 struct obstack *stack)
        -: 3800:{
      321: 3801:  bool used_color_this_time
        -: 3802:    = (print_with_color
      321: 3803:       && print_color_indicator (p, mode, linkok, stat_ok, type));
        -: 3804:
      321: 3805:  if (stack)
      321: 3806:    PUSH_CURRENT_DIRED_POS (stack);
        -: 3807:
      321: 3808:  dired_pos += quote_name (stdout, p, filename_quoting_options, NULL);
        -: 3809:
      321: 3810:  if (stack)
      321: 3811:    PUSH_CURRENT_DIRED_POS (stack);
        -: 3812:
      321: 3813:  if (used_color_this_time)
        -: 3814:    {
    #####: 3815:      process_signals ();
    #####: 3816:      prep_non_filename_text ();
        -: 3817:    }
      321: 3818:}
        -: 3819:
        -: 3820:static void
    #####: 3821:prep_non_filename_text (void)
        -: 3822:{
    #####: 3823:  if (color_indicator[C_END].string != NULL)
    #####: 3824:    put_indicator (&color_indicator[C_END]);
        -: 3825:  else
        -: 3826:    {
    #####: 3827:      put_indicator (&color_indicator[C_LEFT]);
    #####: 3828:      put_indicator (&color_indicator[C_RESET]);
    #####: 3829:      put_indicator (&color_indicator[C_RIGHT]);
        -: 3830:    }
    #####: 3831:}
        -: 3832:
        -: 3833:/* Print the file name of `f' with appropriate quoting.
        -: 3834:   Also print file size, inode number, and filetype indicator character,
        -: 3835:   as requested by switches.  */
        -: 3836:
        -: 3837:static void
    #####: 3838:print_file_name_and_frills (const struct fileinfo *f)
        -: 3839:{
        -: 3840:  char buf[MAX (LONGEST_HUMAN_READABLE + 1, INT_BUFSIZE_BOUND (uintmax_t))];
        -: 3841:
    #####: 3842:  if (print_inode)
    #####: 3843:    printf ("%*s ", format == with_commas ? 0 : inode_number_width,
        -: 3844:	    umaxtostr (f->stat.st_ino, buf));
        -: 3845:
    #####: 3846:  if (print_block_size)
    #####: 3847:    printf ("%*s ", format == with_commas ? 0 : block_size_width,
    #####: 3848:	    human_readable (ST_NBLOCKS (f->stat), buf, human_output_opts,
        -: 3849:			    ST_NBLOCKSIZE, output_block_size));
        -: 3850:
    #####: 3851:  if (print_scontext)
    #####: 3852:    printf ("%*s ", format == with_commas ? 0 : scontext_width, f->scontext);
        -: 3853:
    #####: 3854:  print_name_with_quoting (f->name, FILE_OR_LINK_MODE (f), f->linkok,
    #####: 3855:			   f->stat_ok, f->filetype, NULL);
        -: 3856:
    #####: 3857:  if (indicator_style != none)
    #####: 3858:    print_type_indicator (f->stat_ok, f->stat.st_mode, f->filetype);
    #####: 3859:}
        -: 3860:
        -: 3861:/* Given these arguments describing a file, return the single-byte
        -: 3862:   type indicator, or 0.  */
        -: 3863:static char
    #####: 3864:get_type_indicator (bool stat_ok, mode_t mode, enum filetype type)
        -: 3865:{
        -: 3866:  char c;
        -: 3867:
    #####: 3868:  if (stat_ok ? S_ISREG (mode) : type == normal)
        -: 3869:    {
    #####: 3870:      if (stat_ok && indicator_style == classify && (mode & S_IXUGO))
    #####: 3871:	c = '*';
        -: 3872:      else
    #####: 3873:	c = 0;
        -: 3874:    }
        -: 3875:  else
        -: 3876:    {
    #####: 3877:      if (stat_ok ? S_ISDIR (mode) : type == directory || type == arg_directory)
    #####: 3878:	c = '/';
    #####: 3879:      else if (indicator_style == slash)
    #####: 3880:	c = 0;
    #####: 3881:      else if (stat_ok ? S_ISLNK (mode) : type == symbolic_link)
    #####: 3882:	c = '@';
    #####: 3883:      else if (stat_ok ? S_ISFIFO (mode) : type == fifo)
    #####: 3884:	c = '|';
    #####: 3885:      else if (stat_ok ? S_ISSOCK (mode) : type == sock)
    #####: 3886:	c = '=';
        -: 3887:      else if (stat_ok && S_ISDOOR (mode))
        -: 3888:	c = '>';
        -: 3889:      else
    #####: 3890:	c = 0;
        -: 3891:    }
    #####: 3892:  return c;
        -: 3893:}
        -: 3894:
        -: 3895:static void
    #####: 3896:print_type_indicator (bool stat_ok, mode_t mode, enum filetype type)
        -: 3897:{
    #####: 3898:  char c = get_type_indicator (stat_ok, mode, type);
    #####: 3899:  if (c)
    #####: 3900:    DIRED_PUTCHAR (c);
    #####: 3901:}
        -: 3902:
        -: 3903:/* Returns whether any color sequence was printed. */
        -: 3904:static bool
    #####: 3905:print_color_indicator (const char *name, mode_t mode, int linkok,
        -: 3906:		       bool stat_ok, enum filetype filetype)
        -: 3907:{
        -: 3908:  int type;
        -: 3909:  struct color_ext_type *ext;	/* Color extension */
        -: 3910:  size_t len;			/* Length of name */
        -: 3911:
        -: 3912:  /* Is this a nonexistent file?  If so, linkok == -1.  */
        -: 3913:
    #####: 3914:  if (linkok == -1 && color_indicator[C_MISSING].string != NULL)
    #####: 3915:    type = C_MISSING;
    #####: 3916:  else if (! stat_ok)
        -: 3917:    {
        -: 3918:      static enum indicator_no filetype_indicator[] = FILETYPE_INDICATORS;
    #####: 3919:      type = filetype_indicator[filetype];
        -: 3920:    }
        -: 3921:  else
        -: 3922:    {
    #####: 3923:      if (S_ISREG (mode))
        -: 3924:	{
    #####: 3925:	  type = C_FILE;
    #####: 3926:	  if ((mode & S_ISUID) != 0)
    #####: 3927:	    type = C_SETUID;
    #####: 3928:	  else if ((mode & S_ISGID) != 0)
    #####: 3929:	    type = C_SETGID;
    #####: 3930:	  else if ((mode & S_IXUGO) != 0)
    #####: 3931:	    type = C_EXEC;
        -: 3932:	}
    #####: 3933:      else if (S_ISDIR (mode))
        -: 3934:	{
    #####: 3935:	  if ((mode & S_ISVTX) && (mode & S_IWOTH))
    #####: 3936:	    type = C_STICKY_OTHER_WRITABLE;
    #####: 3937:	  else if ((mode & S_IWOTH) != 0)
    #####: 3938:	    type = C_OTHER_WRITABLE;
    #####: 3939:	  else if ((mode & S_ISVTX) != 0)
    #####: 3940:	    type = C_STICKY;
        -: 3941:	  else
    #####: 3942:	    type = C_DIR;
        -: 3943:	}
    #####: 3944:      else if (S_ISLNK (mode))
    #####: 3945:	type = ((!linkok && color_indicator[C_ORPHAN].string)
    #####: 3946:		? C_ORPHAN : C_LINK);
    #####: 3947:      else if (S_ISFIFO (mode))
    #####: 3948:	type = C_FIFO;
    #####: 3949:      else if (S_ISSOCK (mode))
    #####: 3950:	type = C_SOCK;
    #####: 3951:      else if (S_ISBLK (mode))
    #####: 3952:	type = C_BLK;
    #####: 3953:      else if (S_ISCHR (mode))
    #####: 3954:	type = C_CHR;
        -: 3955:      else if (S_ISDOOR (mode))
        -: 3956:	type = C_DOOR;
        -: 3957:      else
        -: 3958:	{
        -: 3959:	  /* Classify a file of some other type as C_ORPHAN.  */
    #####: 3960:	  type = C_ORPHAN;
        -: 3961:	}
        -: 3962:    }
        -: 3963:
        -: 3964:  /* Check the file's suffix only if still classified as C_FILE.  */
    #####: 3965:  ext = NULL;
    #####: 3966:  if (type == C_FILE)
        -: 3967:    {
        -: 3968:      /* Test if NAME has a recognized suffix.  */
        -: 3969:
    #####: 3970:      len = strlen (name);
    #####: 3971:      name += len;		/* Pointer to final \0.  */
    #####: 3972:      for (ext = color_ext_list; ext != NULL; ext = ext->next)
        -: 3973:	{
    #####: 3974:	  if (ext->ext.len <= len
    #####: 3975:	      && strncmp (name - ext->ext.len, ext->ext.string,
        -: 3976:			  ext->ext.len) == 0)
    #####: 3977:	    break;
        -: 3978:	}
        -: 3979:    }
        -: 3980:
        -: 3981:  {
    #####: 3982:    const struct bin_str *const s
    #####: 3983:      = ext ? &(ext->seq) : &color_indicator[type];
    #####: 3984:    if (s->string != NULL)
        -: 3985:      {
    #####: 3986:	put_indicator (&color_indicator[C_LEFT]);
    #####: 3987:	put_indicator (s);
    #####: 3988:	put_indicator (&color_indicator[C_RIGHT]);
    #####: 3989:	return true;
        -: 3990:      }
        -: 3991:    else
    #####: 3992:      return false;
        -: 3993:  }
        -: 3994:}
        -: 3995:
        -: 3996:/* Output a color indicator (which may contain nulls).  */
        -: 3997:static void
    #####: 3998:put_indicator (const struct bin_str *ind)
        -: 3999:{
    #####: 4000:  if (! used_color)
        -: 4001:    {
    #####: 4002:      used_color = true;
    #####: 4003:      prep_non_filename_text ();
        -: 4004:    }
        -: 4005:
    #####: 4006:  fwrite (ind->string, ind->len, 1, stdout);
    #####: 4007:}
        -: 4008:
        -: 4009:static size_t
    #####: 4010:length_of_file_name_and_frills (const struct fileinfo *f)
        -: 4011:{
    #####: 4012:  size_t len = 0;
        -: 4013:  size_t name_width;
        -: 4014:  char buf[MAX (LONGEST_HUMAN_READABLE + 1, INT_BUFSIZE_BOUND (uintmax_t))];
        -: 4015:
    #####: 4016:  if (print_inode)
    #####: 4017:    len += 1 + (format == with_commas
    #####: 4018:		? strlen (umaxtostr (f->stat.st_ino, buf))
        -: 4019:		: inode_number_width);
        -: 4020:
    #####: 4021:  if (print_block_size)
    #####: 4022:    len += 1 + (format == with_commas
    #####: 4023:		? strlen (human_readable (ST_NBLOCKS (f->stat), buf,
        -: 4024:					  human_output_opts, ST_NBLOCKSIZE,
        -: 4025:					  output_block_size))
        -: 4026:		: block_size_width);
        -: 4027:
    #####: 4028:  if (print_scontext)
    #####: 4029:    len += 1 + (format == with_commas ? strlen (f->scontext) : scontext_width);
        -: 4030:
    #####: 4031:  quote_name (NULL, f->name, filename_quoting_options, &name_width);
    #####: 4032:  len += name_width;
        -: 4033:
    #####: 4034:  if (indicator_style != none)
        -: 4035:    {
    #####: 4036:      char c = get_type_indicator (f->stat_ok, f->stat.st_mode, f->filetype);
    #####: 4037:      len += (c != 0);
        -: 4038:    }
        -: 4039:
    #####: 4040:  return len;
        -: 4041:}
        -: 4042:
        -: 4043:static void
    #####: 4044:print_many_per_line (void)
        -: 4045:{
        -: 4046:  size_t row;			/* Current row.  */
    #####: 4047:  size_t cols = calculate_columns (true);
    #####: 4048:  struct column_info const *line_fmt = &column_info[cols - 1];
        -: 4049:
        -: 4050:  /* Calculate the number of rows that will be in each column except possibly
        -: 4051:     for a short column on the right.  */
    #####: 4052:  size_t rows = cwd_n_used / cols + (cwd_n_used % cols != 0);
        -: 4053:
    #####: 4054:  for (row = 0; row < rows; row++)
        -: 4055:    {
    #####: 4056:      size_t col = 0;
    #####: 4057:      size_t filesno = row;
    #####: 4058:      size_t pos = 0;
        -: 4059:
        -: 4060:      /* Print the next row.  */
        -: 4061:      while (1)
        -: 4062:	{
    #####: 4063:	  struct fileinfo const *f = sorted_file[filesno];
    #####: 4064:	  size_t name_length = length_of_file_name_and_frills (f);
    #####: 4065:	  size_t max_name_length = line_fmt->col_arr[col++];
    #####: 4066:	  print_file_name_and_frills (f);
        -: 4067:
    #####: 4068:	  filesno += rows;
    #####: 4069:	  if (filesno >= cwd_n_used)
    #####: 4070:	    break;
        -: 4071:
    #####: 4072:	  indent (pos + name_length, pos + max_name_length);
    #####: 4073:	  pos += max_name_length;
    #####: 4074:	}
    #####: 4075:      putchar ('\n');
        -: 4076:    }
    #####: 4077:}
        -: 4078:
        -: 4079:static void
    #####: 4080:print_horizontal (void)
        -: 4081:{
        -: 4082:  size_t filesno;
    #####: 4083:  size_t pos = 0;
    #####: 4084:  size_t cols = calculate_columns (false);
    #####: 4085:  struct column_info const *line_fmt = &column_info[cols - 1];
    #####: 4086:  struct fileinfo const *f = sorted_file[0];
    #####: 4087:  size_t name_length = length_of_file_name_and_frills (f);
    #####: 4088:  size_t max_name_length = line_fmt->col_arr[0];
        -: 4089:
        -: 4090:  /* Print first entry.  */
    #####: 4091:  print_file_name_and_frills (f);
        -: 4092:
        -: 4093:  /* Now the rest.  */
    #####: 4094:  for (filesno = 1; filesno < cwd_n_used; ++filesno)
        -: 4095:    {
    #####: 4096:      size_t col = filesno % cols;
        -: 4097:
    #####: 4098:      if (col == 0)
        -: 4099:	{
    #####: 4100:	  putchar ('\n');
    #####: 4101:	  pos = 0;
        -: 4102:	}
        -: 4103:      else
        -: 4104:	{
    #####: 4105:	  indent (pos + name_length, pos + max_name_length);
    #####: 4106:	  pos += max_name_length;
        -: 4107:	}
        -: 4108:
    #####: 4109:      f = sorted_file[filesno];
    #####: 4110:      print_file_name_and_frills (f);
        -: 4111:
    #####: 4112:      name_length = length_of_file_name_and_frills (f);
    #####: 4113:      max_name_length = line_fmt->col_arr[col];
        -: 4114:    }
    #####: 4115:  putchar ('\n');
    #####: 4116:}
        -: 4117:
        -: 4118:static void
    #####: 4119:print_with_commas (void)
        -: 4120:{
        -: 4121:  size_t filesno;
    #####: 4122:  size_t pos = 0;
        -: 4123:
    #####: 4124:  for (filesno = 0; filesno < cwd_n_used; filesno++)
        -: 4125:    {
    #####: 4126:      struct fileinfo const *f = sorted_file[filesno];
    #####: 4127:      size_t len = length_of_file_name_and_frills (f);
        -: 4128:
    #####: 4129:      if (filesno != 0)
        -: 4130:	{
        -: 4131:	  char separator;
        -: 4132:
    #####: 4133:	  if (pos + len + 2 < line_length)
        -: 4134:	    {
    #####: 4135:	      pos += 2;
    #####: 4136:	      separator = ' ';
        -: 4137:	    }
        -: 4138:	  else
        -: 4139:	    {
    #####: 4140:	      pos = 0;
    #####: 4141:	      separator = '\n';
        -: 4142:	    }
        -: 4143:
    #####: 4144:	  putchar (',');
    #####: 4145:	  putchar (separator);
        -: 4146:	}
        -: 4147:
    #####: 4148:      print_file_name_and_frills (f);
    #####: 4149:      pos += len;
        -: 4150:    }
    #####: 4151:  putchar ('\n');
    #####: 4152:}
        -: 4153:
        -: 4154:/* Assuming cursor is at position FROM, indent up to position TO.
        -: 4155:   Use a TAB character instead of two or more spaces whenever possible.  */
        -: 4156:
        -: 4157:static void
    #####: 4158:indent (size_t from, size_t to)
        -: 4159:{
    #####: 4160:  while (from < to)
        -: 4161:    {
    #####: 4162:      if (tabsize != 0 && to / tabsize > (from + 1) / tabsize)
        -: 4163:	{
    #####: 4164:	  putchar ('\t');
    #####: 4165:	  from += tabsize - from % tabsize;
        -: 4166:	}
        -: 4167:      else
        -: 4168:	{
    #####: 4169:	  putchar (' ');
    #####: 4170:	  from++;
        -: 4171:	}
        -: 4172:    }
    #####: 4173:}
        -: 4174:
        -: 4175:/* Put DIRNAME/NAME into DEST, handling `.' and `/' properly.  */
        -: 4176:/* FIXME: maybe remove this function someday.  See about using a
        -: 4177:   non-malloc'ing version of file_name_concat.  */
        -: 4178:
        -: 4179:static void
      321: 4180:attach (char *dest, const char *dirname, const char *name)
        -: 4181:{
      321: 4182:  const char *dirnamep = dirname;
        -: 4183:
        -: 4184:  /* Copy dirname if it is not ".".  */
      321: 4185:  if (dirname[0] != '.' || dirname[1] != 0)
        -: 4186:    {
    #####: 4187:      while (*dirnamep)
    #####: 4188:	*dest++ = *dirnamep++;
        -: 4189:      /* Add '/' if `dirname' doesn't already end with it.  */
    #####: 4190:      if (dirnamep > dirname && dirnamep[-1] != '/')
    #####: 4191:	*dest++ = '/';
        -: 4192:    }
     3056: 4193:  while (*name)
     2414: 4194:    *dest++ = *name++;
      321: 4195:  *dest = 0;
      321: 4196:}
        -: 4197:
        -: 4198:/* Allocate enough column info suitable for the current number of
        -: 4199:   files and display columns, and initialize the info to represent the
        -: 4200:   narrowest possible columns.  */
        -: 4201:
        -: 4202:static void
    #####: 4203:init_column_info (void)
        -: 4204:{
        -: 4205:  size_t i;
    #####: 4206:  size_t max_cols = MIN (max_idx, cwd_n_used);
        -: 4207:
        -: 4208:  /* Currently allocated columns in column_info.  */
        -: 4209:  static size_t column_info_alloc;
        -: 4210:
    #####: 4211:  if (column_info_alloc < max_cols)
        -: 4212:    {
        -: 4213:      size_t new_column_info_alloc;
        -: 4214:      size_t *p;
        -: 4215:
    #####: 4216:      if (max_cols < max_idx / 2)
        -: 4217:	{
        -: 4218:	  /* The number of columns is far less than the display width
        -: 4219:	     allows.  Grow the allocation, but only so that it's
        -: 4220:	     double the current requirements.  If the display is
        -: 4221:	     extremely wide, this avoids allocating a lot of memory
        -: 4222:	     that is never needed.  */
    #####: 4223:	  column_info = xnrealloc (column_info, max_cols,
        -: 4224:				   2 * sizeof *column_info);
    #####: 4225:	  new_column_info_alloc = 2 * max_cols;
        -: 4226:	}
        -: 4227:      else
        -: 4228:	{
    #####: 4229:	  column_info = xnrealloc (column_info, max_idx, sizeof *column_info);
    #####: 4230:	  new_column_info_alloc = max_idx;
        -: 4231:	}
        -: 4232:
        -: 4233:      /* Allocate the new size_t objects by computing the triangle
        -: 4234:	 formula n * (n + 1) / 2, except that we don't need to
        -: 4235:	 allocate the part of the triangle that we've already
        -: 4236:	 allocated.  Check for address arithmetic overflow.  */
        -: 4237:      {
    #####: 4238:	size_t column_info_growth = new_column_info_alloc - column_info_alloc;
    #####: 4239:	size_t s = column_info_alloc + 1 + new_column_info_alloc;
    #####: 4240:	size_t t = s * column_info_growth;
    #####: 4241:	if (s < new_column_info_alloc || t / column_info_growth != s)
    #####: 4242:	  xalloc_die ();
    #####: 4243:	p = xnmalloc (t / 2, sizeof *p);
        -: 4244:      }
        -: 4245:
        -: 4246:      /* Grow the triangle by parceling out the cells just allocated.  */
    #####: 4247:      for (i = column_info_alloc; i < new_column_info_alloc; i++)
        -: 4248:	{
    #####: 4249:	  column_info[i].col_arr = p;
    #####: 4250:	  p += i + 1;
        -: 4251:	}
        -: 4252:
    #####: 4253:      column_info_alloc = new_column_info_alloc;
        -: 4254:    }
        -: 4255:
    #####: 4256:  for (i = 0; i < max_cols; ++i)
        -: 4257:    {
        -: 4258:      size_t j;
        -: 4259:
    #####: 4260:      column_info[i].valid_len = true;
    #####: 4261:      column_info[i].line_len = (i + 1) * MIN_COLUMN_WIDTH;
    #####: 4262:      for (j = 0; j <= i; ++j)
    #####: 4263:	column_info[i].col_arr[j] = MIN_COLUMN_WIDTH;
        -: 4264:    }
    #####: 4265:}
        -: 4266:
        -: 4267:/* Calculate the number of columns needed to represent the current set
        -: 4268:   of files in the current display width.  */
        -: 4269:
        -: 4270:static size_t
    #####: 4271:calculate_columns (bool by_columns)
        -: 4272:{
        -: 4273:  size_t filesno;		/* Index into cwd_file.  */
        -: 4274:  size_t cols;			/* Number of files across.  */
        -: 4275:
        -: 4276:  /* Normally the maximum number of columns is determined by the
        -: 4277:     screen width.  But if few files are available this might limit it
        -: 4278:     as well.  */
    #####: 4279:  size_t max_cols = MIN (max_idx, cwd_n_used);
        -: 4280:
    #####: 4281:  init_column_info ();
        -: 4282:
        -: 4283:  /* Compute the maximum number of possible columns.  */
    #####: 4284:  for (filesno = 0; filesno < cwd_n_used; ++filesno)
        -: 4285:    {
    #####: 4286:      struct fileinfo const *f = sorted_file[filesno];
    #####: 4287:      size_t name_length = length_of_file_name_and_frills (f);
        -: 4288:      size_t i;
        -: 4289:
    #####: 4290:      for (i = 0; i < max_cols; ++i)
        -: 4291:	{
    #####: 4292:	  if (column_info[i].valid_len)
        -: 4293:	    {
    #####: 4294:	      size_t idx = (by_columns
    #####: 4295:			    ? filesno / ((cwd_n_used + i) / (i + 1))
    #####: 4296:			    : filesno % (i + 1));
    #####: 4297:	      size_t real_length = name_length + (idx == i ? 0 : 2);
        -: 4298:
    #####: 4299:	      if (column_info[i].col_arr[idx] < real_length)
        -: 4300:		{
    #####: 4301:		  column_info[i].line_len += (real_length
    #####: 4302:					      - column_info[i].col_arr[idx]);
    #####: 4303:		  column_info[i].col_arr[idx] = real_length;
    #####: 4304:		  column_info[i].valid_len = (column_info[i].line_len
    #####: 4305:					      < line_length);
        -: 4306:		}
        -: 4307:	    }
        -: 4308:	}
        -: 4309:    }
        -: 4310:
        -: 4311:  /* Find maximum allowed columns.  */
    #####: 4312:  for (cols = max_cols; 1 < cols; --cols)
        -: 4313:    {
    #####: 4314:      if (column_info[cols - 1].valid_len)
    #####: 4315:	break;
        -: 4316:    }
        -: 4317:
    #####: 4318:  return cols;
        -: 4319:}
        -: 4320:
        -: 4321:void
    #####: 4322:usage (int status)
        -: 4323:{
    #####: 4324:  if (status != EXIT_SUCCESS)
    #####: 4325:    fprintf (stderr, _("Try `%s --help' for more information.\n"),
        -: 4326:	     program_name);
        -: 4327:  else
        -: 4328:    {
    #####: 4329:      printf (_("Usage: %s [OPTION]... [FILE]...\n"), program_name);
    #####: 4330:      fputs (_("\
        -: 4331:List information about the FILEs (the current directory by default).\n\
        -: 4332:Sort entries alphabetically if none of -cftuvSUX nor --sort.\n\
        -: 4333:\n\
        -: 4334:"), stdout);
    #####: 4335:      fputs (_("\
        -: 4336:Mandatory arguments to long options are mandatory for short options too.\n\
        -: 4337:"), stdout);
    #####: 4338:      fputs (_("\
        -: 4339:  -a, --all                  do not ignore entries starting with .\n\
        -: 4340:  -A, --almost-all           do not list implied . and ..\n\
        -: 4341:      --author               with -l, print the author of each file\n\
        -: 4342:  -b, --escape               print octal escapes for nongraphic characters\n\
        -: 4343:"), stdout);
    #####: 4344:      fputs (_("\
        -: 4345:      --block-size=SIZE      use SIZE-byte blocks\n\
        -: 4346:  -B, --ignore-backups       do not list implied entries ending with ~\n\
        -: 4347:  -c                         with -lt: sort by, and show, ctime (time of last\n\
        -: 4348:                               modification of file status information)\n\
        -: 4349:                               with -l: show ctime and sort by name\n\
        -: 4350:                               otherwise: sort by ctime\n\
        -: 4351:"), stdout);
    #####: 4352:      fputs (_("\
        -: 4353:  -C                         list entries by columns\n\
        -: 4354:      --color[=WHEN]         control whether color is used to distinguish file\n\
        -: 4355:                               types.  WHEN may be `never', `always', or `auto'\n\
        -: 4356:  -d, --directory            list directory entries instead of contents,\n\
        -: 4357:                               and do not dereference symbolic links\n\
        -: 4358:  -D, --dired                generate output designed for Emacs' dired mode\n\
        -: 4359:"), stdout);
    #####: 4360:      fputs (_("\
        -: 4361:  -f                         do not sort, enable -aU, disable -ls --color\n\
        -: 4362:  -F, --classify             append indicator (one of */=>@|) to entries\n\
        -: 4363:      --file-type            likewise, except do not append `*'\n\
        -: 4364:      --format=WORD          across -x, commas -m, horizontal -x, long -l,\n\
        -: 4365:                               single-column -1, verbose -l, vertical -C\n\
        -: 4366:      --full-time            like -l --time-style=full-iso\n\
        -: 4367:"), stdout);
    #####: 4368:      fputs (_("\
        -: 4369:  -g                         like -l, but do not list owner\n\
        -: 4370:"), stdout);
    #####: 4371:      fputs (_("\
        -: 4372:      --group-directories-first\n\
        -: 4373:                             group directories before files.\n\
        -: 4374:                               augment with a --sort option, but any\n\
        -: 4375:                               use of --sort=none (-U) disables grouping\n\
        -: 4376:"), stdout);
    #####: 4377:      fputs (_("\
        -: 4378:  -G, --no-group             in a long listing, don't print group names\n\
        -: 4379:  -h, --human-readable       with -l, print sizes in human readable format\n\
        -: 4380:                               (e.g., 1K 234M 2G)\n\
        -: 4381:      --si                   likewise, but use powers of 1000 not 1024\n\
        -: 4382:"), stdout);
    #####: 4383:      fputs (_("\
        -: 4384:  -H, --dereference-command-line\n\
        -: 4385:                             follow symbolic links listed on the command line\n\
        -: 4386:      --dereference-command-line-symlink-to-dir\n\
        -: 4387:                             follow each command line symbolic link\n\
        -: 4388:                             that points to a directory\n\
        -: 4389:      --hide=PATTERN         do not list implied entries matching shell PATTERN\n\
        -: 4390:                               (overridden by -a or -A)\n\
        -: 4391:"), stdout);
    #####: 4392:      fputs (_("\
        -: 4393:      --indicator-style=WORD  append indicator with style WORD to entry names:\n\
        -: 4394:                               none (default), slash (-p),\n\
        -: 4395:                               file-type (--file-type), classify (-F)\n\
        -: 4396:  -i, --inode                print the index number of each file\n\
        -: 4397:  -I, --ignore=PATTERN       do not list implied entries matching shell PATTERN\n\
        -: 4398:  -k                         like --block-size=1K\n\
        -: 4399:"), stdout);
    #####: 4400:      fputs (_("\
        -: 4401:  -l                         use a long listing format\n\
        -: 4402:  -L, --dereference          when showing file information for a symbolic\n\
        -: 4403:                               link, show information for the file the link\n\
        -: 4404:                               references rather than for the link itself\n\
        -: 4405:  -m                         fill width with a comma separated list of entries\n\
        -: 4406:"), stdout);
    #####: 4407:      fputs (_("\
        -: 4408:  -n, --numeric-uid-gid      like -l, but list numeric user and group IDs\n\
        -: 4409:  -N, --literal              print raw entry names (don't treat e.g. control\n\
        -: 4410:                               characters specially)\n\
        -: 4411:  -o                         like -l, but do not list group information\n\
        -: 4412:  -p, --indicator-style=slash\n\
        -: 4413:                             append / indicator to directories\n\
        -: 4414:"), stdout);
    #####: 4415:      fputs (_("\
        -: 4416:  -q, --hide-control-chars   print ? instead of non graphic characters\n\
        -: 4417:      --show-control-chars   show non graphic characters as-is (default\n\
        -: 4418:                             unless program is `ls' and output is a terminal)\n\
        -: 4419:  -Q, --quote-name           enclose entry names in double quotes\n\
        -: 4420:      --quoting-style=WORD   use quoting style WORD for entry names:\n\
        -: 4421:                               literal, locale, shell, shell-always, c, escape\n\
        -: 4422:"), stdout);
    #####: 4423:      fputs (_("\
        -: 4424:  -r, --reverse              reverse order while sorting\n\
        -: 4425:  -R, --recursive            list subdirectories recursively\n\
        -: 4426:  -s, --size                 print the size of each file, in blocks\n\
        -: 4427:"), stdout);
    #####: 4428:      fputs (_("\
        -: 4429:  -S                         sort by file size\n\
        -: 4430:      --sort=WORD            sort by WORD instead of name: none -U,\n\
        -: 4431:                             extension -X, size -S, time -t, version -v\n\
        -: 4432:      --time=WORD            with -l, show time as WORD instead of modification\n\
        -: 4433:                             time: atime -u, access -u, use -u, ctime -c,\n\
        -: 4434:                             or status -c; use specified time as sort key\n\
        -: 4435:                             if --sort=time\n\
        -: 4436:"), stdout);
    #####: 4437:      fputs (_("\
        -: 4438:      --time-style=STYLE     with -l, show times using style STYLE:\n\
        -: 4439:                             full-iso, long-iso, iso, locale, +FORMAT.\n\
        -: 4440:                             FORMAT is interpreted like `date'; if FORMAT is\n\
        -: 4441:                             FORMAT1<newline>FORMAT2, FORMAT1 applies to\n\
        -: 4442:                             non-recent files and FORMAT2 to recent files;\n\
        -: 4443:                             if STYLE is prefixed with `posix-', STYLE\n\
        -: 4444:                             takes effect only outside the POSIX locale\n\
        -: 4445:"), stdout);
    #####: 4446:      fputs (_("\
        -: 4447:  -t                         sort by modification time\n\
        -: 4448:  -T, --tabsize=COLS         assume tab stops at each COLS instead of 8\n\
        -: 4449:"), stdout);
    #####: 4450:      fputs (_("\
        -: 4451:  -u                         with -lt: sort by, and show, access time\n\
        -: 4452:                               with -l: show access time and sort by name\n\
        -: 4453:                               otherwise: sort by access time\n\
        -: 4454:  -U                         do not sort; list entries in directory order\n\
        -: 4455:  -v                         sort by version\n\
        -: 4456:"), stdout);
    #####: 4457:      fputs (_("\
        -: 4458:  -w, --width=COLS           assume screen width instead of current value\n\
        -: 4459:  -x                         list entries by lines instead of by columns\n\
        -: 4460:  -X                         sort alphabetically by entry extension\n\
        -: 4461:  -Z, --context              print any SELinux security context of each file\n\
        -: 4462:  -1                         list one file per line\n\
        -: 4463:"), stdout);
    #####: 4464:      fputs (HELP_OPTION_DESCRIPTION, stdout);
    #####: 4465:      fputs (VERSION_OPTION_DESCRIPTION, stdout);
    #####: 4466:      fputs (_("\n\
        -: 4467:SIZE may be (or may be an integer optionally followed by) one of following:\n\
        -: 4468:kB 1000, K 1024, MB 1000*1000, M 1024*1024, and so on for G, T, P, E, Z, Y.\n\
        -: 4469:"), stdout);
    #####: 4470:      fputs (_("\
        -: 4471:\n\
        -: 4472:By default, color is not used to distinguish types of files.  That is\n\
        -: 4473:equivalent to using --color=none.  Using the --color option without the\n\
        -: 4474:optional WHEN argument is equivalent to using --color=always.  With\n\
        -: 4475:--color=auto, color codes are output only if standard output is connected\n\
        -: 4476:to a terminal (tty).  The environment variable LS_COLORS can influence the\n\
        -: 4477:colors, and can be set easily by the dircolors command.\n\
        -: 4478:"), stdout);
    #####: 4479:      fputs (_("\
        -: 4480:\n\
        -: 4481:Exit status is 0 if OK, 1 if minor problems, 2 if serious trouble.\n\
        -: 4482:"), stdout);
    #####: 4483:      emit_bug_reporting_address ();
        -: 4484:    }
    #####: 4485:  exit (status);
        -: 4486:}
