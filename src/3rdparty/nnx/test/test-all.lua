
local nnxtest = {}
local precision = 1e-5
local mytester

function nnxtest.SpatialPadding()
   local fanin = math.random(1,3)
   local sizex = math.random(4,16)
   local sizey = math.random(4,16)
   local pad_l = math.random(0,8)
   local pad_r = math.random(0,8)
   local pad_t = math.random(0,8)
   local pad_b = math.random(0,8)
   local module = nn.SpatialPadding(pad_l, pad_r, pad_t, pad_b)
   local input = torch.rand(fanin,sizey,sizex)

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.SpatialLinear()
   local fanin = math.random(1,10)
   local fanout = math.random(1,10)
   local in1 = torch.rand(fanin,1,1)
   local module = nn.SpatialLinear(fanin,fanout)
   local moduleg = nn.Linear(fanin,fanout)
   moduleg.weight:copy(module.weight)
   moduleg.bias:copy(module.bias)
   local out = module:forward(in1)
   local ground = moduleg:forward(in1:select(2,1,1):select(2,1,1))
   local err = out:dist(ground)
   mytester:assertlt(err, precision, torch.typename(module) .. ' - forward err ')

   local fanin = math.random(1,10)
   local fanout = math.random(1,10)
   local sizex = math.random(4,16)
   local sizey = math.random(4,16)
   local module = nn.SpatialLinear(fanin, fanout)
   local input = torch.rand(fanin,sizey,sizex)

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local err = nn.Jacobian.testJacobianParameters(module, input, module.weight, module.gradWeight)
   mytester:assertlt(err, precision, 'error on weight ')

   local err = nn.Jacobian.testJacobianParameters(module, input, module.bias, module.gradBias)
   mytester:assertlt(err, precision, 'error on bias ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.SpatialMaxPooling()
   local fanin = math.random(1,4)
   local osizex = math.random(1,4)
   local osizey = math.random(1,4)
   local mx = math.random(2,6)
   local my = math.random(2,6)
   local sizex = osizex*mx
   local sizey = osizey*my
   local module = nn.SpatialMaxPooling(mx,my)
   local input = torch.rand(fanin,sizey,sizex)

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

local function template_SpatialReSamplingEx(up, mode)
   for iTest = 1,3 do
      local nDims = math.random(2,6)
      local dims = torch.LongStorage(nDims)
      for i = 1,nDims do
	 dims[i] = math.random(5,20/nDims)
      end
      local xratio, yratio
      if up then
	 xratio = torch.uniform(1.5, 10)
	 yratio = torch.uniform(1.5, 10)
      else
	 xratio = torch.uniform(0.41, 0.7)
	 yratio = torch.uniform(0.41, 0.7)
      end
      local ydim = math.random(1,nDims-1)
      local xdim = ydim+1
      local owidth_ = math.floor(dims[xdim]*xratio+0.5)
      local oheight_ = math.floor(dims[ydim]*yratio+0.5)
      local module = nn.SpatialReSamplingEx({owidth=owidth_, oheight=oheight_,
					     xDim=xdim, yDim = ydim, mode=mode})
      local input = torch.rand(dims)
      
      local err = nn.Jacobian.testJacobian(module, input)
      mytester:assertlt(err, precision, 'error on state ')
      
      local ferr, berr = nn.Jacobian.testIO(module, input)
      mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
      mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
   end
end

function nnxtest.SpatialReSamplingEx1() template_SpatialReSamplingEx(true , 'simple'  ) end
function nnxtest.SpatialReSamplingEx2() template_SpatialReSamplingEx(false, 'simple'  ) end
function nnxtest.SpatialReSamplingEx3() template_SpatialReSamplingEx(false, 'average' ) end
function nnxtest.SpatialReSamplingEx4() template_SpatialReSamplingEx(true , 'bilinear') end
function nnxtest.SpatialReSamplingEx5() template_SpatialReSamplingEx(false, 'bilinear') end

function nnxtest.SpatialUpSampling()
   local fanin = math.random(1,4)
   local sizex = math.random(1,4)
   local sizey = math.random(1,4)
   local mx = math.random(2,6)
   local my = math.random(2,6)
   local module = nn.SpatialUpSampling(mx,my)
   local input = torch.rand(fanin,sizey,sizex)

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.SpatialDownSampling()
   local fanin = math.random(1,4)
   local sizex = math.random(1,4)
   local sizey = math.random(1,4)
   local mx = math.random(2,6)
   local my = math.random(2,6)
   local module = nn.SpatialDownSampling(mx,my)
   local input = torch.rand(fanin,sizey,sizex)

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.SpatialReSampling_1()
   local fanin = math.random(1,4)
   local sizex = math.random(4,8)
   local sizey = math.random(4,8)
   local osizex = math.random(2,12)
   local osizey = math.random(2,12)
   local module = nn.SpatialReSampling{owidth=osizex,oheight=osizey}
   local input = torch.rand(fanin,sizey,sizex)

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.SpatialReSampling_2()
   local fanin = math.random(1,4)
   local mx = math.random()*4 + 0.1
   local my = math.random()*4 + 0.1
   local osizex = math.random(4,6)
   local osizey = math.random(4,6)
   local sizex = osizex/mx
   local sizey = osizey/my
   local module = nn.SpatialReSampling{rwidth=mx,rheight=my}
   local input = torch.rand(fanin,sizey,sizex)

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.HardShrink()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ink, inj, ini):zero()

   local module = nn.HardShrink()

   local err = nn.Jacobian.testJacobian(module, input, 0.1, 2)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input, 0.1, 2)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.Threshold()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ink, inj, ini):zero()

   local module = nn.Threshold(torch.uniform(-2,2),torch.uniform(-2,2))

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.Abs()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ink, inj, ini):zero()

   local module = nn.Abs()

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.HardShrink()
   local ini = math.random(5,10)
   local inj = math.random(5,10)
   local ink = math.random(5,10)
   local input = torch.Tensor(ink, inj, ini):zero()

   local module = nn.HardShrink()

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.SpatialConvolution()
   local from = math.random(1,10)
   local to = math.random(1,10)
   local ki = math.random(1,10)
   local kj = math.random(1,10)
   local si = math.random(1,1)
   local sj = math.random(1,1)
   local ini = math.random(10,20)
   local inj = math.random(10,20)
   local module = nn.SpatialConvolution(from, to, ki, kj, si, sj)
   local input = torch.Tensor(from, inj, ini):zero()

   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   local err = nn.Jacobian.testJacobianParameters(module, input, module.weight, module.gradWeight)
   mytester:assertlt(err, precision, 'error on weight ')

   local err = nn.Jacobian.testJacobianParameters(module, input, module.bias, module.gradBias)
   mytester:assertlt(err, precision, 'error on bias ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.SpatialNormalization_Gaussian2D()
   local inputSize = math.random(11,20)
   local kersize = 9
   local nbfeatures = math.random(5,10)
   local kernel = image.gaussian(kersize)
   local module = nn.SpatialNormalization(nbfeatures,kernel,0.1)
   local input = torch.rand(nbfeatures,inputSize,inputSize)
   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')
end

function nnxtest.SpatialNormalization_Gaussian1D()
   local inputSize = math.random(14,20)
   local kersize = 15
   local nbfeatures = math.random(5,10)
   local kernelv = image.gaussian1D(11):resize(11,1)
   local kernelh = kernelv:t()
   local module = nn.SpatialNormalization(nbfeatures, {kernelv,kernelh}, 0.1)
   local input = torch.rand(nbfeatures,inputSize,inputSize)
   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')
end

function nnxtest.SpatialNormalization_io()
   local inputSize = math.random(11,20)
   local kersize = 7
   local nbfeatures = math.random(2,5)
   local kernel = image.gaussian(kersize)
   local module = nn.SpatialNormalization(nbfeatures,kernel)
   local input = torch.rand(nbfeatures,inputSize,inputSize)
   local ferr, berr = nn.Jacobian.testIO(module,input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

local function template_SpatialFovea(fx,fy,bilinear)
   local channels = math.random(1,4)
   local iwidth = 16
   local iheight = 16

   local module = nn.SpatialFovea{nInputPlane = channels,
                                  ratios = {1,2},
                                  preProcessors = {nn.Identity(),
                                                   nn.Identity()},
                                  processors = {nn.SpatialConvolution(channels,4,3,3),
                                                nn.SpatialConvolution(channels,4,3,3)},
                                  bilinear = bilinear,
                                  fov = 3,
                                  sub = 1}

   input = torch.rand(channels, iheight, iwidth)

   module:focus(fx,fy,3)
   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   module:focus()
   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end
function nnxtest.SpatialFovea_focused() template_SpatialFovea(4,7) end
function nnxtest.SpatialFovea_unfocused() template_SpatialFovea() end
function nnxtest.SpatialFovea_bilinear() template_SpatialFovea(nil,nil,true) end

local function template_SpatialPyramid(fx,fy)
   local channels = math.random(1,4)
   local iwidth = 16
   local iheight = 16

   local pyr = nn.SpatialPyramid({1,2},{nn.SpatialConvolution(channels,4,3,3),
				       nn.SpatialConvolution(channels,4,3,3)},
				 3, 3, 1, 1)
   local module = nn.Sequential()
   module:add(pyr)
   module:add(nn.JoinTable(1))

   input = torch.rand(channels, iheight, iwidth)

   pyr:focus(fx,fy,3,3)
   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')

   pyr:focus()
   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end

function nnxtest.SpatialPyramid_focused() template_SpatialPyramid(5,3) end
function nnxtest.SpatialPyramid_unfocused() template_SpatialPyramid() end

local function template_SpatialGraph(channels, iwidth, iheight, dist, norm)
   local module = nn.SpatialGraph{normalize=norm, dist=dist}
   local input = torch.rand(iwidth, iheight, channels)
   local err = nn.Jacobian.testJacobian(module, input, 0.1, 1)
   mytester:assertlt(err, precision, 'error on state ')

   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end
function nnxtest.SpatialGraph_1() template_SpatialGraph(3, 16, 16, 'euclid', true) end
function nnxtest.SpatialGraph_2() template_SpatialGraph(16, 4, 4, 'euclid', true) end
function nnxtest.SpatialGraph_3() template_SpatialGraph(256, 2, 2, 'euclid', false) end
function nnxtest.SpatialGraph_4() template_SpatialGraph(2, 16, 16, 'cosine', false) end
function nnxtest.SpatialGraph_5() template_SpatialGraph(64, 3, 3, 'cosine', false) end

local function template_SpatialMatching(channels, iwidth, iheight, maxw, maxh, full_output)
   local module = nn.Sequential()
   module:add(nn.SplitTable(1))
   local parallel = nn.ParallelTable()
   local seq1 = nn.Sequential()
   seq1:add(nn.Narrow(2, math.floor(maxh/2), iheight-maxh+1))
   seq1:add(nn.Narrow(3, math.floor(maxw/2), iwidth -maxw+1))
   parallel:add(seq1)
   parallel:add(nn.Identity())
   module:add(parallel)
   module:add(nn.SpatialMatching(maxh, maxw, full_output))
   local input = torch.rand(2, channels, iheight, iwidth)
   local err = nn.Jacobian.testJacobian(module, input)
   mytester:assertlt(err, precision, 'error on state ')
   
   local ferr, berr = nn.Jacobian.testIO(module, input)
   mytester:asserteq(ferr, 0, torch.typename(module) .. ' - i/o forward err ')
   mytester:asserteq(berr, 0, torch.typename(module) .. ' - i/o backward err ')
end
function nnxtest.SpatialMatching_1() template_SpatialMatching(4, 16, 16, 5, 5, true) end
function nnxtest.SpatialMatching_2() template_SpatialMatching(4, 16, 16, 5, 5, false) end
function nnxtest.SpatialMatching_3() template_SpatialMatching(3, 16, 16, 6, 6, true) end
function nnxtest.SpatialMatching_4() template_SpatialMatching(3, 20, 20, 4, 4, false) end
function nnxtest.SpatialMatching_5() template_SpatialMatching(3, 12, 16, 5, 7, true) end
--function nnxtest.SpatialMatching_6() template_SpatialMatching(4, 16, 32, 9, 5, false) end

function nnx.test()
   xlua.require('image',true)
   mytester = torch.Tester()
   mytester:add(nnxtest)
   math.randomseed(os.time())
   mytester:run()
end
