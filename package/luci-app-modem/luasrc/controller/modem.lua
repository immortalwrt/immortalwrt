module("luci.controller.modem", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/modem") then
		return
	end
	
	entry({"admin", "network", "modem"}, cbi("modem"), _("Modem Server"), 80).dependent=false
end
