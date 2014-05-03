local SaturatedLU, parent = torch.class('nn.SaturatedLU','nn.Module')

function SaturatedLU:__init(th,v,th2,v2)
   parent.__init(self)
   self.threshold = th or -1.0
   self.val = v or -1.0
   self.threshold2 = th2 or 1.0
   self.val2 = v2 or 1.0
   if (th and type(th) ~= 'number') or (v and type(v) ~= 'number')
      or (th2 and type(th2) ~= 'number') or (v2 and type(v2) ~= 'number') then
	 error('nn.SaturatedLU(lower-bound, value, upper-bound, value2)')
   end
end

function SaturatedLU:updateOutput(input)
   self.output = input:clone()
   self.output[self.output:lt(self.threshold)] = self.val
   self.output[self.output:gt(self.threshold2)] = self.val2
   return self.output
end

function SaturatedLU:updateGradInput(input, gradOutput)
   self.gradInput = gradOutput:clone()
   self.gradInput[input:lt(self.threshold)] = 0
   self.gradInput[input:gt(self.threshold2)] = 0
   return self.gradInput
end