------------------------------------------------------------
-- this simple script demonstrates the use of 
-- approximate second-order information to calibrate
-- the learning rates individually
--
-- given an input vector X, we want to learn a mapping
-- f(X) = \sum_i X_i
-- 
-- we use a two-layer perceptron, just to validate
-- the tanh+linear hessian 
-- (of course learning such a function is much more
--  trivial using a single linear layer :-)
--

-- libs
require 'nnx'

-- fix random seed
random.manualSeed(1)

-- SGD params
learningRate = 1e-3
diagHessianEpsilon = 1e-3
computeDiagHessian = true    -- SET THIS FLAG TO FALSE TO SEE THE EFFECT OF THE DIAG HESSIAN

-- fake data
inputs = {}
targets = {}
for i = 1,1000 do
   inputs[i] = lab.randn(10)
   targets[i] = torch.Tensor(1):fill(inputs[i]:sum())
end

-- create module
module = nn.Sequential()
module:add(nn.Linear(10,10))
module:add(nn.Tanh())
module:add(nn.Linear(10,1))

-- loss
criterion = nn.MSECriterion()

-- get params
parameters = nnx.flattenParameters(nnx.getParameters(module))
gradParameters = nnx.flattenParameters(nnx.getGradParameters(module))

-- compute learning rates
learningRates = torch.Tensor(parameters:size()):fill(1)
if computeDiagHessian then
   -- init diag hessian
   module:initDiagHessianParameters()
   diagHessianParameters = nnx.flattenParameters(nnx.getDiagHessianParameters(module))

   -- estimate diag hessian over dataset
   diagHessianParameters:zero()
   for i = 1,#inputs do
      local output = module:forward(inputs[i])
      local critDiagHessian = criterion:backwardDiagHessian(output, targets[i])
      module:backwardDiagHessian(inputs[i], critDiagHessian)
      module:accDiagHessianParameters(inputs[i], critDiagHessian)
   end
   diagHessianParameters:div(#inputs)

   -- protect diag hessian (the proper way of doing it is the commented code,
   -- but for speed reasons, the uncommented code just works)
   --diagHessianParameters:apply(function(x) return math.max(x, diagHessianEpsilon) end)
   diagHessianParameters:add(diagHessianEpsilon)

   -- now learning rates are obtained like this:
   learningRates:cdiv(diagHessianParameters)

   -- print info
   print('learning rates calculated to')
   print(learningRates)
end

-- regular SGD
for epoch = 1,100 do
   error = 0
   for i = 1,#inputs do
      -- backprop gradients
      local output = module:forward(inputs[i])
      local critGradInput = criterion:backward(output, targets[i])
      module:backward(inputs[i], critGradInput)

      -- print current error
      error = error + criterion:forward(output, targets[i])

      -- gradients wrt parameters
      gradParameters:zero()
      module:accGradParameters(inputs[i], critGradInput)

      -- given a parameter vector, and a gradParameter vector, the update goes like this:
      deltaParameters = deltaParameters or parameters.new()
      deltaParameters:resizeAs(gradParameters):copy(learningRates):cmul(gradParameters)
      parameters:add(-learningRate, deltaParameters)
   end
   error = error / #inputs
   print('current average error: ' .. error)
end

-- test vector
input = lab.randn(10)
groundtruth = input:sum()
output = module:forward(input)
print('test input:') print(input)
print('predicted output:', output[1])
print('groundtruth (\sum_i X_i):', groundtruth)
