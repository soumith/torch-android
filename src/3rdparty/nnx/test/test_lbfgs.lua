dofile('rosenbrock.lua')
dofile('l2.lua')

require 'liblbfgs'
maxIterations = 100
maxLineSearch = 40
-- this is to compare with minFunc
maxEvaluation = 25
linesearch = 0 
sparsity = 0
verbose = 3
nparam = 2

local testfunc = rosenbrock

local parameters  = torch.Tensor(nparam):zero()
local gradParameters = torch.Tensor(nparam):zero()

output, gradParameters = testfunc(parameters,gradParameters)

lbfgs.evaluate 
   = function()
	output, gradParameters = testfunc(parameters,gradParameters)
	return output
     end

-- init LBFGS state
lbfgs.init(parameters, gradParameters,
           maxEvaluation, maxIterations, maxLineSearch,
           sparsity, linesearch, verbose)

output = lbfgs.run()

