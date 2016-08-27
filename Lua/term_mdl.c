// Module for interfacing with terminal functions

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>

#include "lualib.h"
#include "lauxlib.h"
#include "shell.h"
#include "term.h"
#include "lrotable.h"

#include "vmlog.h"
#include "vmfs.h"
#include "ymodem.h"

#define EDITOR_LEFT 7
#define EDITOR_HEAD	1
#define EDITOR_FOOT	2

typedef struct {
	int	ptr;		// pointer to current line position
	int	size;		// size of file in buffer
	int	line;		// current line
	int	first;		// first line on screen
	int	col;		// current column
	int	x;			// last screen x position
	int	len;		// current line length
	int	numlines;	// total number of lines in file
	int	fsize;		// original file size
	int	ins;		// insert or overwrite flag
	int	changed;	// buffer changed flag
	int	show_lnum;	// show line numbers flag
	int buf_size;	// allocated size of the editor buffer
	int scrlin;		// number of visible lines on screen
	int linend;		// line end string length
	char fname[64];	// editor file name
	char *buf;		// editor buffer pointer
} editor_t;

extern int retarget_getc(int tmo);

static editor_t *editor = NULL;



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

// ===== Editor functions =======================

//=======================================
static int luaterm_getstr( lua_State* L )
{
	const char* buf;
    char inbuf[256] = {'\0'};

    int x = luaL_checkinteger( L, 1 );
    int y = luaL_checkinteger( L, 2 );
    int maxlen = luaL_checkinteger( L, 3 );
    if (maxlen < 1) maxlen = 1;
    if (maxlen > 254) maxlen = 254;
    if (lua_isstring(L,4)) {
    	size_t len;
        buf = lua_tolstring( L, 4, &len );
        if (len < 128) snprintf(inbuf, maxlen, "%s", buf);
    }

    term_gotoxy( x, y );
    int res =term_getstr(inbuf, maxlen);
    term_gotoxy( x, y );
    term_clreol();

    if (res < 0) lua_pushnil(L);
    else lua_pushstring(L, inbuf);

    return 1;
}

//---------------------------
static int get_numlines(void)
{
	int curline = 0;
	char *nextLine = NULL;
	int ptr = 0;

	while (1) {
		nextLine = strchr(editor->buf+ptr, '\n');
		curline++;
		if (nextLine == NULL) {
			// EOL not found
			if (editor->buf[0] != 0) curline--;
			break;
		}
		ptr = nextLine - editor->buf + 1;
	}
	return curline;
}

//---------------------
static void get_line()
{
	int curline = 0;
	char *nextLine = NULL;
	int ptr = 0;

	while (curline < editor->line) {
		editor->len = -1;
		// find end of line character
		nextLine = strchr(editor->buf+ptr, '\n');
		if (nextLine == NULL) {
			// new line not found
			if (editor->buf[0] != 0) {
				// the last line
				editor->ptr = ptr;  // save pointer to last line
				editor->len = strlen(editor->buf+ptr);
				break;
			}
			else break;
		}
		// EOL found
		curline++;
		if (curline == editor->line) {
			// found line
			editor->ptr = ptr; // save pointer to found line
			editor->len = (nextLine - (editor->buf+ptr));	// line length
			break;
		}
		ptr = nextLine - editor->buf + 1;
	}
	if (editor->len > 0) {
		if ((editor->len > 0) && (editor->buf[editor->ptr+editor->len-1]) == '\r') editor->len--;
	}
}

//-----------------------
static int screen_y(void)
{
	int n = (editor->line - editor->first + 1) % editor->scrlin;
	if (n == 0) n = editor->scrlin;
	n += EDITOR_HEAD;
	return n;
}

//-------------------------------------
static void show_current_line(int flag)
{
	char info[12];

	if (flag < 2) {
		get_line();
		// adjust current column
		if (editor->col > editor->len) editor->col = editor->len+1;
	}

	if (flag > 0) {
		// show status line
		char buf[80];
		char ins[4];
		char eol[5];
		char chg = ' ';

		if (editor->changed) chg = '*';

		if (editor->ins) sprintf(ins, "INS");
		else sprintf(ins, "OVR");

		if (editor->linend == 1) sprintf(eol, "LF");
		else sprintf(eol, "CRLF");

		term_gotoxy( 1, term_get_lines());
		term_clreol();
		term_putstr("STAT: ", 6);
		sprintf(buf, "Lin: %d/%d  Col: %d/%d  %s  EOL: %s  SIZE: %d %c",
				editor->line, editor->numlines, editor->col, editor->len, ins, eol, editor->size, chg);
		term_putstr(buf, strlen(buf));
		if (flag == 2) { // only show status
			term_gotoxy(editor->x, screen_y());
			return;
		}
	}

	// clear the line
	term_gotoxy(1, screen_y());
	term_clreol();

	// calculate first line char to show
	int pos = (editor->col - term_num_cols + 2);
	if (editor->show_lnum) {
		sprintf(info, "%4d: ", editor->line);
		term_putstr(info, 6);
		pos += 6;
	}
	if (pos < 0) pos = 0;

	char *ptr = editor->buf + editor->ptr;		// pointer to line start
	int n = 0;									// number of displayed chars
	// output line chars
	while ((n < editor->len) && (term_cx < (term_num_cols-2))) {
		term_putch(ptr[pos++]);
		n++;
	}
	// show end char
	term_gotoxy(term_num_cols, screen_y());
	if (n < editor->len) term_putch('>');
	else term_putch('|');

	// position cursor to current column
	editor->x = editor->col + editor->show_lnum;
	if (editor->x > (term_num_cols-2)) editor->x = term_num_cols-2;
	//editor->x -= pos;
	term_gotoxy(editor->x, screen_y());
}

//--------------------------
static void show_editlines()
{
	int lin;
	char info[8];
	int editlin = editor->line; // save current line
    int editcol = editor->col;	// save current column
    editor->col = 1;

	for (lin = 0; lin < editor->scrlin; lin++) {
		if ((lin + editor->first) <= editor->numlines) {
			editor->line = lin + editor->first;
			show_current_line(0);
		}
		else { // blank lines after last editor line
			term_gotoxy(1, lin + EDITOR_HEAD + 1);
			term_clreol();
			term_gotoxy(term_num_cols, lin + EDITOR_HEAD + 1);
			term_putch('|');
		}
	}
	editor->line = editlin; // restore current line
	editor->col = editcol;  // restore current column
	show_current_line(1);
}

//--------------------------
static int buf_insert(int n)
{
	int isok = 1;
	int from = editor->ptr + editor->col - 1;
	if ((editor->size+n) > editor->buf_size) {
		char *newb;
		newb = realloc(editor->buf, editor->buf_size+256);
		if (newb != NULL) editor->buf = newb;
		else isok = 0;
	}
	if (isok) {
		memmove(editor->buf+from+n, editor->buf+from, editor->size-from);
		editor->size += n;
		editor->buf[editor->size] = '\0';
	}
	return isok;
}

//-----------------------
static int edit_lin(void)
{
	editor->line = 1;
	editor->first = 1;
	editor->col = 1;

	// === Show lines ===
	show_editlines();
	if (editor->len < 0) return -9;

	int c;

    while(1) {
        c = term_getch(TERM_INPUT_WAIT);
        if (c < 0) continue;

        switch(c) {
        	case KC_ENTER:
        		// insert new line
				if (buf_insert(editor->linend)) {
					if (editor->linend == 2) {
						editor->buf[editor->ptr + editor->col - 1] = '\r';
						editor->buf[editor->ptr + editor->col] = '\n';
					}
					else editor->buf[editor->ptr + editor->col - 1] = '\n';

					editor->col = 1;
					editor->line++;
					if ((editor->line-editor->first) >= editor->scrlin) editor->first++;
					editor->numlines++;
					editor->changed++;

					show_editlines();
				}
        		break;

        	case KC_BACKSPACE:
                if (editor->col > 1) {
            		// delete character before current column
                	int from = editor->ptr + editor->col - 1;
                	memmove(editor->buf+from-1, editor->buf+from, editor->size-from);
                	editor->changed++;
                	editor->size--;
                	editor->buf[editor->size] = '\0';
                	editor->col--;
                	show_current_line(1);
                }
                else if (editor->line > 1) {
                	// join with previous line
                	// save prev line length
                	int lin = editor->line;
                	editor->line--;
					get_line();
					int col = editor->len;
					editor->line = lin;
					get_line();  // restore current line
                	int n = 0;
                	if (editor->buf[editor->ptr-1] == '\n') {
                		n = 1;
                    	if (editor->buf[editor->ptr-2] == '\r') n = 2;
                	}
                	if (n > 0) {
						memmove(editor->buf+editor->ptr-n, editor->buf+editor->ptr, editor->size-editor->ptr);
	                	editor->changed++;
						editor->size -= n;
						editor->buf[editor->size] = '\0';
						editor->numlines--;

						if (screen_y() > (EDITOR_HEAD+1)) editor->line--; // not on first line
						else { // on first line
							editor->first--;
							editor->line = editor->first;
						}
						show_editlines();
                	}
                }
        		break;

        	case KC_DEL:
                if (editor->col <= editor->len) {
            		// delete character on current column
                	int from = editor->ptr + editor->col;
                	memmove(editor->buf+from-1, editor->buf+from, editor->size-from);
                	editor->changed++;
                	editor->size--;
                	editor->buf[editor->size] = '\0';
                	show_current_line(1);
                }
                else if (editor->line < editor->numlines) {
                	// join with next line
                	int n = 0;
                	int from = editor->ptr + editor->len;
                	if (editor->buf[from] == '\r') n++;
                    if (editor->buf[from+1] == '\n') n++;
                    from += n;
					memmove(editor->buf+from-n, editor->buf+from, editor->size-from);
					editor->changed++;
					editor->size -= n;
					editor->buf[editor->size] = '\0';
					editor->numlines--;

					show_editlines();
                }
        		break;

            case KC_CTRL_C:
            	return -1;  // exit editor, ignore changes
            	break;

            case KC_CTRL_Z:
            	return 0;  // exit editor, save changes
            	break;

            case KC_CTRL_D:
            	// delete line
            	break;

            case KC_CTRL_L:
            	if (editor->show_lnum) editor->show_lnum = 0;
            	else editor->show_lnum = 6;
				show_editlines();
            	break;

            case KC_INS:
            	editor->ins ^= 1;
            	show_current_line(2);
            	break;

            case KC_LEFT:
                // left arrow
                if (editor->col > 1) {
                	editor->col--;
                	editor->x--;
                	int f = 2;
                	if ((editor->x - editor->show_lnum) < 1) f = 1;
                	show_current_line(f);
                }
                else {
                    if (editor->line > 1) {
    					editor->col = 9999;
    					goto _up;
                    }
                }
                break;

            case KC_RIGHT:
                // right arrow
                if ((editor->col) <= editor->len) {
                	editor->col++;
                	show_current_line(1);
                }
                else {
                	editor->col = 1;
                	goto _down;
                }
                break;

            case KC_HOME:
            	editor->col = 1;
            	show_current_line(1);
                break;

            case KC_END:
            	editor->col = editor->len + 1;
            	show_current_line(1);
                break;

            case KC_UP:
_up:
            	if (screen_y() > EDITOR_HEAD+1) {
            		// not on first line
        			editor->line--;
                	show_current_line(1);
            	}
            	else {
            		// on first line
            		if (editor->line > 1) {
            			editor->first--;
            			editor->line = editor->first;
            			show_editlines();
            		}
            	}
            	break;

            case KC_DOWN:
_down:
            	if ((screen_y()-EDITOR_HEAD) < editor->scrlin) {
            		// not on last line
                	editor->line++;
            		if (editor->line > editor->numlines) editor->line = editor->numlines;
                	show_current_line(1);
            	}
            	else {
            		// on last line
            		if (editor->line < editor->numlines) {
            			editor->first++;
            			editor->line = editor->first + editor->scrlin - 1;
                		if (editor->line > editor->numlines) editor->line = editor->numlines;
            			show_editlines();
            		}
            	}
            	break;

            case KC_PAGEUP:
				if (editor->first > 1) {
					int linpos =  editor->line - editor->first;
					editor->col = 1;
					editor->first -= editor->scrlin;
					if (editor->first < 1) editor->first = 1;
					editor->line = editor->first + linpos;
					show_editlines();
				}
            	break;

            case KC_PAGEDOWN:
            	{
					int linpos =  editor->line - editor->first;
            		editor->col = 1;
					editor->first += editor->scrlin;
					if (editor->first > editor->numlines) editor->first = editor->numlines;
					editor->line = editor->first + linpos;
					if (editor->line > editor->numlines) editor->line = editor->numlines;
					show_editlines();
            	}
            	break;

            case KC_TAB:
        		// insert 4 spaces
				if (buf_insert(4)) {
					memset(editor->buf+editor->ptr+editor->col-1, ' ', 4);
					editor->col += 4;
                	editor->changed++;
                	show_current_line(1);
        		}
            	break;

            default:
                if ((c >= 32) && (c < 127)) {
                	if (editor->ins) {
                		// insert character
        				if (buf_insert(1)) {
		               		editor->buf[editor->ptr + editor->col - 1] = c;
							editor->col++;
		                	editor->changed++;
                		}
                	}
                	else {
						editor->col++;
                		editor->buf[editor->ptr + editor->col - 1] = c;
                    	editor->changed++;
                	}
                	show_current_line(1);
                }
                break;
        }
    }
}

// Lua: edit(fname)
//========================================
static int luaterm_edit( lua_State* L )
{
	const char *fname;
	size_t len;
	int isnew = 0;
	char buf[128];
	fname = luaL_checklstring( L, 1, &len );
	editor_t edit;

	editor = &edit;

	// Open the file
	int ffd = file_open(fname, 0);
	if (ffd < 0) {
        ffd = file_open(fname, O_CREAT);
    	if (ffd < 0) {
    		return luaL_error(L, "Error creating file.");
    	}
    	file_close(ffd);
    	ffd = file_open(fname, 0);
    	if (ffd < 0) {
    		return luaL_error(L, "Error opening file.");
    	}
    	isnew = 1;
	}
	// Get file size
	editor->fsize = file_size(ffd);
	if (editor->fsize < 0) {
	    file_close(ffd);
		return luaL_error(L, "Error getting file size.");
	}

	editor->buf_size = editor->fsize + 1024;
	editor->buf = malloc(editor->buf_size);
	if (editor->buf == NULL) return luaL_error(L, "Error allocating edit buffer.");

	memset(editor->buf, 0, editor->buf_size);
	editor->ptr = 0;
	editor->linend = 1;
	editor->ins = 1;
	sprintf(editor->fname, "%s", fname);

	// read file to buffer
	int was_cr = 0;
	int rd = 0;
	int tabsize = 0;
	do {
		rd = file_read(ffd, buf, 128);
		if (rd > 0) {
			int i;
			for (i=0; i<rd; i++) {
				if (buf[i] == 13) {
					editor->buf[editor->ptr] = buf[i];
					editor->ptr++;
					was_cr = 1;
					editor->linend = 2;
					continue;
				}

				if (buf[i] == 10) {
					if (was_cr) was_cr = 0;
					else if (editor->linend == 2) {
						rd = -101;
						goto endread;
					}
					editor->buf[editor->ptr] = buf[i];
					editor->ptr++;
					continue;
				}

				was_cr = 0;

				if (buf[i] == 9) {
					memset(editor->buf + editor->ptr, ' ', 4);
					editor->ptr += 4;
					tabsize += 3;
					continue;
				}

				if ((buf[i] >= 32) && (buf[i] < 127)) {
					editor->buf[editor->ptr] = buf[i];
					editor->ptr++;
					continue;
				}

				rd = -102;
				goto endread;
			}

			if (rd < 128) goto endread;

			// in the next pass we need max 4 characters
			if ((editor->ptr + 4) >= editor->buf_size) {
				// need more space
				char *newb = realloc(editor->buf, editor->buf_size + 1024);
				if (newb == NULL) {
					rd = -103;
					goto endread;
				}
				editor->buf = newb;
			}
		}
	} while (rd > 0);

endread:
	editor->buf[editor->ptr] = '\0';

	file_close(ffd);

	if (rd < 0) {
		free(editor->buf);
		if (rd == -101) return luaL_error(L, "EOL format error, not text file");
		else if (rd == -102) return luaL_error(L, "Non printable characters, binary file");
		else if (rd == -103) return luaL_error(L, "Error reallocating edit buffer");
		else return luaL_error(L, "Error reading file");
	}

	editor->size = editor->ptr;
	int read_size = editor->ptr;

	if (editor->size > 0) editor->numlines = get_numlines();
	else {
		sprintf(editor->buf, "\n");
		editor->numlines = 1;
	}
	editor->show_lnum = 6;
	// Set number of visible lines
	editor->scrlin = term_get_lines() - EDITOR_HEAD - EDITOR_FOOT;
	editor->changed = 0;

	// === Show header ===
	term_clrscr();
	term_putstr("===== Edit file: ", 17);
	term_putstr(editor->fname, strlen(editor->fname));
	if (isnew) term_putstr(" (new) ", 7);
	else term_putch(' ');
	while (term_get_cx() <= term_num_cols) { term_putch('='); }

	term_gotoxy( 1, term_get_lines()-1);
	while (term_get_cx() <= term_num_cols) { term_putch('='); }

	// === Do edit ======
	int res = edit_lin();
	// ==================

	// === Save file if needed ===
	term_gotoxy( 1, term_get_lines());
	term_clreol();
	if ((res == 0) && (editor->changed)) {
		res = -1;
		term_putstr("Save file ? (y/n) ", 18);
        int c = term_getch(TERM_INPUT_WAIT);
    	term_gotoxy( 1, term_get_lines());
    	term_clreol();
        if ((c == 'y') || (c == 'Y')) {
    		sprintf(buf, "Save as [%s]: ", fname);
    		res = term_getstr(buf, 64);
    		if (res == 0) {
    			if (strlen(buf) == 0) sprintf(buf, "%s", fname);
    			// save file
                ffd = file_open(buf, O_CREAT);
                if (ffd >= 0) {
                	res = file_write(ffd, editor->buf, editor->size);
                	file_close(ffd);
                }
                else res = -1;
    		}
        }
    	term_gotoxy( 1, term_get_lines());
    	term_clreol();
    	if (res < 0) term_putstr("File not saved\n", 15);
    	else term_putstr("File saved\n", 11);
	}
	else {
		if ((editor->changed == 0) && (read_size != editor->fsize) && (read_size != (editor->fsize+tabsize))) {
			printf("Warning: file size (%d) <> read size (%d) [%d]\n", editor->fsize, read_size, read_size+tabsize);
		}
	}

	// free editor buffer
	free(editor->buf);

	return 0;
}

//-------------------------------
int _fs_free_space( lua_State* L)
{
	g_shell_result = vm_fs_get_disk_free_space(vm_fs_get_internal_drive_letter())-(1024*10);
	vm_signal_post(g_shell_signal);
	return 0;
}


//==================================
static int file_recv( lua_State* L )
{
  int fsize = 0;
  unsigned char c, gnm;
  char fnm[64];

  remote_CCall(L, &_fs_free_space);
  unsigned int max_len = g_shell_result-(1024*10);

  gnm = 0;
  if (lua_gettop(L) == 1 && lua_type( L, 1 ) == LUA_TSTRING) {
    // file name is given
    size_t len;
    const char *fname = luaL_checklstring( L, 1, &len );
    strcpy(fnm, fname);
    gnm = 1; // file name ok
  }
  if (gnm == 0) memset(fnm, 0x00, 64);

  while (retarget_getc(0) >= 0) {};

  vm_log_info("Start Ymodem file transfer, max size: %d", max_len);

  fsize = Ymodem_Receive(fnm, max_len, gnm);

  while (retarget_getc(0) >= 0) {};

  if (fsize > 0) printf("\nReceived successfully, %d\n",fsize);
  else if (fsize == -1) printf("\nFile write error!\n");
  else if (fsize == -2) printf("\nFile open error!\n");
  else if (fsize == -3) printf("\nAborted.\n");
  else if (fsize == -4) printf("\nFile size too big, aborted.\n");
  else if (fsize == -5) printf("\nWrong file name or file exists!\n");
  else printf("\nReceive failed!\n");

  g_shell_result = 0;
  vm_signal_post(g_shell_signal);
  return 0;
}

//==================================
static int file_send( lua_State* L )
{
  char res = 0;
  char newname = 0;
  unsigned char c;
  const char *fname;
  size_t len;
  int fsize = 0;

  fname = luaL_checklstring( L, 1, &len );
  char filename[64] = {0};
  const char * newfname;

  if (lua_gettop(L) >= 2 && lua_type( L, 2 ) == LUA_TSTRING) {
    size_t len;
    newfname = luaL_checklstring( L, 2, &len );
    newname = 1;
  }

  // Open the file
  int ffd = file_open(fname, 0);
  if (ffd < 0) {
    l_message(NULL,"Error opening file.");
    goto exit;
  }

  // Get file size
  fsize = file_size(ffd);
  if (fsize < 0) {
    file_close(ffd);
    l_message(NULL,"Error opening file.");
    goto exit;
  }

  printf("Sending '%s' (%d bytes)", fname, fsize);
  if (newname == 1) {
    printf(" as '%s'\n", newfname);
    strcpy(filename, newfname);
  }
  else {
	  printf("\n");
	  strcpy(filename, fname);
  }

  l_message(NULL,"Start Ymodem file transfer...");

  while (retarget_getc(0) >= 0) {};

  res = Ymodem_Transmit(filename, fsize, ffd);

  vm_thread_sleep(500);
  while (retarget_getc(0) >= 0) {};

  file_close(ffd);

  if (res == 0) {
    l_message(NULL,"\nFile sent successfuly.");
  }
  else if (res == 99) {
    l_message(NULL,"\nNo response.");
  }
  else if (res == 98) {
    l_message(NULL,"\nAborted.");
  }
  else {
    l_message(NULL,"\nError sending file.");
  }

exit:
  g_shell_result = 0;
  vm_signal_post(g_shell_signal);
  return 0;
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
  { LSTRKEY( "clrscr" ),	LFUNCVAL( luaterm_clrscr ) },
  { LSTRKEY( "clreol" ),	LFUNCVAL( luaterm_clreol ) },
  { LSTRKEY( "moveto" ),	LFUNCVAL( luaterm_moveto ) },
  { LSTRKEY( "moveup" ),	LFUNCVAL( luaterm_moveup ) },
  { LSTRKEY( "movedown" ),	LFUNCVAL( luaterm_movedown ) },
  { LSTRKEY( "moveleft" ),	LFUNCVAL( luaterm_moveleft ) },
  { LSTRKEY( "moveright" ),	LFUNCVAL( luaterm_moveright ) },
  { LSTRKEY( "getlines" ),	LFUNCVAL( luaterm_getlines ) },
  { LSTRKEY( "getcols" ),	LFUNCVAL( luaterm_getcols ) },
  { LSTRKEY( "setlines" ),	LFUNCVAL( luaterm_setlines ) },
  { LSTRKEY( "setcols" ),	LFUNCVAL( luaterm_setcols ) },
  { LSTRKEY( "print" ),		LFUNCVAL( luaterm_print ) },
  { LSTRKEY( "getcx" ),		LFUNCVAL( luaterm_getcx ) },
  { LSTRKEY( "getcy" ),		LFUNCVAL( luaterm_getcy ) },
  { LSTRKEY( "getchar" ),	LFUNCVAL( luaterm_getchar ) },
  { LSTRKEY( "getstr" ),	LFUNCVAL( luaterm_getstr ) },
  { LSTRKEY( "setctype" ),	LFUNCVAL( luaterm_setctype ) },
  { LSTRKEY( "edit" ),		LFUNCVAL( luaterm_edit ) },
  { LSTRKEY( "yrecv" ),		LFUNCVAL( file_recv ) },
  { LSTRKEY( "ysend" ),		LFUNCVAL( file_send ) },
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
  lua_pushnumber( L, KC_CTRL_L );
  lua_setfield( L, -2, "CTRL_L" );
  lua_pushnumber( L, KC_DEL );
  lua_setfield( L, -2, "DEL" );
  lua_pushnumber( L, KC_INS );
  lua_setfield( L, -2, "INS" );
  lua_pushnumber( L, KC_UNKNOWN );
  lua_setfield( L, -2, "UNKNOWN" );

  return 1;
#endif // # if LUA_OPTIMIZE_MEMORY > 0
}

