#!/usr/bin/lua

local mtwifi_utils = {}

local mtwifi_logfile = "/tmp/mtwifi.log"

function mtwifi_utils.esc(x)
    return (x:gsub('%%', '%%%%')
             :gsub('^%^', '%%^')
             :gsub('%$$', '%%$')
             :gsub('%(', '%%(')
             :gsub('%)', '%%)')
             :gsub('%.', '%%.')
             :gsub('%[', '%%[')
             :gsub('%]', '%%]')
             :gsub('%*', '%%*')
             :gsub('%+', '%%+')
             :gsub('%-', '%%-')
             :gsub('%?', '%%?'))
end

function mtwifi_utils.trim(s)
    if s then return (s:gsub("^%s*(.-)%s*$", "%1")) end
end

function mtwifi_utils.exists(path)
    local fp = io.open(path, "rb")
    if fp then fp:close() end
    return fp ~= nil
end

function mtwifi_utils.read_pipe(pipe)
    local retry_count = 10
    local fp, txt, err
    repeat
        fp = io.popen(pipe)
        txt, err = fp:read("*a")
        fp:close()
        retry_count = retry_count - 1
    until err == nil or retry_count == 0
    return txt
end

function mtwifi_utils.__cfg2list(str)
    -- delimeter == ";"
    local i = 1
    local list = {}
    for k in string.gmatch(str, "([^;]+)") do
        list[i] = k
        i = i + 1
    end
    return list
end

function mtwifi_utils.token_set(str, n, v)
    -- n start from 1
    -- delimeter == ";"
    if not str then return end
    if v == nil then return end
    local tmp = mtwifi_utils.__cfg2list(str)
    if type(v) ~= type("") and type(v) ~= type(0) then
        nixio.syslog("err", "invalid value type in token_set, "..type(v))
        return
    end
    if #tmp < tonumber(n) then
        for i=#tmp, tonumber(n) do
            if not tmp[i] then
                tmp[i] = v -- pad holes with v !
            end
        end
    else
        tmp[n] = v
    end
    return table.concat(tmp, ";"):gsub("^;*(.-);*$", "%1"):gsub(";+",";")
end

function mtwifi_utils.token_get(str, n, v)
    -- n starts from 1
    -- v is the backup in case token n is nil
    if not str then return v end
    local tmp = mtwifi_utils.__cfg2list(str)
    return tmp[tonumber(n)] or v
end

function mtwifi_utils.sleep(s)
    local ntime = os.clock() + s
    repeat until os.clock() > ntime
end

function mtwifi_utils.log2file(str)
    logfile = io.open(mtwifi_logfile, "a")
    logfile:write(os.date("%H:%M:%S", os.time()) .. " " .. str .. "\n")
    logfile:close()
end

return mtwifi_utils
