-- Touch pannel calibration for ILI9341 based displays
-- !! WORK IN PROGRESS !!

if dispType == nil then
	dispType = lcd.ILI9341
end

lcd_OK = spi.setup({speed=10000, cs=1, dc=1})
if lcd_OK ~= 0 then
	print("SPI not initialized")
	return
end

lcd.init(dispType,lcd.LANDSCAPE)
if lcd.gettype() < 0 then
	print("LCD not initialized")
	return
end

rx = {}
ry = {}


function drawCrossHair(x, y)
  lcd.rectangle(x-10, y-10, x+10, y+10)
  lcd.line(x-5, y, x+5, y)
  lcd.line(x, y-5, x, y+5)
end

function readCoordinates()
  local x = 0
  local y = 0
  
  lcd.setcolor(lcd.WHITE)
  lcd.write(lcd.CENTER, text_y_center, "*  PRESS  *")
  touch, x, y = lcd.gettouch()
  while touch <= 0 do
    touch, x, y = lcd.gettouch()
  end
  lcd.write(lcd.CENTER, text_y_center, "           ")
  return x,y
end


function calibrate(int x, int y, int i)
  lcd.setcolor(lcd.ORANGE)
  drawCrossHair(x,y)

  rx[i], ry[i] = readCoordinates()

  touch, x, y = lcd.gettouch()
  while touch > 0 do
    touch, x, y = lcd.gettouch()
  end

  lcd.setcolor(lcd.GREEN)
  drawCrossHair(x,y)
end

dispx = 320
dispy = 240

function calib()
  drawCrossHair(dispx-11, 10)
  drawCrossHair(dispx/2, 10)
  drawCrossHair(10, 10)
  drawCrossHair(dispx-11, dispy/2)
  drawCrossHair(10, dispy/2)
  drawCrossHair(dispx-11, dispy-11)
  drawCrossHair(dispx/2, dispy-11)
  drawCrossHair(10, dispy-11)

  calibrate(10, 10, 1)
  calibrate(10, dispy/2, 2)
  calibrate(10, dispy-11, 3)
  calibrate(dispx/2, 10, 4)
  calibrate(dispx/2, dispy-11, 5)
  calibrate(dispx-11, 10, 6)
  calibrate(dispx-11, dispy/2, 7)
  calibrate(dispx-11, dispy-11, 8)

  px = math.abs((((rx[5]+rx[6]+rx[7]) / 3) - ((rx[0]+rx[1]+rx[2]) / 3)) / (dispy-20))  -- LANDSCAPE
  clx = (((rx[0]+rx[1]+rx[2])/3))  -- LANDSCAPE
  crx = (((rx[5]+rx[6]+rx[7])/3))  -- LANDSCAPE

  if (clx<crx) then
    clx = clx - (px*10)
    crx = crx + (px*10)
  else
    clx = clx + (px*10)
    crx = crx - (px*10)
  end

  py = abs((((ry[0]+ry[3]+ry[5])/3) - ((ry[2]+ry[4]+ry[7])/3))/(dispx-20))  -- LANDSCAPE
  cty = (((ry[0]+ry[3]+ry[5])/3))  -- LANDSCAPE
  cby = (((ry[2]+ry[4]+ry[7])/3))  -- LANDSCAPE

  if (cty<cby) then
    cty = cty - (py*10)
    cby = cby + (py*10)
  else
    cty = cty + (py*10)
    cby = cby - (py*10)
  end

  calx = (clx * 16384) + crx
  caly = (cty * 16384) + cby
  cals = cals + 2147483648 -- LANDSCAPE

end

