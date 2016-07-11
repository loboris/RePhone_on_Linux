print('Hello, lua & rephone')

net.setapn({apn="web.htgprs"})

msg = "RePhone\n"
starttime=os.date("*t")
smsnum = "0992382166"
eml = {
    host="smtp.gmail.com",
    port=465,
    user="loboris@gmail.com",
    pass="1412LoBo956330009",
    to="loboris@gmail.com",
    from="loboris@gmail.com",
    from_name="rephone",
    subject="isvjestaj",
    msg="test" }

nled = 1
ledcolor = 0xff0000
ledbgcolor = 0x080808
tled = {0,0,0,0,0,0,0,0}
leddir = 0

   
-- set wake up interval 10 minutes
sys.wkupint(10)

-- Measure and report result
function measure()
    nled = 0
    gpio.ws2812(13,0x400000,8)
    
    starttime=os.date("*t")
    local st=os.date()
    msg = msg..st
    
    -- Get temperature
    local ts = 0
    if sensor.ds_init(18) == 1 then
        ts = sensor.ds_search()
    end
    local temp = -9999
    local stat = -1
    if ts > 0 then
        sensor.ds_setres(ts, 10)
        temp, stat = sensor.ds_gettemp(ts)
    end
    if stat == 0 then
        msg = msg.."\nT = "..string.format("%1.2f C", temp)
    end
    
    -- Get battery voltage
    local res = gpio.adc_config(0)
    if res == 0 then
        local vbat = gpio.adc_start(0)
        local vstr = string.format("%1.3f V", vbat)
        msg = msg.."\nVbat="..vstr
    else
        msg = msg.."\nVbat=error"
    end
    
    -- Send SMS at full hour + 10 min
    if starttime["min"] == 10 then
        if (starttime["hour"] % 4) == 0 then
            gpio.ws2812(13,0x004000)
            if sys.usb() == 1 then
                print("Send SMS")
            end
            gsm.sms_send(smsnum, msg)
        end
    end
    
    -- Send email at full hour + 40 min
    if starttime["min"] == 40 then
        gpio.ws2812(13,0x000040)
        if sys.usb() == 1 then
            print("Send email")
        end
        eml["msg"] = "Satni izvjestaj:\r\n"..msg
        res = email.send(eml)
    end
    nled = 1
end

-- On shutdown cb function
function shdown_cb()
    timer.delete(ledtimer)
    gpio.ws2812(13,0,8)
    sys.shutdown()
end

-- SNTP callback -----
function sntp_cb(stat)
    sys.showlog(LOG_NONE)
    msg = "RePhone\n"
    
    if stat ~= 0 then
        msg = msg.."SNTP error\n"
    end

    measure()
    
    if sys.usb() == 0 then
        -- if usb is not connected, shutdown
        --sys.schedule(0)
        ledcolor = 0x000040
        ledbgcolor = 0x040404
        sys.noacttime(15)  -- shut down after 15 seconds
        sys.onshutdown(shdown_cb)
    else
        print(msg)
        sys.showlog(LOG_ALL)
    end
end


-- Alarm callback -----
function alarm_cb(stat)
    msg = "RePhone Alarm\n"
    
    measure()
    
    if sys.usb() == 1 then
        print(msg)
    end
    
end

-- NeoPixel test
function led_test()
    if nled == 0 then
        return
    end
    local n
    for n=1,8,1 do
        if n == nled then
            tled[n] = ledcolor
        else
            tled[n] = ledbgcolor
        end
    end
    if (leddir % 2) == 0 then
        nled = nled + 1
        if nled > 8 then
            nled = 8
            leddir = leddir + 1
        end
    else
        nled = nled - 1
        if nled < 1 then
            nled = 1
            leddir = leddir + 1
        end
    end
    gpio.ws2812(13,tled)
end

-- start bluetooth
-- bt.start("RePhone")
-- bt.spp_start()

--[[
function led_tmr(n)
    gpio.toggle(13)
end

function test_tmr(n)
    print(n,gpio.adc_start(0))
end
]]--

--led_t=timer.create(500,led_tmr)
--test_t=timer.create(1500,test_tmr)

-- get time from ntp server
sim_stat = gsm.sim_info()
if sim_stat == 1 then
    net.ntptime(2, sntp_cb)
else
    if sys.usb() == 0 then
        -- if usb is not connected, shutdown
        -- sys.schedule(0)
        sys.noacttime(15)  -- shut down after 15 seconds
        sys.onshutdown(shdown_cb)
    else
        print ("No SIM!: "..sim_stat)
    end
end

sys.onalarm(alarm_cb)
ledtimer = timer.create(50, led_test)
