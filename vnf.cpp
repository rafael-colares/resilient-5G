#include "vnf.hpp"

/****************************************************************************************/
/*										Constructor										*/
/****************************************************************************************/

/** Constructor. **/
VNF::VNF(const int id_, const std::string name_, const double cons, const int n) : 
            id(id_), name(name_), consumption(cons), nbNodes(n) 
{
    placement_cost.resize(nbNodes, 0.0);
    is_placed.resize(nbNodes, false);
}

/****************************************************************************************/
/*										Display											*/
/****************************************************************************************/
/* Displays information about the vnf. */
void VNF::print(){
    std::cout << "Id: " << id << ", "
              << "Name: " << name << ", "
              << "Consumption: " << consumption << std::endl;
    std::cout << "\tCost of placement: ";
    for (unsigned int i = 0; i < placement_cost.size(); i++){
        std::cout << placement_cost[i] << ", ";
    }
    std::cout << std::endl;
}