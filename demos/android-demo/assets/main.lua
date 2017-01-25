-- Torch Android demo script
-- Script: main.lua
-- Copyright (C) 2013 Soumith Chintala

require 'torch'
require 'cutorch'
require 'cunn'
require 'nnx'
require 'dok'
require 'image'

function demoluafn()
   ret = 'Called demo function from inside lua'
   print()
   return ret
end

print("Hello from Lua")

---
print("Running cutorch.test()")
cutorch.test()


print("Running cunn.test()")
cunn.test()

-- nn.testcuda{'pointwise_forward'}

--- print('Running VolumetricConvolution_forward_batch:')
--- nn.testcuda{'VolumetricConvolution_forward_batch'}
