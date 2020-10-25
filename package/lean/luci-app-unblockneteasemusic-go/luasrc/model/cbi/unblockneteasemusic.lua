
mp = Map("unblockneteasemusic", translate("解除网易云音乐播放限制 (Golang)"))
mp.description = translate("原理：采用 [酷我/酷狗/咪咕] 音源(后续有空补充)，替换网易云音乐 灰色 歌曲链接<br/>具体使用方法参见：https://github.com/cnsilvan/luci-app-unblockneteasemusic<br/>提示：客户端网易云音乐能用就别升级app，最新版本不一定能用")
mp:section(SimpleSection).template = "unblockneteasemusic/unblockneteasemusic_status"

s = mp:section(TypedSection, "unblockneteasemusic")
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

music_source = s:option(ListValue, "music_source", translate("音源选择"))
music_source:value("default", translate("默认"))
music_source:value("customize", translate("自定义"))
music_source.description = translate("默认为kuwo")
music_source.default = "default"
music_source.rmempty = false

music_customize_source = s:option(Value, "music_customize_source", translate("自定义音源"))
music_customize_source.description = translate("自定义音源设置，如kuwo:kugou:migu ,以:隔开,migu在某些运营商下无法使用可能会导致卡顿")
music_customize_source.default = "kuwo:kugou"
music_customize_source.rmempty = true
music_customize_source:depends("music_source", "customize")

hijack = s:option(ListValue, "hijack_ways", translate("劫持方法"))
hijack:value("dont_hijack", translate("不开启劫持"))
hijack:value("use_ipset", translate("使用IPSet劫持"))
hijack:value("use_hosts", translate("使用Hosts劫持"))
hijack.description = translate("如果使用Hosts劫持，请将HTTP/HTTPS端口设置为80/443，路由器不建议使用Hosts劫持")
hijack.default = "dont_hijack"
hijack.rmempty = false




return mp
