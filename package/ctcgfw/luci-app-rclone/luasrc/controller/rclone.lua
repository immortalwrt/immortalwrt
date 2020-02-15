module("luci.controller.rclone", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/rclone") then
		return
	end
	entry({"admin", "nas", "rclone"}, cbi("rclone"), _("Rclone"), 100 ).dependent = false
end
