local Probe, parent = torch.class('nn.Probe', 'nn.Module')

function Probe:__init(name,display)
   parent.__init(self)
   self.name = name or 'unnamed'
   self.display = display
   nn._ProbeTimer = nn._ProbeTimer or torch.Timer()
end

function Probe:updateOutput(input)
   self.output = input
   local legend = '<' .. self.name .. '>.output'
   local size = {}
   for i = 1,input:dim() do
      size[i] = input:size(i)
   end
   size = table.concat(size,'x')
   local diff = nn._ProbeTimer:time().real - (nn._ProbeLast or 0)
   nn._ProbeLast = nn._ProbeTimer:time().real
   print('')
   print(legend)
   print('  + size = ' .. size)
   print('  + mean = ' .. input:mean())
   print('  + std = ' .. input:std())
   print('  + min = ' .. input:min())
   print('  + max = ' .. input:max())
   print('  + time since last probe = ' .. string.format('%0.1f',diff*1000) .. 'ms')
   if self.display then
      self.winf = image.display{image=input, win=self.winf, legend=legend}
   end
   return self.output
end

function Probe:updateGradInput(input, gradOutput)
   self.gradInput = gradOutput
   local legend = 'layer<' .. self.name .. '>.gradInput'
   local size = {}
   for i = 1,gradOutput:dim() do
      size[i] = gradOutput:size(i)
   end
   size = table.concat(size,'x')
   local diff = nn._ProbeTimer:time().real - (nn._ProbeLast or 0)
   nn._ProbeLast = nn._ProbeTimer:time().real
   print('')
   print(legend)
   print('  + size = ' .. size)
   print('  + mean = ' .. gradOutput:mean())
   print('  + std = ' .. gradOutput:std())
   print('  + min = ' .. gradOutput:min())
   print('  + max = ' .. gradOutput:max())
   print('  + time since last probe = ' .. string.format('%0.1f',diff*1000) .. 'ms')
   if self.display then
      self.winb = image.display{image=gradOutput, win=self.winb, legend=legend}
   end
   return self.gradInput
end
