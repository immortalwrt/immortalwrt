m = Map("qbittorrent", translate("qBittorrent"), translate("qBittorrent is a cross-platform free and open-source BitTorrent client.").."<br/>"..translate("Default login username: admin, password: adminadmin."))

m:section(SimpleSection).template="qbittorrent/qbittorrent_status"

s = m:section(TypedSection, "qbittorrent")
s.anonymous=true

enable = s:option(Flag, "enable", translate("Enable"))
enable.rmempty = false

port = s:option(Value,"port",translate("Port"),translate("WebUI listening port"))
port.default = "8080"
port.placeholder = "8080"
port.rmempty = false

profile_dir = s:option(Value,"profile_dir",translate("Profile Dir"),translate("Store configuration files in the Path"))
profile_dir.default = "/etc"
profile_dir.placeholder = "/etc"
profile_dir.rmempty = false

download_dir = s:option(Value,"download_dir",translate("Download Dir"),translate("Store download files in the Path"))
download_dir.default = "/mnt/download"
download_dir.placeholder = "/mnt/download"
download_dir.rmempty = false

return m
