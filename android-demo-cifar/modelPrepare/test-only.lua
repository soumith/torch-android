----------------------------------------------------------------------
-- This script test Cifar-10 test data set with pre-trained network.
-- 
-- This script is based on file from below address.
--
-- https://github.com/torch/demos/blob/master/train-on-cifar/train-on-cifar.lua
-- 
-- Training code are removed.
-- This script require trained model network and std_u, mean_u, std_v, mean_v files 
-- from train-on-cifar.lua in same directory
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
require 'optim'
require 'image'

----------------------------------------------------------------------
-- parse command-line options
--
dname,fname = sys.fpath()
cmd = torch.CmdLine()
cmd:text()
cmd:text('CIFAR Training')
cmd:text()
cmd:text('Options:')
cmd:option('-save', fname:gsub('.lua',''), 'subdirectory to save/log experiments in')
cmd:option('-network', '', 'reload pretrained network')
cmd:option('-model', 'convnet', 'type of model to train: convnet | mlp | linear')
cmd:option('-full', false, 'use full dataset (50,000 samples)')
cmd:option('-visualize', false, 'visualize input data and weights during training')
cmd:option('-seed', 1, 'fixed input seed for repeatable experiments')
cmd:option('-optimization', 'SGD', 'optimization method: SGD | ASGD | CG | LBFGS')
cmd:option('-learningRate', 1e-3, 'learning rate at t=0')
cmd:option('-batchSize', 1, 'mini-batch size (1 = pure stochastic)')
cmd:option('-weightDecay', 0, 'weight decay (SGD only)')
cmd:option('-momentum', 0, 'momentum (SGD only)')
cmd:option('-t0', 1, 'start averaging at t0 (ASGD only), in nb of epochs')
cmd:option('-maxIter', 5, 'maximum nb of iterations for CG and LBFGS')
cmd:option('-threads', 8, 'nb of threads to use')
cmd:text()
opt = cmd:parse(arg)

-- fix seed
torch.manualSeed(opt.seed)

-- threads
torch.setnumthreads(opt.threads)
print('<torch> set nb of threads to ' .. opt.threads)

----------------------------------------------------------------------
-- define model to train
-- on the 10-class classification problem
--
classes = {'airplane', 'automobile', 'bird', 'cat',
           'deer', 'dog', 'frog', 'horse', 'ship', 'truck'}

print(opt.network)
model=torch.load(opt.network)

-- retrieve parameters and gradients
parameters,gradParameters = model:getParameters()

-- verbose
print('<cifar> using model:')
print(model)

----------------------------------------------------------------------
-- loss function: negative log-likelihood
--
model:add(nn.LogSoftMax())
criterion = nn.ClassNLLCriterion()

----------------------------------------------------------------------
-- get/create dataset
--
if opt.full then
   tesize = 10000
else
   tesize = 1000
end

-- download dataset
if not paths.dirp('cifar-10-batches-t7') then
   local www = 'http://torch7.s3-website-us-east-1.amazonaws.com/data/cifar-10-torch.tar.gz'
   local tar = paths.basename(www)
   os.execute('wget ' .. www .. '; '.. 'tar xvf ' .. tar)
end

-- load dataset
subset = torch.load('cifar-10-batches-t7/test_batch.t7', 'ascii')
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

----------------------------------------------------------------------
-- preprocess/normalize train/test sets
--

print '<trainer> preprocessing data (color space + normalization)'

-- preprocess trainSet
normalization = nn.SpatialContrastiveNormalization(1, image.gaussian1D(7))
-- preprocess testSet
for i = 1,testData:size() do
   -- rgb -> yuv
   local rgb = testData.data[i]
   local yuv = image.rgb2yuv(rgb)
   -- normalize y locally:
   yuv[{1}] = normalization(yuv[{{1}}])
   testData.data[i] = yuv
end
mean_u=torch.load('mean_u')
mean_v=torch.load('mean_v')
std_u=torch.load('std_u')
std_v=torch.load('std_v')
-- normalize u globally:
testData.data[{ {},2,{},{} }]:add(-mean_u)
testData.data[{ {},2,{},{} }]:div(-std_u)
-- normalize v globally:
testData.data[{ {},3,{},{} }]:add(-mean_v)
testData.data[{ {},3,{},{} }]:div(-std_v)

----------------------------------------------------------------------
-- define training and testing functions
--

-- this matrix records the current confusion across classes
confusion = optim.ConfusionMatrix(classes)

-- log results to files
testLogger = optim.Logger(paths.concat(opt.save, 'test.log'))

-- display function
function display(input)
   iter = iter or 0
   require 'image'
   win_input = image.display{image=input, win=win_input, zoom=2, legend='input'}
   if iter%10 == 0 then
      if opt.model == 'convnet' then
         win_w1 = image.display{image=model:get(2).weight, zoom=4, nrow=10,
                                min=-1, max=1,
                                win=win_w1, legend='stage 1: weights', padding=1}
         win_w2 = image.display{image=model:get(6).weight, zoom=4, nrow=30,
                                min=-1, max=1,
                                win=win_w2, legend='stage 2: weights', padding=1}
      elseif opt.model == 'mlp' then
         local W1 = torch.Tensor(model:get(2).weight):resize(2048,1024)
         win_w1 = image.display{image=W1, zoom=0.5,
                                min=-1, max=1,
                                win=win_w1, legend='W1 weights'}
         local W2 = torch.Tensor(model:get(2).weight):resize(10,2048)
         win_w2 = image.display{image=W2, zoom=0.5,
                                min=-1, max=1,
                                win=win_w2, legend='W2 weights'}
      end
   end
   iter = iter + 1
end

-- test function
function test(dataset)
   -- local vars
   local time = sys.clock()

   -- averaged param use?
   if average then
      cachedparams = parameters:clone()
      parameters:copy(average)
   end

   -- test over given dataset
   print('<trainer> on testing Set:')
   for t = 1,dataset:size() do
      -- disp progress
      xlua.progress(t, dataset:size())

      -- get new sample
      local input = dataset.data[t]
      local target = dataset.labels[t]

      -- test sample
      local pred = model:forward(input)
      confusion:add(pred, target)
   end

   -- timing
   time = sys.clock() - time
   time = time / dataset:size()
   print("<trainer> time to test 1 sample = " .. (time*1000) .. 'ms')

   -- print confusion matrix
   print(confusion)
   testLogger:add{['% mean class accuracy (test set)'] = confusion.totalValid * 100}
   confusion:zero()

   -- averaged param use?
   if average then
      -- restore parameters
      parameters:copy(cachedparams)
   end
end

----------------------------------------------------------------------
-- and test!
--
-- train/test
test(testData)
-- plot errors
testLogger:style{['% mean class accuracy (test set)'] = '-'}
testLogger:plot()
