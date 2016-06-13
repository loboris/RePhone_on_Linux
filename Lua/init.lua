print('Hello, lua & rephone')

gprs.setapn({apn="web.htgprs"})

msg = "RePhone\n"
starttime=os.date("*t")
smsnum = "0992382166"

-- set wake up interval 10 minutes
sys.wkupint(10)


-- SNTP callback--    
function sntp_cb(stat)
    if stat ~= 0 then
        msg = msg.."SNTP error\n"
    end
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
    if starttime["min"] == 20 then
        print("Send SMS")
        gsm.sms_send(smsnum, msg)
    end
    
    if sys.usb() == 0 then
        -- if usb is not connected, shutdown
        sys.schedule(0)
    else
        print(msg)
    end
    
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
    sys.ntptime(2, sntp_cb)
else
    if sys.usb() == 0 then
        -- if usb is not connected, shutdown
        sys.schedule(0)
    else
        print ("No SIM!: "..sim_stat)
    end
end

