--[[
A bare-metal editor written in eLua
https://www.seanet.com/~karllunt/elua_editor.html
--]]

lines = {}
pastebuff = {}
showlinenums = true
filepath = ""

CMD_QUIT = string.byte('q')
CMD_OPEN = string.byte('o')
CMD_GOTO = string.byte('g')
CMD_INSERT = string.byte('i')
CHAR_CTRL_Z = 268             -- ctrl-Z
CMD_WRITE = string.byte('w')
CMD_DELETE = string.byte('d')
CMD_TOP = string.byte('t')
CMD_BOTTOM = string.byte('b')
CMD_NEXT_BLK = string.byte('N')
CMD_PREV_BLK = string.byte('P')
CMD_APPEND = string.byte('a')
CMD_HELP = string.byte('?')
CMD_REPLACE = string.byte('r')
CMD_COPY = string.byte('c')
CMD_PASTE = string.byte('v')
CMD_CUT = string.byte('x')
CMD_TOGGLE_NUMS = string.byte('#')
CMD_EDIT = string.byte('e')
CMD_NEXT_LINE = string.byte('n')
CMD_PREV_LINE = string.byte('p')
CMD_EXECUTE = string.byte('!')

CMD_YES = string.byte('y')
CMD_NO = string.byte('n')


local  prevstatusmsg = ""

--LINES_IN_DISPLAY = term.getlines()
LINES_IN_DISPLAY = 22
LINE_ERROR_DISPLAY = LINES_IN_DISPLAY+2
LINE_STATUS = LINES_IN_DISPLAY+1
LINE_CMD = LINES_IN_DISPLAY+2


LINE_ENDING = "\n"


function ShowHelp()
  term.clrscr()
  print("   " .. string.char(CMD_HELP)     .. "   show this help screen")
  print()
  print("   " .. string.char(CMD_OPEN)     .. "   open a file for editing")
  print("   " .. string.char(CMD_WRITE)    .. "   write text to a file")
  print("   " .. string.char(CMD_QUIT)     .. "   exit editor, DOES NOT SAVE!")
  print("  " .. string.char(CMD_TOP) .. "/" .. string.char(CMD_BOTTOM)
                                          .. "  display text at top/bottom of file")
  print("  " .. string.char(CMD_NEXT_LINE) .. "/" .. string.char(CMD_NEXT_BLK)
                                          .. "  display next line/block of text")
  print("  " .. string.char(CMD_PREV_LINE) .. "/" .. string.char(CMD_PREV_BLK)
                                           .. "  display previous line/block of text")
  print("   " .. string.char(CMD_TOGGLE_NUMS) .. "   show/hide line numbers")
  print()
  print("   " .. string.char(CMD_REPLACE)  .. "   replace selected line")
  print("   " .. string.char(CMD_EDIT)     .. "   edit selected line")
  print("   " .. string.char(CMD_INSERT)   .. "   add new lines of text at selected line")
  print("   " .. string.char(CMD_APPEND)   .. "   add new lines of text at end of file")
  print("   " .. string.char(CMD_DELETE)   .. "   delete one or more lines of text")
  print()
  print("   " .. string.char(CMD_COPY)     .. "   copy one or more lines of text to paste buffer")
  print("   " .. string.char(CMD_CUT)      .. "   cut (copy and delete) one or more lines of text")
  print("   " .. string.char(CMD_PASTE)    .. "   insert text from paste buffer")
  print()
  print("   " .. string.char(CMD_GOTO)     .. "   go to selected line")
  print("   " .. string.char(CMD_EXECUTE)  .. "   execute a file as an eLua script")
  print()
  term.print(1, LINE_CMD, "Press Enter to continue: ")
  repeat
    c = term.getchar(term.WAIT)
  until (c == term.KC_ENTER)
end


function ModifyLine(linenum, cmd)
  if (cmd:sub(1,1) == string.sub('?',1,1)) then
    term.clrscr()
    print("Enter a substitution string to edit this line.")
    print("A substitution string has a pattern and a replacement")
    print("string, separated by delimiters.")
    print()
    print("For example:")
    print("  /foo/bar/")
    print("will change the first occurence of 'foo' to 'bar'.")
    print()
    print("You can use either '/' or '\\' as delimiters, but you must")
    print("use the same delimiter throughout the string.")
    print()
    term.print(1, LINE_CMD, "Press Enter to continue: ")
    repeat
      c = term.getchar(term.WAIT)
    until (c == term.KC_ENTER)
  else
    delim = cmd:sub(1,1)
    if ((delim == string.sub('/', 1, 1)) or (delim == string.sub('\\', 1, 1))) then
      patend = cmd:find(delim, 2)
      if (patend ~= nil) then
        pattern = cmd:sub(2, patend-1)
        replend = cmd:find(delim, patend+1)
        if (replend ~= nil) then
          repl = cmd:sub(patend+1, replend-1)
          s = lines[linenum]
          s = string.gsub(s, pattern, repl, 1)
          lines[linenum] = s
          filechanged = true
        end
      end
    end
  end
end


function ShowErrorAndWait(errmsg)
  MoveToLineAndPrint(LINE_ERROR_DISPLAY, errmsg)
  repeat
    c = term.getchar(term.WAIT)
  until (c == term.KC_ENTER)
end


function ShowStatus(statusmsg)
  if (statusmsg == nil) then
    MoveToLineAndPrint(LINE_STATUS, prevstatusmsg)
  else
    MoveToLineAndPrint(LINE_STATUS, statusmsg)
    prevstatusmsg = statusmsg
  end
end


function LoadFile(filepath)
  file = io.open(filepath)
  if (file == nil) then
    ShowErrorAndWait("Cannot open file: " .. filepath)
    return
  end

  repeat
    line = file:read("*l")
    if (line ~= nil) then
      table.insert(lines, line)
    end
  until (line == nil)
  file:close()

  ShowStatus("File: " .. filepath)
  filechanged = false
  firstline = 1
end


function SaveFile(savepath)
  file = io.open(savepath, "w")
  if (file == nil) then
    ShowErrorAndWait("Cannot write to file " .. savepath)
    return
  end

  for i, l in ipairs(lines) do
    file:write(l, LINE_ENDING)
  end
  file:flush()
  file:close()
end


function DisplayFile(firstline)
  term.clrscr()
  for i, str in ipairs(lines) do
    if (i >= firstline) then
      if (showlinenums == true) then
        term.print(1, i - firstline + 1, i .. ":" .. str)
      else
        term.print(1, i - firstline + 1, str)
      end
      if ((i - firstline + 1) == LINES_IN_DISPLAY) then
        break
      end
    end
  end
  ShowStatus()
end


function MoveToLineAndPrint(linenum, str)
  term.moveto(1, linenum)
  term.clreol()
  term.print(str)
end


function GetCmd()
  repeat
    MoveToLineAndPrint(LINE_CMD, "Cmd: ")
    c = term.getchar(term.WAIT)

    if (c == CMD_QUIT) then
      if (filechanged == true) then
        MoveToLineAndPrint(LINE_CMD, "File has changed!  Quit without saving? ")
        repeat
          c = term.getchar(term.WAIT)
        until ((c == CMD_YES) or (c == CMD_NO))
      end
      if ((c == CMD_YES) or (c == CMD_QUIT)) then
        done = 1
        return
      end

    elseif (c == CMD_OPEN) then
        if (filechanged == true) then
          MoveToLineAndPrint(LINE_CMD, "File has changed!  Open without saving? ")
            repeat
              c = term.getchar(term.WAIT)
            until ((c == CMD_YES) or (c == CMD_NO))
        end
        if ((c == CMD_YES) or (c == CMD_OPEN)) then
          MoveToLineAndPrint(LINE_CMD, "File: ")
            tfile = io.read("*l")
            if ((tfile ~= nil) and (tfile ~= "")) then
              lines = {}
          filepath = tfile
              LoadFile(filepath)
              firstline = 1
            end
        end
      DisplayFile(firstline)

      elseif (c == CMD_GOTO) then
      MoveToLineAndPrint(LINE_CMD, "Line: ")
      firstline = tonumber(io.read("*l"))
      if firstline ~= nil then
        if (firstline > #lines - (LINES_IN_DISPLAY+1)) then
            firstline = #lines - LINES_IN_DISPLAY + 1
        end
        if (firstline < 1) then firstline = 1 end
      else
          firstline = 1
      end
      DisplayFile(firstline)
      
    elseif (c == CMD_INSERT) then
      if (#lines == 0) then
        insertline = 1
      else
        MoveToLineAndPrint(LINE_CMD, "Insert above which line: ")
        insertline = tonumber(io.read("*l"))
      end
      if (insertline ~= nil) then
        firstline = insertline - LINES_IN_DISPLAY/2
        if (firstline < 1) then firstline = 1 end
        repeat
          MoveToLineAndPrint(LINE_CMD, "Text (or ctrl-z): ")
          text = io.read("*l")
          if (text ~= nil) then
            table.insert(lines, insertline, text)
            insertline = insertline + 1
            if ((insertline - firstline) > LINES_IN_DISPLAY) then
              firstline = firstline + 1
            end
            filechanged = true
          end
          DisplayFile(firstline)
        until (text == nil)
      end
      DisplayFile(firstline)
      
    elseif (c == CMD_WRITE) then
      MoveToLineAndPrint(LINE_CMD, "File to write: ")
      savefile = io.read("*l")
      if (savefile ~= nil) then
        SaveFile(savefile)
        filechanged = false
        filepath = savefile
      end
      DisplayFile(firstline)
      
    elseif (c == CMD_DELETE) then
      if (#lines == 0) then
        ShowErrorAndWait("File is empty!")
      else
        MoveToLineAndPrint(LINE_CMD, "Lines start: ")
        start = tonumber(io.read("*l"))
        MoveToLineAndPrint(LINE_CMD, "Lines stop: ")
        stop = tonumber(io.read("*l"))
        if (start ~= nil) then
          if (stop == nil) then
            stop = start
          end
          max = #lines
          if (start > max) then
            ShowErrorAndWait("File only has " .. max .. " lines!")
          else
            for n = start, stop do
              table.remove(lines, start)
              filechanged = true
            end
          end
          firstline = start
        end
        DisplayFile(firstline)
      end
      
    elseif (c == CMD_TOP) then
      t = firstline
      firstline = 1
      if (t ~= firstline) then DisplayFile(firstline) end
      
    elseif (c == CMD_BOTTOM) then
      t = firstline
      firstline = #lines - LINES_IN_DISPLAY + 1
      if (firstline < 1) then firstline = 1 end
      if (t ~= firstline) then DisplayFile(firstline) end
      
    elseif (c == CMD_NEXT_BLK) then
      t = firstline
      firstline = firstline + LINES_IN_DISPLAY
      if (firstline > #lines) then
        firstline = #lines - LINES_IN_DISPLAY + 1
        if (firstline < 1) then firstline = 1 end
      end
      if (t ~= firstline) then DisplayFile(firstline) end

    elseif (c == CMD_PREV_BLK) then
      t = firstline
      firstline = firstline - LINES_IN_DISPLAY
      if (firstline < 1) then firstline = 1 end
      if (t ~= firstline) then DisplayFile(firstline) end
      
    elseif (c == CMD_NEXT_LINE) then
      t = firstline
      if (firstline < (#lines - LINES_IN_DISPLAY)) then
        firstline = firstline + 1
      end
      if (t ~= firstline) then DisplayFile(firstline) end
      
    elseif (c == CMD_PREV_LINE) then
      t = firstline
      if (firstline > 1) then firstline = firstline - 1 end
      if (t ~= firstline) then DisplayFile(firstline) end

    elseif (c == CMD_APPEND) then
      repeat
        MoveToLineAndPrint(LINE_CMD, "Text (or ctrl-z): ")
        text = io.read("*l")
        if (text ~= nil) then
          table.insert(lines, text)
          filechanged = true
        end
        firstline = #lines - LINES_IN_DISPLAY + 1
        if (firstline < 1) then firstline = 1 end
        DisplayFile(firstline)
      until (text == nil)

    elseif (c == CMD_REPLACE) then
      MoveToLineAndPrint(LINE_CMD, "Line: ")
      text = io.read("*l")
      currline = tonumber(text)
      if ((currline == nil) or (currline < 1)) then
      elseif (currline > #lines) then
        ShowErrorAndWait("No such line; file only has " .. #lines .. " lines.")
      else
        firstline = currline - (LINES_IN_DISPLAY /2)
        if (firstline < 1) then firstline = 1 end
        DisplayFile(firstline)
        tmsg = prevstatusmsg
        ShowStatus(currline .. ":" .. lines[currline])
        MoveToLineAndPrint(LINE_CMD, "Text (or ctrl-z): ")
        text = io.read("*l")
        if (text ~= nil) then
          lines[currline] = text
          filechanged = true
        end
      end
      DisplayFile(firstline)
      ShowStatus(tmsg)

      elseif ((c == CMD_COPY) or
              (c == CMD_CUT)) then
      if (#lines == 0) then
        ShowErrorAndWait("File is empty!")
      else
        MoveToLineAndPrint(LINE_CMD, "Lines start: ")
        start = tonumber(io.read("*l"))
        MoveToLineAndPrint(LINE_CMD, "Lines stop: ")
        stop = tonumber(io.read("*l"))
        if (start ~= nil) then
          if (stop == nil) then
            stop = start
          end
          max = #lines
          if (start > max) then
            ShowErrorAndWait("File only has " .. max .. " lines!")
          else
            pastebuff = {}
            for n = start, stop do
              table.insert(pastebuff, lines[n])
              if (c == CMD_CUT) then table.remove(lines, n) end
            end
          end
          firstline = start
        end
        DisplayFile(firstline)
      end

    elseif (c == CMD_INSERT) then
      if (#lines == 0) then
        insertline = 1
      else
        MoveToLineAndPrint(LINE_CMD, "Insert above which line: ")
        insertline = tonumber(io.read("*l"))
      end
      if (insertline ~= nil) then
        repeat
          MoveToLineAndPrint(LINE_CMD, "Text (or ctrl-z): ")
          text = io.read("*l")
          if (text ~= nil) then
            table.insert(lines, insertline, text)
            insertline = insertline + 1
        if (insertline-firstline > 10) then firstline = firstline + 1 end
            filechanged = true
          end
          DisplayFile(firstline)
        until (text == nil)
      end
      DisplayFile(firstline)

    elseif (c == CMD_PASTE) then
        if (#pastebuff == 0) then
          ShowErrorAndWait("No text in paste buffer!")
        else
        if (#lines == 0) then
          insertline = 1
        else
          MoveToLineAndPrint(LINE_CMD, "Paste above which line: ")
          insertline = tonumber(io.read("*l"))
        end
        if (insertline ~= nil) then
            if (insertline > #lines) then
                insertline = #lines + 1
              end
          firstline = insertline - LINES_IN_DISPLAY/2
              if firstline < 1 then firstline = 1 end
              for _, text in ipairs(pastebuff) do
                table.insert(lines, insertline, text)
                insertline = insertline + 1
                filechanged = true
              end
            end
        end
      DisplayFile(firstline)
      
    elseif (c == CMD_TOGGLE_NUMS) then
      showlinenums = not showlinenums
      DisplayFile(firstline)
      
    elseif (c == CMD_EDIT) then
      if (#lines == 0) then
        ShowErrorAndWait("No lines in file!")
        DisplayFile(firstline)
      else
        MoveToLineAndPrint(LINE_CMD, "Edit line: ")
        editline = tonumber(io.read("*l"))
        if (editline ~= nil) then
          if ((editline < 1) or (editline > #lines)) then
            ShowErrorAndPrint(LINE_CMD, "File only has " .. #lines .. " lines!")
            DisplayFile(firstline)
          else
            repeat
              DisplayFile(firstline)
              MoveToLineAndPrint(LINE_CMD, "Subst (" .. editline .. "): ")
              cmdline = io.read("*l")
              if (cmdline ~= nil) then
                ModifyLine(editline, cmdline)
              end
            until (cmdline == nil)
          end
        end
      end

    elseif (c == CMD_HELP) then
      ShowHelp()
      DisplayFile(firstline)

    elseif (c == CMD_EXECUTE) then
      if (filepath ~= "") then 
        MoveToLineAndPrint(LINE_CMD, "Execute file (" .. filepath .. "): ")
      else
        MoveToLineAndPrint(LINE_CMD, "Execute file: ")
      end
          tfile = io.read("*l")
          if (tfile ~= nil) then
        if ((tfile == "") and (filepath ~= "")) then
          dofile(filepath)
        elseif (tfile ~= "") then
          dofile(tfile)
        end
      end
      ShowErrorAndWait("Press Enter to continue: ")
        DisplayFile(firstline)

    else
      DisplayFile(1)
    end
  until (done == 1)
end


function Edit(fname)
    LoadFile(fname)
    filepath = ""
    DisplayFile(1)
    GetCmd()
end

done = 0
print()

