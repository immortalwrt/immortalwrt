-- Licensed to the public under the Apache License 2.0.

module("luci.controller.ksmbd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ksmbd") then
		return
	end
	
	entry({"admin","nas"},firstchild(),"NAS",44).dependent=false

	local page

	page = entry({"admin", "nas", "ksmbd"}, cbi("ksmbd"), _("Network Shares (KSMBD)"))
	page.dependent = true
end

