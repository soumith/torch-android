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
--- print("Running cutorch.test()")
--- cutorch.test()

--- print("Running nn.testcuda(pointwise_forward:)")
--- nn.testcuda{'pointwise_forward'}

--- print('Running VolumetricConvolution_forward_batch:')
--- nn.testcuda{'VolumetricConvolution_forward_batch'}

torch.setdefaulttensortype('torch.FloatTensor')
-- Doing a small benchmark of the convolution module
in_planes = 3
out_planes = 16
imsz_x = 640
imsz_y = 480
num_ops = 2 -- 2 ops, i.e one for multiply and one for accumulate

test_tensor = torch.rand(in_planes,imsz_x,imsz_y):cuda()

for kernel_sz=1,16 do
   local model = nn.SpatialConvolution(in_planes,out_planes,kernel_sz,kernel_sz):cuda()
   tstart = os.clock()
   output1 = model:forward(test_tensor)
   tend = os.clock()
   print('-------------------------------------------------------------------------------------------')
   print('Input Size: ' .. in_planes .. 'x' .. imsz_x .. 'x' .. imsz_y
            .. '\t\tKernel Size: ' .. kernel_sz .. 'x' .. kernel_sz .. '\t\tOutput Planes:' .. out_planes)
   print('Time taken (in seconds): ' .. (tend-tstart))
   total_ops = in_planes*kernel_sz*kernel_sz*out_planes*(imsz_x-kernel_sz+1)*(imsz_y-kernel_sz+1)*num_ops
   print('GOps:' .. total_ops / 1e9)
   print('Gops/s: ' .. (  total_ops/( (tend-tstart) * 1e9)))
   print('-------------------------------------------------------------------------------------------')
end
