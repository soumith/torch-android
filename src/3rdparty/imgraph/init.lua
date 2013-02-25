----------------------------------------------------------------------
--
-- Copyright (c) 2011 Clement Farabet
--               2006 Pedro Felzenszwalb
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
--
----------------------------------------------------------------------
-- description:
--     imgraph - a graph package for images:
--               this package contains several routines to build graphs
--               on images, and then compute their connected components,
--               watershed, min-spanning trees, and so on.
--
--               The min-spanning three based segmentation (segmentmst)
--               originates from Felzenszwalb's 2004 paper:
--               "Efficient Graph-Based Image Segmentation".
--
-- history:
--     July 14, 2011, 5:52PM  - colorizing function  - Clement Farabet
--     July 14, 2011, 12:49PM - MST + HistPooling    - Clement Farabet
--     July 13, 2011, 6:16PM  - first draft          - Clement Farabet
----------------------------------------------------------------------

-- create global nnx table:
imgraph = {}

----------------------------------------------------------------------
-- computes a graph from an 2D or 3D image
--
function imgraph.graph(...)
   -- get args
   local args = {...}
   local dest, img, connex, distance
   local arg2 = torch.typename(args[2])
   if arg2 and arg2:find('Tensor') then
      dest = args[1]
      img = args[2]
      connex = args[3]
      distance = args[4]
   else
      img = args[1]
      connex = args[2]
      distance = args[3]
   end

   -- defaults
   connex = connex or 4
   distance = ((not distance) and 'e') or ((distance == 'euclid') and 'e') 
              or ((distance == 'angle') and 'a') or ((distance == 'max') and 'm') 

   -- usage
   if not img or (connex ~= 4 and connex ~= 8) or (distance ~= 'e' and distance ~= 'a' and distance ~= 'm') then
      print(xlua.usage('imgraph.graph',
                       'compute an edge-weighted graph on an image', nil,
                       {type='torch.Tensor', help='input tensor (for now KxHxW or HxW)', req=true},
                       {type='number', help='connexity (edges per vertex): 4 | 8', default=4},
                       {type='string', help='distance metric: euclid | angle | max', req='euclid'},
                       "",
                       {type='torch.Tensor', help='destination: existing graph', req=true},
                       {type='torch.Tensor', help='input tensor (for now KxHxW or HxW)', req=true},
                       {type='number', help='connexity (edges per vertex): 4 | 8', default=4},
                       {type='string', help='distance metric: euclid | angle | max', req='euclid'}))
      xlua.error('incorrect arguments', 'imgraph.graph')
   end

   -- create dest
   dest = dest or torch.Tensor():typeAs(img)

   -- compute graph
   img.imgraph.graph(dest, img, connex, distance)

   -- return result
   return dest
end

----------------------------------------------------------------------
-- computes a graph from a .mat hierarchy image
--
function imgraph.mat2graph(...)
   -- get args
   local args = {...}
   local dest, img
   local arg = torch.typename(args[1])
   if arg and arg:find('Tensor') then
      img = args[1]
   end
   uheight = args[2]
   uwidth = args[3]
   -- usage
   if not img  then
      print(xlua.usage('imgraph.mat2graph',
                       'compute an edge-weighted graph from a .mat hierarchy image', nil,
                       {type='torch.Tensor', help='input tensor (for now KxHxW or HxW)', req=true},
                       "",
                      {type='torch.Tensor', help='destination: existing graph', req=true},
                       {type='torch.Tensor', help='input tensor (for now KxHxW or HxW)', req=true}))
      xlua.error('incorrect arguments', 'imgraph.mat2graph')
   end

   dest = torch.Tensor( ( (img:size(1)-1)/2 * (img:size(2)-1)/2 ),2 )
  
   -- compute graph
   img.imgraph.mat2graph(img, dest, uheight, uwidth)

   -- return result
   return dest
end

----------------------------------------------------------------------
-- compute the connected components of a graph
--
function imgraph.connectcomponents(...)
   --get args
   local args = {...}
   local dest, graph, threshold, colorize
   local arg2 = torch.typename(args[2])
   if arg2 and arg2:find('Tensor') then
      dest = args[1]
      graph = args[2]
      threshold = args[3]
      colorize = args[4]
   else
      graph = args[1]
      threshold = args[2]
      colorize = args[3]
   end

   -- defaults
   threshold = threshold or 0.5

   -- usage
   if not graph then
      print(xlua.usage('imgraph.connectcomponents',
                       'compute the connected components of an edge-weighted graph', nil,
                       {type='torch.Tensor', help='input graph', req=true},
                       {type='number', help='threshold for connecting components', default=0.5},
                       {type='boolean', help='replace components id by random colors', default=false},
                       "",
                       {type='torch.Tensor', help='destination tensor', req=true},
                       {type='torch.Tensor', help='input graph', req=true},
                       {type='number', help='threshold for connecting components', default=0.5},
                       {type='boolean', help='replace components id by random colors', default=false}))
      xlua.error('incorrect arguments', 'imgraph.connectcomponents')
   end

   -- create dest
   dest = dest or torch.Tensor():typeAs(graph)

   -- compute image
   local nelts = graph.imgraph.connectedcomponents(dest, graph, threshold, colorize)

   -- return image
   return dest, nelts
end

----------------------------------------------------------------------
-- compute the watershed of a graph
--
function imgraph.watershed(...)
   --get args
   local args = {...}
   local dest, gradient, minHeight, connex, colorize
   local arg2 = torch.typename(args[2])
   if arg2 and arg2:find('Tensor') then
      dest = args[1]
      gradient = args[2]
      minHeight = args[3]
      connex = args[4]
   else
      gradient = args[1]
      minHeight = args[2]
      connex = args[3]
   end

   -- defaults
   minHeight = minHeight or 0.05
   connex = connex or 4

   -- usage
   if not gradient or (gradient:nDimension() ~= 2) then
      print(xlua.usage('imgraph.watershed',
                       'compute the watershed of a gradient map (or arbitrary grayscale image)', nil,
                       {type='torch.Tensor', help='input gradient map (HxW tensor)', req=true},
                       {type='number', help='filter minimas by imposing a minimum height', default=0.05},
                       {type='number', help='connexity: 4 | 8', default=4},
                       "",
                       {type='torch.Tensor', help='destination tensor', req=true},
                       {type='torch.Tensor', help='input gradient map (HxW tensor)', req=true},
                       {type='number', help='filter minimas by imposing a minimum height', default=0.05},
                       {type='number', help='connexity: 4 | 8', default=4}))
      xlua.error('incorrect arguments', 'imgraph.watershed')
   end

   -- create dest
   dest = dest or torch.Tensor():typeAs(gradient)

   -- compute image
   local nelts = gradient.imgraph.watershed(dest, gradient:clone(), minHeight, connex)

   -- return image
   return dest, nelts
end

----------------------------------------------------------------------
-- render a graph into an image
--
function imgraph.graph2map(...)
   --get args
   local args = {...}
   local dest, graph, method
   local arg2 = torch.typename(args[2])
   if arg2 and arg2:find('Tensor') then
      dest = args[1]
      graph = args[2]
      method = args[3]
   else
      graph = args[1]
      method = args[2]
   end

   -- defaults
   method = method or 'maxgrad'

   -- usage
   if not graph or (graph:nDimension() ~= 3) then
      print(xlua.usage('imgraph.graph2map',
                       'render a graph into a 2D image:\n'
                          .. ' + khalimsky: creates a map that is twice larger, for fine visualization\n'
                          .. ' + maxgrad: creates a map that is the same size, by maxing the edge weights',
                       nil,
                       {type='torch.Tensor', help='input graph', req=true},
                       {type='method', help='rendering method: maxgrad | khalimsky', default='maxgrad'},
                       "",
                       {type='torch.Tensor', help='destination tensor', req=true},
                       {type='torch.Tensor', help='input graph', req=true},
                       {type='method', help='rendering method: maxgrad | khalimsky', default='maxgrad'}))
      xlua.error('incorrect arguments', 'imgraph.graph2map')
   end

   -- create dest
   dest = dest or torch.Tensor():typeAs(graph)

   -- render graph
   if method == 'khalimsky' then
      -- warning
      if graph:size(1) ~= 2 then
         print('<imgraph.mergetree> warning: only supporting 4-connexity (discarding other edges)')
      end
      local graphflat = graph:new():resize(2*graph:size(2),graph:size(3))
      graph.imgraph.graph2map(dest, graphflat, mode)
   else
      graph.imgraph.gradient(dest, graph)
   end

   -- return image
   return dest
end

----------------------------------------------------------------------
-- compute the merge tree of a graph
--
function imgraph.mergetree(...)
   --get args
   local args = {...}
   local graph = args[1]

   -- usage
   if not graph or (graph:nDimension() ~= 3) then
      print(xlua.usage('imgraph.mergetree',
                       'compute the merge tree of a graph (dendrogram)', nil,
                       {type='torch.Tensor', help='input graph', req=true}))
      xlua.error('incorrect arguments', 'imgraph.mergetree')
   end

   -- warning
   if graph:size(1) ~= 2 then
      print('<imgraph.mergetree> warning: only supporting 4-connexity (discarding other edges)')
   end

   -- compute merge tree of graph
   local graphflat = graph:new():resize(2*graph:size(2),graph:size(3))
   local mt = graph.imgraph.mergetree(graphflat)

   -- return tree
   return mt
end

----------------------------------------------------------------------
-- compute the hierarchy of [guimaraes et al. ICIP2012] of a graph
--
function imgraph.hierarchyGuimaraes(...)
   --get args
   local args = {...}
   local graph = args[1]

   -- usage
   if not graph or (graph:nDimension() ~= 3) then
      print(xlua.usage('imgraph.hierarchyGuimaraes',
                       'compute the hierarchyGuimaraes of a graph (dendrogram)', nil,
                       {type='torch.Tensor', help='input graph', req=true}))
      xlua.error('incorrect arguments', 'imgraph.hierarchyGuimaraes')
   end

   -- warning
   if graph:size(1) ~= 2 then
      print('<imgraph.hierarchyGuimaraes> warning: only supporting 4-connexity (discarding other edges)')
   end

   -- compute the hierarchy from the graph
   local graphflat = graph:new():resize(2*graph:size(2),graph:size(3))
   local mt = graph.imgraph.hierarchyGuimaraes(graphflat)
   
   -- return tree
   return mt
end

----------------------------------------------------------------------
-- compute a hierarchy from a flat graph representing hierarchy of Arbelatez et at PAMI 2011 
--
function imgraph.hierarchyArb(...)
   --get args
   local args = {...}
   local graph = args[1]

   -- usage
   if not graph or (graph:nDimension() ~= 3) then
      print(xlua.usage('imgraph.hierarchyArb',
                       'compute the hierarchyArb of a graph (dendrogram)', nil,
                       {type='torch.Tensor', help='input graph', req=true}))
      xlua.error('incorrect arguments', 'imgraph.hierarchyArb')
   end

   -- warning
   if graph:size(1) ~= 2 then
      print('<imgraph.hierarchyArb> warning: only supporting 4-connexity (discarding other edges)')
   end

   -- compute the hierarchy from the graph
   local graphflat = graph:new():resize(2*graph:size(2),graph:size(3))
   local mt = graph.imgraph.hierarchyArb(graphflat)
   
   -- return tree
   return mt
end

----------------------------------------------------------------------
-- filter a merge tree so as to equalize surface/volume or other
-- attributes
--
function imgraph.filtertree(...)
   --get args
   local args = {...}
   local tree = args[1]
   local mode = args[2]

   -- usage
   if not tree then
      print(xlua.usage('imgraph.filtertree',
                       'filter a merge tree according to a criterion (in place)', nil,
                       {type='imgraph.MergeTree', help='merge tree to be filtered', req=true},
                       {type='string', help='filter criterion: surface | volume | dynamic', default='surface'}))
      xlua.error('incorrect arguments', 'imgraph.filtertree')
   end

   -- defaults
   if mode == 'surface' then mode = 0
   elseif mode == 'dynamic' then mode = 1
   elseif mode == 'volume' then mode = 2
   elseif mode == 'alphaomega' then mode = 3
   else mode = 0 end

   -- filter merge tree
   torch.Tensor().imgraph.filtertree(tree, mode)
end

----------------------------------------------------------------------
-- dump a tree to disk
--
function imgraph.dumptree(...)
   --get args
   local args = {...}
   local tree = args[1]
   local filename = args[2]

   -- usage
   if not tree or not filename then
      print(xlua.usage('imgraph.dumptree',
                       'dump a tree to disk', nil,
                       {type='imgraph.dumptree', help='merge tree to be filtered', req=true},
                       {type='string', help='filename', req=true}))
      xlua.error('incorrect arguments', 'imgraph.dumptree')
   end

   -- filter merge tree
   torch.Tensor().imgraph.dumptree(tree, filename)
end

----------------------------------------------------------------------
-- assign weights to the nodes of a tree
--
function imgraph.weighttree(...)
   --get args
   local args = {...}
   local tree = args[1]
   local weights = args[2]

   -- usage
   if not tree or not weights then
      print(xlua.usage('imgraph.weighttree',
                       'filter a merge tree according to a criterion (in place)', nil,
                       {type='imgraph.MergeTree', help='merge tree to be weighted', req=true},
                       {type='table', help='a list of weights t[k] = w, with k the index of the node to be weighted', req=true}))
      xlua.error('incorrect arguments', 'imgraph.weighttree')
   end

   -- filter merge tree
   torch.Tensor().imgraph.weighttree(tree, weights)
end

----------------------------------------------------------------------
-- computes a cut in a tree 
--
function imgraph.cuttree(...)
   --get args
   local args = {...}
   local tree = args[1]
   local mode = args[2]

   -- usage
   if not tree then
      print(xlua.usage('imgraph.cuttree',
                       'computes a cut in a tree', nil,
                        {type='imgraph.cuttree', help='computes a cut in a hierchical tree', req=true}, 
              {type='string', help='cutting algorithm : Kruskal | Prim | PWatershed | Graphcuts | MinCover', default='Kruskal'}))
      xlua.error('incorrect arguments', 'imgraph.cuttree')
   end

   -- defaults
   if mode == 'Kruskal' then mode = 0
   elseif mode == 'Prim' then mode = 1
   elseif mode == 'PWatershed' then mode = 2 
   elseif mode == 'Graphcuts' then mode = 3
   elseif mode == 'MinCover' then mode = 4
   else mode = 0 end

   -- compute cut
   local cut = torch.Tensor().imgraph.cuttree(tree, mode)

   -- return cut
   return cut
end

----------------------------------------------------------------------
-- computes a vector of overlap scores from 2 images 
--
function imgraph.overlap(...)

   --get args
   local args = {...}
   local image1 = args[1]
   local image2 = args[2]
   local nb_classes = args[3]


   -- usage
   if not image2 then
      print(xlua.usage('imgraph.overlap',
                       'computes a vector of overlap scores from 2 images', nil,
                        {type='imgraph.overlap', help='computes a vector of overlap scores from 2 images', req=true}
                           ))
      xlua.error('incorrect arguments', 'imgraph.overlap')
   end

   -- compute overlap

    local overlap_vector = torch.Tensor().imgraph.overlap(image1,image2, image1:size(1), image1:size(2), nb_classes)


   -- return overlap vector
   return overlap_vector
end

----------------------------------------------------------------------
-- computes 
--
function imgraph.decisionSegmentation(...)

   --get args
   local args = {...}
   local segments = args[1]
   local rs = args[2]
   local cs = args[3]
   local nb_segments = args[4]
   local f = args[5]
   local nb_classes = args[6]
   local t1 = args[7]
   local t2 = args[8]
   local t3 = args[9]

   -- usage
   if not segments then
      print(xlua.usage('imgraph.decisionSegmentation',
                       'Computes the final segmentation from an array of segments with associated overlap scores for different classes of objects', nil,
                        {type='imgraph.decisionSegmentation', help='Computes the final segmentation from an array of segments with associated overlap scores for different classes of objects', req=true}
                           ))
      xlua.error('incorrect arguments', 'imgraph.decisionSegmentation')
   end

   -- compute final segmentation

   local output_image = torch.Tensor().imgraph.decisionSegmentation(segments, rs,cs,nb_segments, f,  nb_classes, t1,t2,t3)

   return output_image
end




----------------------------------------------------------------------
-- transform a merge tree back into a graph, for visualization
--
function imgraph.tree2graph(...)
   --get args
   local args = {...}
   local tree = args[1]

   -- usage
   if not tree then
      print(xlua.usage('imgraph.tree2graph',
                       'transform a merge tree into a 2D graph', nil,
                       {type='imgraph.MergeTree', help='merge tree', req=true}))
      xlua.error('incorrect arguments', 'imgraph.tree2graph')
   end

   -- tree -> graph
   local graph = torch.Tensor()
   graph.imgraph.tree2graph(tree, graph)
   graph:resize(2, graph:size(1)/2, graph:size(2))

   -- return graph
   return graph
end

----------------------------------------------------------------------
-- returns the levels (altitude) of a merge tree 
--
function imgraph.levelsOfTree(...)
   --get args
   local args = {...}
   local tree = args[1]

   -- usage
   if not tree then
      print(xlua.usage('imgraph.levelsOfTree',
                       '', nil,
                       {type='imgraph.MergeTree', help='merge tree', req=true}))
      xlua.error('incorrect arguments', 'imgraph.levelsOfTree')
   end

   -- 
   local altitudes = torch.Tensor()
   altitudes= graph.imgraph.levelsOfTree(tree)
  

   -- return levels
   return altitudes
end

----------------------------------------------------------------------
-- segment a graph, by computing its min-spanning tree and
-- merging vertices based on a dynamic threshold
--
function imgraph.segmentmst(...)
   --get args
   local args = {...}
   local dest, graph, thres, minsize, colorize, adaptive
   local arg2 = torch.typename(args[2])
   if arg2 and arg2:find('Tensor') then
      dest = args[1]
      graph = args[2]
      thres = args[3]
      minsize = args[4]
      colorize = args[5]
      adaptive = args[6]
   else
      graph = args[1]
      thres = args[2]
      minsize = args[3]
      colorize = args[4]
      adaptive = args[5]
   end

   -- defaults
   thres = thres or 3
   minsize = minsize or 20
   colorize = colorize or false
   if adaptive == nil then adaptive = true end

   -- usage
   if not graph then
      print(xlua.usage('imgraph.segmentmst',
                       'segment an edge-weighted graph, by thresholding its mininum spanning tree\n'
                       ..'(an adaptive threshold is used by default, as in Felzenszwalb et al.)',
                       nil,
                       {type='torch.Tensor', help='input graph', req=true},
                       {type='number', help='base threshold for merging', default=3},
                       {type='number', help='min size: merge components of smaller size', default=20},
                       {type='boolean', help='replace components id by random colors', default=false},
                       {type='boolean', help='use adaptive threshold (Felzenszwalb trick)', default=true},
                       "",
                       {type='torch.Tensor', help='destination tensor', req=true},
                       {type='torch.Tensor', help='input graph', req=true},
                       {type='number', help='base threshold for merging', default=3},
                       {type='number', help='min size: merge components of smaller size', default=20},
                       {type='boolean', help='replace components id by random colors', default=false},
                       {type='boolean', help='use adaptive threshold (Felzenszwalb trick)', default=true}))
      xlua.error('incorrect arguments', 'imgraph.segmentmst')
   end

   -- compute image
   dest = dest or torch.Tensor():typeAs(graph)
   local nelts
   if graph:nDimension() == 3 then
      -- dense image graph (input is a KxHxW graph, K=1/2 connexity, nnodes=H*W)
      nelts = graph.imgraph.segmentmst(dest, graph, thres, minsize, adaptive, colorize)
   else
      -- sparse graph (input is a Nx3 graph, nnodes=N, each entry input[i] is an edge: {node1, node2, weight})
      nelts = graph.imgraph.segmentmstsparse(dest, graph, thres, minsize, adaptive, colorize)
   end

   -- return image
   return dest, nelts
end

----------------------------------------------------------------------
-- pool the features (or pixels) of an image into a segmentation map
--
function imgraph.histpooling(...)
   --get args
   local args = {...}
   local src, segmentation, lists, histmax, minconfidence
   src = args[1]
   segmentation = args[2]
   histmax = args[3]
   minconfidence = args[4]

   -- defaults
   histmax = histmax or false
   minconfidence = minconfidence or 0

   -- usage
   if not src or not segmentation or not torch.typename(src):find('Tensor') or not torch.typename(segmentation):find('Tensor') then
      print(xlua.usage('imgraph.histpooling',
                       'pool the features (or pixels) of an image into a segmentation map,\n'
                          .. 'using histogram accumulation. this is useful for colorazing a\n'
                          .. 'segmentation with the original pixel colors, or for cleaning up\n'
                          .. 'a dense prediction map.\n\n'
                          .. 'the pooling is done in place (the input is replaced)\n\n'
                          .. 'two extra lists of components are optionally generated:\n'
                          .. 'the first list is an array of these components, \n'
                          .. 'while the second is a hash table; each entry has this format:\n'
                          .. 'entry = {centroid_x, centroid_y, surface, hist_max, id}',
                       nil,
                       {type='torch.Tensor', help='input image/map/matrix to pool (must be KxHxW)', req=true},
                       {type='torch.Tensor', help='segmentation to guide pooling (must be HxW)', req=true},
                       {type='boolean', help='hist max: replace histograms by their max bin', default=false},
                       {type='number', help='min confidence: vectors with a low confidence are not accumulated', default=0}))
      xlua.error('incorrect arguments', 'imgraph.histpooling')
   end

   -- compute image
   local dst = src:clone()
   local iresults, hresults = src.imgraph.histpooling(dst, segmentation, 
                                                      true, histmax, minconfidence)

   -- return image
   return dst, iresults, hresults
end

----------------------------------------------------------------------
-- return the adjacency matrix of a segmentation map
--
function imgraph.adjacency(...)
   -- get args
   local args = {...}
   local input = args[1]
   local components = args[2]
   local directed = args[3] or false

   -- usage
   if not input then
      print(xlua.usage('imgraph.adjacency',
                       'return the adjacency matrix of a segmentation map.\n\n'
                          .. 'a component list can be given, in which case the list\n'
                          .. 'is updated to directly embed the neighboring relationships\n'
                          .. 'and a second adjacency matrix is returned, using the revids\n'
                          .. 'available in the component list',
                       'graph = imgraph.graph(image.lena())\n'
                          .. 'segm = imgraph.segmentmst(graph)\n'
                          .. 'matrix = imgraph.adjacency(segm)\n\n'
                          .. 'components = imgraph.extractcomponents(segm)\n'
                          .. 'segm = imgraph.adjacency(segm, components)\n'
                          .. 'print(components.neighbors) -- list of neighbor IDs\n'
                          .. 'print(components.adjacency) -- adjacency matrix of IDs',
                       {type='torch.Tensor', help='input segmentation map (must be HxW), and each element must be in [1,NCLASSES]', req=true},
                       {type='table', help='component list, as returned by imgraph.extractcomponents()'},
                       '',
                       {type='imgraph.MergeTree', help='merge tree (dendrogram) of a graph', req=true},
                       {type='table', help='component list, as returned by imgraph.extractcomponents()'},
                       {type='boolean', help='if true, returns a directed adjancy matrix, in which only son->parent edges are considered', default=false}))
      xlua.error('incorrect arguments', 'imgraph.adjacency')
   end

   -- support LongTensors
   if torch.typename(input) and torch.typename(input) == 'torch.LongTensor' then
      input = torch.Tensor(input:size(1), input:size(2)):copy(input)
   end

   -- fill matrix
   local adjacency
   if torch.typename(input) then
      adjacency = input.imgraph.adjacency(input, {})
   else
      adjacency = torch.Tensor().imgraph.adjacencyoftree(input, {}, directed)
   end

   -- update component list, if given
   if components then
      components.neighbors = {}
      components.adjacency = {}
      for i = 1,components:size() do
         local neighbors = adjacency[components.id[i]]
         local ntable = {}
         local ktable = {}
         if neighbors then
            for id in pairs(neighbors) do
               table.insert(ntable, components.revid[id])
               ktable[components.revid[id]] = true
            end
         end
         components.neighbors[i] = ntable
         components.adjacency[i] = ktable
      end
   end

   -- return adjacency matrix
   return adjacency
end

----------------------------------------------------------------------
-- extract information/geometry of a segmentation's components
--
function imgraph.extractcomponents(...)
   -- get args
   local args = {...}
   local input = args[1]
   local img = args[2]
   local config = args[3] or 'bbox'
   local minsize = args[4] or 1

   -- usage
   if not input then
      print(
         xlua.usage(
            'imgraph.extractcomponents',
            'return a list of structures describing the components of a segmentation. \n'
               .. 'if a KxHxW image is given, then patches can be extracted from it, \n'
               .. 'and appended to the list returned. \n'
               .. 'the optional config string specifies how these patches should be \n'
               .. 'returned (bbox: raw bounding boxes, mask: binary segmentation mask, \n'
               .. 'masked: bbox masked by segmentation mask)',
            'graph = imgraph.graph(image.lena())\n'
               .. 'segm = imgraph.segmentmst(graph)\n'
               .. 'components = imgraph.extractcomponents(segm)',
            {type='torch.Tensor', 
             help='input segmentation map (must be HxW), and each element must be in [1,NCLASSES]', req=true},
            {type='torch.Tensor', 
             help='auxiliary image: if given, then components are cropped from it (must be KxHxW)'},
            {type='string', 
             help='configuration, one of: bbox | masked', default='bbox'},
            {type='number', 
             help='minimum component size to process', default=1},
            "",
            {type='imgraph.MergeTree',
             help='merge tree (dendrogram) of a graph', req=true},
            {type='torch.Tensor', 
             help='auxiliary image: if given, then components are cropped from it (must be KxHxW)'},
            {type='string', 
             help='configuration, one of: bbox | masked', default='bbox'},
            {type='number', 
             help='minimum component size to process', default=1}
         )
      )
      xlua.error('incorrect arguments', 'imgraph.extractcomponents')
   end

   -- support LongTensors
   if torch.typename(input) == 'torch.LongTensor' then
      input = torch.Tensor(input:size(1), input:size(2)):copy(input)
   end

   -- generate lists
   local hcomponents
   local masks = {}
   if torch.typename(input) then
      hcomponents = input.imgraph.segm2components(input)
   else
      hcomponents,masks = torch.Tensor().imgraph.tree2components(input, true)
   end

   -- reorganize
   local components = {centroid_x={}, centroid_y={}, surface={}, 
                       id = {}, revid = {},
                       bbox_width = {}, bbox_height = {},
                       bbox_top = {}, bbox_bottom = {}, bbox_left = {}, bbox_right = {},
                       bbox_x = {}, bbox_y = {}, patch = {}, mask = {}}
   local i = 0
   for _,comp in pairs(hcomponents) do
      i = i + 1
      components.centroid_x[i]  = comp[1]
      components.centroid_y[i]  = comp[2]
      components.surface[i]     = comp[3]
      components.id[i]          = comp[5]
      components.revid[comp[5]] = i
      components.bbox_left[i]   = comp[6]
      components.bbox_right[i]  = comp[7]
      components.bbox_top[i]    = comp[8]
      components.bbox_bottom[i] = comp[9]
      components.bbox_width[i]  = comp[10]
      components.bbox_height[i] = comp[11]
      components.bbox_x[i]      = comp[12]
      components.bbox_y[i]      = comp[13]
      components.mask[i]        = masks[i]
   end
   components.size = function(self) return #self.surface end

   -- auxiliary image given ?
   if img and img:nDimension() == 3 then
      local c = components
      local maskit = false
      if config == 'masked' then maskit = true end
      for k = 1,i do
         if c.surface[k] >= minsize then
            -- get bounding box corners:
            local top = c.bbox_top[k]
            local height = c.bbox_height[k]
            local left = c.bbox_left[k]
            local width = c.bbox_width[k]

            -- extract patch from image:
            c.patch[k] = img:narrow(2,top,height):narrow(3,left,width):clone()

            -- generate mask, if not available
            if torch.typename(input) and not c.mask[k] then
               -- the input is a grayscale image, crop it to get the mask:
               c.mask[k] = input:narrow(1,top,height):narrow(2,left,width):clone()
               local id = components.id[k]
               local mask = function(x) if x == id then return 1 else return 0 end end
               c.mask[k]:apply(mask)
            end

            -- mask box
            if maskit then
               for i = 1,c.patch[k]:size(1) do
                  c.patch[k][i]:cmul(c.mask[k])
               end
            end
         end
      end
   end

   -- return both lists
   return components
end

----------------------------------------------------------------------
-- colorize a segmentation map
--
function imgraph.colorize(...)
   -- get args
   local args = {...}
   local grayscale = args[1]
   local colormap = args[2]

   -- usage
   if not grayscale or not (grayscale:dim() == 2 or (grayscale:dim() == 3 and grayscale:size(1) == 1)) then
      print(xlua.usage('imgraph.colorize',
                       'colorize a segmentation map',
                       'graph = imgraph.graph(image.lena())\n'
                          .. 'segm = imgraph.segmentmst(graph)\n'
                          .. 'colored = imgraph.colorize(segm)',
                       {type='torch.Tensor', help='input segmentation map (must be HxW), and each element must be in [1,width*height]', req=true},
                       {type='torch.Tensor', help='color map (must be Nx3), if not provided, auto generated'}))
      xlua.error('incorrect arguments', 'imgraph.colorize')
   end

   -- accept 3D grayscale
   if grayscale:dim() == 3 and grayscale:size(1) == 1 then
      grayscale = torch.Tensor(grayscale):resize(grayscale:size(2), grayscale:size(3))
   end

   -- support LongTensors
   if torch.typename(grayscale) == 'torch.LongTensor' then
      grayscale = torch.Tensor(grayscale:size(1), grayscale:size(2)):copy(grayscale)
   end

   -- auto type
   colormap = colormap or torch.Tensor():typeAs(grayscale)
   local colorized = torch.Tensor():typeAs(grayscale)

   -- colorize !
   grayscale.imgraph.colorize(colorized, grayscale, colormap)

   -- return colorized segmentation
   return colorized, colormap
end

----------------------------------------------------------------------
-- create a color map from a table
--
function imgraph.colormap(colors, default, verbose)
   -- usage
   if not colors then
      print(xlua.usage('imgraph.colormap',
                       'create a color map, from a table {id1={r,g,b}, id2={r,g,b}, ...}', nil,
                       {type='table', help='a table of RGB triplets (or more channels)', req=true},
                       {type='number', help='default value or triplet, for unspecified entries', default=0},
                       {type='boolean', help='verbose', default=false}))
      xlua.error('incorrect arguments', 'imgraph.colormap')
   end

   -- tensor?
   for k,entry in pairs(colors) do
      if torch.typename(entry) and torch.typename(entry):find('Tensor') then
         colors[k] = colors[k]:clone():storage():totable()
      end
   end

   -- found max in table
   local max = -math.huge
   for k,entry in pairs(colors) do
      if k > max then max = k end
   end

   -- nb of channels
   local channels = #colors[max]

   -- default val
   default = default or 0

   -- make map
   local nentries = max+1
   if verbose then print('<imgraph.colormap> creating map with ' .. nentries .. ' entries') end
   local colormap = torch.Tensor(nentries, channels):fill(default)
   for k,color in pairs(colors) do
      local c = colormap[k+1]
      for k = 1,channels do
         c[k] = color[k]
      end
   end

   -- return color map
   return colormap
end

----------------------------------------------------------------------
-- a simple test me function
--
imgraph._example = [[
      -- (0) user image
      local args = {...}
      local inputimg = args[1]

      -- (1) build a graph on an input image
      local inputimgg = image.convolve(inputimg, image.gaussian(3), 'same')
      local graph = imgraph.graph(inputimgg)

      -- (2) compute its connected components, and mst segmentation
      local cc = imgraph.connectcomponents(graph, 0.1, true)
      local mstsegm = imgraph.segmentmst(graph, 3, 20)
      local mstsegmcolor = imgraph.colorize(mstsegm)

      -- (3) do a histogram pooling of the original image:
      local pool = imgraph.histpooling(inputimg, mstsegm)

      -- (4) compute the watershed of the graph
      local graph = imgraph.graph(inputimgg, 8)
      local gradient = imgraph.graph2map(graph)
      local watershed = imgraph.watershed(gradient, 0.08, 8)
      local watershedgraph = imgraph.graph(watershed, 8)
      local watershedcc = imgraph.connectcomponents(watershedgraph, 0.5, true)

      -- (5) compute the saliency of a graph
      local graph = imgraph.graph(inputimg)
      tree = imgraph.mergetree(graph)
      local hierarchy = imgraph.graph2map(imgraph.tree2graph(tree))
      imgraph.filtertree(tree, 'volume')
      local filteredhierarchy = imgraph.graph2map(imgraph.tree2graph(tree))

      -- (6) compute the merge tree of the last graph
      local mt = imgraph.mergetree(graph)

      -- (7) display results
      image.display{image=inputimg, legend='input image'}
      image.display{image=cc, legend='thresholded graph'}
      image.display{image=watershed, legend='watershed on the graph'}
      image.display{image=watershedcc, legend='components of watershed'}
      image.display{image=mstsegmcolor, legend='segmented graph, using min-spanning tree'}
      image.display{image=pool, legend='original imaged hist-pooled by segmentation'}
      image.display{image=hierarchy, legend='raw edge-weighted graph watershed'}
      image.display{image=filteredhierarchy, legend='filtered edge-weighted graph watershed'}
]]
function imgraph.testme(usrimg)
   local inputimg
   if usrimg then
      inputimg = image.load(usrimg)
   else
      inputimg = image.lena()
      inputimg = image.scale(inputimg, 256, 256)
   end
   local example = loadstring(imgraph._example)
   print 'imgraph sample code {\n'
   print (imgraph._example)
   print '}'
   example(inputimg)
end
