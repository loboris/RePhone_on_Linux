function dummy_h(hdr)
  print("HTTPS header")
end

https.on("header", dummy_h)

function dummy_r(state, data, len, more)
  print("HTTPS response: "..state)
end

https.on("response", dummy_r)

url_idx = 1

my_urls = {
    "http://api.thingspeak.com/update?key=xxx&field1=8.0",
    "http://api.thingspeak.com/update?key=xxx&field1=9.0",
    "http://api.thingspeak.com/update?key=xxx&field1=10.0"
}

-- use timer cb function to initiate new request
function tmr_cb(ht)
    if (url_idx <= 3) and (https.getstate() == 0) then
        print("Http GET request "..url_idx)
        print(https.get(my_urls[url_idx]))
        url_idx = url_idx + 1
    end
end

-- create and start timer
https_tmr = timer.create(500, tmr_cb)

