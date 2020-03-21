
module("luci.controller.cpufreq", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/cpufreq") then
		return
	end
	
	entry({"admin", "system", "cpufreq"}, cbi("cpufreq"), _("CPU Freq"), 900).dependent=false
end
