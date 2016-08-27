// Terminal function

#include "term.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

extern void retarget_putc(char ch);
extern void retarget_write(const char *str, unsigned int len);
extern int retarget_getc(int tmo);

unsigned term_num_lines = 25;
unsigned term_num_cols = 80;
unsigned term_cx = 1;
unsigned term_cy = 1;
int use_term_input = 1;

// Local variables
static unsigned int skip_0A = 0;

// *****************************************************************************
// Terminal functions


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

  //term_putstr( seq, strlen( seq ) );
  retarget_write(seq, strlen(seq));
}

// set cursor type
void term_curs(int ctype)
{
  /*
  char cs[2] = {0,0};
  cs[0] = (char)(ctype&7)+0x30;
  term_ansi( "%s q", cs);
  */
}

// Clear the screen
void term_clrscr()
{
  term_ansi( "2J" );
  term_ansi( "%u;%uH", 1, 1);
  term_cx = 1;
  term_cy = 1;
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
  term_cx += delta;
}

// Move cursor left "delta" chars
void term_left( unsigned delta )
{
  term_ansi( "%uD", delta );  
  term_cx -= delta;
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
//---------------------------
void term_putch( uint8_t ch )
{
  if ( ch == '\n' ) {
    if ( term_cy < term_num_lines ) term_cy ++;
    term_cx = 1;
  }

  retarget_putc(ch);
  term_cx++;

  if (ch == '\r') term_cx = 1;
}

// Write a string to the terminal
//------------------------------------------------
void term_putstr( const char* str, unsigned size )
{
  //retarget_write(str, size);
  //term_cx += size;
  for (int i=0; i<size; i++) {
	  term_putch(str[i]);
  }
}

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
  int ch, ch1;

  // === term_in ===
  do {
    if ( mode == TERM_INPUT_DONT_WAIT ) ch = retarget_getc(TERM_TIMEOUT_NOWAIT);
    else ch = retarget_getc(TERM_TIMEOUT);
    if ( ch < 0 ) return -1;

    // CR/LF sequence, skip the second char (LF) if applicable
    if ( skip_0A > 0 ) {
      skip_0A = 0;
      if ( ch == 0x0A ) continue;
    }
    break;
  } while( TRUE );

  // === term_translate ===
  if ( isprint( ch ) ) return ch;

  else if( ch == 0x1B ) {
	// ** escape sequence **
    // If we don't get a second char, we got a simple "ESC", so return KC_ESC
    // If we get a second char it must be '[', the next one is relevant for us
	vm_thread_sleep(5);
  	ch = retarget_getc(TERM_TIMEOUT_NOWAIT);  // get 2nd char
    if ( ch < 0 ) return KC_ESC;

    if ( ch == 79) { // '<esc>OF'
        vm_thread_sleep(5);
        ch = retarget_getc(TERM_TIMEOUT_NOWAIT);  // get next char
        if ( ch < 0 ) return KC_UNKNOWN;
        if (ch == 0x46) return KC_END;
        else return KC_UNKNOWN;
    }
    if ( ch != 91 ) return KC_UNKNOWN;

	vm_thread_sleep(5);
  	ch = retarget_getc(TERM_TIMEOUT_NOWAIT);  // get next char
    if ( ch < 0 ) return KC_UNKNOWN;

    if ( (ch >= 0x40) && (ch <= 0x56) )
      switch( ch )
      {
      	case 0x40:
      	  return KC_INS;
        case 0x41:
          return KC_UP;
        case 0x42:
          return KC_DOWN;
        case 0x43:
          return KC_RIGHT;
        case 0x44:
          return KC_LEFT;
        case 0x48:
          return KC_HOME;
        case 0x55:
          return KC_PAGEDOWN;
        case 0x56:
          return KC_PAGEUP;
      }
    else if( ch > 48 && ch < 55 )
    {
      // Extended sequence: read another byte (<esc>[n~)
      vm_thread_sleep(5);
      ch1 = retarget_getc(TERM_TIMEOUT_NOWAIT);
      if ((ch1 < 0) || (ch1 != 126)) return KC_UNKNOWN;
      switch( ch )
      {
        case 49:
          return KC_HOME;
        case 50:
          return KC_INS;
        case 51:
          return KC_DEL;
        case 52:
          return KC_END;
        case 53:
          return KC_PAGEUP;
        case 54:
          return KC_PAGEDOWN;
      }
    }
  }
  else if( ch == 0x0D ) {
    skip_0A = 1;
    return KC_ENTER;
  }
  else if( ch == 0x0A ) {
    return KC_ENTER;
  }
  else {
    switch( ch )
    {
      case 0x09:
        return KC_TAB;
      case 0x7F:
        return KC_DEL; // bogdanm: some terminal emulators (for example screen) return 0x7F for BACKSPACE :(
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
      case 12:
        return KC_CTRL_L;
      case 4:
        return KC_CTRL_D;
    }
  }
  return KC_UNKNOWN;
}

//------------------------------------
int term_getstr(char *buf, int maxlen)
{
	int c;
	int len = 0;
	int startx = term_cx + strlen(buf) + 1;
	int x = 0;

    term_gotoxy(term_cx, term_cy );
    term_clreol();
	term_putstr(buf, strlen(buf));

	memset(buf, 0, maxlen);

    while(1) {
        c = term_getch(TERM_INPUT_WAIT);
        if (c < 0) continue;

        switch(c) {
        	case KC_ENTER:
        		return 0;
        		break;

        	case KC_CTRL_C:
        		return -1;
        		break;

        	case KC_BACKSPACE:
        		if (x > 0) {
        			x--;
        			memmove(buf+x, buf+x+1, len-x-1);
        			len--;
        			buf[len] = '\0';
        		}
        		else continue;
        		break;

        	case KC_DEL:
        		if (x < len) {
        			memmove(buf+x, buf+x+1, len-x);
        			len--;
        			buf[len] = '\0';
        		}
        		else continue;
        		break;

        	case KC_LEFT:
        		if (x == 0) continue;
        		x--;
        		break;

        	case KC_RIGHT:
        		if (x >= len) continue;
        		x++;
        		break;

        	case KC_HOME:
        		x = 0;
        		break;

        	case KC_END:
        		x = len;
        		break;

        	default:
                if ((c >= 32) && (c < 127)) {
                	if (len < maxlen) {
						if (x < len) memmove(buf+x+1, buf+x, len-x);
						len++;
						buf[x] = c;
						buf[len] = '\0';
						x++;
                	}
                }
        		break;
        }
        term_gotoxy( startx, term_cy );
        term_clreol();
    	term_putstr(buf, strlen(buf));
        term_gotoxy( startx+x, term_cy );
    }
}

