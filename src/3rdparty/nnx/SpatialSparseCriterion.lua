local SpatialSparseCriterion, parent = torch.class('nn.SpatialSparseCriterion', 'nn.SparseCriterion')

function SpatialSparseCriterion:__init(...)
   parent.__init(self)

   xlua.unpack_class(self, {...},
      'nn.SpatialSparseCriterion',
      'A spatial extension of the SparseCriterion class.\n'
        ..' Provides a set of parameters to deal with spatial mini-batch training.',
      {arg='nbGradients', type='number', help='number of gradients to backpropagate (-1:all, >=1:nb)', default=-1},
      {arg='sizeAverage', type='number', help='if true, forward() returns an average instead of a sum of errors', default=true}
   )
end

function SpatialSparseCriterion:updateOutput(input)
   self.fullOutput = self.fullOutput or torch.Tensor()
   self.fullOutput:resize(input:size(2), input:size(3))
   input.nn.SpatialSparseCriterion_updateOutput(self, input)
   if self.sizeAverage then
      self.output = self.fullOutput:mean()
   else
      self.output = self.fullOutput:sum()
   end
   return self.output
end

function SpatialSparseCriterion:updateGradInput(input,target)
   -- (1) retrieve adjusted target
   target = self.target
   -- (2) resize input gradient map
   self.gradInput:resizeAs(input):zero()
   -- (3) compute input gradients, based on the nbGradients param
   if self.nbGradients == -1 then
      -- dense gradients
      input.nn.SpatialSparseCriterion_updateGradInput(self, input, self.gradInput)
   elseif self.nbGradients == 1 then
      -- only 1 gradient is computed, sampled in the center
      self.fullGradInput = torch.Tensor() or self.fullGradInput
      self.fullGradInput:resizeAs(input):zero()
      input.nn.SpatialSparseCriterion_updateGradInput(self, input, self.fullGradInput)
      local y = math.ceil(self.gradInput:size(2)/2)
      local x = math.ceil(self.gradInput:size(3)/2)
      self.gradInput:select(3,x):select(2,y):copy(self.fullGradInput:select(3,x):select(2,y))
   else
      -- only N gradients are computed, sampled in random locations
      self.fullGradInput = torch.Tensor() or self.fullGradInput
      self.fullGradInput:resizeAs(input):zero()
      input.nn.SpatialSparseCriterion_updateGradInput(self, input, self.fullGradInput)
      for i = 1,self.nbGradients do
         local x = math.random(1,self.gradInput:size(1))
         local y = math.random(1,self.gradInput:size(2))
         self.gradInput:select(3,x):select(2,y):copy(self.fullGradInput:select(3,x):select(2,y))
      end
   end
   return self.gradInput
end
