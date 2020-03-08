local uci = require("luci.model.uci").cursor()
function uci_del_list(conf, sect, opt, value)
  local lval = uci:get(conf, sect, opt)
  if lval == nil or lval == "" then
    lval = {}
  elseif type(lval) ~= "table" then
    lval = { lval }
  end

  local i
  local changed = false
  for i = #lval, 1 do
    if lval[i] == value then
      table.remove(lval, i)
      changed = true
    end
  end

  if changed then
    if #lval > 0 then
      uci:set(conf, sect, opt, lval)
    else
      uci:delete(conf, sect, opt)
    end
  end
end

function uci_add_list(conf, sect, opt, value)
  local lval = uci:get(conf, sect, opt)
  if lval == nil or lval == "" then
    lval = {}
  elseif type(lval) ~= "table" then
    lval = { lval }
  end

  lval[#lval+1] = value
  uci:set(conf, sect, opt, lval)
end

m = Map("https_dns_proxy", translate("HTTPS DNS Proxy Settings"))
m.template="cbi/map"

s3 = m:section(TypedSection, "https_dns_proxy", translate("Instances"))
s3.template = "cbi/tblsection"
s3.sortable  = false
s3.anonymous = true
s3.addremove = true

local n = 0
uci:foreach("https_dns_proxy", "https_dns_proxy", function(s)
    if s[".name"] == section then
        return false
    end
    n = n + 1
end)

prov = s3:option(ListValue, "url_prefix", translate("Provider"))
prov:value("https://cloudflare-dns.com/dns-query","Cloudflare")
prov:value("https://dns.google/dns-query","Google")
prov.write = function(self, section, value)
  local la_val = la:formvalue(section)
  local lp_val = lp:formvalue(section)
  if not la_val then la_val = "127.0.0.1" end
  if not lp_val then lp_val = n + 5053 end
  if value and value:match("cloudflare") then
    uci:set("https_dns_proxy", section, "bootstrap_dns", "1.1.1.1,1.0.0.1")
    uci:set("https_dns_proxy", section, "url_prefix", "https://cloudflare-dns.com/dns-query")
  else
    uci:set("https_dns_proxy", section, "bootstrap_dns", "8.8.8.8,8.8.4.4")
    uci:set("https_dns_proxy", section, "url_prefix", "https://dns.google/dns-query")
  end
  uci:set("https_dns_proxy", section, "user", "nobody")
  uci:set("https_dns_proxy", section, "group", "nogroup")
  uci:save("https_dns_proxy")
  if n == 0 then
    uci:delete("dhcp", "@dnsmasq[0]", "server")
  end
  uci_del_list("dhcp", "@dnsmasq[0]", "server", tostring(la_val) .. ":" .. tostring(lp_val))
  uci_add_list("dhcp", "@dnsmasq[0]", "server", tostring(la_val) .. ":" .. tostring(lp_val))
  uci:save("dhcp")
end

la = s3:option(Value, "listen_addr", translate("Listen address"))
la.value   = "127.0.0.1"
la.rmempty = true

lp = s3:option(Value, "listen_port", translate("Listen port"))
lp.datatype    = "port"
lp.value       = n + 5053
lp.write = function(self, section, value)
  if not value then
    uci:set("https_dns_proxy", section, "listen_port", n + 5053)
  else
    uci:set("https_dns_proxy", section, "listen_port", value)
  end
  uci:save("https_dns_proxy")
end

sa = s3:option(Value, "subnet_addr", translate("Subnet address"))
sa.datatype = "ip4prefix"
sa.rmempty  = true

ps = s3:option(Value, "proxy_server", translate("Proxy server"))
ps.datatype = "or(ipaddr,hostname)"
ps.rmempty = true

return m
