--Copyright (C) 2019 mleaf <mleaf90@gmail.com>
--微信公众号【WiFi物联网】
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local Status,subTopic,pubTopic
local m,s
local e,mwol
local encryption
local sys = require "luci.sys"

local mac=luci.sys.exec("uci -q get mwol.mwol_conf.id")

m=Map("mwol",translate("Mqtt wake on lan"),translate("请关注微信公众号【WiFi物联网】获取更多信息: \
<a href=\"http://www.mleaf.org/downloads/logo/201911171573997716658567.jpg\" target=\"_blank\">【WiFi物联网】</a>"))

if SYS.call("pidof mwol >/dev/null") == 0 then
	Status = translate("<strong><font color=\"green\">Mwol正在运行...</font></strong>")
else
	Status = translate("<strong><font color=\"red\">Mwol未运行...</font></strong>")
end

subTopic=string.format("<strong><font color=\"red\">Mwol订阅消息主题: /mwol/%s</font></strong>", mac)
pubTopic=string.format("<strong><font color=\"red\">Mwol发布消息主题: /mwol/%s</font></strong>", mac)

--
-- LoRa Gateway config for gateway_conf
--
mwol=m:section(TypedSection,"mwol","")
--mwol.addremove=false
mwol.anonymous=true
mwol.description = translate(string.format("%s<br /><br />%s<br><br />%s", Status, subTopic, pubTopic))

mwol:tab("general",  translate("General Settings"))
mwol:tab("encryptionSettings",  translate("Encryption Settings"))
mwol:tab("ssl",  translate("SSL Settings"))

--
-- MQTT general 
--
e = mwol:taboption("general", Flag, "enable")
e.title = translate("Enable")
e.default = 0
e.rmempty = false

e = mwol:taboption("general", Value, "hostname")
e.title = translate("MQTT Server")
e = mwol:taboption("general", Value, "port")
e.title = translate("MQTT Port")

host = mwol:taboption("general", Value, "presetmac", translate("Host to wake up"),
	translate("Choose the host to wake up or enter a custom MAC address to use"))
host.title = translate("Preset Mac")
sys.net.mac_hints(function(mac, name)
	host:value(mac, "%s (%s)" %{ mac, name })
end)

e = mwol:taboption("general", Value, "id")
e.default = mac
e.rmempty = false;
e.title = translate("MQTT Topic Id")

--
-- MQTT Using encryption 
--
encryption_enable = mwol:taboption("encryptionSettings", ListValue,"encryptionenable",translate("Enable"))
encryption_enable.optional = false;
encryption_enable.rmempty = false;
encryption_enable.default = 0
encryption_enable.datatype = "uinteger"
encryption_enable:value(1, translate("True"))
encryption_enable:value(0, translate("False"))

--
-- MQTT username
--
encryption_username = mwol:taboption("encryptionSettings", Value,"username",translate("MQTT UserName"))
encryption_username.optional = true;
encryption_username.rmempty = true;
encryption_username.default = "mleaf"
encryption_username.datatype = "string"
encryption_username:depends("encryptionenable", "1")

--
-- MQTT password
--
encryption_password = mwol:taboption("encryptionSettings", Value,"password",translate("MQTT Password"))
encryption_password.optional = true;
encryption_password.rmempty = true;
encryption_password.default = "www.mleaf.org"
encryption_password.datatype = "string"
encryption_password:depends("encryptionenable", "1")

--
-- MQTT Using SSL 
--
ssl_enable = mwol:taboption("ssl", ListValue,"sslenable",translate("Enable"))
ssl_enable.optional = false;
ssl_enable.rmempty = false;
ssl_enable.default = 0
ssl_enable.datatype = "uinteger"
ssl_enable:value(1, translate("True"))
ssl_enable:value(0, translate("False"))

--
-- MQTT cafile
--
cafile = mwol:taboption("ssl", Value,"cafile",translate("ca"),
	translate("Edit the ca file path."))
cafile.optional = true;
cafile.rmempty = true;
cafile.default = false
cafile.datatype = "string"
cafile:depends("sslenable", "1")

--
-- MQTT crtfile
--
crtfile = mwol:taboption("ssl", Value,"crtfile",translate("crt"),
	translate("Edit the crt file path."))
crtfile.optional = true;
crtfile.rmempty = true;
crtfile.default = false
crtfile.datatype = "string"
crtfile:depends("sslenable", "1")

--
-- MQTT keyfile
--
keyfile = mwol:taboption("ssl", Value,"keyfile",translate("key"),
	translate("Edit the key file path."))
keyfile.optional = true;
keyfile.rmempty = true;
keyfile.default = false
keyfile.datatype = "string"
keyfile:depends("sslenable", "1")

return m
