#ifndef __model__hpp
#define __model__hpp


/****************************************************************************************/
/*										LIBRARIES										*/
/****************************************************************************************/

/*** Own Libraries ***/
#include "callback.hpp"

/****************************************************************************************/
/*										TYPEDEFS										*/
/****************************************************************************************/

/*** CPLEX ***/	
typedef std::vector<IloNumVar>          IloNumVarVector;
typedef std::vector<IloNumVarVector>    IloNumVarMatrix;
typedef std::vector<IloNumVarMatrix>    IloNumVar3DMatrix;
typedef std::vector<IloNumVar3DMatrix>  IloNumVar4DMatrix;
typedef std::vector<IloNumVar4DMatrix>  IloNumVar5DMatrix;

typedef std::vector<IloNum>            IloNumVector;
typedef std::vector<IloNumVector>      IloNumMatrix;
typedef std::vector<IloNumMatrix>      IloNum3DMatrix;
typedef std::vector<IloNum3DMatrix>    IloNum4DMatrix;
typedef std::vector<IloNum4DMatrix>    IloNum5DMatrix;


/********************************************************************************************
 * This class models the MIP formulation and solves it using CPLEX. 											
********************************************************************************************/

class Model
{
	private:
		/*** General variables ***/
		const IloEnv&   env;    /**< IBM environment **/
	 	IloModel        model;  /**< IBM Model **/
		IloCplex        cplex;  /**< IBM Cplex **/
		const Data&     data;   /**< Data read in data.hpp **/

		/*** Formulation specific ***/
		IloNumVarMatrix 	y;              /**< VNF placement variables **/
		IloNumVar3DMatrix 	x;            	/**< VNF assignement variables **/
		IloObjective    	obj;            /**< Objective function **/
		IloRangeArray   	constraints;    /**< Set of constraints **/
		Callback* 			callback; 		/**< User generic callback **/

		/*** Manage execution and control ***/
		IloNum time;

	public:
	/****************************************************************************************/
	/*										Constructors									*/
	/****************************************************************************************/
		/** Constructor. Builds the model (variables, objective function, constraints and further parameters). **/
		Model(const IloEnv& env, const Data& data);
		Model(const IloEnv& env, const Data&&) = delete;
		Model() = delete;

	/****************************************************************************************/
	/*									    Formulation  									*/
	/****************************************************************************************/
        /** Set up the Cplex parameters. **/
        void setCplexParameters();
        /** Set up the variables. **/
        void setVariables();
        /** Set up the objective function. **/
        void setObjective();
        /** Set up the constraints. **/
        void setConstraints();
        /** Add up the VNF assignment constraints: At least one VNF must be assigned to each section of each demand. **/
        void setVnfAssignmentConstraints();
        /** Add up the VNF placement constraints: a VNF can only be assigned to a demand if it is already placed. **/
        void setVnfPlacementConstraints();
        /** Add up the original aggregated VNF placement constraints. **/
        void setOriginalVnfPlacementConstraints();
        /** Add up the node capacity constraints: the bandwidth treated in a node must respect its capacity. **/
        void setNodeCapacityConstraints();
        /** Add up the strong node capacity constraints. **/
        void setStrongNodeCapacityConstraints();
	/****************************************************************************************/
	/*										   Methods  									*/
	/****************************************************************************************/
		/** Solves the MIP. **/
		void run();

		/*** Display the obtained results ***/
		void printResult();

	/****************************************************************************************/
	/*										Destructors 									*/
	/****************************************************************************************/
		/** Destructor. Free dynamic allocated memory. **/
		~Model();
};


#endif // MODELS_HPP