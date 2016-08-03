// Module for interfacing with terminal functions

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "term.h"
#include "lrotable.h"
#include <string.h>

// Lua: clrscr()
static int luaterm_clrscr( lua_State* L )
{
  term_clrscr();
  return 0;
}

// Lua: clreol()
static int luaterm_clreol( lua_State* L )
{
  term_clreol();
  return 0;
}

// Lua: moveto( x, y )
static int luaterm_moveto( lua_State* L )
{
  unsigned x, y;
  
  x = ( unsigned )luaL_checkinteger( L, 1 );
  y = ( unsigned )luaL_checkinteger( L, 2 );
  term_gotoxy( x, y );
  return 0;
}

// Lua: moveup( lines )
static int luaterm_moveup( lua_State* L )
{
  unsigned delta;
  
  delta = ( unsigned )luaL_checkinteger( L, 1 );
  term_up( delta );
  return 0;
}

// Lua: movedown( lines )
static int luaterm_movedown( lua_State* L )
{
  unsigned delta;
  
  delta = ( unsigned )luaL_checkinteger( L, 1 );
  term_down( delta );
  return 0;
}

// Lua: moveleft( cols )
static int luaterm_moveleft( lua_State* L )
{
  unsigned delta;
  
  delta = ( unsigned )luaL_checkinteger( L, 1 );
  term_left( delta );
  return 0;
}

// Lua: moveright( cols )
static int luaterm_moveright( lua_State* L )
{
  unsigned delta;
  
  delta = ( unsigned )luaL_checkinteger( L, 1 );
  term_right( delta );
  return 0;
}

// Lua: lines = getlines()
static int luaterm_getlines( lua_State* L )
{
  lua_pushinteger( L, term_get_lines() );
  return 1;
}

// Lua: columns = getcols()
static int luaterm_getcols( lua_State* L )
{
  lua_pushinteger( L, term_get_cols() );
  return 1;
}

// Lua: lines = setlines()
static int luaterm_setlines( lua_State* L )
{
  int nlin = luaL_checkinteger(L, 1);
  if ((nlin > 0) && (nlin < 128)) term_num_lines = nlin;

  lua_pushinteger( L, term_get_lines() );
  return 1;
}

// Lua: columns = setcols()
static int luaterm_setcols( lua_State* L )
{
  int ncol = luaL_checkinteger(L, 1);
  if ((ncol > 0) && (ncol < LUA_MAXINPUT)) term_num_cols = ncol;
  lua_pushinteger( L, term_get_cols() );
  return 1;
}

// Lua: print( string1, string2, ... )
// or print( x, y, string1, string2, ... )
static int luaterm_print( lua_State* L )
{
  const char* buf;
  size_t len, i;
  int total = lua_gettop( L ), s = 1;
  int x = -1, y = -1;

  // Check if the function has integer arguments
  if( lua_isnumber( L, 1 ) && lua_isnumber( L, 2 ) )
  {
    x = lua_tointeger( L, 1 );
    y = lua_tointeger( L, 2 );
    term_gotoxy( x, y );
    s = 3;
  } 
  for( ; s <= total; s ++ )
  {
    luaL_checktype( L, s, LUA_TSTRING );
    buf = lua_tolstring( L, s, &len );
    for( i = 0; i < len; i ++ )
      term_putch( buf[ i ] );
  }
  return 0;
}

// Lua: cursorx = getcx()
static int luaterm_getcx( lua_State* L )
{
  lua_pushinteger( L, term_get_cx() );
  return 1;
}

// Lua: cursory = getcy()
static int luaterm_getcy( lua_State* L )
{
  lua_pushinteger( L, term_get_cy() );
  return 1;
}

// Lua: cursory = getcy()
static int luaterm_setctype( lua_State* L )
{
  int ct = luaL_checkinteger(L, 1);
  //if (ct) ct = 4;
  term_curs(ct);
  return 0;
}

// Lua: key = getchar( [ mode ] )
//========================================
static int luaterm_getchar( lua_State* L )
{
  int temp = TERM_INPUT_WAIT;

  if ( lua_isnumber( L, 1 ) ) {
	  temp = lua_tointeger( L, 1 );
	  if (temp == 0) temp = TERM_INPUT_DONT_WAIT;
  }

  lua_pushinteger( L, term_getch( temp ) );

  //g_shell_result = 0;
  //vm_signal_post(g_shell_signal);
  return 1;
}


// Key codes by name
#undef _D
#define _D( x ) #x
static const char* term_key_names[] = { TERM_KEYCODES };

// __index metafunction for term
// Look for all KC_xxxx codes
static int term_mt_index( lua_State* L )
{
  const char *key = luaL_checkstring( L ,2 );
  unsigned i, total = sizeof( term_key_names ) / sizeof( char* );
  
  if( !key || *key != 'K' )
    return 0;
  for( i = 0; i < total; i ++ )
    if( !strcmp( key, term_key_names[ i ] ) )
      break;
  if( i == total )
    return 0;
  else
  {
    lua_pushinteger( L, i + TERM_FIRST_KEY );
    return 1; 
  }
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE term_map[] = 
{
  { LSTRKEY( "clrscr" ), LFUNCVAL( luaterm_clrscr ) },
  { LSTRKEY( "clreol" ), LFUNCVAL( luaterm_clreol ) },
  { LSTRKEY( "moveto" ), LFUNCVAL( luaterm_moveto ) },
  { LSTRKEY( "moveup" ), LFUNCVAL( luaterm_moveup ) },
  { LSTRKEY( "movedown" ), LFUNCVAL( luaterm_movedown ) },
  { LSTRKEY( "moveleft" ), LFUNCVAL( luaterm_moveleft ) },
  { LSTRKEY( "moveright" ), LFUNCVAL( luaterm_moveright ) },
  { LSTRKEY( "getlines" ), LFUNCVAL( luaterm_getlines ) },
  { LSTRKEY( "getcols" ), LFUNCVAL( luaterm_getcols ) },
  { LSTRKEY( "setlines" ), LFUNCVAL( luaterm_setlines ) },
  { LSTRKEY( "setcols" ), LFUNCVAL( luaterm_setcols ) },
  { LSTRKEY( "print" ), LFUNCVAL( luaterm_print ) },
  { LSTRKEY( "getcx" ), LFUNCVAL( luaterm_getcx ) },
  { LSTRKEY( "getcy" ), LFUNCVAL( luaterm_getcy ) },
  { LSTRKEY( "getchar" ), LFUNCVAL( luaterm_getchar ) },
  { LSTRKEY( "setctype" ), LFUNCVAL( luaterm_setctype ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__metatable" ), LROVAL( term_map ) },
  { LSTRKEY( "NOWAIT" ), LNUMVAL( TERM_INPUT_DONT_WAIT ) },
  { LSTRKEY( "WAIT" ), LNUMVAL( TERM_INPUT_WAIT ) },
#endif
  { LSTRKEY( "__index" ), LFUNCVAL( term_mt_index ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_term( lua_State* L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  // Register methods
  luaL_register( L, "term", term_map );
  
  // Set this table as itw own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );  
  
  // Register the constants for "getch"
  lua_pushnumber( L, TERM_INPUT_DONT_WAIT );
  lua_setfield( L, -2, "NOWAIT" );  
  lua_pushnumber( L, TERM_INPUT_WAIT );
  lua_setfield( L, -2, "WAIT" );  

  // Register term keycodes
  lua_pushnumber( L, KC_UP );
  lua_setfield( L, -2, "UP" );
  lua_pushnumber( L, KC_DOWN );
  lua_setfield( L, -2, "DOWN" );
  lua_pushnumber( L, KC_LEFT );
  lua_setfield( L, -2, "LEFT" );
  lua_pushnumber( L, KC_RIGHT );
  lua_setfield( L, -2, "RIGHT" );
  lua_pushnumber( L, KC_HOME );
  lua_setfield( L, -2, "HOME" );
  lua_pushnumber( L, KC_END );
  lua_setfield( L, -2, "END" );
  lua_pushnumber( L, KC_PAGEUP );
  lua_setfield( L, -2, "PAGEUP" );
  lua_pushnumber( L, KC_PAGEDOWN );
  lua_setfield( L, -2, "PAGEDOWN" );
  lua_pushnumber( L, KC_ENTER );
  lua_setfield( L, -2, "ENTER" );
  lua_pushnumber( L, KC_TAB );
  lua_setfield( L, -2, "TAB" );
  lua_pushnumber( L, KC_BACKSPACE );
  lua_setfield( L, -2, "BACKSPACE" );
  lua_pushnumber( L, KC_ESC );
  lua_setfield( L, -2, "ESC" );
  lua_pushnumber( L, KC_CTRL_Z );
  lua_setfield( L, -2, "CTRL_Z" );
  lua_pushnumber( L, KC_CTRL_A );
  lua_setfield( L, -2, "CTRL_A" );
  lua_pushnumber( L, KC_CTRL_E );
  lua_setfield( L, -2, "CTRL_E" );
  lua_pushnumber( L, KC_CTRL_C );
  lua_setfield( L, -2, "CTRL_C" );
  lua_pushnumber( L, KC_CTRL_T );
  lua_setfield( L, -2, "CTRL_T" );
  lua_pushnumber( L, KC_CTRL_U );
  lua_setfield( L, -2, "CTRL_U" );
  lua_pushnumber( L, KC_CTRL_K );
  lua_setfield( L, -2, "CTRL_K" );
  lua_pushnumber( L, KC_DEL );
  lua_setfield( L, -2, "DEL" );
  lua_pushnumber( L, KC_INS );
  lua_setfield( L, -2, "INS" );
  lua_pushnumber( L, KC_UNKNOWN );
  lua_setfield( L, -2, "UNKNOWN" );

  return 1;
#endif // # if LUA_OPTIMIZE_MEMORY > 0
}

