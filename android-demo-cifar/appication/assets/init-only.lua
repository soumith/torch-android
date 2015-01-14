----------------------------------------------------------------------
-- This script is based on file from below address.
--
-- https://github.com/torch/demos/blob/master/train-on-cifar/train-on-cifar.lua
-- 
-- Some lines are added for using model on android demo application.
-- without require Cifar-10 data set again for recognize test.
-- We needs trained model network and values of std_u, mean_u, std_v, mean_v
-- for stand alone recognition demo application.
--
-- Karam Park
----------------------------------------------------------------------
-- This script shows how to train different models on the CIFAR
-- dataset, using multiple optimization techniques (SGD, ASGD, CG)
--
-- This script demonstrates a classical example of training 
-- well-known models (convnet, MLP, logistic regression)
-- on a 10-class classification problem. 
--
-- It illustrates several points:
-- 1/ description of the model
-- 2/ choice of a loss function (criterion) to minimize
-- 3/ creation of a dataset as a simple Lua table
-- 4/ description of training and test procedures
--
-- Clement Farabet
----------------------------------------------------------------------

require 'torch'
require 'nn'
require 'nnx'
require 'dok'
require 'image'
----------------------------------------------------------------------
-- set options

-- fix seed
torch.manualSeed(1)
-- set number of threads
torch.setnumthreads(2)

-- classname for cifar 10
classes = {'airplane', 'automobile', 'bird', 'cat',
  'deer', 'dog', 'frog', 'horse', 'ship', 'truck'}

print('load pretrained network file')
-- load pretrained network file from apk's asset folder
model=torch.load('cifar.net','r','apk')

-- retrieve parameters and gradients
parameters,gradParameters = model:getParameters()

model:add(nn.LogSoftMax())
criterion = nn.ClassNLLCriterion()

-- read mean & std values
mean_u=torch.load('mean_u','r','apk')
--print('mean_u ' .. mean_u)
mean_v=torch.load('mean_v','r','apk')
--print('mean_v ' .. mean_v)
std_u=torch.load('std_u','r','apk')
--print('std_u ' .. std_u)
std_v=torch.load('std_v','r','apk')
--print('std_v ' .. std_v)


-- Test in torch test set 
function testTorchData()

   --set number of test (max 10,000 samples)
   tesize = 1000

   -- load dataset
   subset = torch.load('/sdcard/Cifar/test_batch.t7', 'ascii', 'r')
   testData = {
      data = subset.data:t():double(),
      labels = subset.labels[1]:double(),
      size = function() return tesize end
   }
   testData.labels = testData.labels + 1

   -- resize dataset (if using small version)
   testData.data = testData.data[{ {1,tesize} }]
   testData.labels = testData.labels[{ {1,tesize} }]

   -- reshape data
   testData.data = testData.data:reshape(tesize,3,32,32)
   local correct = 0
   for i = 1,testData:size() do
      local result = getTopOnly(testData.data[i])
      if result == testData.labels[i] then
         correct = correct + 1;
      end
   end
   return correct/tesize
end

-- function for test image
function getRecogResult(sampledata)
  sampledata = sampledata:reshape(3,32,32)
  data = torch.DoubleTensor():set(sampledata)

  normalization = nn.SpatialContrastiveNormalization(1, image.gaussian1D(7))
  -- preprocess testSet

  --print 'rgb -> yuv'
  local rgb = data

  local yuv = image.rgb2yuv(rgb)

  -- print 'normalize y locally'
  yuv[1] = normalization(yuv[{{1}}])
  data = yuv

  -- normalize u globally:
  data[{ 2,{},{} }]:add(-mean_u)
  data[{ 2,{},{} }]:div(-std_u)
  -- normalize v globally:
  data[{ 3,{},{} }]:add(-mean_v)
  data[{ 3,{},{} }]:div(-std_v)

  local result = model:forward(data)
  return result
end

function getTopOnly(sampledata)
  local result = getRecogResult(sampledata)
  local max
  local index = 0
  
   for k=1,10 do
      if max == nil or max < result[k] then
         index = k
         max = result[k]
      end
   end
   return index
end
