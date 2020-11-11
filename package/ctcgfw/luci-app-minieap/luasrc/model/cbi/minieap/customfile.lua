
require("luci.sys")

local f = SimpleForm("minieap",
	translate("minieap LOG"),
	translate("Log file:/var/log/minieap.log"))

return f
