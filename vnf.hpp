#ifndef __vnf__hpp
#define __vnf__hpp

/****************************************************************************************/
/*										LIBRARIES										*/
/****************************************************************************************/

/*** C++ Libraries ***/
#include <iostream>
#include <string>
#include <vector>


/*************************************************
 * This class models a VNF in the network. 
 * Each VNF has an id, a name and.
*************************************************/
class VNF{
    private:
        const int 					id;					/**< VNF id. **/
        const std::string 			name;				/**< VNF name. **/
		const double				consumption;		/**< VNF resource consumption. **/
		std::vector<double> 		placement_cost; 	/**< VNF cost of placement. **/

		const int 			nbNodes; 	/**< Number of nodes in the network. **/
		std::vector<bool>	is_placed;	/**< A vector of booleans stating whether or not the VNF is placed on each node. **/


    public:
	/****************************************************************************************/
	/*										Constructor										*/
	/****************************************************************************************/

	/** Constructor. @param id_ VNF id. @param name_ VNF name. @param cons VNF resource consumption.**/
	VNF(const int id_ = -1, const std::string name_ = "", const double cons = 0.0, const int n = 0);
    

	/****************************************************************************************/
	/*										Getters											*/
	/****************************************************************************************/
    
	/** Returns the vnf's id. **/
	const int& 					getId()				const { return this->id; }
	/** Returns the vnf's name. **/
	const std::string& 			getName() 			const { return this->name; }
	/** Returns the vnf's resource consumption. **/
	const double& 				getConsumption() 	const { return this->consumption; }
	/** Returns the vnf's placement cost vector. **/
	const std::vector<double>& 	getPlacementCost() 	const { return this->placement_cost; }


	/** Returns the vnf's placement cost on a given node. @param nodeId The node's id. **/
	const double& getPlacementCostOnNode(const int nodeId) const { return this->placement_cost[nodeId]; }

	
	/****************************************************************************************/
	/*										Setters											*/
	/****************************************************************************************/

	/** Sets the placement cost on a given node. @param nodeId The node's id. @param cost The new cost of placement. **/
	void setPlacementCost(const int nodeId, const double cost) {this->placement_cost[nodeId] = cost;}

	/****************************************************************************************/
	/*										Display											*/
	/****************************************************************************************/
	/** Displays information about the vnf. **/
	void print();
};

#endif