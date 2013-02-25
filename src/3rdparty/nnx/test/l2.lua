require 'torch'
-- rosenbrock.m This function returns the function value, partial derivatives
-- and Hessian of the (general dimension) rosenbrock function, given by:
--
--       f(x) = sum_{i=1:D-1} 100*(x(i+1) - x(i)^2)^2 + (1-x(i))^2 
--
-- where D is the dimension of x. The true minimum is 0 at x = (1 1 ... 1).
--
-- Carl Edward Rasmussen, 2001-07-21.

function l2(x,dx)

   local xx = x:clone()
   xx:cmul(xx)
   local fout = xx:sum()

   dx:copy(x)
   dx:mul(2)
   return fout,dx

end