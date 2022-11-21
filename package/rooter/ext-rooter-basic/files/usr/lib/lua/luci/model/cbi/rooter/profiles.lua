local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local sys   = require "luci.sys"
local fs = require "nixio.fs" 							  

local maxmodem = luci.model.uci.cursor():get("modem", "general", "max")  
local profsave = luci.model.uci.cursor():get("custom", "profile", "save")  
if profsave == nil then
	profsave ="0"
end
local multilock = luci.model.uci.cursor():get("custom", "multiuser", "multi") or "0"
local rootlock = luci.model.uci.cursor():get("custom", "multiuser", "root") or "0"

m = Map("profile", translate("模块DNS / APN 配置"),translate(""))									 

m.on_after_commit = function(self)
	if profsave == "1" then
		luci.sys.call("/usr/lib/profile/restart.sh &")
	end
end

if profsave == "1" then
	m:section(SimpleSection).template = "rooter/profile"
	ds = m:section(TypedSection, "simpin", translate("SIM卡Pin码  :"), translate("如果配置文件钟没有SIM PIN码则使用"))
	ds.anonymous = true
	
	ms = ds:option(Value, "pin", translate("PIN :")); 
	ms.rmempty = true;
	ms.default = ""
end


-- 
-- Default profile
--

di = m:section(TypedSection, "default", translate("默认"), translate("一般只需要填写您运营商的APN即可，一般已经自动加载识别了，奇葩虚拟运营商你也可以继续填 Tips:模块下次重连后生效，如果你要现在生效，请保存后重启模块）## 华为天际通5G SA APN：5gscuiot ##"))
di.anonymous = true
di:tab("default", translate("通用"))
di:tab("advance", translate("高级"))
di:tab("connect", translate("连接状态监控"))
if (multilock == "0") or (multilock == "1" and rootlock == "1") then
	di:tab("bwidth", translate("Bandwidth Reporting频宽报告"))
end

this_tab = "default"

ma = di:taboption(this_tab, Value, "apn", "APN :"); 
ma.rmempty = true;
ma.default = "broadband"

tt = di:taboption(this_tab, ListValue, "ttl", translate("设定TTL值 :"))
tt:value("0", translate("使用当前值"))
tt:value("1", translate("不配置TTL值"))
tt:value("63", "TTL 63")
tt:value("64", "TTL 64")
tt:value("65", "TTL 65")
tt:value("66", "TTL 66")
tt:value("67", "TTL 67")
tt:value("117", "TTL 117")
tt:value("TTL-INC 1", "TTL-INC 1")
tt.default = "0"

ynl = di:taboption(this_tab, ListValue, "hostless", translate("设定通信模块TTL"));
ynl:value("0", "否")
ynl:value("1", translate("是"))
ynl.default=0

pt = di:taboption(this_tab, ListValue, "pdptype", translate("IP协议类型 :"))
pt:value("IP", "IPv4")
pt:value("IPV6", "IPv6")
pt:value("IPV4V6", "IPv4+IPv6")
pt:value("0", "默认")
pt.default = "0"

cmcc = di:taboption(this_tab, Value, "context", translate("PDP Context for APN :"));
cmcc.optional=false; 
cmcc.rmempty = true;
cmcc.datatype = "and(uinteger,min(1),max(10))"
cmcc.default = "1"

mu = di:taboption(this_tab, Value, "user", translate("用户名 :")); 
mu.optional=false; 
mu.rmempty = true;

mp = di:taboption(this_tab, Value, "passw", translate("密码 :")); 
mp.optional=false; 
mp.rmempty = true;
mp.password = true

mpi = di:taboption(this_tab, Value, "pincode", translate("PIN码 :")); 
mpi.optional=false; 
mpi.rmempty = true;

mau = di:taboption(this_tab, ListValue, "auth", translate("身份验证类型 :"))
mau:value("0", "None")
mau:value("1", "PAP")
mau:value("2", "CHAP")
mau.default = "0"

mtz = di:taboption(this_tab, ListValue, "tzone", translate("自动设置时区"), translate("Tips:就是模块通网后会自动设置时区"));
mtz:value("0", "窝不要")
mtz:value("1", translate("窝要"))
mtz.default=1

ml = di:taboption(this_tab, ListValue, "lock", translate("阻止国际漫游选项并锁定特定运营商 :"));
ml:value("0", translate("否"))
ml:value("1", translate("强硬"))
ml:value("2", translate("软弱"))
ml.default=0

mcc = di:taboption(this_tab, Value, "mcc", translate("MCC :"));
mcc.optional=false; 
mcc.rmempty = true;
mcc.datatype = "and(uinteger,min(1),max(999))"
mcc:depends("lock", "1")
mcc:depends("lock", "2")

mnc = di:taboption(this_tab, Value, "mnc", translate("MNC :"));
mnc.optional=false; 
mnc.rmempty = true;
mnc.datatype = "and(uinteger,min(1),max(999))"
mnc:depends("lock", "1")
mnc:depends("lock", "2")

this_taba = "advance"

mf = di:taboption(this_taba, ListValue, "ppp", translate("不使用常规协议，强制使用PPP拨号协议 :"));
mf:value("0", translate("启用"))
mf:value("1", translate("禁用"))
mf.default=0

md = di:taboption(this_taba, Value, "delay", translate("连接延时(秒为单位) :")); 
md.optional=false; 
md.rmempty = false;
md.default = 5
md.datatype = "and(uinteger,min(5))"

nl = di:taboption(this_taba, ListValue, "nodhcp", translate("强制QMI协议不使用模块的DHCP，从其他位置获取 :"));
nl:value("0", translate("禁用"))
nl:value("1", translate("启用"))
nl.default=0

mdns1 = di:taboption(this_taba, Value, "dns1", translate("自定义DNS 1 :")); 
mdns1.rmempty = true;
mdns1.optional=false;
mdns1.datatype = "ipaddr"

mdns2 = di:taboption(this_taba, Value, "dns2", translate("自定义DNS 2 :")); 
mdns2.rmempty = true;
mdns2.optional=false;
mdns2.datatype = "ipaddr"

mdns3 = di:taboption(this_taba, Value, "dns3", translate("自定义DNS 3 :")); 
mdns3.rmempty = true;
mdns3.optional=false;
mdns3.datatype = "ipaddr"

mdns4 = di:taboption(this_taba, Value, "dns4", translate("自定义DNS 4 :")); 
mdns4.rmempty = true;
mdns4.optional=false;
mdns4.datatype = "ipaddr"


mlog = di:taboption(this_taba, ListValue, "log", translate("启用AT连接日志 :"));
mlog:value("0", translate("禁用"))
mlog:value("1", translate("是"))
mlog.default=0

if nixio.fs.access("/etc/config/mwan3") then
	mlb = di:taboption(this_taba, ListValue, "lb", translate("在多个5G模块中开启流量负载均衡 :"));
	mlb:value("0", translate("禁用"))
	mlb:value("1", translate("启用"))
	mlb.default=0
end

mtu = di:taboption(this_taba, Value, "mtu", translate("自定义MTU值 :"),
		translate("不建议乱整，只能自定义1420到1500的范围的值"));
mtu.optional=true
mtu.rmempty = true
mtu.default = "1500"
mtu.datatype = "range(1420, 1500)"

mat = di:taboption(this_taba, ListValue, "at", translate("连接时发送自定义AT命令 :"));
mat:value("0", translate("禁用"))
mat:value("1", translate("启用"))
mat.default=0

matc = di:taboption(this_taba, Value, "atc", translate("要执行的AT命令 :"));
matc.optional=false;
matc.rmempty = true;

--
-- Default Connection Monitoring
--

this_tab = "connect"

alive = di:taboption(this_tab, ListValue, "alive", translate("连接状态监控 :")); 
alive.rmempty = true;
alive:value("0", translate("禁用"))
alive:value("1", translate("启用掉线写入系统日志"))
alive:value("2", translate("启用 5G模块掉线 则 路由自动重启"))
alive:value("3", translate("启用 5G模块掉线 则 自动重拨"))
alive:value("4", translate("启用 5G模块掉线 则 重新上电模块，需要路由GPIO支持，否则将使用其他方式重连"))
alive.default=0

reliability = di:taboption(this_tab, Value, "reliability", translate("监测网络稳定性  :"),
		translate("范围: 1-100，必须Ping通这些IP地址，这条通信链路才会被视为联网状态"))
reliability.datatype = "range(1, 100)"
reliability.default = "1"
reliability:depends("alive", "1")
reliability:depends("alive", "2")
reliability:depends("alive", "3")
reliability:depends("alive", "4")

count = di:taboption(this_tab, ListValue, "count", translate("监测次数（Ping） :"))
count.default = "1"
count:value("1")
count:value("2")
count:value("3")
count:value("4")
count:value("5")
count:depends("alive", "1")
count:depends("alive", "2")
count:depends("alive", "3")
count:depends("alive", "4")

interval = di:taboption(this_tab, ListValue, "pingtime", translate("监测时间间隔  (Ping)  :"),
		translate("监测Ping之间间隔时间的值"))
interval.default = "10"
interval:value("5", translate("5 秒/次"))
interval:value("10", translate("10 秒/次"))
interval:value("20", translate("20 秒/次 推荐"))
interval:value("30", translate("30 秒次"))
interval:value("60", translate("1 分钟/次 "))
interval:value("300", translate("5 分钟/次"))
interval:value("600", translate("10 分钟/次"))
interval:value("900", translate("15 分钟/次"))
interval:value("1800", translate("30 分钟/次"))
interval:value("3600", translate("1 小时/次"))
interval:depends("alive", "1")
interval:depends("alive", "2")
interval:depends("alive", "3")
interval:depends("alive", "4")

timeout = di:taboption(this_tab, ListValue, "pingwait", translate("Ping 超时  :"))
timeout.default = "2"
timeout:value("1", translate("1 秒"))
timeout:value("2", translate("2 秒"))
timeout:value("3", translate("3 秒"))
timeout:value("4", translate("4 秒"))
timeout:value("5", translate("5 秒"))
timeout:value("6", translate("6 秒"))
timeout:value("7", translate("7 秒"))
timeout:value("8", translate("8 秒"))
timeout:value("9", translate("9 秒"))
timeout:value("10", translate("10 秒"))
timeout:depends("alive", "1")
timeout:depends("alive", "2")
timeout:depends("alive", "3")
timeout:depends("alive", "4")

packetsize = di:taboption(this_tab, Value, "packetsize", translate("Ping报文大小(字节为单位)  :"),
		translate("范围值 :4-56，ping报文发送的数据字节数，可以要根据运营商进行调整"))
	packetsize.datatype = "range(4, 56)"
	packetsize.default = "56"
	packetsize:depends("alive", "1")
	packetsize:depends("alive", "2")
	packetsize:depends("alive", "3")
	packetsize:depends("alive", "4")

down = di:taboption(this_tab, ListValue, "down", translate("链路状态 异常判定次数  :"),
		translate("如果达到设定值次数，Ping依旧失败后，接口就会被当做异常不在线"))
down.default = "3"
down:value("1")
down:value("2")
down:value("3")
down:value("4")
down:value("5")
down:value("6")
down:value("7")
down:value("8")
down:value("9")
down:value("10")
down:depends("alive", "1")
down:depends("alive", "2")
down:depends("alive", "3")
down:depends("alive", "4")

up = di:taboption(this_tab, ListValue, "up", translate("链路状态 正常判定次数  :"),
		translate("Tips:当达到设定值次数Ping监测后，网络都能Ping设定地址后，则链路状态会被视为正常啦~~"))
up.default = "3"
up:value("1")
up:value("2")
up:value("3")
up:value("4")
up:value("5")
up:value("6")
up:value("7")
up:value("8")
up:value("9")
up:value("10")
up:depends("alive", "1")
up:depends("alive", "2")
up:depends("alive", "3")
up:depends("alive", "4")

cb2 = di:taboption(this_tab, DynamicList, "trackip", translate("追踪 IP :"),
		translate("这个IP将被拿来做链路状态检测用，简单说就是Ping它，能通那么就算链路正常，不通那就以你刚配合的方法处理"))
cb2.datatype = "ipaddr"
cb2:depends("alive", "1")
cb2:depends("alive", "2")
cb2:depends("alive", "3")
cb2:depends("alive", "4")
cb2.optional=false;
cb2.default="114.114.114.114"

if (multilock == "0") or (multilock == "1" and rootlock == "1") then
	this_tab = "bwidth"
	bwday = di:taboption(this_tab, ListValue, "bwday", translate("发送监测信息的日期 :"),
		translate("每月发送监测报告文件的日期，请选择在每个月的第几日发送，不需要请选择禁用"))
	bwday.default = "0"
	bwday:value("0", translate("禁用"))
	bwday:value("1", translate("1日"))
	bwday:value("2", translate("2日"))
	bwday:value("3", translate("3日"))
	bwday:value("4", translate("4日"))
	bwday:value("5", translate("5日"))
	bwday:value("6", translate("6日"))
	bwday:value("7", translate("7日"))
	bwday:value("8", translate("8日"))
	bwday:value("9", translate("9日"))
	bwday:value("10", translate("10日"))
	bwday:value("11", translate("11日"))
	bwday:value("12", translate("12日"))
	bwday:value("13", translate("13日"))
	bwday:value("14", translate("14日"))
	bwday:value("15", translate("15日"))
	bwday:value("16", translate("16日"))
	bwday:value("17", translate("17日"))
	bwday:value("18", translate("18日"))
	bwday:value("19", translate("19日"))
	bwday:value("20", translate("20日"))
	bwday:value("21", translate("21日"))
	bwday:value("22", translate("22日"))
	bwday:value("23", translate("23日"))
	bwday:value("24", translate("24日"))
	bwday:value("25", translate("25日"))
	bwday:value("26", translate("26日"))
	bwday:value("27", translate("27日"))
	bwday:value("28", translate("28日"))

	phone = di:taboption(this_tab, Value, "phone", translate("手机号码 :"), translate("我们将会通过这个手机号码给你发送监测日志报告，物联卡就爬吧发不了的，短信费用按你套餐资费扣，介意就别踏马用了"))
	phone.default = "填入接收监测报告的手机号码，我们将会以短信方式给你发送"
	
	bwdelay = di:taboption(this_tab, ListValue, "bwdelay", translate("发送前延迟 :"),
		translate("凌晨后几个小时发送短信给你"))
	bwdelay:value("0", translate("不延迟"))
	bwdelay:value("1", translate("1 小时"))
	bwdelay:value("2", translate("2 小时"))
	bwdelay:value("3", translate("3 小时"))
	bwdelay:value("4", translate("4 小时"))
	bwdelay:value("5", translate("5 小时"))
	bwdelay:value("6", translate("6 小时"))
	bwdelay:value("7", translate("7 小时"))
	bwdelay:value("8", translate("8 小时"))
	bwdelay:value("9", translate("9 小时"))
	bwdelay:value("10", translate("10 小时"))
	bwdelay:value("11", translate("11 小时"))
	bwdelay:value("12", translate("12 小时"))
end

if fs.stat("/usr/lib/autoapn/apn.data") then
	dda = m:section(TypedSection, "disable", translate("Use Automatic APN"), translate("Enable the use of the Automatic APN selection. This disables Custom Profiles."))
	dda.anonymous = true
	aenabled = dda:option(Flag, "autoapn", translate("Enabled"))
	aenabled.default="0"
	aenabled.optional=false;
end

dd = m:section(TypedSection, "disable", translate("禁用自动APN :"), translate("禁用自动APN后，所有4G/5G通信模块将会使用默认配置文件，不勾选就是默认启用"))
dd.anonymous = true

enabled = dd:option(Flag, "enabled", translate("禁用"))
enabled.default="0"
enabled.optional=false;

--
-- Custom profile
--

s = m:section(TypedSection, "custom", translate("定制特定组合配置 :"), translate("可将某个特定配置应用在你所选中的4G/5G模块与SIM卡组合中"))
s.anonymous = true
s.addremove = true
s:tab("custom", translate("简要"))
s:tab("cadvanced", translate("高级"))
s:tab("cconnect", translate("网络异常自动处理"))
if (multilock == "0") or (multilock == "1" and rootlock == "1") then
	s:tab("cbwidth", translate("通信模块频宽日志"))
end

this_ctab = "custom"

name = s:taboption(this_ctab, Value, "name", translate("配置名称 :"))

enabled = s:taboption(this_ctab, Flag, "enabled", translate("启用"))
enabled.default="1"
enabled.optional=false;

select = s:taboption(this_ctab, ListValue, "select", translate("选择对象 :"));
select:value("0", translate("模块 ID"))
select:value("1", translate("模块 IMEI"))
select:value("2", translate("模块名称"))
select:value("3", translate("SIM卡IMSI"))
select:value("4", translate("SIM卡ICCID"))
select.default=0

idV = s:taboption(this_ctab, Value, "vid", translate("USB VID :")); 
idV.optional=false;
idV:depends("select", "0")
idV.default="xxxx"

idP = s:taboption(this_ctab, Value, "pid", translate("USB PID :")); 
idP.optional=false;
idP:depends("select", "0") 
idP.default="xxxx"

imei = s:taboption(this_ctab, Value, "imei", translate("通信模块IMEI :")); 
imei.optional=false;
imei:depends("select", "1")
imei.datatype = "uinteger"
imei.default="1234567"

model = s:taboption(this_ctab, Value, "model", translate("模块名称包含 :")); 
model.optional=false;
model:depends("select", "2")
model.default="xxxx"

imsi = s:taboption(this_ctab, Value, "imsi", translate("SIM卡IMSI  :")); 
imsi.optional=false;
imsi:depends("select", "3")
imsi.datatype = "uinteger"
imsi.default="1234567"

iccid = s:taboption(this_ctab, Value, "iccid", translate("SIM卡ICCID :")); 
iccid.optional=false;
iccid:depends("select", "4")
iccid.datatype = "uinteger"
iccid.default="1234567"

select1 = s:taboption(this_ctab, ListValue, "select1", translate("选择组合对象 :"));
select1:value("0", "模块 ID")
select1:value("1", "模块 IMEI")
select1:value("2", "型号名称")
select1:value("3", "SIM卡IMSI")
select1:value("4", "SIM卡ICCID")
select1:value("10", "None")
select1.default=10

idV1 = s:taboption(this_ctab, Value, "vid1", translate("USB VID :")); 
idV1.optional=false;
idV1:depends("select1", "0")
idV1.default="xxxx"

idP1 = s:taboption(this_ctab, Value, "pid1", translate("USB PID :")); 
idP1.optional=false;
idP1:depends("select1", "0") 
idP1.default="xxxx"

imei1 = s:taboption(this_ctab, Value, "imei1", translate("模块 IMEI :")); 
imei1.optional=false;
imei1:depends("select1", "1")
imei1.datatype = "uinteger"
imei1.default="1234567"

model1 = s:taboption(this_ctab, Value, "model1", translate("模块 名称包含 :")); 
model1.optional=false;
model1:depends("select1", "2")
model1.default="xxxx"

imsi1 = s:taboption(this_ctab, Value, "imsi1", translate("SIM卡IMSI :")); 
imsi1.optional=false;
imsi1:depends("select1", "3")
imsi1.datatype = "uinteger"
imsi1.default="1234567"

iccid1 = s:taboption(this_ctab, Value, "iccid1", translate("SIM卡ICCID :")); 
iccid1.optional=false;
iccid1:depends("select1", "4")
iccid1.datatype = "uinteger"
iccid1.default="1234567"

cma = s:taboption(this_ctab, Value, "apn", "APN :"); 
cma.rmempty = true;

tt = s:taboption(this_ctab, ListValue, "ttl", translate("自定义TTL值 :"))
tt:value("0", translate("Use Current Value"))
tt:value("1", translate("No TTL Value"))
tt:value("63", "TTL 63")
tt:value("64", "TTL 64")
tt:value("65", "TTL 65")
tt:value("66", "TTL 66")
tt:value("67", "TTL 67")
tt:value("117", "TTL 117")
tt:value("TTL-INC 1", "TTL-INC 1")
tt.default = "0"

nl = s:taboption(this_ctab, ListValue, "hostless", translate("调整无主机(无主控)模块的TTL值  :"));
nl:value("0", translate("No"))
nl:value("1", translate("是"))
nl.default=0

pt = s:taboption(this_ctab, ListValue, "pdptype", translate("IP协议类型 :"))
pt:value("IP", "IPv4")
pt:value("IPv6", "IPv6")
pt:value("IPV4V6", "IPv4+IPv6")
pt:value("0", "Default")
pt.default = "0"

cmcc = s:taboption(this_ctab, Value, "context", translate("PDP Context for APN :"));
cmcc.optional=false; 
cmcc.rmempty = true;
cmcc.datatype = "and(uinteger,min(1),max(10))"
cmcc.default = "1"

cmu = s:taboption(this_ctab, Value, "user", translate("用户名 :")); 
cmu.optional=false; 
cmu.rmempty = true;

cmp = s:taboption(this_ctab, Value, "passw", translate("密码 :")); 
cmp.optional=false; 
cmp.rmempty = true;
cmp.password = true

cmpi = s:taboption(this_ctab, Value, "pincode", "PIN码 :"); 
cmpi.optional=false; 
cmpi.rmempty = true;

cmau = s:taboption(this_ctab, ListValue, "auth", translate("身份验证类型 :"))
cmau:value("0", "无")
cmau:value("1", "PAP")
cmau:value("2", "CHAP")
cmau.default = "0"

cmtz = s:taboption(this_ctab, ListValue, "tzone", translate("自动设置时区"), translate("模块通网后自动设置时区"));
cmtz:value("0", translate("禁用"))
cmtz:value("1", translate("启用"))
cmtz.default=1

cml = s:taboption(this_ctab, ListValue, "lock", translate("是否锁定国际漫游运营商选项(国内不用管) :"));
cml:value("0", translate("否"))
cml:value("1", translate("强硬的"))
cml:value("2", translate("柔弱的"))
cml.default=0

cmcc = s:taboption(this_ctab, Value, "mcc", translate("MCC :"));
cmcc.optional=false; 
cmcc.rmempty = true;
cmcc.datatype = "and(uinteger,min(1),max(999))"
cmcc:depends("lock", "1")
cmcc:depends("lock", "2")

cmnc = s:taboption(this_ctab, Value, "mnc", translate("MNC :"));
cmnc.optional=false; 
cmnc.rmempty = true;
cmnc.datatype = "and(uinteger,min(1),max(999))"
cmnc:depends("lock", "1")
cmnc:depends("lock", "2")

this_ctaba = "cadvanced"

cmf = s:taboption(this_ctaba, ListValue, "ppp", translate("强制使用3G PP协议拨号 :"));
cmf:value("0", translate("否"))
cmf:value("1", translate("是"))
cmf.default=0

cmw = s:taboption(this_ctaba, ListValue, "inter", translate("模块WAN口分配 :"));
cmw:value("0", "自动")
cmw:value("1", "WAN1")
cmw:value("2", "WAN2")
cmw:value("3", "关闭")
cmw.default=0

cmd = s:taboption(this_ctaba, Value, "delay", translate("连接延迟(以秒为单位) :")); 
cmd.optional=false; 
cmd.rmempty = false;
cmd.default = 5
cmd.datatype = "and(uinteger,min(5))"

cnl = s:taboption(this_ctaba, ListValue, "nodhcp", translate("强制QMI协议不使用模块的DHCP，从其他位置获取 :"));
cnl:value("0", translate("禁用"))
cnl:value("1", translate("启用"))
cnl.default=0

cmdns1 = s:taboption(this_ctaba, Value, "dns1", translate("自定义1 :")); 
cmdns1.rmempty = true;
cmdns1.optional=false;
cmdns1.datatype = "ipaddr"

cmdns2 = s:taboption(this_ctaba, Value, "dns2", translate("自定义2 :")); 
cmdns2.rmempty = true;
cmdns2.optional=false;
cmdns2.datatype = "ipaddr"

cmdns3 = s:taboption(this_ctaba, Value, "dns3", translate("自定义3 :")); 
cmdns3.rmempty = true;
cmdns3.optional=false;
cmdns3.datatype = "ipaddr"

cmdns4 = s:taboption(this_ctaba, Value, "dns4", translate("自定义4 :")); 
cmdns4.rmempty = true;
cmdns4.optional=false;
cmdns4.datatype = "ipaddr"

cmlog = s:taboption(this_ctaba, ListValue, "log", translate("启用连接日志 :"));
cmlog:value("0", translate("禁用"))
cmlog:value("1", translate("启用"))
cmlog.default=0

if nixio.fs.access("/etc/config/mwan3") then
	cmlb = s:taboption(this_ctaba, ListValue, "lb", translate("在多个5G模块中开启流量负载均衡 :"));
	cmlb:value("0", translate("禁用"))
	cmlb:value("1", translate("启用"))
	cmlb.default=0
end

mtu = s:taboption(this_ctaba, Value, "mtu", translate("自定义MTU值 :"),
		translate("歪歪歪   米乱整窝，纸能自定义1420到1500的蜗   根据不同运营商进行调整的歪，理不鸡丢就不要瞎搞了蛙，等夏列个网炸嘿掉了鳖嘿来找我蜗~~"));
mtu.optional=true
mtu.rmempty = true
mtu.default = "1500"
mtu.datatype = "range(1420, 1500)"

cmat = s:taboption(this_ctaba, ListValue, "at", translate("在连接模块时发送自定义AT命令 :"));
cmat:value("0", translate("禁用"))
cmat:value("1", translate("启用"))
cmat.default=0

cmatc = s:taboption(this_ctaba, Value, "atc", translate("自定义AT命令 :"));
cmatc.optional=false;
cmatc.rmempty = true;

--
-- Custom Connection Monitoring
--

this_ctab = "cconnect"

calive = s:taboption(this_ctab, ListValue, "alive", translate("连接状态监控 :")); 
calive.rmempty = true;
calive:value("0", translate("禁用"))
calive:value("1", translate("启用掉线写入系统日志"))
calive:value("2", translate("启用 5G模块掉线 则 路由自动重启"))
calive:value("3", translate("启用 5G模块掉线 则 自动重拨"))
calive:value("4", translate("启用 5G模块掉线 则 重新上电模块，需要路由GPIO支持，否则将使用其他方式重连"))
calive.default=0

reliability = s:taboption(this_ctab, Value, "reliability", translate("监测网络稳定性  :"),
		translate("Acceptable values: 范围: 1-100，必须Ping通这些IP地址，这条通信链路才会被视为联网状态"))
reliability.datatype = "range(1, 100)"
reliability.default = "1"
reliability:depends("alive", "1")
reliability:depends("alive", "2")
reliability:depends("alive", "3")
reliability:depends("alive", "4")

count = s:taboption(this_ctab, ListValue, "count", translate("监测次数（Ping）:"))
count.default = "1"
count:value("1")
count:value("2")
count:value("3")
count:value("4")
count:value("5")
count:depends("alive", "1")
count:depends("alive", "2")
count:depends("alive", "3")
count:depends("alive", "4")

interval = s:taboption(this_ctab, ListValue, "pingtime", translate("监测时间间隔  (Ping)  :"),
		translate("监测Ping之间间隔时间的值"))
interval.default = "10"
interval:value("5", translate("5 秒"))
interval:value("10", translate("10 秒"))
interval:value("20", translate("20 秒"))
interval:value("30", translate("30 秒"))
interval:value("60", translate("1 分钟"))
interval:value("300", translate("5 分钟"))
interval:value("600", translate("10 分钟"))
interval:value("900", translate("15 分钟"))
interval:value("1800", translate("30 分钟"))
interval:value("3600", translate("1 小时"))
interval:depends("alive", "1")
interval:depends("alive", "2")
interval:depends("alive", "3")
interval:depends("alive", "4")

timeout = s:taboption(this_ctab, ListValue, "pingwait", translate("Ping 超时  :"))
timeout.default = "2"
timeout:value("1", translate("1 秒"))
timeout:value("2", translate("2 秒"))
timeout:value("3", translate("3 秒"))
timeout:value("4", translate("4 秒"))
timeout:value("5", translate("5 秒"))
timeout:value("6", translate("6 秒"))
timeout:value("7", translate("7 秒"))
timeout:value("8", translate("8 秒"))
timeout:value("9", translate("9 秒"))
timeout:value("10", translate("10 秒"))
timeout:depends("alive", "1")
timeout:depends("alive", "2")
timeout:depends("alive", "3")
timeout:depends("alive", "4")

packetsize = s:taboption(this_ctab, Value, "packetsize", translate("Ping报文大小(字节为单位)  :"),
		translate("范围值 :4-56，ping报文发送的数据字节数，可以要根据运营商进行调整"))
	packetsize.datatype = "range(4, 56)"
	packetsize.default = "56"
	packetsize:depends("alive", "1")
	packetsize:depends("alive", "2")
	packetsize:depends("alive", "3")
	packetsize:depends("alive", "4")

down = s:taboption(this_ctab, ListValue, "down", translate("链路状态 异常判定次数  :"),
		translate("如果达到设定值次数，Ping依旧失败后，接口就会被当做异常不在线"))
down.default = "3"
down:value("1")
down:value("2")
down:value("3")
down:value("4")
down:value("5")
down:value("6")
down:value("7")
down:value("8")
down:value("9")
down:value("10")
down:depends("alive", "1")
down:depends("alive", "2")
down:depends("alive", "3")
down:depends("alive", "4")

up = s:taboption(this_ctab, ListValue, "up", translate("链路状态 正常判定次数"),
		translate("当达到设定值次数Ping监测后，网络都能Ping设定地址后，则链路状态会被视为正常啦~~"))
up.default = "3"
up:value("1")
up:value("2")
up:value("3")
up:value("4")
up:value("5")
up:value("6")
up:value("7")
up:value("8")
up:value("9")
up:value("10")
up:depends("alive", "1")
up:depends("alive", "2")
up:depends("alive", "3")
up:depends("alive", "4")

cb2 = s:taboption(this_ctab, DynamicList, "trackip", translate("追踪 IP  :"),
		translate("这个IP将被拿来做链路状态检测用，简单说就是Ping它，能通那么就算链路正常，不通那就以你刚配合的方法处理"))
cb2.datatype = "ipaddr"
cb2:depends("alive", "1")
cb2:depends("alive", "2")
cb2:depends("alive", "3")
cb2:depends("alive", "4")
cb2.optional=false;
cb2.default="114.114.114.114"

if (multilock == "0") or (multilock == "1" and rootlock == "1") then
	this_ctab = "cbwidth"
	bwday = s:taboption(this_ctab, ListValue, "bwday", translate("每个月发送网络稳定日志的日期"),
		translate("每月发送监测报告文件的日期，请选择在每个月的第几日发送，不需要请选择禁用"))
	bwday.default = "0"
	bwday:value("0", translate("禁用"))
	bwday:value("1", translate("1日"))
	bwday:value("2", translate("2日"))
	bwday:value("3", translate("3日"))
	bwday:value("4", translate("4日"))
	bwday:value("5", translate("5日"))
	bwday:value("6", translate("6日"))
	bwday:value("7", translate("7日"))
	bwday:value("8", translate("8日"))
	bwday:value("9", translate("9日"))
	bwday:value("10", translate("10日"))
	bwday:value("11", translate("11日"))
	bwday:value("12", translate("12日"))
	bwday:value("13", translate("13日"))
	bwday:value("14", translate("14日"))
	bwday:value("15", translate("15日"))
	bwday:value("16", translate("16日"))
	bwday:value("17", translate("17日"))
	bwday:value("18", translate("18日"))
	bwday:value("19", translate("19日"))
	bwday:value("20", translate("20日"))
	bwday:value("21", translate("21日"))
	bwday:value("22", translate("22日"))
	bwday:value("23", translate("23日"))
	bwday:value("24", translate("24日"))
	bwday:value("25", translate("25日"))
	bwday:value("26", translate("26日"))
	bwday:value("27", translate("27日"))
	bwday:value("28", translate("28日"))

	phone = s:taboption(this_ctab, Value, "phone", translate("手机号码  :"), translate("我们将会通过这个手机号码给你发送监测日志报告，物联卡就爬吧发不了的，短信费用按你套餐资费扣，介意就不要用了"))
	phone.default = "12223334444"
	
	bwdelay = s:taboption(this_ctab, ListValue, "bwdelay", translate("发送前延迟  :"),
		translate("凌晨后几个小时发送短信给你"))
	bwdelay:value("0", translate("No Delay"))
	bwdelay:value("1", translate("1 小时"))
	bwdelay:value("2", translate("2 小时"))
	bwdelay:value("3", translate("3 小时"))
	bwdelay:value("4", translate("4 小时"))
	bwdelay:value("5", translate("5 小时"))
	bwdelay:value("6", translate("6 小时"))
	bwdelay:value("7", translate("7 小时"))
	bwdelay:value("8", translate("8 小时"))
	bwdelay:value("9", translate("9 小时"))
	bwdelay:value("10", translate("10 小时"))
	bwdelay:value("11", translate("11 小时"))
	bwdelay:value("12", translate("12 小时"))
end

return m

		

