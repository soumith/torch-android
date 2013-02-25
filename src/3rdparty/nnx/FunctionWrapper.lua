local FunctionWrapper, parent = torch.class('nn.FunctionWrapper', 'nn.Module')

local help_desc = [[
      Dummy module that takes a forward and a backward function as argument.
]]

function FunctionWrapper:__init(init, updateOutput, updateGradInput)
   init(self)
   self.fn_updateOutput = updateOutput
   self.fn_updateGradInput = updateGradInput
end

function FunctionWrapper:updateOutput(input)
   self.output = self.fn_updateOutput(self, input)
   return self.output
end

function FunctionWrapper:updateGradInput(input, gradOutput)
   self.gradInput = self.fn_updateGradInput(self, input, gradOutput)
   return self.gradInput
end