dok = {}

if torch then
   torch.include('dok', 'inline.lua')
   torch.include('dok', 'search.lua')
end

dok.options = {}

function table.last(t)
   return t[#t]
end

function string.strip(str)
   return str:gsub('^%s+', ''):gsub('%s+$', '')
end

function dok.dirapply(dir, func)
   for entry in paths.files(dir) do
      local filename = dir .. '/' .. entry
      if entry == '.' or entry == '..' then
      elseif paths.dirp(filename) then
         dok.dirapply(filename, func)
      else
         func(dir, entry)
      end
   end
end

function dok.sectionapply(section, func, isroot)
   func(section, isroot)
   if #section.subsections > 0 then
      for _,subsection in ipairs(section.subsections) do
         dok.sectionapply(subsection, func)
      end
   end
end

function dok.link2wikilink(txt)
   txt = txt:strip()
   if txt == '.' or txt == '..' then
      return txt
   end
   txt = txt:gsub('[^%w%.%-_]', '_')
   txt = txt:gsub('[_]+', '_')
   txt = txt:gsub('^[%.%-_]+', '')
   txt = txt:gsub('[%.%-_]+$', '')
   txt = txt:lower() -- really respect dokuwiki
   return txt
end

function dok.parseSection(txt)
   local root = {rank=0, title='', lines={}, subsections={}, anchors={}}
   local section = root
   for line in txt:gmatch('.-\n') do
      local rank, title = line:match('^=([=]+)(.*)=[=]+%s-$')
      if title then
         rank = math.max(1, 6-#rank)
         title = title:gsub('[=]+$', '')

         while rank < section.rank do
            section = section.parent
         end
         
         if rank > section.rank then
            table.insert(section.subsections, {rank=rank, title=title, lines={}, subsections={}, anchors={}, parent=section})
            section = table.last(section.subsections)
         elseif rank == section.rank then
            table.insert(section.parent.subsections, {rank=rank, title=title, lines={}, subsections={}, anchors={}, parent=section.parent})
            section = table.last(section.parent.subsections)
         end
      else
         table.insert(section.lines, line)
         line:gsub('%{%{anchor%:(.-)%}%}', function(str)
                                              table.insert(section.anchors, str)
                                           end)
      end
   end
   return root
end

function dok.pictURL(txt)
   txt = txt:match('^[^|%?]+')
   txt = txt:gsub('^%s+', '')
   txt = txt:gsub('%s+$', '')
   return txt
end

function dok.pictAlt(txt)
   txt = txt:gsub('^[^|]+', '', 1)
   txt = txt:gsub('^|', '', 1)
   txt = dok.cleanText(txt)
   return txt
end

function dok.pictClass(txt)
   txt = txt:match('^[^|]+')
   local right = txt:match('^%s+')
   local left  = txt:match('%s+$')

   if left and right then
      return 'img-center'
   elseif left then
      return 'img-left'
   elseif right then
      return 'img-right'
   else
      return 'img'
   end

   return txt
end

function dok.pictWidth(txt)
   txt = txt:match('^[^|]+')
   txt = txt:gsub('^[^%?]+', '', 1)
   txt = txt:gsub('^%?', '', 1)
   txt = txt:gsub('%s+$', '', 1)
   txt = txt:match('^[^x]+')
   return txt
end

function dok.pictHeight(txt)
   txt = txt:match('^[^|]+')
   txt = txt:gsub('^[^%?]+', '', 1)
   txt = txt:gsub('^%?', '', 1)
   txt = txt:gsub('%s+$', '', 1)
   txt = txt:gsub('^[^x]+', '', 1)
   txt = txt:gsub('^x', '', 1)

   if txt == '' then
      return nil
   else
      return txt
   end
end

function dok.linkText(lnk)
   local txt = lnk:gsub('^[^|]+', '', 1)
   txt = txt:gsub('^|', '', 1)
   if txt:match('^%s-$') then
      local result,_,file,section = dok.linkURL(lnk)
      if section then
         return section
      elseif file then
         return file
      else
         return result
      end
   else
      txt = dok.cleanText(txt)
      return txt
   end
end

function dok.linkURL(txt)
   -- Global URL? (check below for same thing)
   txt = txt:match('^[^|]+')
   if not txt or txt:match('^%s-$') then -- you never know with 'those' guys...
      return 'index.html'
   end
   txt = string.strip(txt)
   if txt:match('^https?://%S+$') then
      return txt
   elseif txt:match('^www%.%S+%.%S+$') then
      return 'http://' .. txt
   else
      -- ok so we have a wiki url to deal with path/file#section
      txt = txt:gsub('^%.%.', '\002:'):gsub('^%.', '%.:'):gsub('::', ':'):gsub('\002', '%.%.')
      local path = txt:match('^.*%:') or '.:'
      txt = txt:gsub('^.*%:', '')
      local section = txt:match('#(.*)$')
      local file
      if txt:match('^#.*$') then
         if path == '.:' then
            file = '' -- the current page
         else
            file = 'index.html' -- the default index page at the given path
         end
      else
         file = txt:gsub('#.*$', ''):gsub('^%s-$', 'index')
         file = dok.link2wikilink(file) .. '.html'
      end

      path = path:gsub('^([^%.%:])', ':%1'):gsub(':', '/'):gsub('/+', '/'):gsub('^%./', '')
      path = path:gsub('[^/]+', dok.link2wikilink)
      if path:match('^%s-$') then
         path = nil
      end

      if section then
         section = dok.link2wikilink(section)
      end

      local result = ''
      if path then result = result .. path end
      if file then result = result .. file end
      if section then result = result .. '#' .. section end

      return result, path, file, section
   end
end

-- clean characters which could interfer with dokuwiki syntax
-- see http://www.ascii.cl/htmlcodes.htm
-- wizard level to modify that
function dok.cleanText(txt)
   txt = txt:gsub("'", '&#39;')
   txt = txt:gsub('%*', '&#42;')
   txt = txt:gsub('%+', '&#43;')
   txt = txt:gsub('<', '&lt;')
   txt = txt:gsub('>', '&gt;')
   return txt
end

function dok.dok2html(txt)
   local link = {}
   local anch = {}
   local code = {}
   local file = {}
   local lang = {}
   local pict = {}
   local foot = {}
   local issection = false;

   txt = txt .. '\n' -- blank line to close lists and tabulars

   txt = txt:gsub('<code>(.-)</code>', function(str)
                                          table.insert(code, str)
                                          return '\018' .. #code .. '\018'
                                       end)

   -- note: we do not support code-specific colorization yet
   txt = txt:gsub('<file%s-(%w-)%s->(.-)</file>', function(langstr,str)
                                          table.insert(file, str)
                                          table.insert(lang,langstr)
                                          return '\019' .. #file .. '\019'
                                       end)

   -- parse sections (no fancy stuff allowed)
   local stack = {}
   local first = true
   txt = txt:gsub('.-\n', function(line)
                             local rank, title = line:match('^=([=]+)(.*)=[=]+%s-$')
                             if title then
                                local closesection
                                if issection then
                                   closesection = '\n\002/div\003\n'
                                else
                                   closesection = ''
                                   issection = true
                                end
                                rank = math.max(1, 6-#rank)
                                title = title:gsub('[=]+$', '')
                                local classes = ''
                                if stack[rank-1] then
                                   for i = 1,rank-1 do
                                      if stack[i] then
                                         classes = classes .. ' ' .. stack[i]
                                      end
                                   end
                                end
                                if first then
                                   classes = classes .. ' ' .. 'topdiv'
                                end
                                first = false
                                stack[rank] = 'par_' .. dok.link2wikilink(title):gsub('%.','-')
                                return closesection .. '\002div class="level' .. rank .. classes .. '" id="div_' .. dok.link2wikilink(title):gsub('%.','-') .. '"\003\n' .. '\002h' .. rank .. '\003\002a id="' .. dok.link2wikilink(title) .. '"\003' .. dok.cleanText(title) .. '\002/a\003\002/h' .. rank .. '\003\002a name="mybogusanchor"\003\002/a\003\n'
                             else
                                return line
                             end
                          end)

   txt = txt:gsub('%{%{anchor%:(.-)%}%}', function(str)
                                             table.insert(anch, str)
                                             return '\021' .. #anch .. '\021'
                                          end)

   txt = txt:gsub('%{%{(.-)%}%}', function(str)
                                     table.insert(pict, str)
                                     return '\020' .. #pict .. '\020'
                                  end)

   txt = txt:gsub('%[%[(.-)%]%]', function(str)
                                     table.insert(link, str)
                                     return '\017' .. #link .. '\017'
                                  end)

   -- Global URL? (check above for same thing)
   txt = txt:gsub('https?://%S+', function(str)
                                     table.insert(link, str)
                                     return '\017' .. #link .. '\017'
                                  end)

   txt = txt:gsub('www%.%S+%.[%w#]+', function(str)
                                      table.insert(link, str)
                                      return '\017' .. #link .. '\017'
                                   end)
   
   txt = txt:gsub('%(%((.-)%)%)', function(str)
                                     table.insert(foot, str)
                                     return '\002sup\003\002a href="#footnote-' .. #foot .. '"\003' .. #foot .. ')\002/a\003\002/sup\003'
                                  end)

   txt = txt:gsub('%*%*(.-)%*%*', '\002b\003%1\002/b\003')
   txt = txt:gsub('%/%/(.-)%/%/', '\002em\003%1\002/em\003')
   txt = txt:gsub("%'%'(.-)%'%'", '\002code\003%1\002/code\003')
   txt = txt:gsub("__(.-)__", '\002u\003%1\002/u\003')
   txt = txt:gsub('<sub>', '\002sub\003')
   txt = txt:gsub('</sub>', '\002/sub\003')
   txt = txt:gsub('<sup>', '\002sup\003')
   txt = txt:gsub('</sup>', '\002/sup\003')
   txt = txt:gsub('<del>', '\002del\003')
   txt = txt:gsub('</del>', '\002/del\003')
   txt = txt:gsub('(%S)\\\\(%s)', '%1\002br\003%2')
   txt = txt:gsub('\n%s-\n', '\n\002p\003\n')
   txt = txt:gsub('\n%-%-%-[%-]+%s-\n', '\n\002hr\003\n')

   -- parse lists
   local list = {rank=0}
   txt = txt:gsub('.-\n', function(line)
                             local result = ""
                             if line:match('^%s%s+[%*%-]') then
                                local rank = #line:match('^%s(%s+)')
                                local type = line:match('^%s%s+([%*%-])')
                                
                                while rank < list.rank do
                                   if list.type == '*' then
                                      result = result .. '\002/ul\003\n'
                                   else
                                      result = result .. '\002/ol\003\n'
                                   end
                                   list = list.parent
                                end

                                if rank == list.rank and type ~= list.type then
                                   if list.type == '*' then
                                      result = result .. '\002/ul\003\n'
                                   else
                                      result = result .. '\002/ol\003\n'
                                   end
                                   if type == '*' then
                                      result = result .. '\002ul\003\n'
                                   else
                                      result = result .. '\002ol\003\n'
                                   end
                                   list.type = type
                                end

                                if rank > list.rank then
                                   list = {rank=rank, type=type, parent=list}
                                   if list.type == '*' then
                                      result = result .. '\002ul\003\n'
                                   else
                                      result = result .. '\002ol\003\n'
                                   end
                                end

                                result = result .. '\002li\003' .. line:gsub('^%s%s+[%*%-]', '', 1) .. '\002/li\003\n'

                             else
                                while list.rank > 0 do
                                   if list.type == '*' then
                                      result = result .. '\002/ul\003\n'
                                   else
                                      result = result .. '\002/ol\003\n'
                                   end
                                   list = list.parent
                                end
                                result = result .. line
                             end
                             return result
                          end)

   -- parse tabulars
   -- note: force to parse the complete tabular to handle row spans
   local tabular = nil
   txt = txt:gsub('.-\n', function(line)
                             local result = ''
                             if line:match('^[%^|]') then
                                local type = line:match('^[%^|]')
                                if not tabular then
                                   tabular = {}
                                end
                                local row = {}
                                for data, nexttype in line:gmatch('([^%^|]+)([%^|]+)') do
                                   local cell = {}
                                   if type == '^' then
                                      cell.type = 'th'
                                   else
                                      cell.type = 'td'
                                   end
                                   if data:match('^%s%s') and data:match('%s%s$') then
                                      cell.class = 'cell-center'
                                   elseif data:match('^%s%s') then
                                      cell.class = 'cell-right'
                                   else
                                      cell.class = 'cell-left'
                                   end
                                   if data:match('^%s-%:%:%:%s-$') then
                                      local n = #row+1
                                      for i=#tabular,1,-1 do
                                         if tabular[i] and tabular[i][n].data then
                                            tabular[i][n].rowspan = tabular[i][n].rowspan or 1
                                            tabular[i][n].rowspan = tabular[i][n].rowspan + 1
                                            break
                                         end
                                      end
                                      table.insert(row, {})
                                   else
                                      cell.data = data
                                      table.insert(row, cell)
                                   end
                                   if #nexttype > 1 then
                                      cell.colspan = #nexttype
                                      for i=1,#nexttype-1 do
                                         table.insert(row, {})
                                      end
                                   end
                                   type = nexttype:match('[%^|]$')
                                end
                                table.insert(tabular, row)
                             else
                                if tabular then
                                   result = '\002table class="inline"\003\n'
                                   for i=1,#tabular do
                                      local row = tabular[i]
                                      result = result .. '\002tr\003'
                                      for j=1,#row do
                                         local cell = row[j]
                                         if cell.data then
                                            result = result .. '\002'  .. cell.type .. ' class="' .. cell.class .. '"'
                                            if cell.colspan then
                                               result = result .. ' colspan="' .. cell.colspan .. '"'
                                            end
                                            if cell.rowspan then
                                               result = result .. ' rowspan="' .. cell.rowspan .. '"'
                                            end
                                            result = result .. '\003' .. cell.data
                                            result = result .. '\002/' .. cell.type .. '\003'
                                         end
                                      end
                                      result = result .. '\002/tr\003\n'
                                   end
                                   result = result .. '\002/table\003\n' .. line
                                   tabular = nil
                                else
                                   result = line
                                end
                             end
                             return result
                          end)

   txt = dok.cleanText(txt)

   -- close section
   if issection then
      txt = txt .. '\n\002/div\003\n'
   end

   txt = txt:gsub('\002', '<')
   txt = txt:gsub('\003', '>')

   -- add footnotes
   if #foot > 0 then
      txt = txt .. '<div class="footnotes">'
      for id,footnote in ipairs(foot) do
         txt = txt .. '<div class="footnote"><sup><a name="footnote-' .. id .. '">' .. id .. ')</a></sup>&nbsp;' .. footnote .. '</div>\n'
      end
      txt = txt .. '</div>'
   end

   -- put back links
   txt = txt:gsub('\017(%d+)\017', function(id)
                                      id = tonumber(id)
                                      return '<a href="' .. dok.linkURL(link[id]) ..  '" class="anchor">' .. dok.linkText(link[id]) .. '</a>'
                                   end)

   -- put back anchors
   txt = txt:gsub('\021(%d+)\021', function(id)
                                      id = tonumber(id)
                                      return '<a name="' .. dok.link2wikilink(anch[id]) .. '"></a>'
                                   end)

   -- put back pictures (note: after the links!)
   txt = txt:gsub('\020(%d+)\020', function(id)
                                      id = tonumber(id)
                                      local result = '<img src="' .. dok.pictURL(pict[id]) .. '" alt="' .. dok.pictAlt(pict[id]) .. '" class="' .. dok.pictClass(pict[id]) .. '"'
                                      if dok.pictWidth(pict[id]) then
                                         result = result .. ' width="' .. dok.pictWidth(pict[id]) .. '"'
                                      end
                                      if dok.pictHeight(pict[id]) then
                                         result = result .. ' height="' .. dok.pictHeight(pict[id]) .. '"'
                                      end
                                      result = result .. '>'
                                      return result
                                   end)

   -- put back code
   txt = txt:gsub('\018(%d+)\018', function(id)
                                      id = tonumber(id) -- code
                                      return '<pre class="brush: lua;">' .. code[id] .. '</pre>'
                                   end)
   txt = txt:gsub('\019(%d+)\019', function(id)
                                      id = tonumber(id) -- file
                                      local lng = lang[id] or 'lua'
                                      return '<pre class="brush: ' .. lng .. ';">' .. file[id] .. '</pre>'
                                   end)

   return txt
end
