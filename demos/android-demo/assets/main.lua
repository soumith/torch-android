-- Torch Android demo script
-- Script: main.lua
-- Copyright (C) 2013 Soumith Chintala

require 'torch'
require 'cutorch'
require 'cunn'
-- require 'cudnn'
require 'nnx'
require 'dok'
require 'image'

function demoluafn()
   ret = 'Called demo function from inside lua'
   print()
   return ret
end

print("Hello from Lua")

-- CuTorch test needs augmentation due to reduced type set.
-- print("Running cutorch.test()")
-- cutorch.test()


print("Running cunn.test()")
cunn.test('GPU')
print("After cunn.test(GPU)")

-- nn.testcuda{'VolumetricConvolution_forward_batch'}
-- cunn.test()
--print("Running cudnn.test()")
--cudnn.test()

cunn.test('SpatialConvolutionLocal_forward_single')

--- print('Running VolumetricConvolution_forward_batch:')
--- nn.testcuda{'VolumetricConvolution_forward_batch'}
