local netmod = luci.model.network

local proto = netmod:register_protocol("minieap")

function proto.get_i18n(self)
	return luci.i18n.translate("MiniEAP client")
end

function proto.is_installed(self)
	return nixio.fs.access("/lib/netifd/proto/minieap.sh")
end

function proto.opkg_package(self)
	return "minieap"
end

if netmod["register_error_code"] then
	netmod:register_error_code("MISSING_USER_OR_PASS", luci.i18n.translate("Missing username or password"))
	netmod:register_error_code("EXIT_FAILURE", luci.i18n.translate("Program exited with failure. See system log for more information."))
end
