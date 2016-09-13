--[[
  init.lua runs on system startup before Lua shell is executed
--]]

-- === SET YOUR CORRECT APN FOR GPRS OPERATIONS ===
net.setapn({apn="my.apn"})
-- ================================================

-- === CHANGE THIS TO YOUR GSM NUMBER !! ===
smsnum = "123456789"
-- =========================================

-- === Set you time zone (offset from GMT) ===
tmzn = 2
-- ===========================================


--[[ SNTP callback -------------------------------------
We will execute GSM related function in sntp callcack
to allow the module to attach to network and be ready
for sms operations
--]]
function sntp_cb(stat)
	print()
	print("==================================================")
    if stat ~= 0 then
        print("SNTP error\nCheck if your APN is set")
    else
        print("Time set from NTP server: "..os.date())
    end
    print("==================================================")
    
    if sim_stat == 1 then
        -- Check total number of messages
        print("SIM card inserted, IMEI="..imei)
        local nsms = sms.numrec()
        print("You have "..nsms.." messages")
        if nsms > 0 then
            --  get list of unread messages
            local smsidx = sms.list(1)
            local smsfrom, smstime, smsmsg
            print("Unread messages: "..#smsidx)
            if #smsidx > 0 then
                print("Reading 1st message:")
                smsfrom, smstime, smsmsg = sms.read(smsidx[1])
                print("----------")
                print("sms from: "..smsfrom)
                print("received: "..smstime)
                print("----------")
                print(smsmsg)
                print("----------")
                print()
            end
        end
        
        -- Send message
        print("Sending sms to "..smsnum.." ...")
        local smsres = sms.send(smsnum, "Hello from Rephone")
        if smsres ~= 1 then
            print("SMS send failed")
        else
            print("SMS sent")
        end
    else
        print("SIM card not inserted, or not working!")
    end
    -- enable all debug messages
    sys.showlog(LOG_ALL)
end
-- ----------------------------------------------------

print('=====================================================')
print('Hello, this is Lua running on Xadow GSM+BLE (RePhone)')
print('=====================================================')
print()

-- disable debug messages
sys.showlog(LOG_NONE)

-- Get system status
sim_stat,imei,_ = sms.siminfo()
tm = os.date("*t")
version,_,_ = sys.ver()
memused, memtot, heap = sys.mem()

res = gpio.adc_config(0)
if res == 0 then
    vbat = gpio.adc_start(0)
    vstr = string.format("%1.3f V", vbat)
else
    vstr = "adc error"
end

if tm["year"] < 2016 then
    print("Your RTC time is set in the past !\nYou can use ntptime function to set the correct time.\n")
else
    print("Current time is: "..tm.year.."/"..tm.month.."/"..tm.day.." "..tm.hour..":"..tm.min..":"..tm.sec)
    -- or you can directly format in os.date function
    -- print(os.date("Current time is %Y/%m/%d %H:%M:%S"))
end

print()
print("Status:")
print("   Lua version: "..version)
print("   Used memory: "..memused)
print("  Total memory: "..memtot)
print(" C heap memory: "..heap)
print("Battery voltage: "..vstr)
print()
print("After sntp result is ready, the rest of the code will be executed.")
print()

-- request time from ntp server
net.ntptime(tmzn, sntp_cb)
