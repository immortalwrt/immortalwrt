m = Map("oled", translate("OLED"), translate("A LuCI app that helps you config your oled display (SSD1306, 0.91', 128X32) with screensavers! <br /> <br /> Any issues, please go to: ")..[[<a href="https://github.com/natelol/luci-app-oled" target="_blank">luci-app-oled</a>]])

--m.chain("luci")

m:section(SimpleSection).template="oled/status"

s = m:section(TypedSection, "oled", translate(""))
s.anonymous=true
s.addremove=false

--OPTIONS
s:tab("info", translate("Info Display"))
s:tab("screensaver", translate("screensaver"))

o = s:taboption("info", Flag, "enable", translate("Enable"))
o.default=0
o = s:taboption("info", Flag, "autoswitch", translate("Enable Auto switch"))
o.default=0
from = s:taboption("info", ListValue, "from", translate("From"))
to = s:taboption("info", ListValue, "to", translate("To"))
for i=0,23 do
	for j=0,30,30 do
		from:value(i*60+j,string.format("%02d:%02d",i,j))
		to:value(i*60+j,string.format("%02d:%02d",i,j))
	end
end
from:value(1440,"24:00")
to:value(1440,"24:00")
from:depends("autoswitch",'1')
to:depends("autoswitch",'1')
from.default=0
to.default=1440

--informtion  options----
o = s:taboption("info", Flag, "date", translate("Date"), translate('Format YYYY-MM-DD HH:MM:SS'))
o.default=0
o = s:taboption("info", Flag, "lanip", translate("IP"), translate("LAN IP address"))
o.default=0
o = s:taboption("info", Flag, "cputemp", translate("CPU temperature"))
o.default=0
o = s:taboption("info", Flag, "cpufreq", translate("CPU frequency"))
o.default=0
o = s:taboption("info", Flag, "netspeed", translate("Network speed"), translate("1Mbps(m/s)=1,000Kbps(k/s)=1,000,000bps(b/s)"))
o.default=0
o = s:taboption("info", ListValue, "netsource", translate("which eth to monitor"))                       
o:value("eth0","eth0")                                                                                   
o:value("eth1","eth1")                                                                                   
o:depends("netspeed",'1')
o.default='eth0'
o = s:taboption("info", Value, "time", translate("Display interval(s)"), translate('Screensaver will activate in set seconds'))
o.default=0

--screensaver options--
o = s:taboption("screensaver", Flag, "scroll", translate("Scroll Text"))                                 
o.default=1                                                                                              
o = s:taboption("screensaver", Value, "text", translate("Text you want to scroll"))                      
o:depends("scroll",'1')                                                                                 
o.default='OPENWRT' 
o = s:taboption("screensaver", Flag, "drawline", translate("Draw Many Lines"))
o.default=0
o = s:taboption("screensaver", Flag, "drawrect", translate("Draw Rectangles"))
o.default=0
o = s:taboption("screensaver", Flag, "fillrect", translate("Draw Multiple Rectangles"))
o.default=0
o = s:taboption("screensaver", Flag, "drawcircle", translate("Draw Multiple Circles"))
o.default=0
o = s:taboption("screensaver", Flag, "drawroundrect", translate("Draw a white circle, 10 pixel radius"))
o.default=0
o = s:taboption("screensaver", Flag, "fillroundrect", translate("Fill the Round Rectangles"))
o.default=0
o = s:taboption("screensaver", Flag, "drawtriangle", translate("Draw Triangles"))
o.default=0
o = s:taboption("screensaver", Flag, "filltriangle", translate("Fill Triangles"))
o.default=0
o = s:taboption("screensaver", Flag, "displaybitmap", translate("Display miniature bitmap"))
o.default=0
o = s:taboption("screensaver", Flag, "displayinvertnormal", translate("Invert Display Normalize it"))
o.default=0
o = s:taboption("screensaver", Flag, "drawbitmapeg", translate("Draw a bitmap and animate"))
o.default=0

return m

















---
