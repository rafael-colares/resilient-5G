#include "model.hpp"

/* Constructor */
Model::Model(const IloEnv& env_, const Data& data_) : 
                env(env_), model(env), cplex(model), data(data_), 
                obj(env), constraints(env)
{

    std::cout << "=> Building model ... " << std::endl;
    setVariables();
    setObjective();  
    setConstraints();  
    setCplexParameters();

    std::cout << "\t Model was correctly built ! " << std::endl;                 
}


/** Set up the Cplex parameters. **/
void Model::setCplexParameters(){
    /** Callback definitions **/
    callback = new Callback(env, data, x);
    CPXLONG contextmask = 0;
	contextmask |= IloCplex::Callback::Context::Id::Candidate;
	contextmask |= IloCplex::Callback::Context::Id::Relaxation;
	cplex.use(callback, contextmask);

    /** Time limit definition **/
    cplex.setParam(IloCplex::Param::TimeLimit, data.getInput().getTimeLimit());    // Execution time limited
	
    //cplex.setParam(IloCplex::Param::Threads, 1); // Treads limited
}
/* Set up variables */
void Model::setVariables(){

    std::cout << "\t Setting up variables... " << std::endl;

    /* VNF placement variables */
    y.resize(lemon::countNodes(data.getGraph()));
    for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
        int v = data.getNodeId(n);
        y[v].resize(data.getNbVnfs());
        for (int f = 0; f < data.getNbVnfs(); f++){
            int vnf = data.getVnf(f).getId();
            std::string name = "y(" + std::to_string(v) + "," + std::to_string(vnf) + ")";
            if (data.getInput().isRelaxation()){
                y[v][f] = IloNumVar(env, 0.0, 1.0, ILOFLOAT, name.c_str());
            }
            else{
                y[v][f] = IloNumVar(env, 0.0, 1.0, ILOINT, name.c_str());
            }
            model.add(y[v][f]);
        }
    }

    /* VNF assignment variables */
    x.resize(data.getNbDemands());
    for (int k = 0; k < data.getNbDemands(); k++){
        x[k].resize(data.getDemand(k).getNbVNFs());
        for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
            x[k][i].resize(lemon::countNodes(data.getGraph()));
            for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                int v = data.getNodeId(n);
                std::string name = "x(" + std::to_string(v) + "," + std::to_string(i) + "," + std::to_string(data.getDemand(k).getId()) + ")";
                if (data.getInput().isRelaxation()){
                    x[k][i][v] = IloNumVar(env, 0.0, 1.0, ILOFLOAT, name.c_str());
                }
                else{
                    x[k][i][v] = IloNumVar(env, 0.0, 1.0, ILOINT, name.c_str());
                }
                model.add(x[k][i][v]);
            }
        }
    }
    
    
    /* TODO: SFC routing variables */

    
}

/* Set up objective function. */
void Model::setObjective(){

    std::cout << "\t Setting up objective function... " << std::endl;

	IloExpr exp(env);
    /*** Objective: minimize VNF placement cost ***/
	for(NodeIt n(data.getGraph()); n != lemon::INVALID; ++n) {
        int v = data.getNodeId(n);
        for (int i = 0; i < data.getNbVnfs(); i++){
            int f = data.getVnf(i).getId();
            double cost = data.getVnf(i).getPlacementCostOnNode(v);
            exp += ( cost*y[v][f] ); 
        }
    }
	obj.setExpr(exp);
	obj.setSense(IloObjective::Minimize);
    model.add(obj);
	exp.clear();
    exp.end();
}

/* Set up constraints. */
void Model::setConstraints(){

    std::cout << "\t Setting up constraints... " << std::endl;

    setVnfAssignmentConstraints();

    //setOriginalVnfPlacementConstraints();
    setVnfPlacementConstraints();

    setNodeCapacityConstraints();
    setStrongNodeCapacityConstraints();


    model.add(constraints);
}

/* Add up the original aggregated VNF placement constraints. */
void Model::setOriginalVnfPlacementConstraints()
{
    for (int f = 0; f < data.getNbVnfs(); f++){
        for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
            int v = data.getNodeId(n);
            IloExpr exp(env);
            for (int k = 0; k < data.getNbDemands(); k++){
                for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
                    int f_ik = data.getDemand(k).getVNF_i(i);
                    if (f_ik == f){
                        exp += x[k][i][v];
                    }
                }
            }
            int bigM = 0;
            for (int k = 0; k < data.getNbDemands(); k++){
                bigM += data.getDemand(k).getNbVNFs();
            }
            exp -= (bigM * y[v][f]);
            std::string name = "Original_VNF_Placement(" + std::to_string(f) + "," + std::to_string(v) + ")";
            constraints.add(IloRange(env, -IloInfinity, exp, 0, name.c_str()));
            exp.clear();
            exp.end();
        }
    }
}
/* Add up the VNF placement constraints: a VNF can only be assigned to a demand if it is already placed. */
void Model::setVnfPlacementConstraints()
{
    for (int k = 0; k < data.getNbDemands(); k++){
        for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
            int f = data.getDemand(k).getVNF_i(i);
            for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                int v = data.getNodeId(n);
                IloExpr exp(env);
                exp += x[k][i][v];
                exp -= y[v][f];
                std::string name = "VNF_Placement(" + std::to_string(k) + "," + std::to_string(i) + "," + std::to_string(v) + ")";
                constraints.add(IloRange(env, -IloInfinity, exp, 0, name.c_str()));
                exp.clear();
                exp.end();
            }
        }
    }
}

/* Add up the VNF assignment constraints: At least one VNF must be assigned to each section of each demand. */
void Model::setVnfAssignmentConstraints(){
    for (int k = 0; k < data.getNbDemands(); k++){
        for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
            IloExpr exp(env);
            for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                int v = data.getNodeId(n);
                exp += x[k][i][v];
            }
            std::string name = "VNF_Assignment(" + std::to_string(k) + "," + std::to_string(i) + ")";
            constraints.add(IloRange(env, 2, exp, IloInfinity, name.c_str()));
            exp.clear();
            exp.end();
        }
    }
}


/* Add up the node capacity constraints: the bandwidth treated in a node must respect its capacity. */
void Model::setNodeCapacityConstraints(){
    for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
        int v = data.getNodeId(n);
        IloExpr exp(env);
        double capacity = data.getNode(v).getCapacity();
        for (int k = 0; k < data.getNbDemands(); k++){
            for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
                int vnf = data.getDemand(k).getVNF_i(i);
                double coeff = data.getDemand(k).getBandwidth() * data.getVnf(vnf).getConsumption();
                exp += (coeff * x[k][i][v]);
            }
        }
        std::string name = "Node_Capacity(" + std::to_string(v) + ")";
        constraints.add(IloRange(env, 0, exp, capacity, name.c_str()));
        exp.clear();
        exp.end();
    }
}

/* Add up the strong node capacity constraints. */
void Model::setStrongNodeCapacityConstraints(){
    for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
        int v = data.getNodeId(n);
        double capacity = data.getNode(v).getCapacity();
        for (int f = 0; f < data.getNbVnfs(); f++){
            IloExpr exp(env);
            for (int k = 0; k < data.getNbDemands(); k++){
                for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
                    int vnf = data.getDemand(k).getVNF_i(i);
                    if (vnf == f){
                        double coeff = data.getDemand(k).getBandwidth() * data.getVnf(vnf).getConsumption();
                        exp += (coeff * x[k][i][v]);
                    }
                }
            }
            exp -= ( capacity * y[v][f]);
            std::string name = "Strong_Node_Capacity(" + std::to_string(v) + "," + std::to_string(f) + ")";
            constraints.add(IloRange(env, -IloInfinity, exp, 0, name.c_str()));
            exp.clear();
            exp.end();
        }
    }
}

void Model::run()
{
    time = cplex.getCplexTime();
	cplex.solve();

	/* Get final execution time */
	time = cplex.getCplexTime() - time;
}

void Model::printResult(){
    
    std::cout << "=> VNF placement solution ..." << std::endl;
    for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
        int v = data.getNodeId(n);
        std::string vnfs;
        for (int f = 0; f < data.getNbVnfs(); f++){
            if (cplex.getValue(y[v][f]) > 1 - EPS){
                vnfs += data.getVnf(f).getName();
                vnfs += ", ";
            }
        }
        if (!vnfs.empty()){
            vnfs.pop_back();
            vnfs.pop_back();
            vnfs += ".";
            std::cout << "\t" << data.getNode(v).getName() << ": " << vnfs << std::endl;
        }
    }

    std::cout << "Objective value: " << cplex.getValue(obj) << std::endl;
    std::cout << "Nodes evaluated: " << cplex.getNnodes() << std::endl;
    std::cout << "User cuts added: " << callback->getNbUserCuts() << std::endl;
    std::cout << "Lazy constraints added: " << callback->getNbLazyConstraints() << std::endl;
    std::cout << "Time on cuts: " << callback->getTime() << std::endl;
    std::cout << "Total time: " << time << std::endl << std::endl;


/*

	std::ofstream fileReport("../Output/results.txt", std::ios_base::app); // File report
    // If file_output can't be opened 
    if(!fileReport)
    {
        std::cerr << "ERROR: Unable to create report file." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Else, write 
    fileReport << data.getInput().getDemandFile() << ";"
    		   << time << ";"
    		   << cplex.getObjValue() << ";"
    		   << cplex.getBestObjValue() << ";"
    		   << cplex.getMIPRelativeGap()*100 << ";"
    		   << cplex.getNnodes()*0.001 << ";"
    		   << cplex.getNnodesLeft()*0.001 << std::endl;
    		   

    // Finalization ***
    fileReport.close();
*/
}


/****************************************************************************************/
/*										Destructors 									*/
/****************************************************************************************/
Model::~Model(){
    delete callback;
}