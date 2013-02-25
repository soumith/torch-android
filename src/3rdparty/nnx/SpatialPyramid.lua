local SpatialPyramid, parent = torch.class('nn.SpatialPyramid', 'nn.Module')

local help_desc = [[
Simplified (and more flexible regarding sizes) fovea:
From a given image, generates a pyramid of scales, and process each scale
with the given list of processors. 
The result of each module/scale is then
upsampled to produce a homogenous list of 3D feature maps (a table of 3D tensors)
grouping the different scales.

There are two operating modes: focused [mostly training], and global [inference]. 

In global mode,
the entire input is processed.

In focused mode, the fovea is first focused on a particular (x,y) point.
This function has two additional parameters, w and h, that represent the size
of the OUTPUT of the processors.
To focus the fovea, simply call fovea:focus(x,y,w,h) before doing a forward.
A call to fovea:focus(nil) makes it unfocus (go back to global mode).

If prescaled_input is true, then the input has to be a table of pre-downscaled
3D tensors. It does not work in focus mode.
]]

function SpatialPyramid:__init(ratios, processors, kW, kH, dW, dH, xDimIn, yDimIn,
			       xDimOut, yDimOut, prescaled_input)
   parent.__init(self)
   self.prescaled_input = prescaled_input or false
   assert(#ratios == #processors)
   
   self.ratios = ratios
   self.kH = kH
   self.kW = kW
   self.dH = dH
   self.dW = dW
   self.focused = false
   self.x = 0
   self.y = 0
   self.wFocus = 0
   self.hFocus = 0
   self.processors = processors

   local wPad = kW-dW
   local hPad = kH-dH
   local padLeft   = math.floor(wPad/2)
   local padRight  = math.ceil (wPad/2)
   local padTop    = math.floor(hPad/2)
   local padBottom = math.ceil (hPad/2)

   -- focused
   self.focused_pipeline = nn.ConcatTable()
   for i = 1,#self.ratios do
      local seq = nn.Sequential()
      seq:add(nn.SpatialPadding(0,0,0,0, yDimIn, xDimIn))
      seq:add(nn.SpatialReSamplingEx{rwidth=1.0/self.ratios[i], rheight=1.0/self.ratios[i],
				     xDim = xDimIn, yDim = yDimIn, mode='average'})
      seq:add(processors[i])
      self.focused_pipeline:add(seq)
   end

   -- unfocused
   if prescaled_input then
      self.unfocused_pipeline = nn.ParallelTable()
   else
      self.unfocused_pipeline = nn.ConcatTable()
   end
   for i = 1,#self.ratios do
      local seq = nn.Sequential()
      if not prescaled_input then
	 seq:add(nn.SpatialReSamplingEx{rwidth=1.0/self.ratios[i], rheight=1.0/self.ratios[i],
					xDim = xDimIn, yDim = yDimIn, mode='average'})
	 seq:add(nn.SpatialPadding(padLeft, padRight, padTop, padBottom, yDimIn, xDimIn))
      end
      seq:add(processors[i])
      seq:add(nn.SpatialReSamplingEx{rwidth=self.ratios[i], rheight=self.ratios[i],
				     xDim=xDimOut, yDim=yDimOut, mode='simple'})
      self.unfocused_pipeline:add(seq)
   end
end

function SpatialPyramid:focus(x, y, w, h)
   w = w or 1
   h = h or 1
   if x and y then
      self.x = x
      self.y = y
      self.focused = true
      self.winWidth = {}
      self.winHeight = {}
      for i = 1,#self.ratios do
	 self.winWidth[i]  = self.ratios[i] * ((w-1) * self.dW + self.kW)
	 self.winHeight[i] = self.ratios[i] * ((h-1) * self.dH + self.kH)
      end
   else
      self.focused = false
   end
end

function SpatialPyramid:configureFocus(wImg, hImg)
   for i = 1,#self.ratios do
      local padder = self.focused_pipeline.modules[i].modules[1]
      padder.pad_l = -self.x + math.ceil (self.winWidth[i] /2)
      padder.pad_r =  self.x + math.floor(self.winWidth[i] /2) - wImg
      padder.pad_t = -self.y + math.ceil (self.winHeight[i]/2)
      padder.pad_b =  self.y + math.floor(self.winHeight[i]/2) - hImg
   end
end   

function SpatialPyramid:checkSize(input)
   for i = 1,#self.ratios do
      if (math.mod(input:size(2), self.ratios[i]) ~= 0) or
         (math.mod(input:size(3), self.ratios[i]) ~= 0) then
         error('SpatialPyramid: input sizes must be multiple of ratios')
      end
   end
end
 
function SpatialPyramid:updateOutput(input)
   if not self.prescaled_input then
      self:checkSize(input)
   end
   if self.focused then
      self:configureFocus(input:size(3), input:size(2))
      self.output = self.focused_pipeline:updateOutput(input)
   else
      self.output = self.unfocused_pipeline:updateOutput(input)
   end
   return self.output
end

function SpatialPyramid:updateGradInput(input, gradOutput)
   if self.focused then
      self.gradInput = self.focused_pipeline:updateGradInput(input, gradOutput)
   else
      self.gradInput = self.unfocused_pipeline:updateGradInput(input, gradOutput)
   end
   return self.gradInput
end

function SpatialPyramid:zeroGradParameters()
   self.focused_pipeline:zeroGradParameters()
   self.unfocused_pipeline:zeroGradParameters()
end

function SpatialPyramid:accGradParameters(input, gradOutput, scale)
   if self.focused then
      self.focused_pipeline:accGradParameters(input, gradOutput, scale)
   else
      self.unfocused_pipeline:accGradParameters(input, gradOutput, scale)
   end
end

function SpatialPyramid:updateParameters(learningRate)
   if self.focused then
      self.focused_pipeline:updateParameters(learningRate)
   else
      self.unfocused_pipeline:updateParameters(learningRate)
   end
end

function SpatialPyramid:type(type)
   parent.type(self, type)
   self.focused_pipeline:type(type)
   self.unfocused_pipeline:type(type)
   return self
end

function SpatialPyramid:parameters()
   if self.focused then
      return self.focused_pipeline:parameters()
   else
      return self.unfocused_pipeline:parameters()
   end
end

function SpatialPyramid:__tostring__()
   if self.focused then
      local dscr = tostring(self.focused_pipeline):gsub('\n', '\n    |    ')
      return 'SpatialPyramid (focused)\n' .. dscr
   else
      local dscr = tostring(self.unfocused_pipeline):gsub('\n', '\n    |    ')
      return 'SpatialPyramid (unfocused)\n' .. dscr
   end
end