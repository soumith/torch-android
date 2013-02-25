local DistMarginCriterion, parent = torch.class('nn.DistMarginCriterion', 'nn.Criterion')

function DistMarginCriterion:__init()
   parent.__init(self)
   self.sizeAverage = true
end

function DistMarginCriterion:updateOutput(input, target)
   return input.nn.DistMarginCriterion_updateOutput(self, input, target)
end

function DistMarginCriterion:updateGradInput(input, target)
   return input.nn.DistMarginCriterion_updateGradInput(self, input, target)
end
