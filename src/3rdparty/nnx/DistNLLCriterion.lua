local DistNLLCriterion, parent = torch.class('nn.DistNLLCriterion', 'nn.Criterion')

function DistNLLCriterion:__init(opts)
   parent.__init(self)
   -- user options
   opts = opts or {}
   self.inputIsADistance = opts.inputIsADistance or false
   self.inputIsProbability = opts.inputIsProbability or false
   self.inputIsLogProbability = opts.inputIsLogProbability or false
   self.targetIsProbability = opts.targetIsProbability
   if self.targetIsProbability == nil then self.targetIsProbability = true end
   -- internal
   self.targetSoftMax = nn.SoftMax()
   self.inputLogSoftMax = nn.LogSoftMax()
   self.inputLog = nn.Log()
   self.gradLogInput = torch.Tensor()
   self.input = torch.Tensor()
end

function DistNLLCriterion:normalize(input, target)
   -- normalize target
   if not self.targetIsProbability then
      self.probTarget = self.targetSoftMax:updateOutput(target)
   else
      self.probTarget = target
   end

   -- flip input if a distance
   if self.inputIsADistance then
      self.input:resizeAs(input):copy(input):mul(-1)
   else
      self.input = input
   end

   -- normalize input
   if not self.inputIsLogProbability and not self.inputIsProbability then
      self.logProbInput = self.inputLogSoftMax:updateOutput(self.input)
   elseif not self.inputIsLogProbability then
      self.logProbInput = self.inputLog:updateOutput(self.input)
   else
      self.logProbInput = self.input
   end
end

function DistNLLCriterion:denormalize()
   -- denormalize gradients
   if not self.inputIsLogProbability and not self.inputIsProbability then
      self.gradInput = self.inputLogSoftMax:updateGradInput(self.input, self.gradLogInput)
   elseif not self.inputIsLogProbability then
      self.gradInput = self.inputLog:updateGradInput(self.input, self.gradLogInput)
   else
      self.gradInput = self.gradLogInput
   end

   -- if input is a distance, then flip gradients back
   if self.inputIsADistance then
      self.gradInput:mul(-1)
   end
end

function DistNLLCriterion:updateOutput(input, target)
   self:normalize(input, target)
   self.output = 0
   for i = 1,input:size(1) do
      self.output = self.output - self.logProbInput[i] * self.probTarget[i]
   end
   return self.output
end

function DistNLLCriterion:updateGradInput(input, target)
   self:normalize(input, target)
   self.gradLogInput:resizeAs(input)
   for i = 1,input:size(1) do
      self.gradLogInput[i] = -self.probTarget[i]
   end
   self:denormalize()
   return self.gradInput
end
