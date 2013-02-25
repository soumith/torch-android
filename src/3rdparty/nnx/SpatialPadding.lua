local SpatialPadding, parent = torch.class('nn.SpatialPadding', 'nn.Module')

function SpatialPadding:__init(pad_l, pad_r, pad_t, pad_b, y_dim, x_dim)
   parent.__init(self)

   -- usage
   if not pad_l then
      error(xlua.usage('nn.SpatialPadding',
                          'a 2D padder module for images, zero-padding', nil,
                          {type='number', help='left padding', req=true},
                          {type='number', help='right padding'},
                          {type='number', help='top padding'},
                          {type='number', help='bottom padding'},
			  {type='number', help='y dimension', default=2},
			  {type='number', help='x dimension', default=3}))
   end

   self.pad_l = pad_l
   self.pad_r = pad_r or self.pad_l
   self.pad_t = pad_t or self.pad_l
   self.pad_b = pad_b or self.pad_l
   self.x_dim = x_dim or 3
   self.y_dim = y_dim or 2
end

function SpatialPadding:updateOutput(input)
   if self.output:type() ~= input:type() then
      self.output = input.new()
   end
   self.x_dim = self.x_dim or 3
   self.y_dim = self.y_dim or 2
   local h = input:size(self.y_dim) + self.pad_t + self.pad_b
   local w = input:size(self.x_dim) + self.pad_l + self.pad_r
   if w < 1 or h < 1 then error('input is too small') end
   local dims = input:size()
   dims[self.y_dim] = h
   dims[self.x_dim] = w
   self.output:resize(dims)
   self.output:zero()
   -- crop input if necessary
   local c_input = input
   if self.pad_t < 0 then c_input = c_input:narrow(self.y_dim, 1 - self.pad_t, c_input:size(self.y_dim) + self.pad_t) end
   if self.pad_b < 0 then c_input = c_input:narrow(self.y_dim, 1, c_input:size(self.y_dim) + self.pad_b) end
   if self.pad_l < 0 then c_input = c_input:narrow(self.x_dim, 1 - self.pad_l, c_input:size(self.x_dim) + self.pad_l) end
   if self.pad_r < 0 then c_input = c_input:narrow(self.x_dim, 1, c_input:size(self.x_dim) + self.pad_r) end
   -- crop outout if necessary
   local c_output = self.output
   if self.pad_t > 0 then c_output = c_output:narrow(self.y_dim, 1 + self.pad_t, c_output:size(self.y_dim) - self.pad_t) end
   if self.pad_b > 0 then c_output = c_output:narrow(self.y_dim, 1, c_output:size(self.y_dim) - self.pad_b) end
   if self.pad_l > 0 then c_output = c_output:narrow(self.x_dim, 1 + self.pad_l, c_output:size(self.x_dim) - self.pad_l) end
   if self.pad_r > 0 then c_output = c_output:narrow(self.x_dim, 1, c_output:size(self.x_dim) - self.pad_r) end
   -- copy input to output
   c_output:copy(c_input)
   return self.output
end

function SpatialPadding:updateGradInput(input, gradOutput)
   --if input:dim() ~= 3 then error('input must be 3-dimensional') end
   self.gradInput:resizeAs(input):zero()
   -- crop gradInput if necessary
   local cg_input = self.gradInput
   if self.pad_t < 0 then cg_input = cg_input:narrow(self.y_dim, 1 - self.pad_t, cg_input:size(self.y_dim) + self.pad_t) end
   if self.pad_b < 0 then cg_input = cg_input:narrow(self.y_dim, 1, cg_input:size(self.y_dim) + self.pad_b) end
   if self.pad_l < 0 then cg_input = cg_input:narrow(self.x_dim, 1 - self.pad_l, cg_input:size(self.x_dim) + self.pad_l) end
   if self.pad_r < 0 then cg_input = cg_input:narrow(self.x_dim, 1, cg_input:size(self.x_dim) + self.pad_r) end
   -- crop gradOutout if necessary
   local cg_output = gradOutput
   if self.pad_t > 0 then cg_output = cg_output:narrow(self.y_dim, 1 + self.pad_t, cg_output:size(self.y_dim) - self.pad_t) end
   if self.pad_b > 0 then cg_output = cg_output:narrow(self.y_dim, 1, cg_output:size(self.y_dim) - self.pad_b) end
   if self.pad_l > 0 then cg_output = cg_output:narrow(self.x_dim, 1 + self.pad_l, cg_output:size(self.x_dim) - self.pad_l) end
   if self.pad_r > 0 then cg_output = cg_output:narrow(self.x_dim, 1, cg_output:size(self.x_dim) - self.pad_r) end
   -- copy gradOuput to gradInput
   cg_input:copy(cg_output)
   return self.gradInput
end