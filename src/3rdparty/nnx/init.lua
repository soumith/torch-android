----------------------------------------------------------------------
--
-- Copyright (c) 2011 Clement Farabet, Marco Scoffier, 
--                    Koray Kavukcuoglu, Benoit Corda
--
-- 
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
-- 
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
-- LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
-- OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
-- WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-- 
----------------------------------------------------------------------
-- description:
--     xlua - lots of new trainable modules that extend the nn 
--            package.
--
-- history: 
--     July  5, 2011, 8:51PM - import from Torch5 - Clement Farabet
----------------------------------------------------------------------

-- create global nnx table:
nnx = {}

-- tools:
torch.include('nnx', 'Probe.lua')
torch.include('nnx', 'Tic.lua')
torch.include('nnx', 'Toc.lua')

-- spatial (images) operators:
torch.include('nnx', 'SpatialLinear.lua')
torch.include('nnx', 'SpatialClassifier.lua')
torch.include('nnx', 'SpatialNormalization.lua')
torch.include('nnx', 'SpatialPadding.lua')
torch.include('nnx', 'SpatialReSamplingEx.lua')
torch.include('nnx', 'SpatialUpSampling.lua')
torch.include('nnx', 'SpatialDownSampling.lua')
torch.include('nnx', 'SpatialReSampling.lua')
torch.include('nnx', 'SpatialRecursiveFovea.lua')
torch.include('nnx', 'SpatialFovea.lua')
torch.include('nnx', 'SpatialPyramid.lua')
torch.include('nnx', 'SpatialGraph.lua')
torch.include('nnx', 'SpatialMatching.lua')
torch.include('nnx', 'SpatialRadialMatching.lua')
torch.include('nnx', 'SpatialMaxSampling.lua')
torch.include('nnx', 'SpatialColorTransform.lua')

-- other modules
torch.include('nnx', 'FunctionWrapper.lua')

-- criterions:
torch.include('nnx', 'SuperCriterion.lua')
torch.include('nnx', 'DistNLLCriterion.lua')
torch.include('nnx', 'DistMarginCriterion.lua')

-- datasets:
torch.include('nnx', 'DataSet.lua')
torch.include('nnx', 'DataList.lua')
torch.include('nnx', 'DataSetLabelMe.lua')
torch.include('nnx', 'DataSetSamplingPascal.lua')
