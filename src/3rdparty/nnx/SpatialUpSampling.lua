local SpatialUpSampling, parent = torch.class('nn.SpatialUpSampling', 'nn.Module')

local help_desc = [[
Applies a 2D up-sampling over an input image composed of
several input planes. The input tensor in forward(input) is
expected to be a 3D tensor (nInputPlane x width x height).
The number of output planes will be the same as nInputPlane.

The upsampling is done using the simple nearest neighbor
technique. For interpolated (bicubic) upsampling, use 
nn.SpatialReSampling().

If the input image is a 3D tensor nInputPlane x width x height,
the output image size will be nInputPlane x owidth x oheight where

owidth  = width*dW
oheight  = height*dH ]]

function SpatialUpSampling:__init(...)
   parent.__init(self)

   -- get args
   xlua.unpack_class(self, {...}, 'nn.SpatialUpSampling',  help_desc,
                     {arg='dW', type='number', help='stride width', req=true},
                     {arg='dH', type='number', help='stride height', req=true},
		     {arg='yDim', type='number', help='image y dimension', default=2},
		     {arg='xDim', type='number', help='image x dimension', default=3}
		  )
   if self.yDim+1 ~= self.xDim then
      error('nn.SpatialUpSampling: yDim must be equals to xDim-1')
   end
   self.outputSize = torch.LongStorage(4)
   self.inputSize = torch.LongStorage(4)
end

function SpatialUpSampling:updateOutput(input)
   self.inputSize:fill(1)
   for i = 1,self.yDim-1 do
      self.inputSize[1] = self.inputSize[1] * input:size(i)
   end
   self.inputSize[2] = input:size(self.yDim)
   self.inputSize[3] = input:size(self.xDim)
   for i = self.xDim+1,input:nDimension() do
      self.inputSize[4] = self.inputSize[4] * input:size(i)
   end
   self.outputSize[1] = self.inputSize[1]
   self.outputSize[2] = self.inputSize[2] * self.dH
   self.outputSize[3] = self.inputSize[3] * self.dW
   self.outputSize[4] = self.inputSize[4]
   self.output:resize(self.outputSize)
   input.nn.SpatialUpSampling_updateOutput(self, input:reshape(self.inputSize))
   local outputSize2 = input:size()
   outputSize2[self.yDim] = outputSize2[self.yDim] * self.dH
   outputSize2[self.xDim] = outputSize2[self.xDim] * self.dW
   self.output = self.output:reshape(outputSize2)
   return self.output
end

function SpatialUpSampling:updateGradInput(input, gradOutput)
   self.gradInput:resize(self.inputSize)
   input.nn.SpatialUpSampling_updateGradInput(self, input,
					      gradOutput:reshape(self.outputSize))
   self.gradInput = self.gradInput:reshape(input:size())
   return self.gradInput
end
