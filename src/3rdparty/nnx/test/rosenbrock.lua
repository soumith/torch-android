require 'torch'
-- rosenbrock.m This function returns the function value, partial derivatives
-- and Hessian of the (general dimension) rosenbrock function, given by:
--
--       f(x) = sum_{i=1:D-1} 100*(x(i+1) - x(i)^2)^2 + (1-x(i))^2 
--
-- where D is the dimension of x. The true minimum is 0 at x = (1 1 ... 1).
--
-- Carl Edward Rasmussen, 2001-07-21.

function rosenbrock(x,dx)
   
   -- (1) compute f(x)
   local d = x:size(1)
   -- x1 =  x(i)^2
   local x1 = torch.Tensor(d-1):copy(x:narrow(1,1,d-1))
   -- x(i+1) - x(i)^2
   x1:cmul(x1):mul(-1):add(x:narrow(1,2,d-1))

   -- 100*(x(i+1) - x(i)^2)^2
   x1:cmul(x1):mul(100)

   -- x(i)
   local x0 = torch.Tensor(d-1):copy(x:narrow(1,1,d-1))
   -- 1-x(i)
   x0:mul(-1):add(1)
   -- (1-x(i))^2
   x0:cmul(x0)
   -- 100*(x(i+1) - x(i)^2)^2 + (1-x(i))^2
   x1:add(x0)
   local fout = x1:sum()

   -- (2) compute f(x)/dx
   local dxout = torch.Tensor():resizeAs(x):zero()
   -- df(1:D-1) = - 400*x(1:D-1).*(x(2:D)-x(1:D-1).^2) - 2*(1-x(1:D-1));
   
   x1:copy(x:narrow(1,1,d-1))
   x1:cmul(x1):mul(-1):add(x:narrow(1,2,d-1)):cmul(x:narrow(1,1,d-1)):mul(-400)
   x0:copy(x:narrow(1,1,d-1)):mul(-1):add(1):mul(-2)
   x1:add(x0)
   dxout:narrow(1,1,d-1):copy(x1)
   
  -- df(2:D) = df(2:D) + 200*(x(2:D)-x(1:D-1).^2);
   x0:copy(x:narrow(1,1,d-1))
   x0:cmul(x0):mul(-1):add(x:narrow(1,2,d-1)):mul(200)
   dxout:narrow(1,2,d-1):add(x0)

   dx:copy(dxout)
  return fout,dx

end