local SpatialLinear, parent = torch.class('nn.SpatialLinear', 'nn.Module')

function SpatialLinear:__init(fanin, fanout)
   parent.__init(self)

   self.fanin = fanin or 1
   self.fanout = fanout or 1

   self.weightDecay = 0   
   self.weight = torch.Tensor(self.fanout, self.fanin)
   self.bias = torch.Tensor(self.fanout)
   self.gradWeight = torch.Tensor(self.fanout, self.fanin)
   self.gradBias = torch.Tensor(self.fanout)
   
   self.output = torch.Tensor(fanout,1,1)
   self.gradInput = torch.Tensor(fanin,1,1)

   self:reset()
end

function SpatialLinear:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1./math.sqrt(self.weight:size(1))
   end
   for i=1,self.weight:size(1) do
      self.weight:select(1, i):apply(function()
                                        return torch.uniform(-stdv, stdv)
                                     end)
      self.bias[i] = torch.uniform(-stdv, stdv)
   end
end

function SpatialLinear:zeroGradParameters(momentum)
   if momentum then
      self.gradWeight:mul(momentum)
      self.gradBias:mul(momentum)
   else
      self.gradWeight:zero()
      self.gradBias:zero()
   end
end

function SpatialLinear:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   self.bias:add(-learningRate, self.gradBias)
end

function SpatialLinear:decayParameters(decay)
   self.weight:add(-decay, self.weight)
   self.bias:add(-decay, self.bias)
end

function SpatialLinear:updateOutput(input)
   self.output:resize(self.fanout, input:size(2), input:size(3))
   input.nn.SpatialLinear_updateOutput(self, input)
   return self.output
end

function SpatialLinear:updateGradInput(input, gradOutput)
   self.gradInput:resize(self.fanin, input:size(2), input:size(3))
   input.nn.SpatialLinear_updateGradInput(self, input, gradOutput)
   return self.gradInput
end
