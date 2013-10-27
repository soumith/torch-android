local SpatialReSamplingEx, parent = torch.class('nn.SpatialReSamplingEx', 'nn.Module')

local help_desc = [[
      Extended spatial resampling.
]]
function SpatialReSamplingEx:__init(...)
   parent.__init(self)

   -- get args
   xlua.unpack_class(
      self, {...}, 'nn.SpatialReSampling',  help_desc,
      {arg='rwidth', type='number', help='ratio: owidth/iwidth'},
      {arg='rheight', type='number', help='ratio: oheight/iheight'},
      {arg='owidth', type='number', help='output width'},
      {arg='oheight', type='number', help='output height'},
      {arg='mode', type='string', help='Mode : simple | average (only for downsampling) | bilinear', default = 'simple'},
      {arg='yDim', type='number', help='image y dimension', default=2},
      {arg='xDim', type='number', help='image x dimension', default=3}
   )
   if self.yDim+1 ~= self.xDim then
      error('nn.SpatialReSamplingEx: yDim must be equals to xDim-1')
   end
   self.outputSize = torch.LongStorage(4)
   self.inputSize = torch.LongStorage(4)
   if self.mode == 'simple' then self.mode_c = 0 end
   if self.mode == 'average' then self.mode_c = 1 end
   if self.mode == 'bilinear' then self.mode_c = 2 end
   if not self.mode_c then
      error('SpatialReSampling: mode must be simple | average | bilinear')
   end
end

local function round(a)
   return math.floor(a+0.5)
end

function SpatialReSamplingEx:updateOutput(input)
   -- compute iheight, iwidth, oheight and owidth
   self.iheight = input:size(self.yDim)
   self.iwidth = input:size(self.xDim)
   self.oheightCurrent = self.oheight or round(self.rheight*self.iheight)
   self.owidthCurrent = self.owidth or round(self.rwidth*self.iwidth)
   if not ((self.oheightCurrent>=self.iheight) == (self.owidthCurrent>=self.iwidth)) then
      error('SpatialReSamplingEx: Cannot upsample one dimension while downsampling the other')
   end
   
   -- resize input into K1 x iheight x iwidth x K2 tensor
   self.inputSize:fill(1)
   for i = 1,self.yDim-1 do
      self.inputSize[1] = self.inputSize[1] * input:size(i)
   end
   self.inputSize[2] = self.iheight
   self.inputSize[3] = self.iwidth
   for i = self.xDim+1,input:nDimension() do
      self.inputSize[4] = self.inputSize[4] * input:size(i)
   end
   local reshapedInput = input:reshape(self.inputSize)
   
   -- prepare output of size K1 x oheight x owidth x K2
   self.outputSize[1] = self.inputSize[1]
   self.outputSize[2] = self.oheightCurrent
   self.outputSize[3] = self.owidthCurrent
   self.outputSize[4] = self.inputSize[4]
   self.output:resize(self.outputSize)
   
   -- resample over dims 2 and 3
   input.nn.SpatialReSamplingEx_updateOutput(self, input:reshape(self.inputSize))
   
   --resize output into the same shape as input
   local outputSize2 = input:size()
   outputSize2[self.yDim] = self.oheightCurrent
   outputSize2[self.xDim] = self.owidthCurrent
   self.output = self.output:reshape(outputSize2)
   return self.output
end

function SpatialReSamplingEx:updateGradInput(input, gradOutput)
   self.gradInput:resize(self.inputSize)
   input.nn.SpatialReSamplingEx_updateGradInput(self, gradOutput:reshape(self.outputSize))
   self.gradInput = self.gradInput:reshape(input:size())
   return self.gradInput
end
