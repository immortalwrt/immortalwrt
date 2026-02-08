m = Map("athena_led", translate("Athena led Ctrl"), translate("JDCloud AX6600 LED Screen Ctrl"))

m:section(SimpleSection).template = "athena_led/athena_led_status"

s = m:section(TypedSection, "athena_led")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enable", translate("Enabled"))
enable.rmempty = false

seconds = s:option(ListValue, "seconds", translate("Display interval time"))
seconds.default = "5"
seconds.rmempty = false
seconds:value("1")
seconds:value("2")
seconds:value("3")
seconds:value("4")
seconds:value("5")
seconds.description = translate("Enable carousel display and set interval time in seconds")

seconds = s:option(ListValue, "lightLevel", translate("Display light level"))
seconds.default = "5"
seconds.rmempty = false
seconds:value("0")
seconds:value("1")
seconds:value("2")
seconds:value("3")
seconds:value("4")
seconds:value("5")
seconds:value("6")
seconds:value("7")
seconds.description = translate("Display light level desc")

status = s:option(MultiValue, "status", translate("side led status"))
status.default = ""
status.rmempty = true
status:value("time", translate("status time"))
status:value("medal", translate("status medal"))
status:value("upload", translate("status upload"))
status:value("download", translate("status download"))
status.description = translate("side led status desc")

type = s:option(MultiValue, "option", translate("Display Type"))
type.default = "date timeBlink"
type.rmempty = false
type:value("date", translate("Display Type Date"))
type:value("time", translate("Display Type Time"))
type:value("timeBlink", translate("Display Type Time Blink"))
type:value("temp", translate("Display Type temp"))
type:value("string", translate("Display Type String"))
type:value("getByUrl", translate("Display Type getByUrl"))
type.description = translate("Specify comma-separated values for option")

customValue = s:option(Value, "value", translate("Custom Value"))
customValue.default = "abcdefghijklmnopqrstuvwxyz0123456789+-*/=.:：℃"
customValue.rmempty = false
customValue.placeholder = translate("Enter your message here")
customValue.description = translate("Set the custom message to display on the LED screen, Only effective on 'Display Type String'")

url = s:option(Value, "url", translate("api url for get content"))
url.default = "http://www.baidu.com"
url.rmempty = false
url.placeholder = translate("Enter your api url here")
url.description = translate("api url for get content des")

tempFlag = s:option(MultiValue, "tempFlag", translate("tempFlag"))
tempFlag.default = "4"
tempFlag.rmempty = false
tempFlag:value("0", translate("nss-top"))
tempFlag:value("1", translate("nss"))
tempFlag:value("2", translate("wcss-phya0"))
tempFlag:value("3", translate("wcss-phya1"))
tempFlag:value("4", translate("cpu"))
tempFlag:value("5", translate("lpass"))
tempFlag:value("6", translate("ddrss"))
tempFlag.description = translate("Set the item display on the LED screen, Only effective on 'Display Type temp'")

function m.on_after_commit(self)
    local output = luci.util.exec("/etc/init.d/athena_led reload >/dev/null 2>&1")
    luci.util.exec("logger '" .. output .. "'")
end

return m
