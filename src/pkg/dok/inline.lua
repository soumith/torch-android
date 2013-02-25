--------------------------------------------------------------------------------
-- inline help
-- that file defines all the tools and goodies to generate inline help
--------------------------------------------------------------------------------
dok.inline = {}

--paths.install_dok = paths.concat(paths.install_html, '..', 'dok')
--paths.install_dokmedia = paths.concat(paths.install_html, '..', 'dokmedia')

dok.colors = {
   none = '\27[0m',
   black = '\27[0;30m',
   red = '\27[0;31m',
   green = '\27[0;32m',
   yellow = '\27[0;33m',
   blue = '\27[0;34m',
   magenta = '\27[0;35m',
   cyan = '\27[0;36m',
   white = '\27[0;37m',
   Black = '\27[1;30m',
   Red = '\27[1;31m',
   Green = '\27[1;32m',
   Yellow = '\27[1;33m',
   Blue = '\27[1;34m',
   Magenta = '\27[1;35m',
   Cyan = '\27[1;36m',
   White = '\27[1;37m',
   _black = '\27[40m',
   _red = '\27[41m',
   _green = '\27[42m',
   _yellow = '\27[43m',
   _blue = '\27[44m',
   _magenta = '\27[45m',
   _cyan = '\27[46m',
   _white = '\27[47m'
}
local c = dok.colors

local style = {}
function dok.usecolors()
   style = {
      banner = '+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++',
      list = c.blue .. '> ' .. c.none,
      title = c.Magenta,
      pre = c.cyan,
      em = c.Black,
      bold = c.Black,
      img = c.red,
      link = c.red,
      code = c.green,
      error = c.Red,
      none = c.none
   }
end
function dok.dontusecolors()
   style = {
      banner = '+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++',
      list = '> ',
      title = '',
      pre = '',
      em = '',
      bold = '',
      img = '',
      link = '',
      code = '',
      error = '',
      none = ''
   }
end
dok.usecolors()

local function uncleanText(txt)
   txt = txt:gsub('&#39;', "'")
   txt = txt:gsub('&#42;', '%*')
   txt = txt:gsub('&#43;', '%+')
   txt = txt:gsub('&lt;', '<')
   txt = txt:gsub('&gt;', '>')
   return txt
end

local function string2symbol(str)
   local str  = str:gsub(':','.')
   local ok, res = pcall(loadstring('local t = ' .. str .. '; return t'))
   if not ok then
      ok, res = pcall(loadstring('local t = _torchimport.' .. str .. '; return t'))
   end
   return res
end

local function maxcols(str, cols)
   cols = cols or 70
   local res = ''
   local k = 1
   local color = false
   for i = 1,#str do
      res = res .. str:sub(i,i)
      if str:sub(i,i) == '\27' then
         color = true
      elseif str:sub(i,i) == 'm' then
         color = false
      end
      if k == cols then
         if str:sub(i,i) == ' ' then
            res = res .. '\n'
            k = 1
         end
      elseif not color then
         k = k + 1
      end
      if str:sub(i,i) == '\n' then
         k = 1
      end
   end
   return res
end

function dok.stylize(html, package)
   local styled = html
   -- (0) useless white space
   styled = styled:gsub('^%s+','')
   -- (1) function title
   styled = '\n' .. style.banner .. '\n' .. styled
   styled = styled:gsub('<a.-id=".-">%s+(.-)</a>%s*', function(title) return style.title .. title .. style.none .. '\n' end)
   -- (2) lists
   styled = styled:gsub('<ul>(.-)</ul>', function(list) 
                                            return list:gsub('<li>%s*(.-)%s*</li>%s*', style.list .. '%1\n')
                                         end)
   -- (3) code
   styled = styled:gsub('%s*<code>%s*(.-)%s*</code>%s*', style.code .. ' %1 ' .. style.none)
   -- (4) pre
   styled = styled:gsub('<pre.->(.-)</pre>', style.pre .. '%1' .. style.none)
   -- (5) formatting
   styled = styled:gsub('<em>(.-)</em>', style.em .. '%1' .. style.none)
   styled = styled:gsub('<b>(.-)</b>', style.bold .. '%1' .. style.none)
   -- (6) links
   styled = styled:gsub('<a.->(.-)</a>', style.none .. '%1' .. style.none)
   -- (7) images
   styled = styled:gsub('<img.-src="(.-)".->%s*', 
                         style.img .. 'image: file://' 
                         .. paths.concat(paths.install_dokmedia,package,'%1')
                         .. style.none .. '\n')
   -- (-) paragraphs
   styled = styled:gsub('<p>', '')
   -- (-) special chars
   styled = uncleanText(styled)
   -- (-) max columns
   styled = maxcols(styled)
   -- (-) conclude
   styled = styled:gsub('%s*$','')
   --styled = styled .. '\n' .. style.banner
   return styled
end

local function adddok(...)
   local tt = {}
   local arg = {...}
   for i=1,#arg do
      table.insert(tt,arg[i])
   end
   return table.concat(tt,'\n')
end
function dok.html2funcs(html, package)
   local funcs = {}
   local next = html:gfind('<div class="level%d%s.-".->\n<h%d>(<a.-id=".-">.-</a>)%s*</h%d>(.-)</div>')
   for title,body in next do
      for func in body:gfind('<a name="' .. package .. '%.(.-)">.-</a>') do
         if func then
            funcs[func] = adddok(funcs[func],dok.stylize(title .. '\n' .. body:gsub('<a.-name="(.-)"></a>','') , package))
         end
      end
   end
   return funcs
end

function dok.refresh()
   for package in paths.files(paths.install_dok) do
      if package ~= '.' and package ~= '..' and _G[package] then
         local dir = paths.concat(paths.install_dok, package)
         for file in paths.files(dir) do
            if file ~= '.' and file ~= '..' then
               local path = paths.concat(dir, file)
               local f = io.open(path)
               if f then
                  local content = f:read('*all')
                  local html = dok.dok2html(content)
                  local funcs = dok.html2funcs(html, package)
                  local pkg = _G[package]
                  if type(pkg) ~= 'table' and _G._torchimport then 
                     -- unsafe import, use protected import
                     pkg = _G._torchimport[package]
                  end
                  if pkg and type(pkg) == 'table' then
                     -- level 0: the package itself
                     dok.inline[pkg] = dok.inline[pkg] or funcs['dok'] or funcs['reference.dok'] or funcs['overview.dok']
                     -- next levels
                     for key,symb in pairs(pkg) do
                        -- level 1: global functions and objects
                        local entry = (key):lower()
                        if funcs[entry] or funcs[entry..'.dok'] then
                           local sym = string2symbol(package .. '.' .. key)
                           dok.inline[sym] = adddok(funcs[entry..'.dok'],funcs[entry])
                        end
                        -- level 2: objects' methods
                        if type(pkg[key]) == 'table' then
                           local entries = {}
                           for k,v in pairs(pkg[key]) do
                              entries[k] = v
                           end
                           local mt = getmetatable(pkg[key]) or {}
                           for k,v in pairs(mt) do
                              entries[k] = v
                           end
                           for subkey,subsymb in pairs(entries) do
                              local entry = (key .. '.' .. subkey):lower()
                              if funcs[entry] or funcs[entry..'.dok'] then
                                 local sym = string2symbol(package .. '.' .. key .. '.' .. subkey)
                                 dok.inline[sym] = adddok(funcs[entry..'.dok'],funcs[entry])
                                 --dok.inline[string2symbol(package .. '.' .. key .. '.' .. subkey)] = funcs[entry]
                              end
                           end
                        end
                     end
                  end
               end
            end
         end
      end
   end
end

--------------------------------------------------------------------------------
-- help() is the main user-side function: prints help for any given
-- symbol that has an anchor defined in a .dok file.
--------------------------------------------------------------------------------
function dok.help(symbol, asstring)
   -- color detect
   if qtide then
      dok.dontusecolors()
   else
      dok.usecolors()
   end
   -- no symbol? global help
   if not symbol then
      print(style.banner)
      print(style.title .. 'help(symbol)' .. style.none 
            .. '\n\nget inline help on a specific symbol\n'
            .. '\nto browse the complete html documentation, call: '
            .. style.title .. 'browse()' .. style.none)
      print(style.banner)
      return
   end
   -- always refresh (takes time, but insures that 
   -- we generate help for all packages loaded)
   dok.refresh()
   if type(symbol) == 'string' then
      symbol = string2symbol(symbol)
   end
   local inline = dok.inline[symbol]
   if asstring then
      return inline
   else
      if inline then
         --print(style.banner)
         print(inline)
         print(style.banner)
      else
         if type(symbol) == 'function' or type(symbol) == 'table' then
            pcall(symbol)
         else
            print('undocumented symbol')
         end
      end
   end
end

help = dok.help

--------------------------------------------------------------------------------
-- browse() is a simpler function that simply triggers a browser
--------------------------------------------------------------------------------
function dok.browse()
   -- color detect
   if qtide then
      dok.dontusecolors()
   else
      dok.usecolors()
   end
   -- trigger browser
   require 'qtide'
   qtide.help()
   package.loaded.qtide = false
   qtide = nil
end

browse = dok.browse

--------------------------------------------------------------------------------
-- standard usage function: used to display automated help for functions
--
-- @param funcname     function name
-- @param description  description of the function
-- @param example      usage example
-- @param ...          [optional] arguments
--------------------------------------------------------------------------------
function dok.usage(funcname, description, example, ...)
   local str = ''

   local help = help(string2symbol(funcname), true)
   if help then
      str = str .. help
   else
      str = str .. style.banner .. '\n'
      str = str .. style.title .. funcname .. style.none .. '\n'
      if description then
         str = str .. '\n' .. description .. '\n'
      end
   end

   str = str .. '\n' .. style.list .. 'usage:\n' .. style.pre

   -- named arguments:
   local args = {...}
   if args[1].tabled then
      args = args[1].tabled 
   end
   if args[1].arg then
      str = str .. funcname .. '{\n'
      for i,param in ipairs(args) do
         local key
         if param.req then
            key = '    ' .. param.arg .. ' = ' .. param.type
         else
            key = '    [' .. param.arg .. ' = ' .. param.type .. ']'
         end
         -- align:
         while key:len() < 40 do
            key = key .. ' '
         end
         str = str .. key .. '-- ' .. param.help 
         if param.default or param.default == false then
            str = str .. '  [default = ' .. tostring(param.default) .. ']'
         elseif param.defaulta then
            str = str .. '  [default == ' .. param.defaulta .. ']'
         end
         str = str.. '\n'
      end
      str = str .. '}\n'

   -- unnamed args:
   else
      local idx = 1
      while true do
         local param
         str = str .. funcname .. '(\n'
         while true do
            param = args[idx]
            idx = idx + 1
            if not param or param == '' then break end
            local key
            if param.req then
               key = '    ' .. param.type
            else
               key = '    [' .. param.type .. ']'
            end
            -- align:
            while key:len() < 40 do
               key = key .. ' '
            end
            str = str .. key .. '-- ' .. param.help .. '\n'
         end
         str = str .. ')\n'
         if not param then break end
      end
   end
   str = str .. style.none

   if example then
      str = str .. '\n' .. style.pre .. example .. style.none .. '\n'
   end

   str = str .. style.banner
   return str
end

--------------------------------------------------------------------------------
-- standard argument function: used to handle named arguments, and 
-- display automated help for functions
--------------------------------------------------------------------------------
function dok.unpack(args, funcname, description, ...)
   -- put args in table
   local defs = {...}

   -- generate usage string as a closure:
   -- this way the function only gets called when an error occurs
   local fusage = function() 
                     local example
                     if #defs > 1 then
                        example = funcname .. '{' .. defs[2].arg .. '=' .. defs[2].type .. ', '
                           .. defs[1].arg .. '=' .. defs[1].type .. '}\n'
                        example = example .. funcname .. '(' .. defs[1].type .. ',' .. ' ...)'
                     end
                     return dok.usage(funcname, description, example, {tabled=defs})
                  end
   local usage = {}
   --setmetatable(usage, {__tostring=fusage})

   -- get args
   local iargs = {}
   if #args == 0 then 
      print(usage)
      error('error')
   elseif #args == 1 and type(args[1]) == 'table' and #args[1] == 0 
                     and not (torch and torch.typename(args[1]) ~= nil) then
      -- named args
      iargs = args[1]
   else
      -- ordered args
      for i = 1,select('#',...) do
         iargs[defs[i].arg] = args[i]
      end
   end

   -- check/set arguments
   local dargs = {}
   for i = 1,#defs do
      local def = defs[i]
      -- is value requested ?
      if def.req and iargs[def.arg] == nil then
         print(style.error .. 'missing argument: ' .. def.arg .. style.none)
         print(usage)
         error('error')
      end
      -- get value or default
      dargs[def.arg] = iargs[def.arg]
      if dargs[def.arg] == nil then
         dargs[def.arg] = def.default
      end
      if dargs[def.arg] == nil and def.defaulta then
         dargs[def.arg] = dargs[def.defaulta]
      end
      dargs[i] = dargs[def.arg]
   end

   -- return usage too
   dargs.usage = usage

   -- stupid lua bug: we return all args by hand
   if dargs[65] then
      error('<dok.unpack> oups, cant deal with more than 64 arguments :-)')
   end

   -- return modified args
   return dargs,
   dargs[1], dargs[2], dargs[3], dargs[4], dargs[5], dargs[6], dargs[7], dargs[8], 
   dargs[9], dargs[10], dargs[11], dargs[12], dargs[13], dargs[14], dargs[15], dargs[16],
   dargs[17], dargs[18], dargs[19], dargs[20], dargs[21], dargs[22], dargs[23], dargs[24],
   dargs[25], dargs[26], dargs[27], dargs[28], dargs[29], dargs[30], dargs[31], dargs[32],
   dargs[33], dargs[34], dargs[35], dargs[36], dargs[37], dargs[38], dargs[39], dargs[40],
   dargs[41], dargs[42], dargs[43], dargs[44], dargs[45], dargs[46], dargs[47], dargs[48],
   dargs[49], dargs[50], dargs[51], dargs[52], dargs[53], dargs[54], dargs[55], dargs[56],
   dargs[57], dargs[58], dargs[59], dargs[60], dargs[61], dargs[62], dargs[63], dargs[64]
end

--------------------------------------------------------------------------------
-- prints an error with nice formatting. If domain is provided, it is used as
-- following: <domain> msg
--------------------------------------------------------------------------------
function dok.error(message, domain)
   if domain then
      message = '<' .. domain .. '> ' .. message
   end
   local col_msg = style.error .. tostring(message) .. style.none
   error(col_msg)
end
