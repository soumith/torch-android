local SpatialGraph, parent = torch.class('nn.SpatialGraph', 'nn.Module')

local help_desc =
[[Creates an edge-weighted graph from a set of N feature
maps. 

The input is a 3D tensor width x height x nInputPlane, the
output is a 3D tensor width x height x 2. The first slice
of the output contains horizontal edges, the second vertical
edges.

The input features are assumed to be >= 0.
More precisely:
+ dist == 'euclid' and norm == true: the input features should 
  also be <= 1, to produce properly normalized distances (btwn 0 and 1);
+ dist == 'cosine': the input features do not need to be bounded, 
  as the cosine dissimilarity normalizes with respect to each vector.
  An epsilon is automatically added, so that components that are == 0
  are properly considered as being similar.
]]

function SpatialGraph:__init(...)
   parent.__init(self)

   xlua.unpack_class(
      self, {...}, 
      'nn.SpatialGraph',  help_desc,
      {arg='dist', type='string', help='distance metric to use', default='euclid'},
      {arg='normalize', type='boolean', help='normalize euclidean distances btwn 0 and 1 (assumes input range to be btwn 0 and 1)', default=true},
      {arg='connex', type='number', help='connexity', default=4}
   )
   
   if self.connex ~= 4 then
      xlua.error('4 is the only connexity supported, for now', 'nn.SpatialGraph',self.usage)
   end
   self.dist = ((self.dist == 'euclid') and 0) or ((self.dist == 'cosine') and 1)
      or xerror('euclid is the only distance supported, for now','nn.SpatialGraph',self.usage)
   self.normalize = (self.normalize and 1) or 0
   if self.dist == 'cosine' and self.normalize == 1 then
      xerror('normalized cosine is not supported for now [just because I couldnt figure out the gradient :-)]',
             'nn.SpatialGraph', self.usage)
   end
end

function SpatialGraph:updateOutput(input)
   self.output:resize(self.connex / 2, input:size(2), input:size(3))
   input.nn.SpatialGraph_updateOutput(self, input)
   return self.output
end

function SpatialGraph:updateGradInput(input, gradOutput)
   self.gradInput:resizeAs(input)
   input.nn.SpatialGraph_updateGradInput(self, input, gradOutput)
   return self.gradInput
end
