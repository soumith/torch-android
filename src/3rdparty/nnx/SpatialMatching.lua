local SpatialMatching, parent = torch.class('nn.SpatialMatching', 'nn.Module')

function SpatialMatching:__init(maxh, maxw, full_output)
   -- If full_output is false, output is computed on elements of the first input
   -- for which all the possible corresponding elements exist in the second input
   -- In addition, if full_output is set to false, the pixel (1,1) of the first input
   -- is supposed to correspond to the pixel (maxh/2, maxw/2) of the second one
   parent.__init(self)
   self.maxw = maxw or 11
   self.maxh = maxh or 11
   if full_output == nil then
      full_output = false
   end
   self.full_output = full_output
   self.gradInput1 = torch.Tensor()
   self.gradInput2 = torch.Tensor()
end

function SpatialMatching:updateOutput(input)
   -- input is a table of 2 inputs, each one being KxHxW
   -- if not full_output, the 1st one is KxH1xW1 where H1 <= H-maxh+1, W1 <= W-maxw+1
   self.output:resize(input[1]:size(2), input[1]:size(3), self.maxh, self.maxw)
   input[1].nn.SpatialMatching_updateOutput(self, input[1], input[2])
   return self.output
end

function SpatialMatching:updateGradInput(input, gradOutput)
   self.gradInput1:resize(input[1]:size()):zero()
   self.gradInput2:resize(input[2]:size()):zero()
   input[1].nn.SpatialMatching_updateGradInput(self, input[1], input[2], gradOutput)
   self.gradInput = {self.gradInput1, self.gradInput2}
   return self.gradInput
end
