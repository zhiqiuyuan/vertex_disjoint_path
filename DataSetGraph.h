#ifndef DATASETGRAPH_H
#define DATASETGRAPH_H

#include "Graph.h"

// tve format
/*
t vnum enum
v id label degree //in+out degree
e sid tid ...
...
*/
// no edata
class TVEGraph : public Graph
{
public:
    bool buildGraph(std::string fname);
};

// e format
/*
s0 t0
s1 t1
...
*/
// no vdata, no edata
class EGraph : public Graph
{
public:
    bool buildGraph(std::string fname);
};

#endif // DATASETGRAPH_H