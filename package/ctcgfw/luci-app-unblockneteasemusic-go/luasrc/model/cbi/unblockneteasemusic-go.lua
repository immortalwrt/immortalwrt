mp = Map("unblockneteasemusic-go", translate("解除网易云音乐播放限制"))
mp.description = translate("原理：采用 [酷我/酷狗/咕咪] 音源，替换网易云音乐 无版权/收费 歌曲链接<br/>具体使用方法参见：https://github.com/project-openwrt/luci-app-unblockneteasemusic-go")

mp:section(SimpleSection).template = "unblockneteasemusic-go/unblockneteasemusic_go_status"

s = mp:section(TypedSection, "unblockneteasemusic-go")
s.anonymous=true
s.addremove=false

enable = s:option(Flag, "enable", translate("启用本插件"))
enable.description = translate("启用本插件以解除网易云音乐播放限制")
enable.default = 0
enable.rmempty = false

http_port = s:option(Value, "http_port", translate("[HTTP] 监听端口"))
http_port.description = translate("本插件监听的HTTP端口，不可与其他程序/HTTPS共用一个端口")
http_port.placeholder = "5210"
http_port.default = "5210"
http_port.datatype = "port"
http_port.rmempty = false

https_port = s:option(Value, "https_port", translate("[HTTPS] 监听端口"))
https_port.description = translate("本插件监听的HTTPS端口，不可与其他程序/HTTP共用一个端口")
https_port.placeholder = "5211"
https_port.default = "5211"
https_port.datatype = "port"
https_port.rmempty = false

music_source = s:option(ListValue, "music_source", translate("音源接口"))
music_source:value("default", translate("默认"))
music_source:value("kuwo", translate("酷我音乐"))
music_source:value("kugou", translate("酷狗音乐"))
music_source:value("migu", translate("咕咪音乐"))
music_source.description = translate("音源调用接口")
music_source.default = "default"
music_source.rmempty = false

hijack = s:option(ListValue, "hijack_ways", translate("劫持方法"))
hijack:value("dont_hijack", translate("不开启劫持"))
hijack:value("use_ipset", translate("使用IPSet劫持"))
hijack:value("use_hosts", translate("使用Hosts劫持"))
hijack.description = translate("如果使用Hosts劫持，请将HTTP/HTTPS端口设置为80/443")
hijack.default = "dont_hijack"
hijack.rmempty = false

daemon_enable = s:option(Flag, "daemon_enable", translate("启用进程守护"))
daemon_enable.description = translate("开启后，附属程序会自动检测主程序运行状态，在主程序退出时自动重启")
daemon_enable.default = 0
daemon_enable.rmempty = false

return mp
