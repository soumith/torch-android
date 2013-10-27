local Minus, parent = torch.class('nn.Minus', 'nn.Module')

function Minus:updateOutput(input)
   self.output:resizeAs(input):copy(input):mul(-1)
   return self.output
end

function Minus:updateGradInput(input, gradOutput)
   self.gradInput:resizeAs(input):copy(gradOutput):mul(-1)
   return self.gradInput
end
