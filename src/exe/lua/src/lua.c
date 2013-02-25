/*
** $Id: lua.c,v 1.2 2008-08-08 16:15:05 leonb Exp $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lua_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"



static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;



static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}


static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}


static void print_usage (void) {
  fprintf(stderr,
  "usage: %s [options] [script [args]].\n"
  "Available options are:\n"
  "  -e stat  execute string " LUA_QL("stat") "\n"
  "  -l name  require library " LUA_QL("name") "\n"
  "  -i       enter interactive mode after executing " LUA_QL("script") "\n"
  "  -v       show version information\n"
  "  --       stop handling options\n"
  "  -        execute stdin and stop handling options\n"
  ,
  progname);
  fflush(stderr);
}


static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
  }
  return status;
}


static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}


static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}


static void print_version (void) {
  l_message(NULL, LUA_RELEASE "  " LUA_COPYRIGHT);
}


static int getargs (lua_State *L, char **argv, int n) {
  int narg;
  int i;
  int argc = 0;
  while (argv[argc]) argc++;  /* count total number of arguments */
  narg = argc - (n + 1);  /* number of arguments to the script */
  luaL_checkstack(L, narg + 3, "too many arguments to script");
  for (i=n+1; i < argc; i++)
    lua_pushstring(L, argv[i]);
  lua_createtable(L, narg, n + 1);
  for (i=0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - n);
  }
  return narg;
}


static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name) || docall(L, 0, 1);
  return report(L, status);
}


static int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  return report(L, status);
}


static int dolibrary (lua_State *L, const char *name) {
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  return report(L, docall(L, 1, 1));
}

/* ------------------------------------------------------------------------ */

#ifdef LUA_USE_READLINE
/*
** Lua 5.1.4 advanced readline support for the GNU readline and history
** libraries or compatible replacements.
**
** Author: Mike Pall.
** Maintainer: Sean Bolton (sean at smbolton dot com).
**
** Copyright (C) 2004-2006, 2011 Mike Pall. Same license as Lua. See lua.h.
**
** Advanced features:
** - Completion of keywords and global variable names.
** - Recursive and metatable-aware completion of variable names.
** - Context sensitive delimiter completion.
** - Save/restore of the history to/from a file (LUA_HISTORY env variable).
** - Setting a limit for the size of the history (LUA_HISTSIZE env variable).
** - Setting the app name to allow for $if lua ... $endif in ~/.inputrc.
**
** Start lua and try these (replace ~ with the TAB key):
**
** ~~
** fu~foo() ret~fa~end<CR>
** io~~~s~~~o~~~w~"foo\n")<CR>
**
** The ~~ are just for demonstration purposes (io~s~o~w~ suffices, of course).
**
** If you are used to zsh/tcsh-style completion support, try adding
** 'TAB: menu-complete' and 'C-d: possible-completions' to your ~/.inputrc.
**
** The patch has been successfully tested with:
**
** GNU    readline 2.2.1  (1998-07-17)
** GNU    readline 4.0    (1999-02-18) [harmless compiler warning]
** GNU    readline 4.3    (2002-07-16)
** GNU    readline 5.0    (2004-07-27)
** GNU    readline 5.1    (2005-12-07)
** GNU    readline 5.2    (2006-10-11)
** GNU    readline 6.0    (2009-02-20)
** GNU    readline 6.2    (2011-02-13)
** MacOSX libedit  2.11   (2008-07-12)
** NETBSD libedit  2.6.5  (2002-03-25)
** NETBSD libedit  2.6.9  (2004-05-01)
**
** Change Log:
** 2004-2006  Mike Pall   - original patch
** 2009/08/24 Sean Bolton - updated for GNU readline version 6
** 2011/12/14 Sean Bolton - fixed segfault when using Mac OS X libedit 2.11
*/

#include <ctype.h>

static char *lua_rl_hist;
static int lua_rl_histsize;

static lua_State *lua_rl_L;  /* User data is not passed to rl callbacks. */

/* Reserved keywords. */
static const char *const lua_rl_keywords[] = {
  "and", "break", "do", "else", "elseif", "end", "false",
  "for", "function", "if", "in", "local", "nil", "not", "or",
  "repeat", "return", "then", "true", "until", "while", NULL
};

static int valididentifier(const char *s)
{
  if (!(isalpha(*s) || *s == '_')) return 0;
  for (s++; *s; s++) if (!(isalpha(*s) || isdigit(*s) || *s == '_')) return 0;
  return 1;
}

/* Dynamically resizable match list. */
typedef struct {
  char **list;
  size_t idx, allocated, matchlen;
} dmlist;

/* Add prefix + string + suffix to list and compute common prefix. */
static int lua_rl_dmadd(dmlist *ml, const char *p, size_t pn, const char *s,
			int suf)
{
  char *t = NULL;

  if (ml->idx+1 >= ml->allocated &&
      !(ml->list = (char **)realloc(ml->list, sizeof(char *)*(ml->allocated += 32))))
    return -1;

  if (s) {
    size_t n = strlen(s);
    if (!(t = (char *)malloc(sizeof(char)*(pn+n+(suf?2:1))))) return 1;
    memcpy(t, p, pn);
    memcpy(t+pn, s, n);
    n += pn;
    t[n] = suf;
    if (suf) t[++n] = '\0';

    if (ml->idx == 0) {
      ml->matchlen = n;
    } else {
      size_t i;
      for (i = 0; i < ml->matchlen && i < n && ml->list[1][i] == t[i]; i++) ;
      ml->matchlen = i;  /* Set matchlen to common prefix. */
    }
  }

  ml->list[++ml->idx] = t;
  return 0;
}

/* Get __index field of metatable of object on top of stack. */
static int lua_rl_getmetaindex(lua_State *L)
{
  if (!lua_getmetatable(L, -1)) { lua_pop(L, 1); return 0; }
  lua_pushstring(L, "__index");
  lua_rawget(L, -2);
  lua_replace(L, -2);
  if (lua_isnil(L, -1) || lua_rawequal(L, -1, -2)) { lua_pop(L, 2); return 0; }
  lua_replace(L, -2);
  return 1;
}  /* 1: obj -- val, 0: obj -- */

/* Get field from object on top of stack. Avoid calling metamethods. */
static int lua_rl_getfield(lua_State *L, const char *s, size_t n)
{
  int i = 20;  /* Avoid infinite metatable loops. */
  do {
    if (lua_istable(L, -1)) {
      lua_pushlstring(L, s, n);
      lua_rawget(L, -2);
      if (!lua_isnil(L, -1)) { lua_replace(L, -2); return 1; }
      lua_pop(L, 1);
    }
  } while (--i > 0 && lua_rl_getmetaindex(L));
  lua_pop(L, 1);
  return 0;
}  /* 1: obj -- val, 0: obj -- */

/* Completion callback. */
static char **lua_rl_complete(const char *text, int start, int end)
{
  lua_State *L = lua_rl_L;
  dmlist ml;
  const char *s;
  size_t i, n, dot, loop;
  int savetop;

  if (!(text[0] == '\0' || isalpha(text[0]) || text[0] == '_')) return NULL;

  ml.list = NULL;
  ml.idx = ml.allocated = ml.matchlen = 0;

  savetop = lua_gettop(L);
  lua_pushvalue(L, LUA_GLOBALSINDEX);
  for (n = (size_t)(end-start), i = dot = 0; i < n; i++)
    if (text[i] == '.' || text[i] == ':') {
      if (!lua_rl_getfield(L, text+dot, i-dot))
	goto error;  /* Invalid prefix. */
      dot = i+1;  /* Points to first char after dot/colon. */
    }

  /* Add all matches against keywords if there is no dot/colon. */
  if (dot == 0)
    for (i = 0; (s = lua_rl_keywords[i]) != NULL; i++)
      if (!strncmp(s, text, n) && lua_rl_dmadd(&ml, NULL, 0, s, ' '))
	goto error;

  /* Add all valid matches from all tables/metatables. */
  loop = 0;  /* Avoid infinite metatable loops. */
  do {
    if (lua_istable(L, -1) &&
	(loop == 0 || !lua_rawequal(L, -1, LUA_GLOBALSINDEX)))
      for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1))
	if (lua_type(L, -2) == LUA_TSTRING) {
	  s = lua_tostring(L, -2);
	  /* Only match names starting with '_' if explicitly requested. */
	  if (!strncmp(s, text+dot, n-dot) && valididentifier(s) &&
	      (*s != '_' || text[dot] == '_')) {
	    int suf = ' ';  /* Default suffix is a space. */
	    switch (lua_type(L, -1)) {
	    case LUA_TTABLE:	suf = '.'; break;  /* No way to guess ':'. */
	    case LUA_TFUNCTION:	suf = '('; break;
	    case LUA_TUSERDATA:
	      if (lua_getmetatable(L, -1)) { lua_pop(L, 1); suf = ':'; }
	      break;
	    }
	    if (lua_rl_dmadd(&ml, text, dot, s, suf)) goto error;
	  }
	}
  } while (++loop < 20 && lua_rl_getmetaindex(L));

  if (ml.idx == 0) {
error:
    lua_settop(L, savetop);
    return NULL;
  } else {
    /* list[0] holds the common prefix of all matches (may be ""). */
    /* If there is only one match, list[0] and list[1] will be the same. */
    if (!(ml.list[0] = (char *)malloc(sizeof(char)*(ml.matchlen+1))))
      goto error;
    memcpy(ml.list[0], ml.list[1], ml.matchlen);
    ml.list[0][ml.matchlen] = '\0';
    /* Add the NULL list terminator. */
    if (lua_rl_dmadd(&ml, NULL, 0, NULL, 0)) goto error;
  }

  lua_settop(L, savetop);
#if RL_READLINE_VERSION >= 0x0600
  rl_completion_suppress_append = 1;
#endif
  return ml.list;
}

/* Initialize readline library. */
static void lua_rl_init(lua_State *L)
{
  char *s;

  lua_rl_L = L;

  /* This allows for $if lua ... $endif in ~/.inputrc. */
  rl_readline_name = "lua";
  /* Break words at every non-identifier character except '.' and ':'. */
  rl_completer_word_break_characters = 
    (char *)"\t\r\n !\"#$%&'()*+,-/;<=>?@[\\]^`{|}~";
  rl_completer_quote_characters = "\"'";
#if RL_READLINE_VERSION < 0x0600
  rl_completion_append_character = '\0';
#endif
  rl_attempted_completion_function = lua_rl_complete;
  rl_initialize();

  /* Start using history, optionally set history size and load history file. */
  using_history();
  if ((s = getenv("LUA_HISTSIZE")) &&
      (lua_rl_histsize = atoi(s))) stifle_history(lua_rl_histsize);
  if (!(lua_rl_hist = getenv("LUA_HISTORY"))) {
    char *ss = (char*)malloc((strlen(getenv("HOME"))+13)*sizeof(char));
    strcpy(ss,getenv("HOME"));
    lua_rl_hist = strcat(ss, "/.luahistory");
  }
  read_history(lua_rl_hist);
}

/* Finalize readline library. */
static void lua_rl_exit(lua_State *L)
{
  /* Optionally save history file. */
  if (lua_rl_hist) write_history(lua_rl_hist);
}
#else
#define lua_rl_init(L)		((void)L)
#define lua_rl_exit(L)		((void)L)
#endif

/* ------------------------------------------------------------------------ */

static const char *get_prompt (lua_State *L, int firstline) {
  const char *p;
  lua_getfield(L, LUA_GLOBALSINDEX, firstline ? "_PROMPT" : "_PROMPT2");
  p = lua_tostring(L, -1);
  if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
  lua_pop(L, 1);  /* remove global */
  return p;
}


static int incomplete (lua_State *L, int status) {
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
    if (strstr(msg, LUA_QL("<eof>")) == tp) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}


static int pushline (lua_State *L, int firstline) {
  char buffer[LUA_MAXINPUT];
  char *b = buffer;
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  if (lua_readline(L, b, prmt) == 0)
    return 0;  /* no input */
  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[l-1] = '\0';  /* remove it */
  if (firstline && b[0] == '=')  /* first line starts with `=' ? */
    lua_pushfstring(L, "return %s", b+1);  /* change it to `return' */
  else
    lua_pushstring(L, b);
  lua_freeline(L, b);
  return 1;
}


static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1))
    return -1;  /* no input */
  for (;;) {  /* repeat until gets a complete line */
    status = luaL_loadbuffer(L, lua_tostring(L, 1), lua_strlen(L, 1), "=stdin");
    if (!incomplete(L, status)) break;  /* cannot try to add lines? */
    if (!pushline(L, 0))  /* no more input? */
      return -1;
    lua_pushliteral(L, "\n");  /* add a new line... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
  }
  lua_saveline(L, 1);
  lua_remove(L, 1);  /* remove line */
  return status;
}


static void dotty (lua_State *L) {
  int status;
  const char *oldprogname = progname;
  progname = NULL;
  lua_rl_init(L);
  while ((status = loadline(L)) != -1) {
    if (status == 0) status = docall(L, 0, 0);
    report(L, status);
    if (status == 0 && lua_gettop(L) > 0) {  /* any result to print? */
      lua_getglobal(L, "print");
      lua_insert(L, 1);
      if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
        l_message(progname, lua_pushfstring(L,
                               "error calling " LUA_QL("print") " (%s)",
                               lua_tostring(L, -1)));
    }
  }
  lua_settop(L, 0);  /* clear stack */
  fputs("\n", stdout);
  fflush(stdout);
  lua_rl_exit(L);
  progname = oldprogname;
}


static int handle_script (lua_State *L, char **argv, int n) {
  int status;
  const char *fname;
  int narg = getargs(L, argv, n);  /* collect arguments */
  lua_setglobal(L, "arg");
  fname = argv[n];
  if (strcmp(fname, "-") == 0 && strcmp(argv[n-1], "--") != 0) 
    fname = NULL;  /* stdin */
  status = luaL_loadfile(L, fname);
  lua_insert(L, -(narg+1));
  if (status == 0)
    status = docall(L, narg, 0);
  else
    lua_pop(L, narg);      
  return report(L, status);
}


/* check that argument has no extra characters at the end */
#define notail(x)	{if ((x)[2] != '\0') return -1;}


static int collectargs (char **argv, int *pi, int *pv, int *pe) {
  int i;
  for (i = 1; argv[i] != NULL; i++) {
    if (argv[i][0] != '-')  /* not an option? */
        return i;
    switch (argv[i][1]) {  /* option */
      case '-':
        notail(argv[i]);
        return (argv[i+1] != NULL ? i+1 : 0);
      case '\0':
        return i;
      case 'i':
        notail(argv[i]);
        *pi = 1;  /* go through */
      case 'v':
        notail(argv[i]);
        *pv = 1;
        break;
      case 'e':
        *pe = 1;  /* go through */
      case 'l':
        if (argv[i][2] == '\0') {
          i++;
          if (argv[i] == NULL) return -1;
        }
        break;
      default: return -1;  /* invalid option */
    }
  }
  return 0;
}


static int runargs (lua_State *L, char **argv, int n) {
  int i;
  for (i = 1; i < n; i++) {
    if (argv[i] == NULL) continue;
    lua_assert(argv[i][0] == '-');
    switch (argv[i][1]) {  /* option */
      case 'e': {
        const char *chunk = argv[i] + 2;
        if (*chunk == '\0') chunk = argv[++i];
        lua_assert(chunk != NULL);
        if (dostring(L, chunk, "=(command line)") != 0)
          return 1;
        break;
      }
      case 'l': {
        const char *filename = argv[i] + 2;
        if (*filename == '\0') filename = argv[++i];
        lua_assert(filename != NULL);
        if (dolibrary(L, filename))
          return 1;  /* stop if file fails */
        break;
      }
      default: break;
    }
  }
  return 0;
}


static int handle_luainit (lua_State *L) {
  const char *init = getenv(LUA_INIT);
  if (init == NULL) return 0;  /* status OK */
  else if (init[0] == '@')
    return dofile(L, init+1);
  else
    return dostring(L, init, "=" LUA_INIT);
}


struct Smain {
  int argc;
  char **argv;
  int status;
};


static int pmain (lua_State *L) {
  struct Smain *s = (struct Smain *)lua_touserdata(L, 1);
  char **argv = s->argv;
  int script;
  int has_i = 0, has_v = 0, has_e = 0;
  globalL = L;
  if (argv[0] && argv[0][0]) progname = argv[0];
  lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
  luaL_openlibs(L);  /* open libraries */
  lua_gc(L, LUA_GCRESTART, 0);
  s->status = handle_luainit(L);
  if (s->status != 0) return 0;
  script = collectargs(argv, &has_i, &has_v, &has_e);
  if (script < 0) {  /* invalid args? */
    print_usage();
    s->status = 1;
    return 0;
  }
  if (has_v) print_version();
  s->status = runargs(L, argv, (script > 0) ? script : s->argc);
  if (s->status != 0) return 0;
  if (script)
    s->status = handle_script(L, argv, script);
  if (s->status != 0) return 0;
  if (has_i)
    dotty(L);
  else if (script == 0 && !has_e && !has_v) {
    if (lua_stdin_is_tty()) {
      print_version();
      dotty(L);
    }
    else dofile(L, NULL);  /* executes stdin as a file */
  }
  return 0;
}


int main (int argc, char **argv) {
  int status;
  struct Smain s;
#if HAVE_LUA_EXECUTABLE_DIR
  lua_executable_dir(argv[0]);
#endif
  lua_State *L = lua_open();  /* create state */
  if (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    return EXIT_FAILURE;
  }
  s.argc = argc;
  s.argv = argv;
  status = lua_cpcall(L, &pmain, &s);
  report(L, status);
  lua_close(L);
  return (status || s.status) ? EXIT_FAILURE : EXIT_SUCCESS;
}

