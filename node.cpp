#include "node.hpp"

/****************************************************************************************/
/*										Constructor										*/
/****************************************************************************************/

/** Constructor. **/
Node::Node(const int id_, const std::string name_, const double x, const double y, const double cap, const double avail) : 
                id(id_), name(name_), coordinate_x(x), coordinate_y(y), capacity(cap), availability(avail) {}

/****************************************************************************************/
/*										Display										    */
/****************************************************************************************/

/* Displays information about the node. */
void Node::print(){
    std::cout << "Id: " << id << ", "
              << "Name: " << name << ", "
              << "x: " << coordinate_x << ", "
              << "y: " << coordinate_y << ", "
              << "Capacity: " << capacity << ", "
              << "Availability: " << availability << std::endl;
}