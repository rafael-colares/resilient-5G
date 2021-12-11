#include "data.hpp"

/****************************************************************************************/
/*										Constructor										*/
/****************************************************************************************/

/** Constructor. **/
Data::Data(const std::string &parameter_file) : params(parameter_file)
{
	std::cout << "=> Defining data ..." << std::endl;
	readNodeFile(params.getNodeFile());
	readLinkFile(params.getLinkFile());
	readVnfFile(params.getVnfFile());
	readDemandFile(params.getDemandFile());

	buildGraph();

	std::cout << "\t Data was correctly constructed !" << std::endl;
	
}

/****************************************************************************************/
/*										Getters 										*/
/****************************************************************************************/
/* Returns the id from the node with the given name. */
int Data::getIdFromNodeName(const std::string name) const
{
	auto search = hashNode.find(name);
    if (search != hashNode.end()) {
        return search->second;
    } 
	else {
        std::cerr << "ERROR: Could not find a node with name '"<< name << "'... Abort." << std::endl;
		exit(EXIT_FAILURE);
    }
	int invalid = -1;
	return invalid;
}
/* Returns the id from the vnf with the given name. */
int Data::getIdFromVnfName(const std::string name) const
{
	for( const auto& it : hashVnf ) {
		if (it.first == name){
			return it.second;
		}
    }
    
	std::cerr << "ERROR: Could not find a vnf with name '"<< name << "'... Abort." << std::endl;
	exit(EXIT_FAILURE);
    
	return -1;
}

/* Returns the probability that all nodes fail simoustaneously. */
const double Data::getFailureProb (const std::vector<int>& nodes) const
{
    double prob_fail = 1.0;
    for (unsigned int j = 0; j < nodes.size(); j++){                    
        int v = nodes[j];
        prob_fail *= (1.0 - getNode(v).getAvailability());
    }
    return prob_fail;
}
/* Returns the chain availability based on the availability of each section. */
const double Data::getChainAvailability (const std::vector<double>& sectionAvail) const
{
    double prob = 1.0;
    for (unsigned int i = 0; i < sectionAvail.size(); i++){
        prob *= sectionAvail[i];
    }
    return prob;
}

/****************************************************************************************/
/*										Methods 										*/
/****************************************************************************************/

/* Reads the node file and fills the set of nodes. */
void Data::readNodeFile(const std::string filename)
{
    if (filename.empty()){
		std::cerr << "ERROR: A node file MUST be declared in the parameters file.\n";
		exit(EXIT_FAILURE);
	}
    std::cout << "\t Reading " << filename << " ..."  << std::endl;
	Reader reader(filename);
	/* dataList is a vector of vectors of strings. */
	/* dataList[0] corresponds to the first line of the document and dataList[0][i] to the i-th word.*/
	std::vector<std::vector<std::string> > dataList = reader.getData();
	// skip the first line (headers)
	for (unsigned int i = 1; i < dataList.size(); i++)	{
		int nodeId = (int)i - 1;
		std::string nodeName = dataList[i][0];
		double nodeX = atof(dataList[i][1].c_str());
		double nodeY = atof(dataList[i][2].c_str());
		double capacity = atof(dataList[i][3].c_str());
		double avail = atof(dataList[i][4].c_str());
		this->tabNodes.push_back(Node(nodeId, nodeName, nodeX, nodeY, capacity, avail));
		hashNode.insert({nodeName, nodeId});
	}
}

/* Reads the link file and fills the set of links. */
void Data::readLinkFile(const std::string filename)
{
    if (filename.empty()){
		std::cerr << "ERROR: A link file MUST be declared in the parameters file.\n";
		exit(EXIT_FAILURE);
	}
    std::cout << "\t Reading " << filename << " ..."  << std::endl;
	Reader reader(filename);
	/* dataList is a vector of vectors of strings. */
	/* dataList[0] corresponds to the first line of the document and dataList[0][i] to the i-th word.*/
	std::vector<std::vector<std::string> > dataList = reader.getData();
	// skip the first line (headers)
	for (unsigned int i = 1; i < dataList.size(); i++)	{
		int linkId = (int)i - 1;
		std::string linkName = dataList[i][0];
		int source = getIdFromNodeName(dataList[i][1]);
		int target = getIdFromNodeName(dataList[i][2]);
		double delay = atof(dataList[i][3].c_str());;
		double bandwidth = atof(dataList[i][4].c_str());
		this->tabLinks.push_back(Link(linkId, linkName, source, target, delay, bandwidth));
	}
}



/* Reads the vnf file and fills the set of vnfs. */
void Data::readVnfFile(const std::string filename)
{
    if (filename.empty()){
		std::cerr << "ERROR: A vnf file MUST be declared in the parameters file.\n";
		exit(EXIT_FAILURE);
	}
    std::cout << "\t Reading " << filename << " ..."  << std::endl;
	Reader reader(filename);
	/* dataList is a vector of vectors of strings. */
	/* dataList[0] corresponds to the first line of the document and dataList[0][i] to the i-th word.*/
	std::vector<std::vector<std::string> > dataList = reader.getData();
	// skip the first line (headers)
	for (unsigned int i = 1; i < dataList.size(); i++)	{
		int vnfId = (int)i - 1;
		std::string vnfName = dataList[i][0];
		double resource_consumption = atof(dataList[i][1].c_str());
		int nbNodes = dataList[i].size() - 2;
		if (nbNodes != getNbNodes()){
			std::cerr << "ERROR: Number of nodes in vnf file does not match the node file one.\n"; 
			exit(EXIT_FAILURE);
		}
		this->tabVnfs.push_back(VNF(vnfId, vnfName, resource_consumption, nbNodes));
		hashVnf.insert({vnfName, vnfId});
		for (unsigned int j = 2; j < dataList[i].size(); j++){
			int index = j - 2;
			tabVnfs[vnfId].setPlacementCost(index, atof(dataList[i][j].c_str()));
		}
	}
}

/** Reads the demand file and fills the set of demands. @param filename The demand file to be read. **/
void Data::readDemandFile(const std::string filename)
{
    if (filename.empty()){
		std::cerr << "ERROR: A demand file MUST be declared in the parameters file.\n";
		exit(EXIT_FAILURE);
	}
    std::cout << "\t Reading " << filename << " ..."  << std::endl;
	Reader reader(filename);
	/* dataList is a vector of vectors of strings. */
	/* dataList[0] corresponds to the first line of the document and dataList[0][i] to the i-th word.*/
	std::vector<std::vector<std::string> > dataList = reader.getData();
	// skip the first line (headers)
	for (unsigned int i = 1; i < dataList.size(); i++)	{
		int demandId = (int)i - 1;
		std::string demandName = dataList[i][0];
		int source = getIdFromNodeName(dataList[i][1]);
		int target = getIdFromNodeName(dataList[i][2]);
		double latency = atof(dataList[i][3].c_str());
		double band = atof(dataList[i][4].c_str());
		double availability = atof(dataList[i][5].c_str());
		this->tabDemands.push_back(Demand(demandId, demandName, source, target, latency, band, availability));
		std::vector<std::string> list = split(dataList[i][6], ",");
		for (unsigned int j = 0; j < list.size(); j++){
			if (!list[j].empty()){
				int vnfId = getIdFromVnfName(list[j]);
				tabDemands[demandId].addVNF(vnfId);
			}
		}
	}
}

/** Builds the network graph from data stored in tabNodes and tabLinks. **/
void Data::buildGraph()
{
	
	std::cout << "\t Creating graph..." << std::endl;
	/* Dymanic allocation of graph */
    graph = new Graph();
	nodeId = new NodeMap(*graph);
	lemonNodeId = new NodeMap(*graph);
	arcId = new ArcMap(*graph);
	lemonArcId = new ArcMap(*graph);
	
	/* Define nodes */
	for (unsigned int i = 0; i < tabNodes.size(); i++){
        Graph::Node n = graph->addNode();
        setNodeId(n, tabNodes[i].getId());
        setLemonNodeId(n, graph->id(n));
    }

	/* Define arcs */
	for (unsigned int i = 0; i < tabLinks.size(); i++){
        int source = tabLinks[i].getSource();
        int target = tabLinks[i].getTarget();
        Graph::Node sourceNode = lemon::INVALID;
        Graph::Node targetNode = lemon::INVALID;
        for (NodeIt v(getGraph()); v != lemon::INVALID; ++v){
            if(getNodeId(v) == source){
                sourceNode = v;
            }
            if(getNodeId(v) == target){
                targetNode = v;
            }
        }
        if (targetNode != lemon::INVALID && sourceNode != lemon::INVALID){
            Arc a = graph->addArc(sourceNode, targetNode);
            setLemonArcId(a, graph->id(a));
            setArcId(a, tabLinks[i].getId());
        }
    }
}

/****************************************************************************************/
/*										Display											*/
/****************************************************************************************/
void Data::print(){
	printNodes();
	printLinks();
	printVnfs();
	printDemands();
}


void Data::printNodes(){
	for (unsigned int i = 0; i < tabNodes.size(); i++){
        tabNodes[i].print();
    }
	std::cout << std::endl;
}
void Data::printLinks(){
	for (unsigned int i = 0; i < tabLinks.size(); i++){
        tabLinks[i].print();
    }
	std::cout << std::endl;
}

void Data::printVnfs(){
	for (unsigned int i = 0; i < tabVnfs.size(); i++){
        tabVnfs[i].print();
    }
	std::cout << std::endl;
}
void Data::printDemands(){
	for (unsigned int i = 0; i < tabDemands.size(); i++){
        tabDemands[i].print();
    }
	std::cout << std::endl;
}




/****************************************************************************************/
/*										Destructor										*/
/****************************************************************************************/

/** Desstructor. **/
Data::~Data()
{
    this->tabLinks.clear();
    this->tabNodes.clear();
	this->hashNode.clear();
	this->tabDemands.clear();
	this->tabVnfs.clear();
	delete nodeId;
	delete lemonNodeId;
	delete arcId;
	delete lemonArcId;
	delete graph;
}
