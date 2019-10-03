mp = Map("unblockneteasemusic", translate("解除网易云音乐播放限制"))
mp.description = translate("原理：采用 [QQ/虾米/百度/酷狗/酷我/咕咪/JOOX] 等音源，替换网易云音乐 无版权/收费 歌曲链接<br/>具体使用方法参见：https://github.com/project-openwrt/luci-app-unblockneteasemusic")

mp:section(SimpleSection).template  = "unblockneteasemusic/unblockneteasemusic_status"

s = mp:section(TypedSection, "unblockneteasemusic")
s.anonymous=true
s.addremove=false

enabled = s:option(Flag, "enabled", translate("启用本插件"))
enabled.description = translate("启用本插件以解除网易云音乐播放限制")
enabled.default = 0
enabled.rmempty = false

button_update_core = s:option (Button, "_button_update_core", translate("更新主程序"))
button_update_core.description = translate("更新完毕后会自动重启插件；更新插件需要一点时间，请不要反复点击按钮")
local latest_ver = luci.sys.exec("/bin/bash /usr/share/unblockneteasemusic/check_update.sh")
button_update_core.inputtitle = translate (latest_ver)
button_update_core.inputstyle = "apply"
function button_update_core.write (self, section, value)
	luci.sys.call ("/bin/bash /usr/share/unblockneteasemusic/update_core.sh &")
end

account = s:option(Value, "http_port", translate("[HTTP] 监听端口"))
account.description = translate("本插件监听的HTTP端口，不可与其他程序/HTTPS共用一个端口")
account.placeholder = "5200"
account.default = "5200"
account.datatype = "port"
enabled.rmempty = false
account = s:option(Value, "https_port", translate("[HTTPS] 监听端口"))
account.description = translate("本插件监听的HTTPS端口，不可与其他程序/HTTP共用一个端口")
account.placeholder = "5201"
account.default = "5201"
account.datatype = "port"
enabled.rmempty = false

speedtype = s:option(ListValue, "musicapptype", translate("音源接口"))
speedtype:value("default", translate("默认"))
speedtype:value("qq", translate("QQ音乐"))
speedtype:value("xiami", translate("虾米音乐"))
speedtype:value("baidu", translate("百度音乐"))
speedtype:value("kugou", translate("酷狗音乐"))
speedtype:value("kuwo", translate("酷我音乐"))
speedtype:value("migu", translate("咕咪音乐"))
speedtype:value("joox", translate("JOOX音乐"))
speedtype:value("all", translate("所有平台"))
speedtype.description = translate("音源调用接口")
speedtype.default = "default"
speedtype.rmempty = false

hijack = s:option(ListValue, "hijack_ways", translate("劫持方法"))
hijack:value("dont_hijack", translate("不开启劫持"))
hijack:value("use_ipset", translate("使用IPSet劫持"))
hijack:value("use_hosts", translate("使用Hosts劫持"))
hijack.description = translate("如果使用Hosts劫持，请将HTTP/HTTPS端口设置为80/443")
hijack.default = "dont_hijack"
hijack.rmempty = false

enabled = s:option(Flag, "auto_update", translate("启用自动更新"))
enabled.description = translate("启用后会在每天凌晨检测最新版本并自动更新")
enabled.default = 0
enabled.rmempty = false

enabled = s:option(Flag, "advanced_mode", translate("启用进阶设置"))
enabled.description = translate("仅推荐高级玩家使用")
enabled.default = 0
enabled.rmempty = false

enabled = s:option(Flag, "pub_access", translate("部署到公网"))
enabled.description = translate("默认仅监听局域网，如需提供公开访问请勾选此选项；与此同时，建议勾选“启用严格模式”")
enabled.default = 0
enabled.rmempty = false
enabled:depends("advanced_mode", 1)

enabled = s:option(Flag, "strict_mode", translate("启用严格模式"))
enabled.description = translate("若将服务部署到公网，则强烈建议使用严格模式，此模式下仅放行网易云音乐所属域名的请求")
enabled.default = 0
enabled.rmempty = false
enabled:depends("advanced_mode", 1)

account = s:option(Value, "netease_server_ip", translate("网易云服务器IP"))
account.description = translate("通过 ping music.163.com 即可获得IP地址，仅限填写一个")
account.placeholder = "59.111.181.38"
account.datatype = "ipaddr"
account:depends("set_netease_server_ip", 1)
account:depends("advanced_mode", 1)

account = s:option(Value, "endpoint_url", translate("EndPoint"))
account.description = translate("具体说明参见：https://github.com/nondanee/UnblockNeteaseMusic")
account.placeholder = "https://music.163.com"
account.datatype = "string"
account:depends("advanced_mode", 1)

account = s:option(Value, "proxy_server_ip", translate("代理服务器地址"))
account.description = translate("使用代理服务器获取音乐信息")
account.placeholder = "http(s)://host:port"
account.datatype = "string"
account:depends("advanced_mode", 1)

return mp
