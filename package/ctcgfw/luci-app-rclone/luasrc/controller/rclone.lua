module("luci.controller.rclone", package.seeall)

function index()
	if not nixio.fs.access("/var/etc/config/rclone") then
		return
	end
	entry({"admin", "services", "rclone"}, cbi("rclone"), _("Rclone"), 100 ).dependent = false
end
