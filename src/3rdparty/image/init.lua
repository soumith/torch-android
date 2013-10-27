----------------------------------------------------------------------
--
-- Copyright (c) 2011 Ronan Collobert, Clement Farabet
-- 
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
-- 
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
-- LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
-- OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
-- WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-- 
----------------------------------------------------------------------
-- description:
--     image - an image toolBox, for Torch
--
-- history: 
--     July  1, 2011, 7:42PM - import from Torch5 - Clement Farabet
----------------------------------------------------------------------

----------------------------------------------------------------------
-- types lookups
-- 
local type2tensor = {
   float = torch.FloatTensor(),
   double = torch.DoubleTensor(),
   byte = torch.ByteTensor(),
}
local template = function(type)
   if type then
      return type2tensor[type]
   else
      return torch.Tensor()
   end
end

----------------------------------------------------------------------
-- save/load in multiple formats
--
local function loadPNG(filename, depth, tensortype)
   if not xlua.require 'libpng' then
      dok.error('libpng package not found, please install libpng','image.loadPNG')
   end
   local MAXVAL = 255
   local a = template(tensortype).libpng.load(filename)
   if tensortype ~= 'byte' then
      a:mul(1/MAXVAL)
   end
   if depth and depth == 1 then
      if a:nDimension() == 2 then
         -- all good
      elseif a:size(1) == 3 or a:size(1) == 4 then
         a = image.rgb2y(a:narrow(1,1,3))[1]
      elseif a:size(1) == 2 then
         a = a:narrow(1,1,1)
      elseif a:size(1) ~= 1 then
         dok.error('image loaded has wrong #channels','image.loadPNG')
      end
   elseif depth and depth == 3 then
      if a:size(1) == 3 then
         -- all good
      elseif a:size(1) == 4 then
         a = a:narrow(1,1,3)
      else
         dok.error('image loaded has wrong #channels','image.loadPNG')
      end
   end
   return a
end
rawset(image, 'loadPNG', loadPNG)

local function savePNG(filename, tensor)
   if not xlua.require 'libpng' then
      dok.error('libpng package not found, please install libpng','image.savePNG')
   end
   local MAXVAL = 255
   local a = torch.Tensor():resize(tensor:size()):copy(tensor)
   a.image.saturate(a) -- bound btwn 0 and 1
   a:mul(MAXVAL)       -- remap to [0..255]
   a.libpng.save(filename, a)
end  
rawset(image, 'savePNG', savePNG)

function image.getPNGsize(filename)
   if not xlua.require 'libpng' then
      dok.error('libpng package not found, please install libpng','image.getPNGsize')
   end
   return torch.Tensor().libpng.size(filename)
end

local function loadJPG(filename, depth, tensortype)
   if not xlua.require 'libjpeg' then
      dok.error('libjpeg package not found, please install libjpeg','image.loadJPG')
   end
   local MAXVAL = 255
   local a = template(tensortype).libjpeg.load(filename)
   if tensortype ~= 'byte' then
      a:mul(1/MAXVAL)
   end
   if depth and depth == 1 then
      if a:nDimension() == 2 then
         -- all good
      elseif a:size(1) == 3 or a:size(1) == 4 then
         a = image.rgb2y(a:narrow(1,1,3))[1]
      elseif a:size(1) == 2 then
         a = a:narrow(1,1,1)
      elseif a:size(1) ~= 1 then
         dok.error('image loaded has wrong #channels','image.loadJPG')
      end
   elseif depth and depth == 3 then
      if a:size(1) == 3 then
         -- all good
      elseif a:size(1) == 4 then
         a = a:narrow(1,1,3)
      else
         dok.error('image loaded has wrong #channels','image.loadJPG')
      end
   end
   return a
end
rawset(image, 'loadJPG', loadJPG)

local function saveJPG(filename, tensor)
   if not xlua.require 'libjpeg' then
      dok.error('libjpeg package not found, please install libjpeg','image.saveJPG')
   end
   local MAXVAL = 255
   local a = torch.Tensor():resize(tensor:size()):copy(tensor)
   a.image.saturate(a) -- bound btwn 0 and 1
   a:mul(MAXVAL)       -- remap to [0..255]
   a.libjpeg.save(filename, a)
end
rawset(image, 'saveJPG', saveJPG)

function image.getJPGsize(filename)
   if not xlua.require 'libjpeg' then
      dok.error('libjpeg package not found, please install libjpeg','image.getJPGsize')
   end
   return torch.Tensor().libjpeg.size(filename)
end

local function loadPPM(filename, depth, tensortype)
   require 'libppm'
   local MAXVAL = 255
   local a = template(tensortype).libppm.load(filename)
   if tensortype ~= 'byte' then
      a:mul(1/MAXVAL)
   end
   if depth and depth == 1 then
      if a:nDimension() == 2 then
         -- all good
      elseif a:size(1) == 3 or a:size(1) == 4 then
         a = image.rgb2y(a:narrow(1,1,3))[1]
      elseif a:size(1) == 2 then
         a = a:narrow(1,1,1)
      elseif a:size(1) ~= 1 then
         dok.error('image loaded has wrong #channels','image.loadPPM')
      end
   elseif depth and depth == 3 then
      if a:size(1) == 3 then
         -- all good
      elseif a:size(1) == 4 then
         a = a:narrow(1,1,3)
      else
         dok.error('image loaded has wrong #channels','image.loadPPM')
      end
   end
   return a
end
rawset(image, 'loadPPM', loadPPM)

local function savePPM(filename, tensor)
   require 'libppm'
   if tensor:nDimension() ~= 3 or tensor:size(1) ~= 3 then
      dok.error('can only save 3xHxW images as PPM', 'image.savePPM')
   end
   local MAXVAL = 255
   local a = torch.Tensor():resize(tensor:size()):copy(tensor)
   a.image.saturate(a) -- bound btwn 0 and 1
   a:mul(MAXVAL)       -- remap to [0..255]
   a.libppm.save(filename, a)
end
rawset(image, 'savePPM', savePPM)

local function savePGM(filename, tensor)
   require 'libppm'
   if tensor:nDimension() == 3 and tensor:size(1) ~= 1 then
      dok.error('can only save 1xHxW or HxW images as PGM', 'image.savePGM')
   end
   local MAXVAL = 255
   local a = torch.Tensor():resize(tensor:size()):copy(tensor)
   a.image.saturate(a) -- bound btwn 0 and 1
   a:mul(MAXVAL)       -- remap to [0..255]
   a.libppm.save(filename, a)
end
rawset(image, 'savePGM', savePGM)

local filetypes = {
   jpg = {loader = image.loadJPG, saver = image.saveJPG},
   png = {loader = image.loadPNG, saver = image.savePNG},
   ppm = {loader = image.loadPPM, saver = image.savePPM},
   pgm = {loader = image.loadPGM, saver = image.savePGM}
}

filetypes['JPG']  = filetypes['jpg']
filetypes['JPEG'] = filetypes['jpg']
filetypes['jpeg'] = filetypes['jpg']
filetypes['PNG']  = filetypes['png']
filetypes['PPM']  = filetypes['ppm']
filetypes['PGM']  = filetypes['pgm']
rawset(image, 'supported_filetypes', filetypes)

local function is_supported(suffix)
   return filetypes[suffix] ~= nil
end
rawset(image, 'is_supported', is_supported)

local function load(filename, depth, tensortype)
   if not filename then
      print(dok.usage('image.load',
                       'loads an image into a torch.Tensor', nil,
                       {type='string', help='path to file', req=true},
                       {type='number', help='force destination depth: 1 | 3'},
                       {type='string', help='type: byte | float | double'}))
      dok.error('missing file name', 'image.load')
   end
   local ext = string.match(filename,'%.(%a+)$')
   local tensor
   if image.is_supported(ext) then
      tensor = filetypes[ext].loader(filename, depth, tensortype)
   else
      dok.error('unknown image type: ' .. ext, 'image.load')
   end

   return tensor
end
rawset(image, 'load', load)

local function save(filename, tensor)
   if not filename or not tensor then
      print(dok.usage('image.save',
                       'saves a torch.Tensor to a disk', nil,
                       {type='string', help='path to file', req=true},
                       {type='torch.Tensor', help='tensor to save (NxHxW, N = 1 | 3)'}))
      dok.error('missing file name | tensor to save', 'image.save')
   end
   local ext = string.match(filename,'%.(%a+)$')
   if image.is_supported(ext) then
      tensor = filetypes[ext].saver(filename, tensor)
   else
      dok.error('unknown image type: ' .. ext, 'image.save')
   end
end
rawset(image, 'save', save)

----------------------------------------------------------------------
-- crop
--
local function crop(...)
   local dst,src,startx,starty,endx,endy
   local args = {...}
   if select('#',...) == 6 then
      dst = args[1]
      src = args[2]
      startx = args[3]
      starty = args[4]
      endx = args[5]
      endy = args[6]
   elseif select('#',...) == 5 then
      src = args[1]
      startx = args[2]
      starty = args[3]
      endx = args[4]
      endy = args[5]
   elseif select('#',...) == 4 then
      dst = args[1]
      src = args[2]
      startx = args[3]
      starty = args[4]
   elseif select('#',...) == 3 then
      src = args[1]
      startx = args[2]
      starty = args[3]
   else
      print(dok.usage('image.crop',
                       'crop an image', nil,
                       {type='torch.Tensor', help='input image', req=true},
                       {type='number', help='start x', req=true},
                       {type='number', help='start y', req=true},
                       {type='number', help='end x'},
                       {type='number', help='end y'},
                       '',
                       {type='torch.Tensor', help='destination', req=true},
                       {type='torch.Tensor', help='input image', req=true},
                       {type='number', help='start x', req=true},
                       {type='number', help='start y', req=true},
                       {type='number', help='end x'},
                       {type='number', help='end y'}))
      dok.error('incorrect arguments', 'image.crop')
   end
   if endx==nil then
      return src.image.cropNoScale(src,dst,startx,starty)
   else
      local depth=1
      local x
      if src:nDimension() > 2 then
         x = src.new(src:size(1),endy-starty,endx-startx)
      else
         x = src.new(endy-starty,endx-startx)
      end
      src.image.cropNoScale(src,x,startx,starty)
      dst = dst or src.new():resizeAs(x)
      image.scale(dst,x)
   end
   return dst
end
rawset(image, 'crop', crop)

----------------------------------------------------------------------
-- translate
--
local function translate(...)
   local dst,src,x,y
   local args = {...}
   if select('#',...) == 4 then
      dst = args[1]
      src = args[2]
      x = args[3]
      y = args[4]
   elseif select('#',...) == 3 then
      src = args[1]
      x = args[2]
      y = args[3]
   else
      print(dok.usage('image.translate',
                       'translate an image', nil,
                       {type='torch.Tensor', help='input image', req=true},
                       {type='number', help='horizontal translation', req=true},
                       {type='number', help='vertical translation', req=true},
                       '',
                       {type='torch.Tensor', help='destination', req=true},
                       {type='torch.Tensor', help='input image', req=true},
                       {type='number', help='horizontal translation', req=true},
                       {type='number', help='vertical translation', req=true}))
      dok.error('incorrect arguments', 'image.translate')
   end
   dst = dst or src.new()
   dst:resizeAs(src)
   dst:zero()
   src.image.translate(src,dst,x,y)
   return dst
end
rawset(image, 'translate', translate)

----------------------------------------------------------------------
-- scale
--
local function scale(...)
   local dst,src,width,height,mode,size
   local args = {...}
   if select('#',...) == 4 then
      src = args[1]
      width = args[2]
      height = args[3]
      mode = args[4]
   elseif select('#',...) == 3 then
      if type(args[3]) == 'string' then
         if type(args[2]) == 'string' or type(args[2]) == 'number' then
            src = args[1]
            size = args[2]
            mode = args[3]
         else
            dst = args[1]
            src = args[2]
            mode = args[3]
         end
      else
         src = args[1]
         width = args[2]
         height = args[3]
      end
   elseif select('#',...) == 2 then
      if type(args[2]) == 'string' or type(args[2]) == 'number' then
         src = args[1]
         size = args[2]
      else
         dst = args[1]
         src = args[2]
      end
   else
      print(dok.usage('image.scale',
                       'rescale an image (geometry)', nil,
                       {type='torch.Tensor', help='input image', req=true},
                       {type='number', help='destination width', req=true},
                       {type='number', help='destination height', req=true},
                       {type='string', help='mode: bilinear | simple', default='bilinear'},
                       '',
                       {type='torch.Tensor', help='input image', req=true},
                       {type='string | number', help='destination size: "WxH" or "MAX" or "^MIN" or MAX', req=true},
                       {type='string', help='mode: bilinear | simple', default='bilinear'},
                       '',
                       {type='torch.Tensor', help='destination image', req=true},
                       {type='torch.Tensor', help='input image', req=true},
                       {type='string', help='mode: bilinear | simple', default='bilinear'}))
      dok.error('incorrect arguments', 'image.scale')
   end
   if size then
      local iwidth,iheight
      if src:nDimension() == 3 then
         iwidth,iheight = src:size(3),src:size(2)
      else
         iwidth,iheight = src:size(2),src:size(1)
      end
      local imax = math.max(iwidth,iheight)
      local omax = tonumber(size)
      if omax then
         height = iheight / imax * omax
         width = iwidth / imax * omax
      else
         width,height = size:gfind('(%d*)x(%d*)')()
         if not width or not height then
            local imin = math.min(iwidth,iheight)
            local omin = size:gfind('%^(%d*)')()
            if omin then
               height = iheight / imin * omin
               width = iwidth / imin * omin
            end
         end
      end
   end
   if not dst and (not width or not height) then
      dok.error('could not find valid dest size', 'image.scale')
   end
   if not dst then
      if src:nDimension() == 3 then
         dst = src.new(src:size(1), height, width)
      else
         dst = src.new(height, width)
      end
   end
   mode = mode or 'bilinear'
   if mode=='bilinear' then
      src.image.scaleBilinear(src,dst)
   elseif mode=='simple' then
      src.image.scaleSimple(src,dst)
   else
      dok.error('mode must be one of: simple | bilinear', 'image.scale')
   end
   return dst
end
rawset(image, 'scale', scale)

----------------------------------------------------------------------
-- rotate
--
local function rotate(...)
   local dst,src,theta
   local args = {...}
   if select('#',...) == 3 then
      dst = args[1]
      src = args[2]
      theta = args[3]
   elseif select('#',...) == 2 then
      src = args[1]
      theta = args[2]
   else
      print(dok.usage('image.rotate',
                       'rotate an image by theta radians', nil,
                       {type='torch.Tensor', help='input image', req=true},
                       {type='number', help='rotation angle (in radians)', req=true},
                       '',
                       {type='torch.Tensor', help='destination', req=true},
                       {type='torch.Tensor', help='input image', req=true},
                       {type='number', help='rotation angle (in radians)', req=true}))
      dok.error('incorrect arguments', 'image.rotate')
   end
   dst = dst or src.new()
   dst:resizeAs(src)
   src.image.rotate(src,dst,theta)
   return dst  
end
rawset(image, 'rotate', rotate)

----------------------------------------------------------------------
-- warp
--
local function warp(...)
   local dst,src,field
   local mode = 'bilinear'
   local offset_mode = true
   local args = {...}
   if select('#',...) == 5 then
      dst = args[1]
      src = args[2]
      field = args[3]
      mode = args[4]
      offset_mode = args[5]
   elseif select('#',...) == 4 then
      if type(args[3]) == 'string' then
    src = args[1]
    field = args[2]
         mode = args[3]
    offset_mode = args[4]
      else
    dst = args[1]
    src = args[2]
    field = args[3]
    mode = args[4]
      end
   elseif select('#',...) == 3 then
      if type(args[3]) == 'string' then
         src = args[1]
         field = args[2]
         mode = args[3]
      else
         dst = args[1]
         src = args[2]
         field = args[3]
      end
   elseif select('#',...) == 2 then
      src = args[1]
      field = args[2]
   else
      print(dok.usage('image.warp',
                       'warp an image, according to a flow field', nil,
                       {type='torch.Tensor', help='input image (KxHxW)', req=true},
                       {type='torch.Tensor', help='(y,x) flow field (2xHxW)', req=true},
                       {type='string', help='mode: lanczos | bicubic | bilinear | simple', default='bilinear'},
                       {type='string', help='offset mode (add (x,y) to flow field)', default=true},
                       '',
                       {type='torch.Tensor', help='destination', req=true},
                       {type='torch.Tensor', help='input image (KxHxW)', req=true},
                       {type='torch.Tensor', help='(y,x) flow field (2xHxW)', req=true},
                       {type='string', help='mode: lanczos | bicubic | bilinear | simple', default='bilinear'},
                       {type='string', help='offset mode (add (x,y) to flow field)', default=true}))
      dok.error('incorrect arguments', 'image.warp')
   end
   -- This is a little messy, but convert mode string to an enum
   if (mode == 'simple') then
      mode = 0
   elseif (mode == 'bilinear') then
      mode = 1
   elseif (mode == 'bicubic') then
      mode = 2
   elseif (mode == 'lanczos') then
      mode = 3
   else
      dok.error('Incorrect arguments (mode is not lanczos | bicubic | bilinear | simple)!', 'image.warp')
   end
   local dim2 = false
   if src:nDimension() == 2 then
      dim2 = true
      src = src:reshape(1,src:size(1),src:size(2))
   end
   dst = dst or src.new()
   dst:resize(src:size(1), field:size(2), field:size(3))
   
   src.image.warp(dst,src,field,mode,offset_mode)
   if dim2 then
      dst = dst[1]
   end
   return dst
end
rawset(image, 'warp', warp)

----------------------------------------------------------------------
-- hflip
--
local function hflip(...)
   local dst,src
   local args = {...}
   if select('#',...) == 2 then
      dst = args[1]
      src = args[2]
   elseif select('#',...) == 1 then
      src = args[1]
   else
      print(dok.usage('image.hflip',
                       'flips an image horizontally', nil,
                       {type='torch.Tensor', help='input image', req=true},
                       '',
                       {type='torch.Tensor', help='destination', req=true},
                       {type='torch.Tensor', help='input image', req=true}))
      dok.error('incorrect arguments', 'image.hflip')
   end
   dst = dst or src.new():resizeAs(src)
   dst:resizeAs(src)
   if src:nDimension() == 2 then
      src = src:new():resize(1,src:size(1),src:size(2))
   end
   local flow = src.new(2,src:size(2),src:size(3))
   flow[1] = torch.ger( torch.linspace(0,src:size(2)-1,src:size(2)), torch.ones(src:size(3)) )
   flow[2] = torch.ger( torch.ones(src:size(2)), torch.linspace(-(src:size(3)-1),0,src:size(3))*-1 )
   dst[{}] = image.warp(src,flow,'simple',false)
   return dst
end
rawset(image, 'hflip', hflip)

----------------------------------------------------------------------
-- vflip
--
local function vflip(...)
   local dst,src
   local args = {...}
   if select('#',...) == 2 then
      dst = args[1]
      src = args[2]
   elseif select('#',...) == 1 then
      src = args[1]
   else
      print(dok.usage('image.vflip',
                       'flips an image horizontally', nil,
                       {type='torch.Tensor', help='input image', req=true},
                       '',
                       {type='torch.Tensor', help='destination', req=true},
                       {type='torch.Tensor', help='input image', req=true}))
      dok.error('incorrect arguments', 'image.vflip')
   end
   dst = dst or src.new()
   dst:resizeAs(src)
   if src:nDimension() == 2 then
      src = src:new():resize(1,src:size(1),src:size(2))
   end
   local flow = src.new(2,src:size(2),src:size(3))
   flow[1] = torch.ger( torch.linspace(-(src:size(2)-1),0,src:size(2))*-1, torch.ones(src:size(3)) )
   flow[2] = torch.ger( torch.ones(src:size(2)), torch.linspace(0,src:size(3)-1,src:size(3)) )
   dst[{}] = image.warp(src,flow,'simple',false)
   return dst
end
rawset(image, 'vflip', vflip)

----------------------------------------------------------------------
-- convolve(dst,src,ker,type)
-- convolve(dst,src,ker)
-- dst = convolve(src,ker,type)
-- dst = convolve(src,ker)
--
local function convolve(...)
   local dst,src,kernel,mode
   local args = {...}
   if select('#',...) == 4 then
      dst = args[1]
      src = args[2]
      kernel = args[3]
      mode = args[4]
   elseif select('#',...) == 3 then
      if type(args[3]) == 'string' then
         src = args[1]
         kernel = args[2]
         mode = args[3]
      else
         dst = args[1]
         src = args[2]
         kernel = args[3]
      end
   elseif select('#',...) == 2 then      
      src = args[1]
      kernel = args[2]
   else
      print(dok.usage('image.convolve',
                       'convolves an input image with a kernel, returns the result', nil,
                       {type='torch.Tensor', help='input image', req=true},
                       {type='torch.Tensor', help='kernel', req=true},
                       {type='string', help='type: full | valid | same', default='valid'},
                       '',
                       {type='torch.Tensor', help='destination', req=true},
                       {type='torch.Tensor', help='input image', req=true},
                       {type='torch.Tensor', help='kernel', req=true},
                       {type='string', help='type: full | valid | same', default='valid'}))
      dok.error('incorrect arguments', 'image.convolve')
   end
   if mode and mode ~= 'valid' and mode ~= 'full' and mode ~= 'same' then
      dok.error('mode has to be one of: full | valid', 'image.convolve')
   end
   local md = (((mode == 'full') or (mode == 'same')) and 'F') or 'V'
   if kernel:nDimension() == 2 and src:nDimension() == 3 then
      local k3d = torch.Tensor(src:size(1), kernel:size(1), kernel:size(2))
      for i = 1,src:size(1) do
         k3d[i]:copy(kernel)
      end
      kernel = k3d
   end
   if dst then
      torch.conv2(dst,src,kernel,md)
   else
      dst = torch.conv2(src,kernel,md)
   end
   if mode == 'same' then
      local cx = dst:dim()
      local cy = cx-1
      local ofy = math.ceil(kernel:size(cy)/2)
      local ofx = math.ceil(kernel:size(cx)/2)
      dst = dst:narrow(cy, ofy, src:size(cy)):narrow(cx, ofx, src:size(cx))
   end
   return dst
end
rawset(image, 'convolve', convolve) 

----------------------------------------------------------------------
-- compresses an image between min and max
--
local function minmax(args)
   local tensor = args.tensor
   local min = args.min
   local max = args.max 
   local symm = args.symm or false
   local inplace = args.inplace or false
   local saturate = args.saturate or false
   local tensorOut = args.tensorOut or (inplace and tensor) 
      or torch.Tensor(tensor:size()):copy(tensor)

   -- resize
   if args.tensorOut then
      tensorOut:resize(tensor:size()):copy(tensor)
   end

   -- saturate useless if min/max inferred
   if min == nil and max == nil then
      saturate = false
   end

   -- rescale min
   local fmin = 0
   if (min == nil) then
      if args.symm then
         fmin = math.max(math.abs(tensorOut:min()),math.abs(tensorOut:max()))
         min = -fmin
      else
         min = tensorOut:min()
      end
   end
   if (min ~= 0) then tensorOut:add(-min) end

   -- rescale for max
   if (max == nil) then
      if args.symm then
         max = fmin*2
      else
         max = tensorOut:max()
      end
   else
      max = max - min
   end
   if (max ~= 0) then tensorOut:div(max) end
      
   -- saturate
   if saturate then
      tensorOut.image.saturate(tensorOut)
   end

   -- and return
   return tensorOut
end
rawset(image, 'minmax', minmax) 

local function toDisplayTensor(...)
   -- usage
   local _, input, padding, nrow, scaleeach, min, max, symm, saturate = dok.unpack(
      {...},
      'image.toDisplayTensor',
      'given a pack of tensors, returns a single tensor that contains a grid of all in the pack',
      {arg='input',type='torch.Tensor | table', help='input (HxW or KxHxW or Kx3xHxW or list)',req=true},
      {arg='padding', type='number', help='number of padding pixels between images', default=0},
      {arg='nrow',type='number',help='number of images per row', default=6},
      {arg='scaleeach', type='boolean', help='individual scaling for list of images', default=false},
      {arg='min', type='number', help='lower-bound for range'},
      {arg='max', type='number', help='upper-bound for range'},
      {arg='symmetric',type='boolean',help='if on, images will be displayed using a symmetric dynamic range, useful for drawing filters', default=false},
      {arg='saturate', type='boolean', help='saturate (useful when min/max are lower than actual min/max', default=true}
   )

   if type(input) == 'table' then
      -- pack images in single tensor
      local ndims = input[1]:dim()
      local channels = ((ndims == 2) and 1) or input[1]:size(1)
      local height = input[1]:size(ndims-1)
      local width = input[1]:size(ndims)
      local packed = torch.Tensor(#input,channels,height,width)
      for i,img in ipairs(input) do
         if scaleeach then
       packed[i] = image.minmax{tensor=input[i], min=min, max=max, symm=symm, saturate=saturate}
         else
            packed[i]:copy(input[i])
         end
      end
      return toDisplayTensor{input=packed,padding=padding,nrow=nrow,min=min,max=max,symmetric=symm,saturate=saturate}
   end

   if input:nDimension() == 4 and (input:size(2) == 3 or input:size(2) == 1) then
      -- arbitrary number of color images: lay them out on a grid
      local nmaps = input:size(1)
      local xmaps = math.min(nrow, nmaps)
      local ymaps = math.ceil(nmaps / xmaps)
      local height = input:size(3)+padding
      local width = input:size(4)+padding
      local grid = torch.Tensor(input:size(2), height*ymaps, width*xmaps):fill(input:max())
      local k = 1
      for y = 1,ymaps do
         for x = 1,xmaps do
            if k > nmaps then break end
            grid:narrow(2,(y-1)*height+1+padding/2,height-padding):narrow(3,(x-1)*width+1+padding/2,width-padding):copy(input[k])
            k = k + 1
         end
      end
      local mminput = image.minmax{tensor=grid, min=min, max=max, symm=symm, saturate=saturate}
      return mminput
   elseif input:nDimension() == 2  or (input:nDimension() == 3 and (input:size(1) == 1 or input:size(1) == 3)) then
      -- Rescale range
      local mminput = image.minmax{tensor=input, min=min, max=max, symm=symm, saturate=saturate}
      return mminput
   elseif input:nDimension() == 3 then
      -- arbitrary number of channels: lay them out on a grid
      local nmaps = input:size(1)
      local xmaps = math.min(nrow, nmaps)
      local ymaps = math.ceil(nmaps / xmaps)
      local height = input:size(2)+padding
      local width = input:size(3)+padding
      local grid = torch.Tensor(height*ymaps, width*xmaps):fill(input:max())
      local k = 1
      for y = 1,ymaps do
         for x = 1,xmaps do
            if k > nmaps then break end
            grid:narrow(1,(y-1)*height+1+padding/2,height-padding):narrow(2,(x-1)*width+1+padding/2,width-padding):copy(input[k])
            k = k + 1
         end
      end
      local mminput = image.minmax{tensor=grid, min=min, max=max, symm=symm, saturate=saturate}
      return mminput
   else
      xerror('input must be a HxW or KxHxW or Kx3xHxW tensor, or a list of tensors', 'image.toDisplayTensor')
   end
end
rawset(image,'toDisplayTensor',toDisplayTensor)

----------------------------------------------------------------------
-- super generic display function
--
local function display(...)
   -- usage
   local _, input, zoom, min, max, legend, w, ox, oy, scaleeach, gui, offscreen, padding, symm, nrow, saturate = dok.unpack(
      {...},
      'image.display',
      'displays a single image, with optional saturation/zoom',
      {arg='image', type='torch.Tensor | table', help='image (HxW or KxHxW or Kx3xHxW or list)', req=true},
      {arg='zoom', type='number', help='display zoom', default=1},
      {arg='min', type='number', help='lower-bound for range'},
      {arg='max', type='number', help='upper-bound for range'},
      {arg='legend', type='string', help='legend', default='image.display'},
      {arg='win', type='qt window', help='window descriptor'},
      {arg='x', type='number', help='x offset (only if win is given)', default=0},
      {arg='y', type='number', help='y offset (only if win is given)', default=0},
      {arg='scaleeach', type='boolean', help='individual scaling for list of images', default=false},
      {arg='gui', type='boolean', help='if on, user can zoom in/out (turn off for faster display)',
       default=true},
      {arg='offscreen', type='boolean', help='offscreen rendering (to generate images)',
       default=false},
      {arg='padding', type='number', help='number of padding pixels between images', default=0},
      {arg='symmetric',type='boolean',help='if on, images will be displayed using a symmetric dynamic range, useful for drawing filters', default=false},
      {arg='nrow',type='number',help='number of images per row', default=6},
      {arg='saturate', type='boolean', help='saturate (useful when min/max are lower than actual min/max', default=true}
   )
   
   -- dependencies
   require 'qt'
   require 'qttorch'
   require 'qtwidget'
   require 'qtuiloader'

   input = image.toDisplayTensor{input=input, padding=padding, nrow=nrow, saturate=saturate,
                                 scaleeach=scaleeach, min=min, max=max, symmetric=symm}
   -- if image is a table, then we treat if as a list of images
   -- if 2 dims or 3 dims and 1/3 channels, then we treat it as a single image
   if input:nDimension() == 2  or (input:nDimension() == 3 and (input:size(1) == 1 or input:size(1) == 3)) then
      -- Rescale range
      local mminput = input--image.minmax{tensor=input, min=min, max=max, symm=symm}
      -- Compute width
      local d = input:nDimension()
      local x = input:size(d)*zoom
      local y = input:size(d-1)*zoom

      -- if gui active, then create interactive window (with zoom, clicks and so on)
      if gui and not w and not offscreen then
         -- create window context
         local closure = w
         local hook_resize, hook_mouse
         if closure and closure.window and closure.image then
            closure.image = mminput
            closure.refresh(x,y)
         else
            closure = {image=mminput}
            hook_resize = function(wi,he)
                             local qtimg = qt.QImage.fromTensor(closure.image)
                             closure.painter:image(0,0,wi,he,qtimg)
                             collectgarbage()
                          end
            hook_mouse = function(x,y,button)
                            --local size = closure.window.frame.size:totable()
                            --size.width = 
                            --size.height = 
                            if button == 'LeftButton' then
                            elseif button == 'RightButton' then
                            end
                            --closure.window.frame.size = qt.QSize(size)
                         end
            closure.window, closure.painter = image.window(hook_resize,hook_mouse)
            closure.refresh = hook_resize
         end
         closure.window.size = qt.QSize{width=x,height=y}
         closure.window.windowTitle = legend
         closure.window:show()
         hook_resize(x,y)
         closure.isclosure = true
         return closure
      else
         if offscreen then
            w = w or qt.QtLuaPainter(x,y)
         else
            w = w or qtwidget.newwindow(x,y,legend)
         end
         if w.isclosure then
            -- window was created with gui, just update closure
            local closure = w
            closure.image = mminput
            local size = closure.window.size:totable()
            closure.window.windowTitle = legend
            closure.refresh(size.width, size.height)
         else
            -- if no gui, create plain window, and blit
            local qtimg = qt.QImage.fromTensor(mminput)
            w:image(ox,oy,x,y,qtimg)
         end
      end
   else
      xerror('image must be a HxW or KxHxW or Kx3xHxW tensor, or a list of tensors', 'image.display')
   end
   -- return painter
   return w
end
rawset(image, 'display', display)

----------------------------------------------------------------------
-- creates a window context for images
--
local function window(hook_resize, hook_mousepress, hook_mousedoublepress)
   require 'qt'
   require 'qttorch'
   require 'qtwidget'
   require 'qtuiloader'
   local pathui = paths.concat(sys.fpath(), 'win.ui')
   local win = qtuiloader.load(pathui)
   local painter = qt.QtLuaPainter(win.frame)
   if hook_resize then
      qt.connect(qt.QtLuaListener(win.frame), 
                 'sigResize(int,int)', 
                 hook_resize)
   end
   if hook_mousepress then
      qt.connect(qt.QtLuaListener(win.frame),
                 'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)', 
                 hook_mousepress)
   end
   if hook_mousedoublepress then
      qt.connect(qt.QtLuaListener(win.frame),
                 'sigMouseDoubleClick(int,int,QByteArray,QByteArray,QByteArray)',
                 hook_mousedoublepress)
   end
   local ctrl = false
   qt.connect(qt.QtLuaListener(win),
              'sigKeyPress(QString,QByteArray,QByteArray)',
              function (str, s2)
                 if s2 and s2 == 'Key_Control' then
                    ctrl = true
                 elseif s2 and s2 == 'Key_W' and ctrl then
                    win:close()
                 else
                    ctrl = false
                 end
              end)
   return win,painter
end
rawset(image, 'window', window)

----------------------------------------------------------------------
-- lena is always useful
--
local function lena(full)
   local fname = 'lena'
   if full then fname = fname .. '_full' end
   if xlua.require 'libjpeg' then
      lena = image.load(paths.concat(sys.fpath(), fname .. '.jpg'), 3)
   elseif xlua.require 'libpng' then
      lena = image.load(paths.concat(sys.fpath(), fname .. '.png'), 3)
   else
      dok.error('no bindings available to load images (libjpeg AND libpng missing)', 'image.lena')
   end
   return lena
end
rawset(image, 'lena', lena)

----------------------------------------------------------------------
-- image.rgb2lab(image)
-- converts a RGB image to YUV
--
function image.rgb2lab(...)   
   -- arg check
   local output,input
   local args = {...}
   if select('#',...) == 2 then
      output = args[1]
      input = args[2]
   elseif select('#',...) == 1 then
      input = args[1]
   else
      print(dok.usage('image.rgb2lab',
                      'transforms an image from RGB to Lab', nil,
                      {type='torch.Tensor', help='input image', req=true},
                      '',
                      {type='torch.Tensor', help='output image', req=true},
                      {type='torch.Tensor', help='input image', req=true}
                      ))
      dok.error('missing input', 'image.rgb2lab')
   end

   -- resize
   output = output or input.new()
   output:resizeAs(input)
   
   -- output chanels
   local xyz = output:clone()
   local outputX = xyz[1]
   local outputY = xyz[2]
   local outputZ = xyz[3]

   -- output chanels
   local outputL = output[1]
   local outputA = output[2]
   local outputB = output[3]


   -- Set a threshold
   local T = 0.008856;

   local RGB = torch.Tensor():set(input):resize(3,input:size(2)*input:size(3))

   -- RGB to XYZ
   local MAT = torch.Tensor({{0.412453, 0.357580, 0.180423},
              {0.212671, 0.715160, 0.072169},
              {0.019334, 0.119193, 0.950227}})
   local XYZ = MAT * RGB;

   -- Normalize for D65 white point
   XYZ[1]:div(0.950456);
   XYZ[3]:div(1.088754);
   local Y3 = torch.pow(XYZ[2],1/3)

   local thres = function(x) 
      if x > T then 
    return x^(1/3) 
      else 
    return 1/3*(29/6)^2 * x + 16/116 
      end 
   end
   XYZ:apply(thres)
   
   outputL:mul(XYZ[2],116):add(-16):div(100)
   outputA:copy(XYZ[1]):add(-1,XYZ[2]):mul(500):add(110):div(220)
   outputB:copy(XYZ[2]):add(-1,XYZ[3]):mul(200):add(110):div(220)
    
   -- return LAB image
   return output
end
----------------------------------------------------------------------
-- image.rgb2yuv(image)
-- converts a RGB image to YUV
--
function image.rgb2yuv(...)   
   -- arg check
   local output,input
   local args = {...}
   if select('#',...) == 2 then
      output = args[1]
      input = args[2]
   elseif select('#',...) == 1 then
      input = args[1]
   else
      print(dok.usage('image.rgb2yuv',
                      'transforms an image from RGB to YUV', nil,
                      {type='torch.Tensor', help='input image', req=true},
                      '',
                      {type='torch.Tensor', help='output image', req=true},
                      {type='torch.Tensor', help='input image', req=true}
                      ))
      dok.error('missing input', 'image.rgb2yuv')
   end

   -- resize
   output = output or input.new()
   output:resizeAs(input)
   
   -- input chanels
   local inputRed = input[1]
   local inputGreen = input[2]
   local inputBlue = input[3]
   
   -- output chanels
   local outputY = output[1]
   local outputU = output[2]
   local outputV = output[3]
   
   -- convert
   outputY:zero():add(0.299, inputRed):add(0.587, inputGreen):add(0.114, inputBlue)
   outputU:zero():add(-0.14713, inputRed):add(-0.28886, inputGreen):add(0.436, inputBlue)
   outputV:zero():add(0.615, inputRed):add(-0.51499, inputGreen):add(-0.10001, inputBlue)

   -- return YUV image
   return output
end

----------------------------------------------------------------------
-- image.yuv2rgb(image)
-- converts a YUV image to RGB
--
function image.yuv2rgb(...)      
   -- arg check
   local output,input
   local args = {...}
   if select('#',...) == 2 then
      output = args[1]
      input = args[2]
   elseif select('#',...) == 1 then
      input = args[1]
   else
      print(dok.usage('image.yuv2rgb',
                      'transforms an image from YUV to RGB', nil,
                      {type='torch.Tensor', help='input image', req=true},
                      '',
                      {type='torch.Tensor', help='output image', req=true},
                      {type='torch.Tensor', help='input image', req=true}
                      ))
      dok.error('missing input', 'image.yuv2rgb')
   end

   -- resize
   output = output or input.new()
   output:resizeAs(input)
   
   -- input chanels
   local inputY = input[1]
   local inputU = input[2]
   local inputV = input[3]
   
   -- output chanels
   local outputRed = output[1]
   local outputGreen = output[2]
   local outputBlue = output[3]
   
   -- convert
   outputRed:copy(inputY):add(1.13983, inputV)
   outputGreen:copy(inputY):add(-0.39465, inputU):add(-0.58060, inputV)      
   outputBlue:copy(inputY):add(2.03211, inputU)
   
   -- return RGB image
   return output
end

----------------------------------------------------------------------
-- image.rgb2y(image)
-- converts a RGB image to Y (discards U/V)
--
function image.rgb2y(...)
   -- arg check
   local output,input
   local args = {...}
   if select('#',...) == 2 then
      output = args[1]
      input = args[2]
   elseif select('#',...) == 1 then
      input = args[1]
   else
      print(dok.usage('image.rgb2y',
                      'transforms an image from RGB to Y', nil,
                      {type='torch.Tensor', help='input image', req=true},
                      '',
                      {type='torch.Tensor', help='output image', req=true},
                      {type='torch.Tensor', help='input image', req=true}
                      ))
      dok.error('missing input', 'image.rgb2y')
   end

   -- resize
   output = output or input.new()
   output:resize(1, input:size(2), input:size(3))
   
   -- input chanels
   local inputRed = input[1]
   local inputGreen = input[2]
   local inputBlue = input[3]
   
   -- output chanels
   local outputY = output[1]
   
   -- convert
   outputY:zero():add(0.299, inputRed):add(0.587, inputGreen):add(0.114, inputBlue)
   
   -- return YUV image
   return output
end

----------------------------------------------------------------------
-- image.rgb2hsl(image)
-- converts an RGB image to HSL
--
function image.rgb2hsl(...)   
   -- arg check
   local output,input
   local args = {...}
   if select('#',...) == 2 then
      output = args[1]
      input = args[2]
   elseif select('#',...) == 1 then
      input = args[1]
   else
      print(dok.usage('image.rgb2hsl',
                      'transforms an image from RGB to HSL', nil,
                      {type='torch.Tensor', help='input image', req=true},
                      '',
                      {type='torch.Tensor', help='output image', req=true},
                      {type='torch.Tensor', help='input image', req=true}
                      ))
      dok.error('missing input', 'image.rgb2hsl')
   end

   -- resize
   output = output or input.new()
   output:resizeAs(input)

   -- compute
   input.image.rgb2hsl(input,output)
   
   -- return HSL image
   return output
end

----------------------------------------------------------------------
-- image.hsl2rgb(image)
-- converts an HSL image to RGB
--
function image.hsl2rgb(...)
   -- arg check
   local output,input
   local args = {...}
   if select('#',...) == 2 then
      output = args[1]
      input = args[2]
   elseif select('#',...) == 1 then
      input = args[1]
   else
      print(dok.usage('image.hsl2rgb',
                      'transforms an image from HSL to RGB', nil,
                      {type='torch.Tensor', help='input image', req=true},
                      '',
                      {type='torch.Tensor', help='output image', req=true},
                      {type='torch.Tensor', help='input image', req=true}
                      ))
      dok.error('missing input', 'image.hsl2rgb')
   end

   -- resize
   output = output or input.new()
   output:resizeAs(input)

   -- compute
   input.image.hsl2rgb(input,output)
   
   -- return HSL image
   return output
end

----------------------------------------------------------------------
-- image.rgb2hsv(image)
-- converts an RGB image to HSV
--
function image.rgb2hsv(...)
   -- arg check
   local output,input
   local args = {...}
   if select('#',...) == 2 then
      output = args[1]
      input = args[2]
   elseif select('#',...) == 1 then
      input = args[1]
   else
      print(dok.usage('image.rgb2hsv',
                      'transforms an image from RGB to HSV', nil,
                      {type='torch.Tensor', help='input image', req=true},
                      '',
                      {type='torch.Tensor', help='output image', req=true},
                      {type='torch.Tensor', help='input image', req=true}
                      ))
      dok.error('missing input', 'image.rgb2hsv')
   end

   -- resize
   output = output or input.new()
   output:resizeAs(input)

   -- compute
   input.image.rgb2hsv(input,output)
   
   -- return HSV image
   return output
end

----------------------------------------------------------------------
-- image.hsv2rgb(image)
-- converts an HSV image to RGB
--
function image.hsv2rgb(...)
   -- arg check
   local output,input
   local args = {...}
   if select('#',...) == 2 then
      output = args[1]
      input = args[2]
   elseif select('#',...) == 1 then
      input = args[1]
   else
      print(dok.usage('image.hsv2rgb',
                      'transforms an image from HSV to RGB', nil,
                      {type='torch.Tensor', help='input image', req=true},
                      '',
                      {type='torch.Tensor', help='output image', req=true},
                      {type='torch.Tensor', help='input image', req=true}
                      ))
      dok.error('missing input', 'image.hsv2rgb')
   end

   -- resize
   output = output or input.new()
   output:resizeAs(input)

   -- compute
   input.image.hsv2rgb(input,output)
   
   -- return HSV image
   return output
end

----------------------------------------------------------------------
-- image.rgb2nrgb(image)
-- converts an RGB image to normalized-RGB
--
function image.rgb2nrgb(...)
   -- arg check
   local output,input
   local args = {...}
   if select('#',...) == 2 then
      output = args[1]
      input = args[2]
   elseif select('#',...) == 1 then
      input = args[1]
   else
      print(dok.usage('image.rgb2nrgb',
                      'transforms an image from RGB to normalized RGB', nil,
                      {type='torch.Tensor', help='input image', req=true},
                      '',
                      {type='torch.Tensor', help='output image', req=true},
                      {type='torch.Tensor', help='input image', req=true}
                      ))
      dok.error('missing input', 'image.rgb2nrgb')
   end

   -- resize
   output = output or input.new()
   output:resizeAs(input)
   local sum = input.new()
   sum:resize(input:size(2), input:size(3))
   
   -- compute sum and normalize
   sum:copy(input[1]):add(input[2]):add(input[3]):add(1e-6)
   output:copy(input)
   output[1]:cdiv(sum)
   output[2]:cdiv(sum)
   output[3]:cdiv(sum)
   
   -- return HSV image
   return output
end

----------------------------------------------------------------------
--- Returns a gaussian kernel.
--
function image.gaussian(...)
   -- process args
   local _, size, sigma, amplitude, normalize, 
   width, height, sigma_horz, sigma_vert, mean_horz, mean_vert = dok.unpack(
      {...},
      'image.gaussian',
      'returns a 2D gaussian kernel',
      {arg='size', type='number', help='kernel size (size x size)', default=3},
      {arg='sigma', type='number', help='sigma (horizontal and vertical)', default=0.25},
      {arg='amplitude', type='number', help='amplitute of the gaussian (max value)', default=1},
      {arg='normalize', type='number', help='normalize kernel (exc Amplitude)', default=false},
      {arg='width', type='number', help='kernel width', defaulta='size'},
      {arg='height', type='number', help='kernel height', defaulta='size'},
      {arg='sigma_horz', type='number', help='horizontal sigma', defaulta='sigma'},
      {arg='sigma_vert', type='number', help='vertical sigma', defaulta='sigma'},
      {arg='mean_horz', type='number', help='horizontal mean', default=0.5},
      {arg='mean_vert', type='number', help='vertical mean', default=0.5}
   )

   -- generate kernel
   local gauss = torch.Tensor(height, width)
   gauss.image.gaussian(gauss, amplitude, normalize, sigma_horz, sigma_vert, mean_horz, mean_vert)
   
   return gauss
end

function image.gaussian1D(...)
   -- process args
   local _, size, sigma, amplitude, normalize, mean
      = dok.unpack(
      {...},
      'image.gaussian1D',
      'returns a 1D gaussian kernel',
      {arg='size', type='number', help='size the kernel', default=3},
      {arg='sigma', type='number', help='Sigma', default=0.25},
      {arg='amplitude', type='number', help='Amplitute of the gaussian (max value)', default=1},
      {arg='normalize', type='number', help='Normalize kernel (exc Amplitude)', default=false},
      {arg='mean', type='number', help='Mean', default=0.5}
   )

   -- local vars
   local center = mean * size + 0.5
   
   -- generate kernel
   local gauss = torch.Tensor(size)
   for i=1,size do
      gauss[i] = amplitude * math.exp(-(math.pow((i-center)
                                              /(sigma*size),2)/2))
   end
   if normalize then
      gauss:div(gauss:sum())
   end
   return gauss
end

----------------------------------------------------------------------
--- Returns a Laplacian kernel.
--
function image.laplacian(...)
   -- process args
   local _, size, sigma, amplitude, normalize, 
   width, height, sigma_horz, sigma_vert, mean_horz, mean_vert = dok.unpack(
      {...},
      'image.gaussian',
      'returns a 2D gaussian kernel',
      {arg='size', type='number', help='kernel size (size x size)', default=3},
      {arg='sigma', type='number', help='sigma (horizontal and vertical)', default=0.1},
      {arg='amplitude', type='number', help='amplitute of the gaussian (max value)', default=1},
      {arg='normalize', type='number', help='normalize kernel (exc Amplitude)', default=false},
      {arg='width', type='number', help='kernel width', defaulta='size'},
      {arg='height', type='number', help='kernel height', defaulta='size'},
      {arg='sigma_horz', type='number', help='horizontal sigma', defaulta='sigma'},
      {arg='sigma_vert', type='number', help='vertical sigma', defaulta='sigma'},
      {arg='mean_horz', type='number', help='horizontal mean', default=0.5},
      {arg='mean_vert', type='number', help='vertical mean', default=0.5}
   )

   -- local vars
   local center_x = mean_horz * width + 0.5
   local center_y = mean_vert * height + 0.5
   
   -- generate kernel
   local logauss = torch.Tensor(height,width)
   for i=1,height do
      for j=1,width do
         local xsq = math.pow((i-center_x)/(sigma_horz*width),2)/2
         local ysq = math.pow((j-center_y)/(sigma_vert*height),2)/2
         local derivCoef = 1 - (xsq + ysq)
         logauss[i][j] = derivCoef * amplitude * math.exp(-(xsq + ysq))
      end
   end
   if normalize then
      logauss:div(logauss:sum())
   end
   return logauss
end

----------------------------------------------------------------------
--- Gaussian Pyramid
--
function image.gaussianpyramid(...)
   local dst,src,scales
   local args = {...}
   if select('#',...) == 3 then
      dst = args[1]
      src = args[2]
      scales = args[3]
   elseif select('#',...) == 2 then
      dst = {}
      src = args[1]
      scales = args[2]
   else
      print(dok.usage('image.gaussianpyramid',
                       'construct a Gaussian pyramid from an image', nil,
                       {type='torch.Tensor', help='input image', req=true},
                       {type='table', help='list of scales', req=true},
                       '',
                       {type='table', help='destination (list of Tensors)', req=true},
                       {type='torch.Tensor', help='input image', req=true},
                       {type='table', help='list of scales', req=true}))
      dok.error('incorrect arguments', 'image.gaussianpyramid')
   end
   if src:nDimension() == 2 then
      for i = 1,#scales do
         dst[i] = dst[i] or torch.Tensor()
         dst[i]:resize(src:size(1)*scales[i], src:size(2)*scales[i])
      end
   elseif src:nDimension() == 3 then
      for i = 1,#scales do
         dst[i] = dst[i] or torch.Tensor()
         dst[i]:resize(src:size(1), src:size(2)*scales[i], src:size(3)*scales[i])
      end
   else
      dok.error('src image must be 2D or 3D', 'image.gaussianpyramid')
   end
   local k = image.gaussian{width=3, normalize=true}
   local tmp = src
   for i = 1,#scales do
      if scales[i] == 1 then
         dst[i][{}] = tmp
      else
         image.scale(dst[i], tmp, 'simple')
      end
      tmp = image.convolve(dst[i], k, 'same')
   end
   return dst
end

----------------------------------------------------------------------
--- Creates a random color mapping
--
function image.colormap(nbColor)
   -- note: the best way of obtaining optimally-spaced
   -- colors is to generate them around the HSV wheel,
   -- by varying the Hue component
   local map = torch.Tensor(nbColor,3)
   local huef = 0
   local satf = 0
   for i = 1,nbColor do
      -- HSL
      local hue = math.mod(huef,360)
      local sat = math.mod(satf,0.7) + 0.3
      local light = 0.5
      huef = huef + 39
      satf = satf + 1/9
      -- HSL -> RGB
      local c = (1 - math.abs(2*light-1))*sat
      local huep = hue/60
      local x = c*(1-math.abs(math.mod(huep,2)-1))
      local redp
      local greenp
      local bluep
      if huep < 1 then
         redp = c; greenp = x; bluep = 0
      elseif huep < 2 then
         redp = x; greenp = c; bluep = 0
      elseif huep < 3 then
         redp = 0; greenp = c; bluep = x
      elseif huep < 4 then
         redp = 0; greenp = x; bluep = c
      elseif huep < 5 then
         redp = x; greenp = 0; bluep = c
      else
         redp = c; greenp = 0; bluep = x
      end
      local m = light - c/2
      map[i][1] = redp + m
      map[i][2] = greenp + m
      map[i][3] = bluep + m
   end
   return map
end

----------------------------------------------------------------------
--- Creates a jet colour mapping - Inspired by http://www.metastine.com/?p=7
--
function image.jetColormap(nbColour)
   local map = torch.Tensor(nbColour,3)
   for i = 1,nbColour do
      local fourValue = 4 * i / nbColour
      map[i][1] = math.max(math.min(fourValue - 1.5, -fourValue + 4.5, 1),0)
      map[i][2] = math.max(math.min(fourValue -  .5, -fourValue + 3.5, 1),0)
      map[i][3] = math.max(math.min(fourValue +  .5, -fourValue + 2.5, 1),0)
   end
   return map
end



------------------------------------------------------------------------
--- Local contrast normalization of an image
--
-- do local contrast normalization on a given image tensor using kernel ker.
-- of kernel is not given, then a default 9x9 gaussian will be used
function image.lcn(im,ker)

   ker = ker or image.gaussian({size=9,sigma=1.591/9,normalize=true})
   local im = im:clone():type('torch.DoubleTensor')
   if not(im:dim() == 2 or (im:dim() == 3 and im:size(1) == 1)) then
     error('grayscale image expected')
   end
   if im:dim() == 3 then
      im = im[1]
   end
   mn = im:mean()
   sd = im:std()
   -- print(ker)

   -- 1. subtract the mean and divide by the standard deviation
   im:add(-mn)
   im:div(sd)

   -- 2. calculate local mean and std and normalize each pixel

   -- mean
   local lmn = torch.conv2(im, ker)
   -- variance
   local imsq = im:clone():cmul(im)
   local lmnsq = torch.conv2(imsq, ker)
   local lvar = lmn:clone():cmul(lmn)
   lvar:add(-1,lmnsq):mul(-1)
   -- avoid numerical errors
   lvar:apply(function(x) if x < 0 then return 0 end end)
   -- standard deviation
   local lstd  = lvar:sqrt()
   lstd:apply(function (x) if x < 1 then return 1 end end)

   -- apply normalization
   local shifti = math.floor(ker:size(1)/2)+1
   local shiftj = math.floor(ker:size(2)/2)+1
   --print(shifti,shiftj,lstd:size(),im:size())
   local dim = im:narrow(1,shifti,lstd:size(1)):narrow(2,shiftj,lstd:size(2)):clone()
   dim:add(-1,lmn)
   dim:cdiv(lstd)
   return dim:clone()

end

