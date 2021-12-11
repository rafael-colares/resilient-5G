#ifndef __callback__hpp
#define __callback__hpp

/****************************************************************************************/
/*										LIBRARIES										*/
/****************************************************************************************/

/*** C++ Libraries ***/
#include <thread>
#include <mutex>

/*** CPLEX Libraries ***/
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

/*** Own Libraries ***/
#include "../instance/data.hpp"

/****************************************************************************************/
/*										TYPEDEFS										*/
/****************************************************************************************/

/*** CPLEX ***/	
typedef std::vector<IloNumVar>          IloNumVarVector;
typedef std::vector<IloNumVarVector>    IloNumVarMatrix;
typedef std::vector<IloNumVarMatrix>    IloNumVar3DMatrix;
typedef std::vector<IloNumVar3DMatrix>  IloNumVar4DMatrix;
typedef std::vector<IloNumVar4DMatrix>  IloNumVar5DMatrix;

typedef std::vector<IloNum>             IloNumVector;
typedef std::vector<IloNumVector>       IloNumMatrix;
typedef std::vector<IloNumMatrix>       IloNum3DMatrix;
typedef std::vector<IloNum3DMatrix>     IloNum4DMatrix;
typedef std::vector<IloNum4DMatrix>     IloNum5DMatrix;

typedef IloCplex::Callback::Context     Context;

/****************************************************************************************/
/*										DEFINES			    							*/
/****************************************************************************************/
#define EPS 1e-4 // Tolerance, about float precision
#define EPSILON 1e-6 // Tolerance, about float precision




/************************************************************************************
 * This class implements the generic callback interface. It has two main 
 * functions: addUserCuts and addLazyConstraints.
 ************************************************************************************/
class Callback: public IloCplex::Callback::Function {

    
private:
    /*** General variables ***/
    const IloEnv&   env;    /**< IBM environment **/
    const Data&     data;   /**< Data read in data.hpp **/


    /*** LP data ***/
	const IloNumVar3DMatrix&    x;          /**< VNF assignement variables **/


    /*** Manage execution and control ***/
    std::mutex  thread_flag;                /**< A mutex for synchronizing multi-thread operations. **/
    int         nb_cuts_avail_heuristic;    /**< Number of availability cuts added through heuristic procedure. **/
    int         nbLazyConstraints;          /**< Number of lazy constraints added. **/
    IloNum      timeAll;                    /**< Total time spent on callback. **/


public:

	/****************************************************************************************/
	/*										Constructors									*/
	/****************************************************************************************/
    /** Constructor. Initializes callback variables. **/
	Callback(const IloEnv& env, const Data& data, const IloNumVar3DMatrix& x);


    /****************************************************************************************/
	/*									Auxliary Structs     								*/
	/****************************************************************************************/
    /** Stores the section id and its availability. Used for the separation of integer solutions. **/
    struct MapAvailability { 
        int section;
        double availability; 
    }; 


	/****************************************************************************************/
	/*									Main operations  									*/
	/****************************************************************************************/
    /** CPLEX will call this method during the solution process at the places that we asked for. @param context Defines on which places the method is called and we use it do define what to do in each case.**/
    void            invoke                  (const Context& context);

    /** Solves the separation problems for a given fractional solution. @note Should only be called within relaxation context.**/
	void            addUserCuts             (const Context& context); 
    
    /** Solves the separation problems for a given integer solution. @note Should only be called within candidate context.**/
    void            addLazyConstraints      (const Context& context);
    
    /** Returns the current integer solution. @note Should only be called within candidate context. **/ 
    IloNum3DMatrix  getIntegerSolution      (const Context &context) const;
    
    /** Returns the current fractional solution. @note Should only be called within relaxation context. **/ 
    IloNum3DMatrix  getFractionalSolution   (const Context &context) const;


	/****************************************************************************************/
	/*							Availability Separation Methods  							*/
	/****************************************************************************************/
    /** Greedly solves the separation problem associated with the availability constraints. **/
    void heuristicSeparationOfAvailibilityConstraints(const Context &context, const IloNum3DMatrix& xSol);

    /** Initializes the availability heuristic. **/
    void initiateHeuristic(const int k, std::vector< std::vector<int> >& coeff, std::vector< std::vector<int> >& sectionNodes, std::vector< double >& sectionAvailability, const IloNum3DMatrix& xSol);
	
    /** Computes the availability increment resulted from the instalation of a new vnf. @param CHAIN_AVAIL The chain required availability. @param deltaAvail The matrix to be computed. @param sectionAvail THe current section availabilities. @param coeff The matrix of coefficients storing the possible vnfs to be placed. **/
    void computeDeltaAvailability(const double CHAIN_AVAIL, std::vector< std::vector<double> >& deltaAvail, const std::vector< double >& sectionAvail, const std::vector< std::vector<int> >& coeff);
    
    /** Tries to add new vnf placements to the current solution without changing its availability violation. @param xSol The current solution for a given demand. @param availabilityRequired The SFC required availability. @param sectionAvailability The current section availabilities. @param nbSections The number of sections that can be modified. **/
    void lift(IloNumMatrix& xSol, const double& availabilityRequired, std::vector<MapAvailability>& sectionAvailability, const int& nbSections);
	

    /****************************************************************************************/
	/*							    Integer solution query methods 							*/
	/****************************************************************************************/
    /** Returns the availability of the i-th section of a SFC demand obtained from an integer solution. @param k The demand id. @param i The section id. @param xSol The current integer solution. **/
    double getAvailabilityOfSection (const int& k, const int& i, const IloNum3DMatrix& xSol) const;
    
    /** Returns the availabilities of the sections of a SFC demand obtained from an integer solution. @param k The demand id. @param xSol The current integer solution. **/
    std::vector<MapAvailability> getAvailabilitiesOfSections (const int& k, const IloNum3DMatrix& xSol) const;
    

	/****************************************************************************************/
	/*								      Query Methods	    	    	    				*/
	/****************************************************************************************/
    /** Returns the number of user cuts added so far. **/ 
    const int    getNbUserCuts()           const{ return nb_cuts_avail_heuristic; }

    /** Returns the number of lazy constraints added so far. **/ 
    const int    getNbLazyConstraints()    const{ return nbLazyConstraints; }

    /** Returns the total time spent on callback so far. **/ 
    const IloNum getTime()                 const{ return timeAll; }

    /** Checks if all placement variables of a given SFC demand are integers. @param k The demand id. @param xSol The current solution. **/
    const bool isIntegerAssignment (const int& k, const IloNum3DMatrix& xSol) const;
    

	/****************************************************************************************/
	/*								Thread Protected Methods			    				*/
	/****************************************************************************************/
    /** Increase by one the number of lazy constraints added. **/
    void incrementLazyConstraints();
    /** Increase by one the number of availability cuts added through the heuristic procedure. **/
    void incrementAvailabilityCutsHeuristic();
    /** Increases the total callback time. @param time The time to be added. **/
    void incrementTime(const IloNum time);


	/****************************************************************************************/
	/*										Destructors			    						*/
	/****************************************************************************************/
    /** Destructor **/
    ~Callback() {}

};

/** Checks if the availability of a is lower than the one of b. **/
bool compareAvailability(Callback::MapAvailability a, Callback::MapAvailability b);

#endif