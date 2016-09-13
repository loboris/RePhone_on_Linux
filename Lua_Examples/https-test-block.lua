-- no response cb function should be registered
https.on("response")

print("Http GET request 1")
print(https.get("http://api.thingspeak.com/update?key=xxx&field1=8.0"))

print("Http GET request 2")
print(https.get("http://api.thingspeak.com/update?key=xxx&field1=9.0"))

print("Http GET request 3")
print(https.get("http://api.thingspeak.com/update?key=xxx&field1=10.0"))

