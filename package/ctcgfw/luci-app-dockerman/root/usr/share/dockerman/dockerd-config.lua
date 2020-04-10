require "luci.util"
fs = require "nixio.fs"
uci = (require "luci.model.uci").cursor()

raw_file_dir = arg[1]

raw_json_str = fs.readfile(raw_file_dir) or "[]"
raw_json = luci.jsonc.parse(raw_json_str) or {}

new_json = {}
new_json["data-root"] = uci:get("dockerman", "local", "daemon_data_root")
new_json["hosts"] = uci:get("dockerman", "local", "daemon_hosts") or {}
new_json["registry-mirrors"] = uci:get("dockerman", "local", "daemon_registry_mirrors") or {}
new_json["log-level"] = uci:get("dockerman", "local", "daemon_log_level")

function comp(raw, new)
  for k, v in pairs(new) do
    if type(v) == "table" and raw[k] then
      if #v == #raw[k] then
        comp(raw[k], v)
      else
        changed = true
      raw[k] = v
      end
    elseif raw[k] ~= v then
      changed = true
      raw[k] = v
    end
  end
  for k, v in ipairs(new) do
    if type(v) == "table" and raw[k] then
      if #v == #raw[k] then
        comp(raw[k], v)
      else
        changed = true
      raw[k] = v
      end
    elseif raw[k] ~= v then
      changed = true
      raw[k] = v
    end
  end
end
comp(raw_json, new_json)
if changed then
  if next(raw_json["registry-mirrors"]) == nil then raw_json["registry-mirrors"] = nil end
  if next(raw_json["hosts"]) == nil then raw_json["hosts"] = nil end
  fs.writefile(raw_file_dir, luci.jsonc.stringify(raw_json, true):gsub("\\", ""))
  os.exit(0)
else
  os.exit(1)
end
