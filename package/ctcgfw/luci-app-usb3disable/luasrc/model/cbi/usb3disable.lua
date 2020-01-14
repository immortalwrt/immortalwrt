local sys=require("luci.sys")
local m,s,o,d,e,p
m=Map("usb3disable","usb3disable")

s=m:section(TypedSection,"usb3disable")
s.anonymous=true
s.addremove=false
e=sys.exec("cat /etc/modules.d/$(ls /etc/modules.d | grep usb3) >/tmp/run/usb3module ; lsmod | awk '{gsub(\"_\",\"-\",$1);print $1}' >>/tmp/run/usb3module ; sort /tmp/run/usb3module | uniq -d | sort -n ; rm /tmp/run/usb3module")
if e=="" then
	o=s:option(Button,"enabled",translate("enable now"))
	o.optional=false
	o.inputtitle=translate("enable")
	o.write=function()
		sys.exec("cat /etc/modules.d/$(ls /etc/modules.d | grep usb3) | awk '{print \"insmod \"$0}' >/var/run/usb3module ; sh /var/run/usb3module; rm /var/run/usb3module")
		luci.http.redirect(luci.dispatcher.build_url("admin","system","usb3disable"))
	end
else
	o=s:option(Button,"disabled",translate("disable now"))
	o.optional=false
	o.inputtitle=translate("Disable")
	o.write=function()
		sys.exec("cat /etc/modules.d/$(ls /etc/modules.d | grep usb3) | sed -n '1!G;h;$p' | awk '{print \"rmmod \"$0}' >/var/run/usb3module ; sh /var/run/usb3module; rm /var/run/usb3module")
		luci.http.redirect(luci.dispatcher.build_url("admin","system","usb3disable"))
	end
end
d=sys.exec("ls /etc/modules-boot.d | grep usb3")
if d=="" then
	o=s:option(Button,"enabledstartup",translate("enable on boot"))
	o.optional=false
	o.inputtitle=translate("Enable")
	o.write=function()
		sys.exec("ln -s /etc/modules.d/*usb3 /etc/modules-boot.d/")
		luci.http.redirect(luci.dispatcher.build_url("admin","system","usb3disable"))
	end
else
o=s:option(Button,"disabledstartup",translate("disable on boot"))
	o.optional=false
	o.inputtitle=translate("Disable")
	o.write=function()
		sys.exec("rm /etc/modules-boot.d/*-usb3")
		luci.http.redirect(luci.dispatcher.build_url("admin","system","usb3disable"))
	end
	
end
return m