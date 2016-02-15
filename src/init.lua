require 'torch'
require 'libtorchandroid'

function torch.load(filename, mode, referenced)
   assert(mode == nil or mode == 'binary' or mode == 'ascii' 
	     or mode == 'apkbinary32' or mode == 'apkbinary64' or mode == 'apkascii', 
	  '"binary" or "ascii" or "apkbinary32" or "apkbinary64" or "apkascii" (or nil) expected for mode')
   assert(referenced == nil or referenced == true or referenced == false, 
	  'true or false (or nil) expected for referenced')
   mode = mode or 'apkbinary'
   referenced = referenced == nil and true or referenced
   local file 
   if mode == 'apkbinary32' or mode == 'apkbinary64' or mode == 'apkascii' then
      file = torch.ApkFile(filename, 'r')
      mode = mode:sub(4)
      if mode == 'binary32' then
	 file:longSize(4)
	 mode = mode:sub(1, #mode-2)
      elseif mode == 'binary64' then
	 file:longSize(8)
	 mode = mode:sub(1, #mode-2)
      end
   else
      file = torch.DiskFile(filename, 'r')
   end
   file[mode](file)
   file:referenced(referenced)
   local object = file:readObject()
   file:close()
   return object
end
