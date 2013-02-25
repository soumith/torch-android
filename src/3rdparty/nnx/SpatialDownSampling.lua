local SpatialDownSampling, parent = torch.class('nn.SpatialDownSampling', 'nn.Module')

local help_desc = [[
Applies a 2D down-sampling over an input image composed of
several input planes. The input tensor in forward(input) is
expected to be a 3D tensor (nInputPlane x width x height).
The number of output planes will be the same as nInputPlane.

The downsampling is done using the simple average
technique. For interpolated (bicubic) downsampling, use 
nn.SpatialReSampling().

If the input image is a 3D tensor nInputPlane x width x height,
the output image size will be nInputPlane x owidth x oheight where

owidth  = floor(width/rW)
oheight  = floor(height/rH) ]]

function SpatialDownSampling:__init(...)
   parent.__init(self)

   -- get args
   xlua.unpack_class(self, {...}, 'nn.SpatialDownSampling',  help_desc,
                     {arg='rW', type='number', help='ratio width', req=true},
                     {arg='rH', type='number', help='ratio height', req=true})
end

function SpatialDownSampling:updateOutput(input)
   self.output:resize(input:size(1), math.floor(input:size(2) / self.rH),
		      math.floor(input:size(3) / self.rW))
   input.nn.SpatialDownSampling_updateOutput(self, input)
   return self.output
end

function SpatialDownSampling:updateGradInput(input, gradOutput)
   self.gradInput:resizeAs(input)
   input.nn.SpatialDownSampling_updateGradInput(self, gradOutput)
   return self.gradInput
end
