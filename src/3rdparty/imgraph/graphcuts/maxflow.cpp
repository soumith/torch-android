/* maxflow.cpp */


#include <stdio.h>
#include <stdint.h>
#include <graph.h>
#include <sys/types.h>
#include <maxflow.h>
#include <MTree_utils.h>

#include <math.h>

/*
	special constants for node->parent
*/
#define TERMINAL ( (arc *) 1 )		/* to terminal */
#define ORPHAN   ( (arc *) 2 )		/* orphan */


#define INFINITE_D ((int)(((unsigned)-1)/2))		/* infinite distance to the terminal */

/***********************************************************************/

/*
	Functions for processing active list.
	i->next points to the next node in the list
	(or to i, if i is the last node in the list).
	If i->next is NULL iff i is not in the list.

	There are two queues. Active nodes are added
	to the end of the second queue and read from
	the front of the first queue. If the first queue
	is empty, it is replaced by the second queue
	(and the second queue becomes empty).
*/


template <typename captype, typename tcaptype, typename flowtype> 
	inline void Graph<captype,tcaptype,flowtype>::set_active(node *i)
{
	if (!i->next)
	{
		/* it's not in the list yet */
		if (queue_last[1]) queue_last[1] -> next = i;
		else               queue_first[1]        = i;
		queue_last[1] = i;
		i -> next = i;
	}
}

/*
	Returns the next active node.
	If it is connected to the sink, it stays in the list,
	otherwise it is removed from the list
*/
template <typename captype, typename tcaptype, typename flowtype> 
	inline typename Graph<captype,tcaptype,flowtype>::node* Graph<captype,tcaptype,flowtype>::next_active()
{
	node *i;

	while ( 1 )
	{
		if (!(i=queue_first[0]))
		{
			queue_first[0] = i = queue_first[1];
			queue_last[0]  = queue_last[1];
			queue_first[1] = NULL;
			queue_last[1]  = NULL;
			if (!i) return NULL;
		}

		/* remove it from the active list */
		if (i->next == i) queue_first[0] = queue_last[0] = NULL;
		else              queue_first[0] = i -> next;
		i -> next = NULL;

		/* a node in the list is active iff it has a parent */
		if (i->parent) return i;
	}
}

/***********************************************************************/

template <typename captype, typename tcaptype, typename flowtype> 
	inline void Graph<captype,tcaptype,flowtype>::set_orphan_front(node *i)
{
	nodeptr *np;
	i -> parent = ORPHAN;
	np = nodeptr_block -> New();
	np -> ptr = i;
	np -> next = orphan_first;
	orphan_first = np;
}

template <typename captype, typename tcaptype, typename flowtype> 
	inline void Graph<captype,tcaptype,flowtype>::set_orphan_rear(node *i)
{
	nodeptr *np;
	i -> parent = ORPHAN;
	np = nodeptr_block -> New();
	np -> ptr = i;
	if (orphan_last) orphan_last -> next = np;
	else             orphan_first        = np;
	orphan_last = np;
	np -> next = NULL;
}

/***********************************************************************/

template <typename captype, typename tcaptype, typename flowtype> 
	inline void Graph<captype,tcaptype,flowtype>::add_to_changed_list(node *i)
{
	if (changed_list && !i->is_in_changed_list)
	{
		node_id* ptr = changed_list->New();
		*ptr = (node_id)(i - nodes);
		i->is_in_changed_list = true;
	}
}

/***********************************************************************/

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::maxflow_init()
{
	node *i;

	queue_first[0] = queue_last[0] = NULL;
	queue_first[1] = queue_last[1] = NULL;
	orphan_first = NULL;

	TIME = 0;

	for (i=nodes; i<node_last; i++)
	{
		i -> next = NULL;
		i -> is_marked = 0;
		i -> is_in_changed_list = 0;
		i -> TS = TIME;
		if (i->tr_cap > 0)
		{
			/* i is connected to the source */
			i -> is_sink = 0;
			i -> parent = TERMINAL;
			set_active(i);
			i -> DIST = 1;
		}
		else if (i->tr_cap < 0)
		{
			/* i is connected to the sink */
			i -> is_sink = 1;
			i -> parent = TERMINAL;
			set_active(i);
			i -> DIST = 1;
		}
		else
		{
			i -> parent = NULL;
		}
	}
}

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::maxflow_reuse_trees_init()
{
	node* i;
	node* j;
	node* queue = queue_first[1];
	arc* a;
	nodeptr* np;

	queue_first[0] = queue_last[0] = NULL;
	queue_first[1] = queue_last[1] = NULL;
	orphan_first = orphan_last = NULL;

	TIME ++;

	while ((i=queue))
	{
		queue = i->next;
		if (queue == i) queue = NULL;
		i->next = NULL;
		i->is_marked = 0;
		set_active(i);

		if (i->tr_cap == 0)
		{
			if (i->parent) set_orphan_rear(i);
			continue;
		}

		if (i->tr_cap > 0)
		{
			if (!i->parent || i->is_sink)
			{
				i->is_sink = 0;
				for (a=i->first; a; a=a->next)
				{
					j = a->head;
					if (!j->is_marked)
					{
						if (j->parent == a->sister) set_orphan_rear(j);
						if (j->parent && j->is_sink && a->r_cap > 0) set_active(j);
					}
				}
				add_to_changed_list(i);
			}
		}
		else
		{
			if (!i->parent || !i->is_sink)
			{
				i->is_sink = 1;
				for (a=i->first; a; a=a->next)
				{
					j = a->head;
					if (!j->is_marked)
					{
						if (j->parent == a->sister) set_orphan_rear(j);
						if (j->parent && !j->is_sink && a->sister->r_cap > 0) set_active(j);
					}
				}
				add_to_changed_list(i);
			}
		}
		i->parent = TERMINAL;
		i -> TS = TIME;
		i -> DIST = 1;
	}

	//test_consistency();

	/* adoption */
	while ((np=orphan_first))
	{
		orphan_first = np -> next;
		i = np -> ptr;
		nodeptr_block -> Delete(np);
		if (!orphan_first) orphan_last = NULL;
		if (i->is_sink) process_sink_orphan(i);
		else            process_source_orphan(i);
	}
	/* adoption end */

	//test_consistency();
}

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::augment(arc *middle_arc)
{
	node *i;
	arc *a;
	tcaptype bottleneck;


	/* 1. Finding bottleneck capacity */
	/* 1a - the source tree */
	bottleneck = middle_arc -> r_cap;
	for (i=middle_arc->sister->head; ; i=a->head)
	{
		a = i -> parent;
		if (a == TERMINAL) break;
		if (bottleneck > a->sister->r_cap) bottleneck = a -> sister -> r_cap;
	}
	if (bottleneck > i->tr_cap) bottleneck = i -> tr_cap;
	/* 1b - the sink tree */
	for (i=middle_arc->head; ; i=a->head)
	{
		a = i -> parent;
		if (a == TERMINAL) break;
		if (bottleneck > a->r_cap) bottleneck = a -> r_cap;
	}
	if (bottleneck > - i->tr_cap) bottleneck = - i -> tr_cap;


	/* 2. Augmenting */
	/* 2a - the source tree */
	middle_arc -> sister -> r_cap += bottleneck;
	middle_arc -> r_cap -= bottleneck;
	for (i=middle_arc->sister->head; ; i=a->head)
	{
		a = i -> parent;
		if (a == TERMINAL) break;
		a -> r_cap += bottleneck;
		a -> sister -> r_cap -= bottleneck;
		if (!a->sister->r_cap)
		{
			set_orphan_front(i); // add i to the beginning of the adoption list
		}
	}
	i -> tr_cap -= bottleneck;
	if (!i->tr_cap)
	{
		set_orphan_front(i); // add i to the beginning of the adoption list
	}
	/* 2b - the sink tree */
	for (i=middle_arc->head; ; i=a->head)
	{
		a = i -> parent;
		if (a == TERMINAL) break;
		a -> sister -> r_cap += bottleneck;
		a -> r_cap -= bottleneck;
		if (!a->r_cap)
		{
			set_orphan_front(i); // add i to the beginning of the adoption list
		}
	}
	i -> tr_cap += bottleneck;
	if (!i->tr_cap)
	{
		set_orphan_front(i); // add i to the beginning of the adoption list
	}


	flow += bottleneck;
}

/***********************************************************************/

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::process_source_orphan(node *i)
{
	node *j;
	arc *a0, *a0_min = NULL, *a;
	int d, d_min = INFINITE_D;

	/* trying to find a new parent */
	for (a0=i->first; a0; a0=a0->next)
	if (a0->sister->r_cap)
	{
		j = a0 -> head;
		if (!j->is_sink && (a=j->parent))
		{
			/* checking the origin of j */
			d = 0;
			while ( 1 )
			{
				if (j->TS == TIME)
				{
					d += j -> DIST;
					break;
				}
				a = j -> parent;
				d ++;
				if (a==TERMINAL)
				{
					j -> TS = TIME;
					j -> DIST = 1;
					break;
				}
				if (a==ORPHAN) { d = INFINITE_D; break; }
				j = a -> head;
			}
			if (d<INFINITE_D) /* j originates from the source - done */
			{
				if (d<d_min)
				{
					a0_min = a0;
					d_min = d;
				}
				/* set marks along the path */
				for (j=a0->head; j->TS!=TIME; j=j->parent->head)
				{
					j -> TS = TIME;
					j -> DIST = d --;
				}
			}
		}
	}

	if ((i->parent = a0_min) != NULL)
	{
		i -> TS = TIME;
		i -> DIST = d_min + 1;
	}
	else
	{
		/* no parent is found */
		add_to_changed_list(i);

		/* process neighbors */
		for (a0=i->first; a0; a0=a0->next)
		{
			j = a0 -> head;
			if (!j->is_sink && (a=j->parent))
			{
				if (a0->sister->r_cap) set_active(j);
				if (a!=TERMINAL && a!=ORPHAN && a->head==i)
				{
					set_orphan_rear(j); // add j to the end of the adoption list
				}
			}
		}
	}
}

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::process_sink_orphan(node *i)
{
	node *j;
	arc *a0, *a0_min = NULL, *a;
	int d, d_min = INFINITE_D;

	/* trying to find a new parent */
	for (a0=i->first; a0; a0=a0->next)
	if (a0->r_cap)
	{
		j = a0 -> head;
		if (j->is_sink && (a=j->parent))
		{
			/* checking the origin of j */
			d = 0;
			while ( 1 )
			{
				if (j->TS == TIME)
				{
					d += j -> DIST;
					break;
				}
				a = j -> parent;
				d ++;
				if (a==TERMINAL)
				{
					j -> TS = TIME;
					j -> DIST = 1;
					break;
				}
				if (a==ORPHAN) { d = INFINITE_D; break; }
				j = a -> head;
			}
			if (d<INFINITE_D) /* j originates from the sink - done */
			{
				if (d<d_min)
				{
					a0_min = a0;
					d_min = d;
				}
				/* set marks along the path */
				for (j=a0->head; j->TS!=TIME; j=j->parent->head)
				{
					j -> TS = TIME;
					j -> DIST = d --;
				}
			}
		}
	}

	if ((i->parent = a0_min) != NULL)
	{
		i -> TS = TIME;
		i -> DIST = d_min + 1;
	}
	else
	{
		/* no parent is found */
		add_to_changed_list(i);

		/* process neighbors */
		for (a0=i->first; a0; a0=a0->next)
		{
			j = a0 -> head;
			if (j->is_sink && (a=j->parent))
			{
				if (a0->r_cap) set_active(j);
				if (a!=TERMINAL && a!=ORPHAN && a->head==i)
				{
					set_orphan_rear(j); // add j to the end of the adoption list
				}
			}
		}
	}
}

/***********************************************************************/

template <typename captype, typename tcaptype, typename flowtype> 
	flowtype Graph<captype,tcaptype,flowtype>::maxflow(bool reuse_trees, Block<node_id>* _changed_list)
{
	node *i, *j, *current_node = NULL;
	arc *a;
	nodeptr *np, *np_next;

	if (!nodeptr_block)
	{
		nodeptr_block = new DBlock<nodeptr>(NODEPTR_BLOCK_SIZE, error_function);
	}

	changed_list = _changed_list;
	if (maxflow_iteration == 0 && reuse_trees) { if (error_function) (*error_function)((char*)"reuse_trees cannot be used in the first call to maxflow()!"); exit(1); }
	if (changed_list && !reuse_trees) { if (error_function) (*error_function)((char*)"changed_list cannot be used without reuse_trees!"); exit(1); }

	if (reuse_trees) maxflow_reuse_trees_init();
	else             maxflow_init();

	// main loop
	while ( 1 )
	{
		// test_consistency(current_node);

		if ((i=current_node))
		{
			i -> next = NULL; /* remove active flag */
			if (!i->parent) i = NULL;
		}
		if (!i)
		{
			if (!(i = next_active())) break;
		}

		/* growth */
		if (!i->is_sink)
		{
			/* grow source tree */
			for (a=i->first; a; a=a->next)
			if (a->r_cap)
			{
				j = a -> head;
				if (!j->parent)
				{
					j -> is_sink = 0;
					j -> parent = a -> sister;
					j -> TS = i -> TS;
					j -> DIST = i -> DIST + 1;
					set_active(j);
					add_to_changed_list(j);
				}
				else if (j->is_sink) break;
				else if (j->TS <= i->TS &&
				         j->DIST > i->DIST)
				{
					/* heuristic - trying to make the distance from j to the source shorter */
					j -> parent = a -> sister;
					j -> TS = i -> TS;
					j -> DIST = i -> DIST + 1;
				}
			}
		}
		else
		{
			/* grow sink tree */
			for (a=i->first; a; a=a->next)
			if (a->sister->r_cap)
			{
				j = a -> head;
				if (!j->parent)
				{
					j -> is_sink = 1;
					j -> parent = a -> sister;
					j -> TS = i -> TS;
					j -> DIST = i -> DIST + 1;
					set_active(j);
					add_to_changed_list(j);
				}
				else if (!j->is_sink) { a = a -> sister; break; }
				else if (j->TS <= i->TS &&
				         j->DIST > i->DIST)
				{
					/* heuristic - trying to make the distance from j to the sink shorter */
					j -> parent = a -> sister;
					j -> TS = i -> TS;
					j -> DIST = i -> DIST + 1;
				}
			}
		}

		TIME ++;

		if (a)
		{
			i -> next = i; /* set active flag */
			current_node = i;

			/* augmentation */
			augment(a);
			/* augmentation end */

			/* adoption */
			while ((np=orphan_first))
			{
				np_next = np -> next;
				np -> next = NULL;

				while ((np=orphan_first))
				{
					orphan_first = np -> next;
					i = np -> ptr;
					nodeptr_block -> Delete(np);
					if (!orphan_first) orphan_last = NULL;
					if (i->is_sink) process_sink_orphan(i);
					else            process_source_orphan(i);
				}

				orphan_first = np_next;
			}
			/* adoption end */
		}
		else current_node = NULL;
	}
	// test_consistency();

	if (!reuse_trees || (maxflow_iteration % 64) == 0)
	{
		delete nodeptr_block; 
		nodeptr_block = NULL; 
	}

	maxflow_iteration ++;
	return flow;
}

/***********************************************************************/



template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::test_consistency(node* current_node)
{
	node *i;
	arc *a;
	int r;
	int num1 = 0, num2 = 0;

	// test whether all nodes i with i->next!=NULL are indeed in the queue
	for (i=nodes; i<node_last; i++)
	{
		if (i->next || i==current_node) num1 ++;
	}
	for (r=0; r<3; r++)
	{
		i = (r == 2) ? current_node : queue_first[r];
		if (i)
		for ( ; ; i=i->next)
		{
			num2 ++;
			if (i->next == i)
			{
				if (r<2) assert(i == queue_last[r]);
				else     assert(i == current_node);
				break;
			}
		}
	}
	assert(num1 == num2);

	for (i=nodes; i<node_last; i++)
	{
		// test whether all edges in seach trees are non-saturated
		if (i->parent == NULL) {}
		else if (i->parent == ORPHAN) {}
		else if (i->parent == TERMINAL)
		{
			if (!i->is_sink) assert(i->tr_cap > 0);
			else             assert(i->tr_cap < 0);
		}
		else
		{
			if (!i->is_sink) assert (i->parent->sister->r_cap > 0);
			else             assert (i->parent->r_cap > 0);
		}
		// test whether passive nodes in search trees have neighbors in
		// a different tree through non-saturated edges
		if (i->parent && !i->next)
		{
			if (!i->is_sink)
			{
				assert(i->tr_cap >= 0);
				for (a=i->first; a; a=a->next)
				{
					if (a->r_cap > 0) assert(a->head->parent && !a->head->is_sink);
				}
			}
			else
			{
				assert(i->tr_cap <= 0);
				for (a=i->first; a; a=a->next)
				{
					if (a->sister->r_cap > 0) assert(a->head->parent && a->head->is_sink);
				}
			}
		}
		// test marking invariants
		if (i->parent && i->parent!=ORPHAN && i->parent!=TERMINAL)
		{
			assert(i->TS <= i->parent->head->TS);
			if (i->TS == i->parent->head->TS) assert(i->DIST > i->parent->head->DIST);
		}
	}
}




template <typename captype, typename tcaptype, typename flowtype> 
	Graph<captype, tcaptype, flowtype>::Graph(int node_num_max, int edge_num_max, void (*err_function)(char *))
	: node_num(0),
	  nodeptr_block(NULL),
	  error_function(err_function)
{
	if (node_num_max < 16) node_num_max = 16;
	if (edge_num_max < 16) edge_num_max = 16;

	nodes = (node*) malloc(node_num_max*sizeof(node));
	arcs = (arc*) malloc(2*edge_num_max*sizeof(arc));
	if (!nodes || !arcs) { if (error_function) (*error_function)((char*)"Not enough memory!"); exit(1); }

	node_last = nodes;
	node_max = nodes + node_num_max;
	arc_last = arcs;
	arc_max = arcs + 2*edge_num_max;

	maxflow_iteration = 0;
	flow = 0;
}

template <typename captype, typename tcaptype, typename flowtype> 
	Graph<captype,tcaptype,flowtype>::~Graph()
{
	if (nodeptr_block) 
	{ 
		delete nodeptr_block; 
		nodeptr_block = NULL; 
	}
	free(nodes);
	free(arcs);
}

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::reset()
{
	node_last = nodes;
	arc_last = arcs;
	node_num = 0;

	if (nodeptr_block) 
	{ 
		delete nodeptr_block; 
		nodeptr_block = NULL; 
	}

	maxflow_iteration = 0;
	flow = 0;
}

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::reallocate_nodes(int num)
{
	int node_num_max = (int)(node_max - nodes);
	node* nodes_old = nodes;

	node_num_max += node_num_max / 2;
	if (node_num_max < node_num + num) node_num_max = node_num + num;
	nodes = (node*) realloc(nodes_old, node_num_max*sizeof(node));
	if (!nodes) { if (error_function) (*error_function)((char*)"Not enough memory!"); exit(1); }

	node_last = nodes + node_num;
	node_max = nodes + node_num_max;

	if (nodes != nodes_old)
	{
		arc* a;
		for (a=arcs; a<arc_last; a++)
		{
			a->head = (node*) ((char*)a->head + (((char*) nodes) - ((char*) nodes_old)));
		}
	}
}

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::reallocate_arcs()
{
	int arc_num_max = (int)(arc_max - arcs);
	int arc_num = (int)(arc_last - arcs);
	arc* arcs_old = arcs;

	arc_num_max += arc_num_max / 2; if (arc_num_max & 1) arc_num_max ++;
	arcs = (arc*) realloc(arcs_old, arc_num_max*sizeof(arc));
	if (!arcs) { if (error_function) (*error_function)((char*)"Not enough memory!"); exit(1); }

	arc_last = arcs + arc_num;
	arc_max = arcs + arc_num_max;

	if (arcs != arcs_old)
	{
		node* i;
		arc* a;
		for (i=nodes; i<node_last; i++)
		{
			if (i->first) i->first = (arc*) ((char*)i->first + (((char*) arcs) - ((char*) arcs_old)));
		}
		for (a=arcs; a<arc_last; a++)
		{
			if (a->next) a->next = (arc*) ((char*)a->next + (((char*) arcs) - ((char*) arcs_old)));
			a->sister = (arc*) ((char*)a->sister + (((char*) arcs) - ((char*) arcs_old)));
		}
	}
}


/*================================================*/
void Insert1(list **sl, int index)
/*================================================*/
{
  list *tmp = NULL;
  list *csl = *sl;
  list *elem = (list*) malloc(sizeof(list));
  if(!elem) exit(EXIT_FAILURE);
  elem->index = index;
  while(csl)
    {
      tmp = csl;
      csl = csl->next;
    }
  elem->next = csl;
  if(tmp) tmp->next = elem;
  else *sl = elem;
}


/*================================================*/
void PrintList1(list *sl)
/*================================================*/
{
  fprintf(stderr, "Nodes of the cut:\n");
  while(sl)
    {
      printf("%d\n",sl->index);
      sl = sl->next;
    }
}



/* =============================================================== */
list * Graph_Cuts(MergeTree *MT )
/* =============================================================== */
{
  /* Segment a tree into two components.
  Returns a list of nodes correspunding to the Min cut,
  computed using a Max Flow algorithm */
  int y, i, j, tmp;
  int nb_markers; int nb_leafs;
  long N, M; 
 // -------- Gathering usefull input graph (MT) informations ----------- 
  float val=1; //weight parameter for leafs.
  mtree * T= MT->tree;
  float * W = MT->weights;
  // mergeTreePrint(T);
  JCctree *CT = T->CT;
  int root_node = CT->root;

  //nb nodes
  M = CT->nbnodes;

  // nb_edges
  nb_leafs = 0;
  for (i = 0; i < M; i++)
    if (CT->tabnodes[i].nbsons == 0)
      nb_leafs++;

  nb_markers = nb_leafs+1;
  N=M+nb_markers;
  M=N-1;
  // printf("Nb nodes CT = %d  Nb nodes:%d Nb edges: %d Nb leafs :%d \n",CT->nbnodes, N, M, nb_leafs);



  // printf("index root = %d   \n",CT->root);
   
   double CTE_WEIGHTS = 100000;
   // printf("CTE_WEIGHTS =  %f %d \n",CTE_WEIGHTS, INFINITE_D );
   JCsoncell *s;

   int * SeededNodes = (int*)malloc((CT->nbnodes+1)*sizeof(int));
   if (SeededNodes == NULL) { fprintf(stderr, "kruskal : malloc failed\n"); exit(0); }
   

   // markers
   
   j=0;
   for (i = 0; i < CT->nbnodes; i++)
      if (CT->tabnodes[i].nbsons == 0)
     {
	 SeededNodes[i]= j+CT->nbnodes;
	 j++;
      }
   SeededNodes[CT->root]= M;   


   // weights
   
   float * weights = (float *)malloc(N*sizeof(float));
   for(i=0;i<CT->nbnodes;i++)
     weights[i]=W[i];
   
   for(i=0;i<nb_markers;i++)
     weights[CT->nbnodes+i]=val;
   
   
   // ---------- Graph construction to call the Max Flow routine --------------
   typedef Graph<int,int,int> GraphType;
   GraphType *g = new GraphType(N,  M);//estimated # of nodes, estimated # of edges
   
   for (i=0;i<N;i++)
     g -> add_node(); 
   
   g -> add_tweights(SeededNodes[CT->root], INFINITE_D, 0 );
 
   for (i = 0; i < CT->nbnodes; i++)
      if (CT->tabnodes[i].nbsons == 0)
	{
	  g -> add_tweights(SeededNodes[i], 0, INFINITE_D);
	  // fprintf(stderr,"adding the sink edge to node %d \n", SeededNodes[i] );
	}   
   // fprintf(stderr,"adding the source edge to node %d \n", SeededNodes[CT->root] );
   for  (i=0;i<CT->nbnodes;i++)
     {
       tmp = CT->tabnodes[i].nbsons;
       if ((tmp==0)) //leaf
	 {
	  
	   // fprintf(stderr,"node %d is a leaf \n", i);
	   y= SeededNodes[i]; // edge index
	   //fprintf(stderr,"edge (%d %d) \n", i,y);
	   g -> add_edge( i, y, (int)(weights[y]*CTE_WEIGHTS), (int)(weights[y]*CTE_WEIGHTS) );
	   // fprintf(stderr,"coucou5\n");
	   // fprintf(stderr,"Nb nodes:%d Nb edges: %d Nb leafs :%d \n", N, M, nb_leafs);
	   
 
	 } 
       else
	 {
	   for ( s = CT->tabnodes[i].sonlist;s!=NULL;s = s->next)  
	     {
	       // fprintf(stderr,"coucou2 \n");
	       y=s->son;
	       //fprintf(stderr,"edge (%d %d) %d \n", i,y,  (int)(weights[y]*CTE_WEIGHTS ));
	       g -> add_edge( i, y, (int)(weights[y]*CTE_WEIGHTS), (int)(weights[y]*CTE_WEIGHTS) );
	       // fprintf(stderr,"coucou4 %d\n", i);
	       //printf("Nb nodes:%d Nb edges: %d Nb leafs :%d \n", N, M, nb_leafs);
	     }
	 }
     }

g -> add_edge( root_node, M, (int)(weights[root_node]*CTE_WEIGHTS), (int)(weights[root_node]*CTE_WEIGHTS) );

//fprintf(stderr,"coucou\n");
   
 // ------------------ Calling the Max Flow algorithm ----------------------
 fprintf(stderr,"Max flow walue  =  %d \n", g -> maxflow());
   
 
  int * Map = (int *)malloc(N*sizeof(int));
  
  for  (i=0;i<N;i++)
    {
      if (g->what_segment(i) == GraphType::SOURCE)
	Map[i]=0;  
      else if (g->what_segment(i) == GraphType::SINK)
	Map[i]=1;
      else printf("Graphcut error\n");
      //printf("Map[%d]=%d\n", i, Map[i]);
    }
   delete g;
  free(weights);  
 
 // ------------------ Process the tree to find the cut ----------------------
  list * cut = NULL;
 for (i = 0; i < CT->nbnodes; i++)
    {
      // nodes having a different value than their father are in the cut
      if ((CT->tabnodes[i].father != -1) && (Map[CT->tabnodes[i].father] != Map[i]))
        Insert1(&cut, i);
      // leafs having the same label as the root are in the cut
      if ((CT->tabnodes[i].nbsons == 0) && (Map[i]==0))
        Insert1(&cut, i);
    }

 
 if (cut == NULL)  Insert1(&cut, root_node);
 //PrintList1(cut);
  free(Map);
  free(SeededNodes);

  return cut;
}
