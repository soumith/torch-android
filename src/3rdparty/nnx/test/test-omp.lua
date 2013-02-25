
function nnx.test_omp(nThread)

   require 'lunit'
   require 'sys'

   nnx._test_all_ = nil
   module("nnx._test_omp_", lunit.testcase, package.seeall)

   math.randomseed(os.time())

   if openmp then
      nThread = nThread or openmp.getNumThreads()
   else
      nThread = nThread or error('please specify number of threads')
   end

   -- test dimensions
   width = 100
   height = 100
   maps = 64
   maps2 = 16

   -- accumulate times
   times = {}

   -- precision
   precision = 1e-10

   -- generic test function
   local function forward(name)
      n.nThread = 1
      res = n:forward(vec)
      res1 = torch.Tensor():resizeAs(res)
      res2 = torch.Tensor():resizeAs(res)

      t=sys.clock()
      res1:copy( n:forward(vec) )
      ts.c = sys.clock()-t

      res:zero()

      t=sys.clock()
      n.nThread = nThread
      res2:copy( n:forward(vec) )
      ts.omp = sys.clock()-t

      err = (res1-res2):abs():max()
      assert_equal((err < precision), true, name .. ": error = " .. err)
   end

   -- generic test function
   local function backward(name)
      n.nThread = 1
      n:forward(vec)
      res = n:backward(vec,vecb)
      res1 = torch.Tensor():resizeAs(res)
      res2 = torch.Tensor():resizeAs(res)

      t=sys.clock()
      res1:copy( n:backward(vec,vecb) )
      tsb.c = sys.clock()-t

      res:zero()

      t=sys.clock()
      n.nThread = nThread
      res2:copy( n:backward(vec,vecb) )
      tsb.omp = sys.clock()-t

      err = (res1-res2):abs():max()
      assert_equal((err < precision), true, name .. ": error = " .. err)
   end

   -- tests
   function test_SpatialMaxPooling()
      ts = {}
      times['SpatialMaxPooling_forward'] = ts
      n = nn.SpatialMaxPooling(4,4)
      vec = lab.randn(maps,height,width)
      forward('SpatialMaxPooling_forward')

--       ts = {}
--       times['SpatialMaxPooling_backward'] = ts
--       local tbl = nn.tables.random(maps,maps2,math.min(maps,8))
--       vec = lab.randn(maps,height,width)
--       vecb = lab.randn(maps,height/4,width/4)
--       backward('SpatialMaxPooling_backward')
   end

   -- run all tests
   lunit.main()

   -- report
   print '\nTiming report:'
   ntests = 0
   glob_speedup = 0
   for module,times in pairs(times) do
      local speedup = (times.c/times.omp)
      print(module .. ' in C: ' .. times.c .. ', with OMP: ' .. times.omp
            .. ', speedup: ' .. speedup .. 'x')
      glob_speedup = glob_speedup + speedup
      ntests = ntests + 1
   end
   print('Average speedup: ' .. (glob_speedup/ntests) .. 'x')

end
