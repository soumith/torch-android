
-- We are using paths.require to appease mkl
paths = {
  require = require
}

--- package stuff
function torch.packageLuaPath(name)
   if not name then
      local ret = string.match(torch.packageLuaPath('torch'), '(.*)/')
       if not ret then --windows?
           ret = string.match(torch.packageLuaPath('torch'), '(.*)\\')
       end
       return ret 
   end
   for path in string.gmatch(package.path, "(.-);") do
      path = string.gsub(path, "%?", name)
      local f = io.open(path)
      if f then
         f:close()
         local ret = string.match(path, "(.*)/")
         if not ret then --windows?
             ret = string.match(path, "(.*)\\")
         end
         return ret
      end
   end
end

function torch.include(package, file)
   local req = package .. '.' .. file:gsub('.lua$','')
   require(req)
end

function torch.class(tname, parenttname)

   local function constructor(...)
      local self = {}
      torch.setmetatable(self, tname)
      if self.__init then
         self:__init(...)
      end
      return self
   end
   
   local function factory()
      local self = {}
      torch.setmetatable(self, tname)
      return self
   end

   local mt = torch.newmetatable(tname, parenttname, constructor, nil, factory)
   local mpt
   if parenttname then
      mpt = torch.getmetatable(parenttname)
   end
   return mt, mpt
end

function torch.setdefaulttensortype(typename)
   assert(type(typename) == 'string', 'string expected')
   if torch.getconstructortable(typename) then
      torch.Tensor = torch.getconstructortable(typename)
      torch.Storage = torch.getconstructortable(torch.typename(torch.Tensor(1):storage()))
   else
      error(string.format("<%s> is not a string describing a torch object", typename))
   end
end

torch.setdefaulttensortype('torch.FloatTensor')

torch.include('torch','Tensor.lua')
torch.include('torch','File.lua')
torch.include('torch','CmdLine.lua')
torch.include('torch','Tester.lua')

return torch
