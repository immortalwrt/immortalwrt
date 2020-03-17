--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

require "luci.util"
local uci = luci.model.uci.cursor()
local docker = require "luci.model.docker"
local dk = docker.new()

local containers, images
local res = dk.images:list()
if res.code <300 then images = res.body else return end
res = dk.containers:list({query = {all=true}})
if res.code <300 then containers = res.body else return end

function get_images()
  local data = {}
  for i, v in ipairs(images) do
    local index = v.Created .. v.Id
    data[index]={}
    data[index]["_selected"] = 0
    data[index]["id"] = v.Id:sub(8)
    data[index]["_id"] = '<a href="javascript:new_tag(\''..v.Id:sub(8,20)..'\')" class="dockerman-link" title="'..translate("New tag")..'">' .. v.Id:sub(8,20) .. '</a>'
    if v.RepoTags and next(v.RepoTags)~=nil then
      for i, v1 in ipairs(v.RepoTags) do
        data[index]["_tags"] =(data[index]["_tags"] and ( data[index]["_tags"] .. "<br>" )or "") .. ((v1:match("<none>") or (#v.RepoTags == 1)) and v1 or ('<a href="javascript:un_tag(\''..v1..'\')" class="dockerman_link" title="'..translate("Remove tag")..'" >' .. v1 .. '</a>'))
        if not data[index]["tag"] then
          data[index]["tag"] = v1--:match("<none>") and nil or v1
        end
      end
    else
      data[index]["_tags"] = v.RepoDigests[1] and v.RepoDigests[1]:match("^(.-)@.+")
      data[index]["_tags"] = (data[index]["_tags"] and data[index]["_tags"] or  "<none>" ).. ":<none>"
    end
    data[index]["_tags"] = data[index]["_tags"]:gsub("<none>","&lt;none&gt;")
    -- data[index]["_tags"] = '<a href="javascript:handle_tag(\''..data[index]["_id"]..'\')">' .. data[index]["_tags"] .. '</a>'
    for ci,cv in ipairs(containers) do
      if v.Id == cv.ImageID then
        data[index]["_containers"] = (data[index]["_containers"] and (data[index]["_containers"] .. " | ") or "")..
        '<a href='..luci.dispatcher.build_url("admin/services/docker/container/"..cv.Id)..' class="dockerman_link" title="'..translate("Container detail")..'">'.. cv.Names[1]:sub(2).."</a>"
      end
    end
    data[index]["_size"] = string.format("%.2f", tostring(v.Size/1024/1024)).."MB"
    data[index]["_created"] = os.date("%Y/%m/%d %H:%M:%S",v.Created)
  end
  return data
end

local image_list = get_images()

-- m = Map("docker", translate("Docker"))
m = SimpleForm("docker", translate("Docker"))
m.template = "dockerman/cbi/xsimpleform"
m.submit=false
m.reset=false

local pull_value={_image_tag_name="", _registry="index.docker.io"}
local pull_section = m:section(SimpleSection, translate("Pull Image"))
pull_section.template="cbi/nullsection"
local tag_name = pull_section:option(Value, "_image_tag_name")
tag_name.template = "dockerman/cbi/inlinevalue"
tag_name.placeholder="hello-world:latest"
local action_pull = pull_section:option(Button, "_pull")
action_pull.inputtitle= translate("Pull")
action_pull.template = "dockerman/cbi/inlinebutton"
action_pull.inputstyle = "add"
tag_name.write = function(self, section, value)
  local hastag = value:find(":")
  if not hastag then
    value = value .. ":latest"
  end
  pull_value["_image_tag_name"] = value
end
action_pull.write = function(self, section)
  local tag = pull_value["_image_tag_name"]
  local json_stringify = luci.jsonc and luci.jsonc.stringify
  if tag and tag ~= "" then
    docker:write_status("Images: " .. "pulling" .. " " .. tag .. "...\n")
    -- local x_auth = nixio.bin.b64encode(json_stringify({serveraddress= server})) , header={["X-Registry-Auth"] = x_auth}
    local res = dk.images:create({query = {fromImage=tag}}, docker.pull_image_show_status_cb)
    -- {"errorDetail": {"message": "failed to register layer: ApplyLayer exit status 1 stdout:  stderr: write \/docker: no space left on device" }, "error": "failed to register layer: ApplyLayer exit status 1 stdout:  stderr: write \/docker: no space left on device" }
    if res and res.code == 200 and (res.body[#res.body] and not res.body[#res.body].error and res.body[#res.body].status and (res.body[#res.body].status == "Status: Downloaded newer image for ".. tag)) then
      docker:clear_status()
    else
      docker:append_status("code:" .. res.code.." ".. (res.body[#res.body] and res.body[#res.body].error or (res.body.message or res.message)).. "\n")
    end
  else
    docker:append_status("code: 400 please input the name of image name!")
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/services/docker/images"))
end

-- local import_section = m:section(SimpleSection, translate("Import Images"))
-- local im = import_section:option(DummyValue, "_image_import")
-- im.template = "dockerman/images_import"

local image_table = m:section(Table, image_list, translate("Images"))

local image_selecter = image_table:option(Flag, "_selected","")
image_selecter.disabled = 0
image_selecter.enabled = 1
image_selecter.default = 0

local image_id = image_table:option(DummyValue, "_id", translate("ID"))
image_id.rawhtml = true
image_table:option(DummyValue, "_tags", translate("RepoTags")).rawhtml = true
image_table:option(DummyValue, "_containers", translate("Containers")).rawhtml = true
image_table:option(DummyValue, "_size", translate("Size"))
image_table:option(DummyValue, "_created", translate("Created"))
image_selecter.write = function(self, section, value)
  image_list[section]._selected = value
end

local remove_action = function(force)
  local image_selected = {}
  -- 遍历table中sectionid
  local image_table_sids = image_table:cfgsections()
  for _, image_table_sid in ipairs(image_table_sids) do
    -- 得到选中项的名字
    if image_list[image_table_sid]._selected == 1 then
      image_selected[#image_selected+1] = (image_list[image_table_sid]["_tags"]:match("<br>") or image_list[image_table_sid]["_tags"]:match("&lt;none&gt;")) and image_list[image_table_sid].id or image_list[image_table_sid].tag
    end
  end
  if next(image_selected) ~= nil then
    local success = true
    docker:clear_status()
    for _,img in ipairs(image_selected) do
      docker:append_status("Images: " .. "remove" .. " " .. img .. "...")
      local query
      if force then query = {force = true} end
      local msg = dk.images:remove({id = img, query = query})
      if msg.code ~= 200 then
        docker:append_status("code:" .. msg.code.." ".. (msg.body.message and msg.body.message or msg.message).. "\n")
        success = false
      else
        docker:append_status("done\n")
      end
    end
    if success then docker:clear_status() end
    luci.http.redirect(luci.dispatcher.build_url("admin/services/docker/images"))
  end
end

local docker_status = m:section(SimpleSection)
docker_status.template = "dockerman/apply_widget"
docker_status.err = docker:read_status()
docker_status.err = docker_status.err and docker_status.err:gsub("\n","<br>"):gsub(" ","&nbsp;")
if docker_status.err then docker:clear_status() end

local action = m:section(Table,{{}})
action.notitle=true
action.rowcolors=false
action.template="cbi/nullsection"

local btnremove = action:option(Button, "remove")
btnremove.inputtitle= translate("Remove")
btnremove.template = "dockerman/cbi/inlinebutton"
btnremove.inputstyle = "remove"
btnremove.forcewrite = true
btnremove.write = function(self, section)
  remove_action()
end

local btnforceremove = action:option(Button, "forceremove")
btnforceremove.inputtitle= translate("Force Remove")
btnforceremove.template = "dockerman/cbi/inlinebutton"
btnforceremove.inputstyle = "remove"
btnforceremove.forcewrite = true
btnforceremove.write = function(self, section)
  remove_action(true)
end

local btnsave = action:option(Button, "save")
btnsave.inputtitle= translate("Save")
btnsave.template = "dockerman/cbi/inlinebutton"
btnsave.inputstyle = "edit"
btnsave.forcewrite = true
btnsave.write = function (self, section)
  local image_selected = {}
  local image_table_sids = image_table:cfgsections()
  for _, image_table_sid in ipairs(image_table_sids) do
    if image_list[image_table_sid]._selected == 1 then
      image_selected[#image_selected+1] = image_list[image_table_sid].id --image_id:cfgvalue(image_table_sid)
    end
  end
  if next(image_selected) ~= nil then
    local names
    for _,img in ipairs(image_selected) do
      names = names and (names .. "&names=".. img) or img
    end
    local first
    local cb = function(res, chunk)
      if res.code == 200 then
        if not first then
          first = true
          luci.http.header('Content-Disposition', 'inline; filename="images.tar"')
          luci.http.header('Content-Type', 'application\/x-tar')
        end
        luci.ltn12.pump.all(chunk, luci.http.write)
      else
        if not first then
          first = true
          luci.http.prepare_content("text/plain")
        end
        luci.ltn12.pump.all(chunk, luci.http.write)
      end
    end
    docker:write_status("Images: " .. "save" .. " " .. table.concat(image_selected, "\n") .. "...")
    local msg = dk.images:get({query = {names = names}}, cb)
    if msg.code ~= 200 then
      docker:append_status("code:" .. msg.code.." ".. (msg.body.message and msg.body.message or msg.message).. "\n")
      success = false
    else
      docker:clear_status()
    end
  end
end

local btnload = action:option(Button, "load")
btnload.inputtitle= translate("Load")
btnload.template = "dockerman/images_load"
btnload.inputstyle = "add"
return m
