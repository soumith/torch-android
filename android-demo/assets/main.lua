-- Torch Android demo script
-- Script: main.lua
-- Copyright (C) 2013 Soumith Chintala

require 'torch'
require 'image'

function demoluafn()
   ret = 'Called demo function from inside lua'
   print()
   return ret
end

print("Hello from Lua")
a=torch.ones(3,100,100)
print("Created a torch Tensor " .. " of size: " ..
      a:size(1) .. 'x' .. a:size(2) .. 'x' .. a:size(3) ..
      " having a sum of " .. a:sum())
