local SparseCriterion, parent = torch.class('nn.SparseCriterion', 'nn.Criterion')

function SparseCriterion:__init()
   parent.__init(self)
   self.sizeAverage = true
end

function SparseCriterion:updateOutput(input)
   input.nn.SparseCriterion_updateOutput(self, input)
   return self.output
end

function SparseCriterion:updateGradInput(input)
   input.nn.SparseCriterion_updateGradInput(self, input)
   return self.gradInput
end
