module("luci.controller.https_dns_proxy", package.seeall)
function index()
	if not nixio.fs.access("/etc/config/https_dns_proxy") then
		return
	end
	entry({"admin", "services", "https_dns_proxy"}, cbi("https_dns_proxy"), _("HTTPS DNS Proxy"))
end
