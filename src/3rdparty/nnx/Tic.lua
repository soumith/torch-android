require 'nn'
local Tic, parent = torch.class('nn.Tic', 'nn.Module')

function Tic:__init(name)
   parent.__init(self)
   self.name = name or 'default'
   tic_modules = tic_modules or {}
   tic_modules[self.name] = torch.Timer()
end

function Tic:updateOutput(input)
   tic_modules[self.name]:reset()
   self.output = input
   return self.output
end

function Tic:updateGradInput(input, gradOutput) 
   self.gradInput = gradOutput
   return self.gradInput
end
