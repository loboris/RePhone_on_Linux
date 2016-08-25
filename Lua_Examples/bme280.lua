--[[
bme280.lua for Rephone
  Read temperature, preassure and relative humidity from BME280 sensors
  More info at  https://github.com/avaldebe/AQmon

                Written by: Alvaro Valdebenito,
Modified for LuaRephone by: LoBo
  based on:
  - bme280.lua by WG
      https://github.com/wogum/esp12
  - BME280_driver by BoschSensortec
      https://github.com/BoschSensortec/BME280_driver
  - bme280.py by Kieran Brownlees
      https://github.com/kbrownlees/bme280
  - Adafruit_BME280.py by adafruit
      https://github.com/adafruit/Adafruit_Python_BME280
  - SparkFunBME280.cpp by sparkfun
      https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
  - Unsigned to signed conversion, eg uint16_t (unsigned short) to int16_t (short)
      http://stackoverflow.com/questions/17152300/unsigned-to-signed-without-comparison

Note:
  bit.rshift(x,n)~=x/2^n for n<0, use bit.arshift(x,n) instead

MIT license, http://opensource.org/licenses/MIT
]]

M={
  name=...,         -- module name, upvalue from require('module-name')
  model=nil,        -- sensor model: BME280
  verbose=nil,      -- verbose output
  debug=nil,        -- additional ckecks
  oss=0x01,         -- default oversamplig: 0=skip, 1=x1 .. 5=x16
  mode=0x03,        -- default sampling: 0=sleep, 1&2=forced(on demand), 3:normal(continious)
  temperature=nil, -- float value of temperature [0.01 C]
  pressure   =nil, -- float value of preassure [Pa]=[0.01 hPa]
  humidity   =nil  -- float value of rel.humidity [0.01 %]
}
_G[M.name]=M

-- BME280 has 2 possible addresses
local ADDR = {0x76,0x77}

-- calibration coefficients
local T,P,H={},{},{} -- T1,..,T3,P1,..,P9,H1,..,H6

local function int16_t(uint,nbits)
-- first negative number
  -- uint8_t (unsigned char ): 2^7
  -- uint16_t(unsigned short): 2^15
  -- uint32_t(unsigned long ): 2^31
  local first_neg=({[8]=0x80,[16]=0x8000})[nbits or 16]
  return uint-bit.band(uint,first_neg)*2
end

-- i2c helper functions
local function i2c_write(addr,...)
  if arg.n>0 then i2c.write(unpack(arg)) end
  return true
end

local function i2c_read(addr,reg,nbyte)
  local c = i2c.txrx(reg, nbyte)
  return c
end

-- sampling configuration
local init=false
local function config(...)
-- ensure module is initialized
  assert(init,('Need %s.init(...) before %s.config(...)'):format(M.name,M.name))

  local REG_COMMAND
-- sampling: normal/continous mode (M.mode:3)
  if M.mode==0x03 then
  -- Continious sampling setup (if M.mode==0x03), see DS 7.4.6.
  -- dt: sample every dt; dt=1000ms (5<<5).
  -- IIR: data=(data_new+(IIR-1)*data_old)/IIR; IIR=4 (2<<2).
  -- spi3w: enhable 3-wire SPI interface; na (0<<1).
    REG_COMMAND=0xA8 -- 5*2^5+2*2^2+0*2^1
  --REG_COMMAND=0xA0 -- 5*2^5+0*2^2+0*2^1 IIR disabled
  -- REG_CONFIG 0xF5 swriteable only in sleep mode, update only if needed
    local c = i2c_read(ADDR,0xF5,1)
    if REG_COMMAND~=c:byte() then
      i2c_write(ADDR,0xF4,        -- REG_CONTROL_MEAS,REG_CONFIG
                0x00,REG_COMMAND) -- sleep mode      ,config
    end
  end

-- oversampling: all modes
  local oss_t,oss_h,oss_p=...
-- H oversampling 2^(M.oss_h-1):
  i2c_write(ADDR,0xF2,  -- REG_CONTROL_HUM
            bit.band(oss_h or M.oss,0x07))
-- T oversampling 2^(M.oss_t-1), P oversampling 2^(M.oss_p-1),  mode M.mode
  i2c_write(ADDR,0xF4,  -- REG_CONTROL_MEAS
            bit.band(oss_t or M.oss,0x07)*32
           +bit.band(oss_p or M.oss,0x07)*4
           +bit.band(M.mode,0x03))

-- oversampling delay: forced/on-demmand mode (M.mode:1|2), see DS 11.1
  if M.mode==0x01 or M.mode==0x02 then
-- t_meas,max=1.25 [ms]+t_temp,max+t_pres,max+t_rhum,max, where
  -- t_temp,max= 2.3*2^oss_t [ms]
  -- t_pres,max= 2.3*2^oss_p + 0.575 [ms]
  -- t_rhum,max= 2.3*2^oss_h + 0.575 [ms]
-- then, t_meas,max=2.4+2.3*(2^oss_t+2^oss_h+2^oss_p) [ms]
    local WAIT=2400+2300*(bit.bit(oss_t or M.oss)
                         +bit.bit(oss_h or M.oss)
                         +bit.bit(oss_p or M.oss)) -- 9.3,..,112.8 ms
    tmr.delay(WAIT)
  end
end

-- initialize module
function M.init(...)
-- volatile module
  if volatile==true then
    _G[M.name],package.loaded[M.name]=nil,nil
  end

-- init i2c bus
  i2c.setup(0x76, 400)

-- M.init suceeded after/when read calibration coeff.
  init=(next(T)~=nil)and(next(P)~=nil)and(next(H)~=nil)

  if not init then
    local found,c
-- verify device address
    for c=1,#ADDR do
      found=i2c_write(ADDR[c])
      if found then
        ADDR=ADDR[c]
        break
      end
    end
    if M.verbose==true then
      print(found and
        ('%s: address 0x%02X.'):format(M.name,ADDR) or
        ('%s: unknown address.'):format(M.name) )
    end
-- verify device ID
    if found then
      c = i2c_read(ADDR,0xD0,1) -- REG_CHIPID
    -- CHIPID: BMP085/BMP180 0x55, BMP280 0x58, BME280 0x60
      M.model=({[0x55]='BMP180',[0x58]='BMP280',[0x60]='BME280'})[c:byte()]
      found=(M.model=='BME280')
    end
    if M.verbose==true then
      print(found and
        ('%s: model %q.'):format(M.name,M.model) or
        ('%s: unknown model.'):format(M.name))
    end
-- read calibration coeff.
    if found then
      c = i2c_read(ADDR,0x88,24) -- calib00 0x88 .. calib23 0x9F
        ..i2c_read(ADDR,0xA1, 1) -- calib25 0xA1
        ..i2c_read(ADDR,0xE1, 7) -- calib26 0xE1 .. calib32 0xE7
      if M.debug==true then
        print(('%s:'):format(M.name))
        local i
        for i=1,24 do
          print(('--calib%02d=0x%02X:c:byte(%02d)=0x%02X')
            :format(i-1,0x88+i-1,i,c:byte(i)))
        end
        i=25
          print(('--calib%02d=0x%02X:c:byte(%02d)=0x%02X')
            :format(i,0xA1,i,c:byte(i)))
        for i=26,32 do
          print(('--calib%02d=0x%02X:c:byte(%02d)=0x%02X')
            :format(i,0xE1+i-26,i,c:byte(i)))
        end
      end
    -- unpack CALIBRATION: T1,..,T3,P1,..,P9,H1,..,H7
      T[1]=        c:byte( 1)+c:byte( 2)*256  -- 0x88,0x89; unsigned short
      T[2]=int16_t(c:byte( 3)+c:byte( 4)*256) -- 0x8A,0x8B; (signed) short
      T[3]=int16_t(c:byte( 5)+c:byte( 6)*256) -- 0x8C,0x8D; (signed) short
      P[1]=        c:byte( 7)+c:byte( 8)*256  -- 0x8E,0x8F; unsigned short
      P[2]=int16_t(c:byte( 9)+c:byte(10)*256) -- 0x90,0x91; (signed) short
      P[3]=int16_t(c:byte(11)+c:byte(12)*256) -- 0x92,0x93; (signed) short
      P[4]=int16_t(c:byte(13)+c:byte(14)*256) -- 0x94,0x95; (signed) short
      P[5]=int16_t(c:byte(15)+c:byte(16)*256) -- 0x96,0x97; (signed) short
      P[6]=int16_t(c:byte(17)+c:byte(18)*256) -- 0x98,0x99; (signed) short
      P[7]=int16_t(c:byte(19)+c:byte(20)*256) -- 0x9A,0x9B; (signed) short
      P[8]=int16_t(c:byte(21)+c:byte(22)*256) -- 0x9C,0x9D; (signed) short
      P[9]=int16_t(c:byte(23)+c:byte(24)*256) -- 0x9E,0x9F; (signed) short
      H[1]=        c:byte(25)                 -- 0xA1     ; unsigned char
      H[2]=int16_t(c:byte(26)+c:byte(27)*256) -- 0xE1,0xE2; (signed) short
      H[3]=        c:byte(28)                 -- 0xE3     ; unsigned char
      H[4]=bit.band(c:byte(30),0x0F)          -- 0xE5[3:0],...
      H[4]=int16_t(H[4]+c:byte(29)*16)      --  ...,0xE4; (signed) short
      H[5]=bit.rshift(c:byte(30),4)           -- 0xE5[7:4],...
      H[5]=int16_t(H[5]+c:byte(31)*16)      --  ...,0xE6; (signed) short
      H[6]=int16_t(c:byte(32),8)              -- 0xE7     ; (signed) char
	  
	  --[[ Same usin struct.unpack
	  local i,b
	  T[1],T[2],T[3],i=struct.unpack("Hhh",c,0)
	  P[1],P[2],P[3],P[4],P[5],P[6],P[7],P[8],P[9],i=struct.unpack("Hhhhhhhhh",c,i)
	  H[1],H[2],H[3],i=struct.unpack("BhB",c,i)
      H[4]=bit.band(c:byte(30),0x0F)          -- 0xE5[3:0],...
      H[4]=int16_t(H[4]+c:byte(29)*16)      --  ...,0xE4; (signed) short
      H[5]=bit.rshift(c:byte(30),4)           -- 0xE5[7:4],...
      H[5]=int16_t(H[5]+c:byte(31)*16)      --  ...,0xE6; (signed) short
      H[6]=int16_t(c:byte(32),8)              -- 0xE7     ; (signed) char
      ]]
	  
	  c=nil
    end
    -- M.init suceeded
    init=found
  end
  if init and M.verbose==true then
    print(('%s: cal.coeff.'):format(M.name))
    print(('--T={%d,%d,%d}.'):format(unpack(T)))
    print(('--P={%d,%d,%d,%d,%d,%d,%d,%d,%d}.'):format(unpack(P)))
    print(('--H={%d,%d,%d,%d,%d,%d}.'):format(unpack(H)))
  end

-- Sampling setup
  if init then config(...) end

-- M.init suceeded after/when read calibration coeff.
  return init
end

-- read temperature, pressure and relative humidity from BME
-- oss: oversampling setting. 0..5
function M.read(...)
-- ensure module is initialized
  assert(init,('Need %s.init(...) before %s.read(...)'):format(M.name,M.name))

-- oversampling: forced/on-demmand mode (M.mode:1|2)
  if M.mode==0x01 or M.mode==0x02 then config(...) end

-- read RAW DATA
  local c = i2c_read(ADDR,0xF7,8) -- REG_PRESSURE_MSB 0xF7 .. REG_HUMIDITY_LSB 0xFE
  local p,t,h                                 -- uncompensated
  p=c:byte(1)*4096+c:byte(2)*16+c:byte(3)/16  --   pressure
  t=c:byte(4)*4096+c:byte(5)*16+c:byte(6)/16  --   temperature
  h=c:byte(7)* 256+c:byte(8)                  --   humidity
  c=nil
  if M.verbose==true then
    print(('%s: UP=%d,UT=%d,UH=%d.'):format(M.name,p,t,h))
  end

--[[ Calculate actual temperature from uncompensated temperature.
     Returns the value in degree Centigrade (DegC). ]]
  local var1,var2,var3,tfine,fp,fh
  var1 = (t / 16384.0 - T[1] / 1024.0) * T[2]
  var2 = ((t / 131072.0 - T[1] / 8192.0) * (t / 131072.0 - T[1] / 8192.0)) * T[3]
  tfine = var1 + var2
  t = tfine / 5120.0
  if M.verbose==true then
    print(('%s: tfine=%d.'):format(M.name,tfine))
  end

--[[ Calculate actual pressure from uncompensated pressure.
     Returns the value in hect Pascal (hPa). ]]
  var1 = tfine / 2.0 - 64000.0
  var2 = var1 * var1 * P[6] / 32768.0
  var2 = var2 + var1 * P[5] * 2.0
  var2 = var2 / 4.0 + P[4] * 65536.0
  var1 = (P[3] * var1 * var1 / 524288.0 + P[2] * var1) / 524288.0
  var1 = (1.0 + var1 / 32768.0) * P[1]
  if var1 == 0 then
    p=nil
  else
    p = 1048576.0 - p
    p = ((p - var2 / 4096.0) * 6250.0) / var1
    var1 = P[9] * p * p / 2147483648.0
    var2 = p * P[8] / 32768.0
    p = p + (var1 + var2 + P[7]) / 16.0
  end  

--[[ Calculte actual humidity from uncompensated humidity.
     Returns the value in %rH. ]]
  fh = tfine - 76800.0
  fh = (h - (H[4] * 64.0 + H[5] / 16384.8 * fh)) * (H[2] / 65536.0 * (1.0 + H[6] / 67108864.0 * fh * (1.0 + H[3] / 67108864.0 * fh)))
  fh = fh * (1.0 - H[1] * fh / 524288.0)
  if fh > 100 then
    fh = 100
  end
  if fh < 0 then
    fh = 0
  end
  h = fh

-- expose results
  M.temperature=t -- float value of temperature [C]
  M.pressure   =p -- float value of preassure   [hPa]
  M.humidity   =h -- float value of rel.humidity[%]
  if M.verbose==true then
    print(('%s: p=%1.1f [Pa], t=%1.2f [C], h=%1.1f [%%].'):format(M.name,p,t,h))
  end
end

return M

