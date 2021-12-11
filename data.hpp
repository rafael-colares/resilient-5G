#ifndef __data__hpp
#define __data__hpp


/****************************************************************************************/
/*										LIBRARIES										*/
/****************************************************************************************/

/*** C++ Libraries ***/
#include <float.h>
#include <unordered_map>
#include <algorithm>

/*** LEMON Libraries ***/     
#include <lemon/list_graph.h>

/*** Own Libraries ***/  
#include "input.hpp"
#include "../network/demand.hpp"
#include "../network/node.hpp"
#include "../network/link.hpp"
#include "../network/vnf.hpp"
#include "../tools/reader.hpp"


/****************************************************************************************/
/*										TYPEDEFS										*/
/****************************************************************************************/

/*** LEMON ***/
/* Structures */
typedef lemon::ListDigraph 	Graph;
typedef Graph::Arc 			Arc;

/* Iterators */
typedef Graph::NodeIt 		NodeIt;
typedef Graph::ArcIt 		ArcIt;

/* Maps */
typedef Graph::NodeMap<int> NodeMap;
typedef Graph::ArcMap<int> ArcMap;

/********************************************************************************************
 * This class stores the data needed for modeling an instance of the Resilient SFC routing 
 * and VNF placement problem. This consists of a network graph, 											
********************************************************************************************/
class Data {

private:
	Input 				params;						/**< Input parameters. **/
    std::vector<Node> 	tabNodes;         			/**< Set of nodes. **/
	std::vector<Link> 	tabLinks;					/**< Set of links. **/
	std::vector<VNF> 	tabVnfs;					/**< Set of VNFs. **/
	std::vector<Demand> tabDemands;					/**< Set of demands. **/

	Graph* 				graph;						/**< The network graph. **/
	NodeMap* 			nodeId;						/**< A map storing the nodes' ids. **/
	NodeMap* 			lemonNodeId;				/**< A map storing the nodes' lemon ids. **/
	ArcMap* 			arcId;						/**< A map storing the arcs' ids. **/
	ArcMap* 			lemonArcId;					/**< A map storing the arcs' lemon ids. **/

	std::unordered_map<std::string, int> hashNode; 	/**< A map for locating node id's from its name. **/
	std::unordered_map<std::string, int> hashVnf; 	/**< A map for locating vnf id's from its name. **/
	
public:

	/****************************************************************************************/
	/*										Constructor										*/
	/****************************************************************************************/

	/** Constructor initializes the object with the information of an Input. @param parameter_file The parameters file.**/
	Data(const std::string &parameter_file);



	/****************************************************************************************/
	/*										Getters											*/
	/****************************************************************************************/

	const Input& 			 	getInput 		 () const { return params; }
	const Graph& 			 	getGraph     	 () const { return *graph; }
	const NodeMap& 			 	getNodeIds   	 () const { return *nodeId; }
	const NodeMap& 			 	getLemonNodeIds  () const { return *lemonNodeId; }
	const ArcMap& 			 	getArcIds    	 () const { return *arcId; }
	const ArcMap& 			 	getLemonArcIds   () const { return *lemonArcId; }
	const std::vector<Node>& 	getNodes     	 () const { return tabNodes; }
	const std::vector<Link>& 	getLinks     	 () const { return tabLinks; }
	const std::vector<VNF>&  	getVnfs     	 () const { return tabVnfs; }
	const std::vector<Demand>&  getDemands     	 () const { return tabDemands; }

	const VNF& 		getVnf    (const int i) 		 const { return tabVnfs[i]; }
	const Demand& 	getDemand (const int i) 		 const { return tabDemands[i]; }
	const Link& 	getLink   (const int i) 		 const { return tabLinks[i]; }
	const Node& 	getNode   (const int i) 		 const { return tabNodes[i]; }

	const int  getNbNodes     () 					 const { return (int)tabNodes.size(); }
	const int  getNbVnfs      () 					 const { return (int)tabVnfs.size(); }
	const int  getNbDemands   () 					 const { return (int)tabDemands.size(); }
	const int& getNodeId   	  (const Graph::Node& v) const { return (*nodeId)[v]; }
	const int& getLemonNodeId (const Graph::Node& v) const { return (*lemonNodeId)[v]; }
	const int& getArcId    	  (const Arc& a) 		 const { return (*arcId)[a]; }
	const int& getLemonArcId  (const Arc& a) 		 const { return (*lemonArcId)[a]; }

	/** Returns the id from the node with the given name. @param name The node name. **/
	int	 	   getIdFromNodeName(const std::string name) const;

	/** Returns the id from the vnf with the given name. @param name The vnf name. **/
	int	 	   getIdFromVnfName(const std::string name) const;

    /** Returns the probability that a set of nodes fail simoustaneously. @param nodes The set of nodes to fail. **/
    const double getFailureProb(const std::vector<int>& nodes) const;
    
    /** Returns the chain availability based on the availability of each section. @note The chain availability is the product of the availability of its sections. @param sectionAvail The sections availability. **/
    const double getChainAvailability(const std::vector<double>& sectionAvail) const;

	/****************************************************************************************/
	/*										Setters											*/
	/****************************************************************************************/
	void setNodeId 		(const Graph::Node& v, const int &id) { (*nodeId)[v] = id; }
	void setLemonNodeId (const Graph::Node& v, const int &id) { (*lemonNodeId)[v] = id; }
	void setArcId 		(const Graph::Arc& a, const int &id)  { (*arcId)[a] = id; }
	void setLemonArcId 	(const Graph::Arc& a, const int &id)  { (*lemonArcId)[a] = id; }


	/****************************************************************************************/
	/*										Methods											*/
	/****************************************************************************************/

	/** Reads the node file and fills the set of nodes. @param filename The node file to be read. **/
	void readNodeFile(const std::string filename);

	/** Reads the link file and fills the set of link. @param filename The link file to be read. **/
	void readLinkFile(const std::string filename);

	/** Reads the vnf file and fills the set of vnfs. @param filename The vnf file to be read. **/
	void readVnfFile(const std::string filename);

	/** Reads the demand file and fills the set of demands. @param filename The demand file to be read. **/
	void readDemandFile(const std::string filename);

	/** Builds the network graph from data stored in tabNodes and tabLinks. **/
	void buildGraph();

	/****************************************************************************************/
	/*										Display											*/
	/****************************************************************************************/
	void print();
	void printNodes();
	void printLinks();
	void printDemands();
	void printVnfs();


	/****************************************************************************************/
	/*										Destructor										*/
	/****************************************************************************************/

	/** Destructor. Clears the vectors of demands and links. **/
	~Data();
};

#endif