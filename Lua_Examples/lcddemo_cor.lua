-- init spi first

if dispType == nil then
	dispType = lcd.XADOW_V0
	-- dispType = lcd.XADOW_V1
	-- dispType = lcd.ILI9341
	-- dispType = lcd.ST7735B -- probably will work
	-- dispType = lcd.ST7735  -- try if not
	-- dispType = lcd.ST7735G -- or this one
end

if dispType < 8 then
	-- if not using Xadow display we have to initialize SPI interface
	lcd_OK = spi.setup({speed=10000, cs=2, dc=1})
	if lcd_OK ~= 0 then
		print("SPI not initialized")
		return
	end
end

lcd.init(dispType,lcd.PORTRAIT_FLIP)
if lcd.gettype() < 0 then
	print("LCD not initialized")
	return
end

fontnames = {
	lcd.FONT_DEFAULT,
	lcd.FONT_7SEG,
	"@font\\DejaVuSans18.fon",
	"@font\\DotMatrix_M.fon",
	"@font\\OCR_A_Extended_M.fon"
}

-- print display header
function header(tx)
	sys.random(1000,0,1)
	maxx, maxy = lcd.getscreensize()
	lcd.clear()
	lcd.setcolor(lcd.CYAN)
	if maxx < 240 then
		lcd.setfont("@font\\SmallFont.fon")
	else
		lcd.setfont(lcd.FONT_DEFAULT)
	end
	miny = lcd.getfontheight() + 5
	lcd.rect(0,0,maxx-1,miny-1,lcd.OLIVE,{8,16,8})
	lcd.settransp(1)
	lcd.write(lcd.CENTER,2,tx)
	lcd.settransp(0)
end

-- Display available fonts
function dispFont(sec)
	header("DISPLAY FONTS")

	local tx
	if maxx < 240 then
		tx = "RePhone"
	else
		tx = "Hi from LoBo"
	end
	local starty = miny + 4

	local x,y
	local n = sys.tick()
	while sys.elapsed(n) < (sec*1000000) do
		y = starty
		x = 0
		local i,j
		for i=1, 3, 1 do
			for j=1, #fontnames, 1 do
				lcd.setcolor(sys.random(0xFFFF))
				lcd.setfont(fontnames[j])
				if j ~= 2 then
					lcd.write(x,y,tx)
				else
					lcd.write(x,y,"-12.45/")
				end
				y = y + lcd.getfontheight()
				if y > (maxy-lcd.getfontheight()) then
					break
				end
			end
			y = y + 2
			if y > (maxy-lcd.getfontheight()) then
				break
			end
			if i == 1 then 
				x = lcd.CENTER
			end
			if i == 2 then
				x = lcd.RIGHT
			end
		end
	end
end

function fontDemo(sec, rot)
	local tx = "FONTS"
	if rot > 0 then
		tx = "ROTATED "..tx
	end
	header(tx)

	lcd.setclipwin(0,miny,maxx,maxy)
	tx = "RePhone"
	local x, y, color, i
	local n = sys.tick()
	while sys.elapsed(n) < (sec*1000000) do
		if rot == 1 then
			lcd.setrot(math.floor(sys.random(359)/5)*5);
		end
		for i=1, #fontnames, 1 do
			if (rot == 0) or (i ~= 1) then
				lcd.setcolor(sys.random(0xFFFF))
				lcd.setfont(fontnames[i])
				x = sys.random(maxx-8)
				y = sys.random(maxy-lcd.getfontheight(),miny)
				if i ~= 2 then
					lcd.write(x,y,tx)
				else
					lcd.write(x,y,"-12.45/")
				end
			end
		end
	end
	lcd.resetclipwin()
	lcd.setrot(0)
end

function lineDemo(sec)
	header("LINE DEMO")

	lcd.setclipwin(0,miny,maxx,maxy)
	local x1, x2,y1,y2,color
	local n = sys.tick()
	while sys.elapsed(n) < (sec*1000000) do
		x1 = sys.random(maxx-4)
		y1 = sys.random(maxy-4,miny)
		x2 = sys.random(maxx-1)
		y2 = sys.random(maxy-1,miny)
		color = sys.random(0xFFFF)
		lcd.line(x1,y1,x2,y2,color)
	end;
	lcd.resetclipwin()
end;

function circleDemo(sec,dofill)
	local tx = "CIRCLE"
	if dofill > 0 then
		tx = "FILLED "..tx
	end
	header(tx)

	lcd.setclipwin(0,miny,maxx,maxy)
	local x, y, r, color, fill
	local n = sys.tick()
	while sys.elapsed(n) < (sec*1000000) do
		x = sys.random(maxx-2,4)
		y = sys.random(maxy-2,miny+2)
		if x < y then
			r = sys.random(x,2)
		else
			r = sys.random(y,2)
		end
		color = sys.random(0xFFFF)
		if dofill > 0 then
			fill = sys.random(0xFFFF)
			lcd.circle(x,y,r,color,fill)
		else
			lcd.circle(x,y,r,color)
		end
	end;
	lcd.resetclipwin()
end;

function rectDemo(sec,dofill)
	local tx = "RECTANGLE"
	if dofill > 0 then
		tx = "FILLED "..tx
	end
	header(tx)

	lcd.setclipwin(0,miny,maxx,maxy)
	local x, y, w, h, color, fill
	local n = sys.tick()
	while sys.elapsed(n) < (sec*1000000) do
		x = sys.random(maxx-2,4)
		y = sys.random(maxy-2,miny)
		w = sys.random(maxx-x,2)
		h = sys.random(maxy-y,2)
		color = sys.random(0xFFFF)
		if dofill > 0 then
			fill = sys.random(0xFFFF)
			lcd.rect(x,y,w,h,color,fill)
		else
			lcd.rect(x,y,w,h,color)
		end
	end;
	lcd.resetclipwin()
end;

function triangleDemo(sec,dofill)
	local tx = "TRIANGLE"
	if dofill > 0 then
		tx = "FILLED "..tx
	end
	header(tx)

	lcd.setclipwin(0,miny,maxx,maxy)
	local x1, y1, x2, y2, x3, y3, color, fill
	local n = sys.tick()
	while sys.elapsed(n) < (sec*1000000) do
		x1 = sys.random(maxx-2,4)
		y1 = sys.random(maxy-2,miny)
		x2 = sys.random(maxx-2,4)
		y2 = sys.random(maxy-2,miny)
		x3 = sys.random(maxx-2,4)
		y3 = sys.random(maxy-2,miny)
		color = sys.random(0xFFFF)
		if dofill > 0 then
			fill = sys.random(0xFFFF)
			lcd.triangle(x1,y1,x2,y2,x3,y3,color,fill)
		else
			lcd.triangle(x1,y1,x2,y2,x3,y3,color)
		end
	end;
	lcd.resetclipwin()
end;

function pixelDemo(sec)
	header("PUTPIXEL")

	lcd.setclipwin(0,miny,maxx,maxy)
	local x, y, color
	local n = sys.tick()
	while sys.elapsed(n) < (sec*1000000) do
		x = sys.random(maxx-1)
		y = sys.random(maxy-1,miny)
		color = sys.random(0xFFFF)
		lcd.putpixel(x,y,color)
	end;
	lcd.resetclipwin()
	if (maxx > maxy) then
		if os.exists("nature_160x123.img") ~= 0 then
			lcd.image(math.floor((maxx-160) / 2),miny + 4,160,123,"nature_160x123.img")
		else
			lcd.write(lcd.CENTER,miny+4,"Image not found")
		end
	else
		if os.exists("newyear_128x96.img") ~= 0 then
			lcd.image(math.floor((maxx-128) / 2),miny + 4,128,96,"newyear_128x96.img")
		else
			lcd.write(lcd.CENTER,miny+4,"Image not found")
		end
	end
end

function intro(sec)
	maxx, maxy = lcd.getscreensize()
	local inc = 360 / maxy
	local i
	for i=0,maxy-1,1 do
		lcd.line(0,i,maxx-1,i,lcd.hsb2rgb(i*inc,1,1))
	end
	lcd.setrot(0);
	lcd.setcolor(lcd.BLACK)
	lcd.setfont(lcd.FONT_DEFAULT)
	local y = (maxy/2) - (lcd.getfontheight() / 2)
	lcd.settransp(1)
	lcd.write(lcd.CENTER,y,"RePhone")
	y = y + lcd.getfontheight()
	lcd.write(lcd.CENTER,y,"LCD demo")
	lcd.settransp(0)
	for i=1, sec, 1 do
		msleep(1000)
	end
end

function lcdDemo(sec, orient)
	lcd.setorient(orient)

	intro(sec)
	dispFont(sec)
	msleep(2000)
	fontDemo(sec,0)
	msleep(2000)
	fontDemo(sec,1)
	msleep(2000)
	lineDemo(sec,1)
	msleep(2000)
	circleDemo(sec,0)
	msleep(2000)
	circleDemo(sec,1)
	msleep(2000)
	rectDemo(sec,0)
	msleep(2000)
	rectDemo(sec,1)
	msleep(2000)
	triangleDemo(sec,0)
	msleep(2000)
	triangleDemo(sec,1)
	msleep(2000)
	pixelDemo(sec, orient)
	msleep(2000)
end

function fullDemo(sec, rpt)
	while rpt > 0 do
		lcd.setrot(0);
		lcd.setcolor(lcd.CYAN)
		lcd.setfont(lcd.FONT_DEFAULT)

		lcdDemo(sec, lcd.LANDSCAPE)
		msleep(5000)
		lcdDemo(sec, lcd.PORTRAIT_FLIP)
		msleep(5000)

		lcd.setcolor(lcd.YELLOW)
		lcd.write(lcd.CENTER,maxy-lcd.getfontheight() - 4,"That's all folks!")
		rpt = rpt - 1
	end
end

function cotmr_cb_dummy()
end

coRun = 3000
coTmr = timer.create(20, cotmr_cb_dummy, 1)

function rectDemoCo()
	local tx = "CO RECTANGLE"
	header(tx)
	
	lcd.setclipwin(0,miny,maxx,maxy)
	local x, y, w, h, color, fill
	while coRun > 0 do
		coRun = coRun - 1
		--print("Run = "..coRun)
		x = sys.random(maxx-2,4)
		y = sys.random(maxy-2,miny)
		w = sys.random(maxx-x,2)
		h = sys.random(maxy-y,2)
		color = sys.random(0xFFFF)
		fill = sys.random(0xFFFF)
		lcd.rect(x,y,w,h,color,fill)
		
		timer.resume(coTmr)
		coroutine.yield()
		timer.pause(coTmr)
	end;
	lcd.resetclipwin()
end

function cotmr_cb()
	if coRun > 0 then
		if coroutine.status(CoRect) == "suspended" then
			coroutine.resume(CoRect)
		end
	else
		if coroutine.status(CoRect) == "suspended" then
			coroutine.resume(CoRect)
		end
		timer.pause(coTmr)
	end
end

-- create a coroutine with rectDemoCo as the entry
CoRect = coroutine.create(rectDemoCo)
timer.changecb(coTmr, cotmr_cb)

header("RePhone")

print(coroutine.status(CoRect))
print(coTmr)

--fullDemo(6, 1)
