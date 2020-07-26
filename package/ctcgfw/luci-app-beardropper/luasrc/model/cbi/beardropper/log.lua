f = SimpleForm("logview")
f.reset = false
f.submit = false
t = f:field(TextValue, "conf")
t.rmempty = true
t.rows = 20
function t.cfgvalue()
local logs = luci.util.execi("logread | grep authpriv | grep beardropper")
local s = ""
for line in logs do
s = line .. "\n" .. s
end
return s
end
t.readonly="readonly"

return f
