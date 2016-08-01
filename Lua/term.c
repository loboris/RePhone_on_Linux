// Terminal function

#include "term.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

extern void retarget_putc(char ch);
extern int retarget_waitc(unsigned char *c, int timeout);
extern void retarget_write(const char *str, unsigned int len);

// Local variables
static unsigned term_num_lines = 25;
static unsigned term_num_cols = 80;
static unsigned term_cx = 0;
static unsigned term_cy = 0;
static unsigned int skip_0A = 0;

// *****************************************************************************
// Terminal functions

//-----------------------------------------
static void term_out( unsigned char data )
{
	retarget_putc(data);
}

/*
//----------------------------
static int term_in( int mode )
{
  unsigned char c;
  int res;
  do {
    if ( mode == TERM_INPUT_DONT_WAIT ) res = retarget_waitc(&c, 1);
    else res = retarget_waitc(&c, 600000);

    // CR/LF sequence, skip the second char (LF) if applicable
    if ( skip_0A > 0 ) {
      skip_0A = 0;
      if ( c == 0x0A )
        continue;
    }
    break;
  } while( TRUE );
  if (res >= 0) res = c;
  return res;
}

//------------------------------------
static int term_translate( int data )
{
  unsigned char c, c2;
  int res;

  if ( isprint( data ) ) return data;

  else if( data == 0x1B ) // escape sequence
  {
    // If we don't get a second char, we got a simple "ESC", so return KC_ESC
    // If we get a second char it must be '[', the next one is relevant for us
    if ( retarget_waitc(&c, TERM_TIMEOUT) == -1 )
      return KC_ESC;

    if ( ( retarget_waitc(&c, TERM_TIMEOUT) ) == -1 ) return KC_UNKNOWN;

    if ( c >= 0x41 && c <= 0x44 )
      switch( c )
      {
        case 0x41:
          return KC_UP;
        case 0x42:
          return KC_DOWN;
        case 0x43:
          return KC_RIGHT;
        case 0x44:
          return KC_LEFT;
      }
    else if( c > 48 && c < 55 )
    {
      // Extended sequence: read another byte
      res = retarget_waitc(&c2, TERM_TIMEOUT);
      if ((res < 0) || (c != 126)) return KC_UNKNOWN;
      switch( c )
      {
        case 49:
          return KC_HOME;
        case 52:
          return KC_END;
        case 53:
          return KC_PAGEUP;
        case 54:
          return KC_PAGEDOWN;
      }
    }
  }
  else if( data == 0x0D )
  {
    skip_0A=1;
    return KC_ENTER;
  }
  else if( data == 0x0A )
  {
    return KC_ENTER;
  }
  else
  {
    switch( data )
    {
      case 0x09:
        return KC_TAB;
      case 0x7F:
//        return KC_DEL; // bogdanm: some terminal emulators (for example screen) return 0x7F for BACKSPACE :(
      case 0x08:
        return KC_BACKSPACE;
      case 26:
        return KC_CTRL_Z;
      case 1:
        return KC_CTRL_A;
      case 5:
        return KC_CTRL_E;
      case 3:
        return KC_CTRL_C;
      case 20:
        return KC_CTRL_T;
      case 21:
        return KC_CTRL_U;
      case 11:
        return KC_CTRL_K;
    }
  }
  return KC_UNKNOWN;
}
*/

// Helper function: send the requested string to the terminal

#ifdef __GNUC__
static void term_ansi( const char* fmt, ... )  __attribute__ ((format (printf, 1, 2)));
#endif

static void term_ansi( const char* fmt, ... )
{
  char seq[ TERM_MAX_ANSI_SIZE + 1 ];
  va_list ap;
    
  seq[ TERM_MAX_ANSI_SIZE ] = '\0';
  seq[ 0 ] = '\x1B';
  seq[ 1 ] = '[';
  va_start( ap, fmt );
  vsnprintf( seq + 2, TERM_MAX_ANSI_SIZE - 2, fmt, ap );
  va_end( ap );
  term_putstr( seq, strlen( seq ) );
}

// Clear the screen
void term_clrscr()
{
  term_ansi( "2J" );
  term_cx = term_cy = 0;
}

// Clear to end of line
void term_clreol()
{
  term_ansi( "K" );
}

// Move cursor to (x, y)
void term_gotoxy( unsigned x, unsigned y )
{
  term_ansi( "%u;%uH", y, x );
  term_cx = x;
  term_cy = y;
}

// Move cursor up "delta" lines
void term_up( unsigned delta )
{
  term_ansi( "%uA", delta );  
  term_cy -= delta;
}

// Move cursor down "delta" lines
void term_down( unsigned delta )
{
  term_ansi( "%uB", delta );  
  term_cy += delta;
}

// Move cursor right "delta" chars
void term_right( unsigned delta )
{
  term_ansi( "%uC", delta );  
  term_cx -= delta;
}

// Move cursor left "delta" chars
void term_left( unsigned delta )
{
  term_ansi( "%uD", delta );  
  term_cx += delta;
}

// Return the number of terminal lines
unsigned term_get_lines()
{
  return term_num_lines;
}

// Return the number of terminal columns
unsigned term_get_cols()
{
  return term_num_cols;
}

// Write a character to the terminal
void term_putch( uint8_t ch )
{
  if( ch == '\n' )
  {
    if( term_cy < term_num_lines )
      term_cy ++;
    term_cx = 0;
  }
  term_out( ch );
}

// Write a string to the terminal
void term_putstr( const char* str, unsigned size )
{
  retarget_write(str, size);
}

// Write a string of 
 
// Return the cursor "x" position
unsigned term_get_cx()
{
  return term_cx;
}

// Return the cursor "y" position
unsigned term_get_cy()
{
  return term_cy;
}

// Return a char read from the terminal
// If "mode" is TERM_INPUT_DONT_WAIT, return the char only if it is available,
// otherwise return -1
// Calls the translate function to translate the terminal's physical key codes
// to logical key codes (defined in the term.h header)
int term_getch( int mode )
{
  int ch, res;
  unsigned char c, c2;

  // term_in()
  do {
    if ( mode == TERM_INPUT_DONT_WAIT ) ch = retarget_waitc(&c, 1);
    else ch = retarget_waitc(&c, TERM_TIMEOUT_NOWAIT);

    // CR/LF sequence, skip the second char (LF) if applicable
    if ( skip_0A > 0 ) {
      skip_0A = 0;
      if ( c == 0x0A )
        continue;
    }
    break;
  } while( TRUE );
  
  if (ch >= 0) ch = c;
  else return -1;

  // term_translate()
  if ( isprint( ch ) ) return ch;

  else if( ch == 0x1B ) // escape sequence
  {
    // If we don't get a second char, we got a simple "ESC", so return KC_ESC
    // If we get a second char it must be '[', the next one is relevant for us
    if ( retarget_waitc(&c, TERM_TIMEOUT) == -1 )
      return KC_ESC;

    if ( ( retarget_waitc(&c, TERM_TIMEOUT) ) == -1 ) return KC_UNKNOWN;

    if ( c >= 0x41 && c <= 0x44 )
      switch( c )
      {
        case 0x41:
          return KC_UP;
        case 0x42:
          return KC_DOWN;
        case 0x43:
          return KC_RIGHT;
        case 0x44:
          return KC_LEFT;
      }
    else if( c > 48 && c < 55 )
    {
      // Extended sequence: read another byte
      res = retarget_waitc(&c2, TERM_TIMEOUT);
      if ((res < 0) || (c != 126)) return KC_UNKNOWN;
      switch( c )
      {
        case 49:
          return KC_HOME;
        case 52:
          return KC_END;
        case 53:
          return KC_PAGEUP;
        case 54:
          return KC_PAGEDOWN;
      }
    }
  }
  else if( ch == 0x0D )
  {
    skip_0A=1;
    return KC_ENTER;
  }
  else if( ch == 0x0A )
  {
    return KC_ENTER;
  }
  else
  {
    switch( ch )
    {
      case 0x09:
        return KC_TAB;
      case 0x7F:
//        return KC_DEL; // bogdanm: some terminal emulators (for example screen) return 0x7F for BACKSPACE :(
      case 0x08:
        return KC_BACKSPACE;
      case 26:
        return KC_CTRL_Z;
      case 1:
        return KC_CTRL_A;
      case 5:
        return KC_CTRL_E;
      case 3:
        return KC_CTRL_C;
      case 20:
        return KC_CTRL_T;
      case 21:
        return KC_CTRL_U;
      case 11:
        return KC_CTRL_K;
    }
  }
  return KC_UNKNOWN;
}
