module("luci.model.cbi.passwall.api.brook", package.seeall)
local fs = require "nixio.fs"
local sys = require "luci.sys"
local util = require "luci.util"
local i18n = require "luci.i18n"
local api = require "luci.model.cbi.passwall.api.api"

local brook_api = "https://api.github.com/repos/txthinking/brook/releases/latest"

function to_check(arch)
    local app_path = api.get_brook_path() or ""
    if app_path == "" then
        return {
            code = 1,
            error = i18n.translatef("You did not fill in the %s path. Please save and apply then update manually.", "Brook")
        }
    end
    if not arch or arch == "" then arch = api.auto_get_arch() end

    local file_tree, sub_version = api.get_file_info(arch)

    if file_tree == "" then
        return {
            code = 1,
            error = i18n.translate("Can't determine ARCH, or ARCH not supported.")
        }
    end

    file_tree = "_linux_" .. file_tree

    local json = api.get_api_json(brook_api)

    if json.tag_name == nil then
        return {
            code = 1,
            error = i18n.translate("Get remote version info failed.")
        }
    end

    local now_version = api.get_brook_version()
    local remote_version = json.tag_name:match("[^v]+")
    local needs_update = api.compare_versions(now_version, "<", remote_version)
    local html_url, download_url

    if needs_update then
        html_url = json.html_url
        for _, v in ipairs(json.assets) do
            if v.name and v.name:match(file_tree .. sub_version) then
                download_url = v.browser_download_url
                break
            end
        end
    end

    if needs_update and not download_url then
        return {
            code = 1,
            now_version = now_version,
            version = remote_version,
            html_url = html_url,
            error = i18n.translate("New version found, but failed to get new version download url.")
        }
    end

    return {
        code = 0,
        update = needs_update,
        now_version = now_version,
        version = remote_version,
        url = {html = html_url, download = download_url}
    }
end

function to_download(url)
    local app_path = api.get_brook_path() or ""
    if app_path == "" then
        return {
            code = 1,
            error = i18n.translatef("You did not fill in the %s path. Please save and apply then update manually.", "Brook")
        }
    end
    if not url or url == "" then
        return {code = 1, error = i18n.translate("Download url is required.")}
    end

    sys.call("/bin/rm -f /tmp/brook_download.*")

    local tmp_file = util.trim(util.exec("mktemp -u -t brook_download.XXXXXX"))

    local result = api.exec(api.curl, {api._unpack(api.curl_args), "-o", tmp_file, url}, nil, api.command_timeout) == 0

    if not result then
        api.exec("/bin/rm", {"-f", tmp_file})
        return {
            code = 1,
            error = i18n.translatef("File download failed or timed out: %s", url)
        }
    end

    return {code = 0, file = tmp_file}
end

function to_move(file)
    local app_path = api.get_brook_path() or ""
    if app_path == "" then
        return {
            code = 1,
            error = i18n.translatef("You did not fill in the %s path. Please save and apply then update manually.", "Brook")
        }
    end
    if not file or file == "" or not fs.access(file) then
        sys.call("/bin/rm -rf /tmp/brook_download.*")
        return {code = 1, error = i18n.translate("Client file is required.")}
    end

    local new_version = api.get_brook_version(file)
    if new_version == "" then
        sys.call("/bin/rm -rf /tmp/brook_download.*")
        return {
            code = 1,
            error = i18n.translate("The client file is not suitable for current device.")
        }
    end

    local app_path_bak

    if fs.access(app_path) then
        app_path_bak = app_path .. ".bak"
        api.exec("/bin/mv", {"-f", app_path, app_path_bak})
    end

    local result = api.exec("/bin/mv", {"-f", file, app_path}, nil, api.command_timeout) == 0

    if not result or not fs.access(app_path) then
        sys.call("/bin/rm -rf /tmp/brook_download.*")
        if app_path_bak then
            api.exec("/bin/mv", {"-f", app_path_bak, app_path})
        end
        return {
            code = 1,
            error = i18n.translatef("Can't move new file to path: %s", app_path)
        }
    end

    api.exec("/bin/chmod", {"755", app_path})

    if app_path_bak then api.exec("/bin/rm", {"-f", app_path_bak}) end

    sys.call("/bin/rm -rf /tmp/brook_download.*")

    return {code = 0}
end
