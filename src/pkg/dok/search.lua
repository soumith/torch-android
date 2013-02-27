--------------------------------------------------------------------------------
-- search in help
-- that file defines all the tools and goodies to allow search
--------------------------------------------------------------------------------
local entries = {}

-- paths.install_dok = paths.concat(paths.install_html, '..', 'dok')
-- paths.install_dokmedia = paths.concat(paths.install_html, '..', 'dokmedia')

local function html2entries(html, package, file)
   local dnext = html:gfind('<div.->(.-)</div>')
   for div in dnext do
      local next = div:gfind('<h%d><a.->%s+(.-)%s+</a></h%d><a.-></a>\n<a name="(.-)"></a>\n(.-)$')
      for title,link,body in next do
         link = package .. '/' .. file:gsub('.txt','.html') .. '#' .. link
         body = body:gsub('<img.->','')
         entries.global = entries.global or {}
         table.insert(entries.global, {title, link, body})
         entries[package] = entries[package] or {}
         table.insert(entries[package], {title, '../' .. link, body})
      end
   end
   return entries
end

local function install(entries, dir)
   local vars = {}
   for i,entry in ipairs(entries) do
      table.insert(vars, 's[' .. (i-1) .. '] = "' 
                .. table.concat(entry, '^~^'):gsub('"','\\"'):gsub('\n',' ') .. '";')
   end
   local array = table.concat(vars, '\n')
   array = array:gsub('%%','PERCENTFUCK')
   local f = paths.concat(paths.install_html, paths.basename(dir), 'jse_form.js')
   if paths.filep(f) then
      local js = io.open(f):read('*all')
      js = js:gsub('// SEARCH_ARRAY //', array)
      js = js:gsub('PERCENTFUCK','%')
      local w = io.open(f,'w')
      w:write(js)
      w:close()
   end
end

function dok.installsearch()
   print('-- parsing dok files to build search index')
   for package in paths.files(paths.install_dok) do
      if package ~= '.' and package ~= '..' then
         local dir = paths.concat(paths.install_dok, package)
         for file in paths.files(dir) do
            if file ~= '.' and file ~= '..' then
               print('-+ parsing file: ' .. paths.concat(dir,file))
               local path = paths.concat(dir, file)
               local f = io.open(path)
               if f then
                  local content = f:read('*all')
                  local html = dok.dok2html(content)
                  local entries = html2entries(html, package, file)
               end
            end
         end
      end
   end
   for package,entries in pairs(entries) do
      print('-+ installing search for package: ' .. package)
      if package == 'global' then package = '.' end
      install(entries, package)
   end
end
