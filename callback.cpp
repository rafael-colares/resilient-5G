#include "callback.hpp"

/****************************************************************************************/
/*										CONSTRUCTOR										*/
/****************************************************************************************/

Callback::Callback(const IloEnv& env_, const Data& data_, const IloNumVar3DMatrix& x_) :
	                env(env_), data(data_),	x(x_)
{	
	/*** Control ***/
    thread_flag.lock();
	nb_cuts_avail_heuristic = 0;
    nbLazyConstraints = 0;
	timeAll = 0.0;
    thread_flag.unlock();
}

/****************************************************************************************/
/*										Operations  									*/
/****************************************************************************************/
void Callback::invoke(const Context& context)
{
    IloNum time = context.getDoubleInfo(IloCplex::Callback::Context::Info::Time);
    switch (context.getId()){
        /* Fractional solution */
        case Context::Id::Relaxation:
            addUserCuts(context);
            break;

        /* Integer solution */
        case Context::Id::Candidate:
			if (context.isCandidatePoint()) {
	    		addLazyConstraints(context);
			}
            break;

        /* Not an option for callback */
		default:
			throw IloCplex::Exception(-1, "ERROR: Unexpected context id !");
    }
    time = context.getDoubleInfo(IloCplex::Callback::Context::Info::Time) - time;
    incrementTime(time);
}

void Callback::addUserCuts(const Context &context)
{
    
    //std::cout << "Entering user cut separation... "  << std::endl;
    try {    
        IloNum3DMatrix xSol = getFractionalSolution(context);
        heuristicSeparationOfAvailibilityConstraints(context, xSol);
    }
    catch (...) {
        throw;
    }
    //std::cout << "Leaving user cut separation... " << std::endl;
}


void Callback::initiateHeuristic(const int k, std::vector< std::vector<int> >& coeff, std::vector< std::vector<int> >& sectionNodes, std::vector< double >& sectionAvailability, const IloNum3DMatrix& xSol)
{
    
    coeff.resize(data.getDemand(k).getNbVNFs());
    sectionNodes.resize(data.getDemand(k).getNbVNFs());
    sectionAvailability.resize(data.getDemand(k).getNbVNFs());
    for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
        coeff[i].resize(xSol[k][i].size());
        for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
            int v = data.getNodeId(n);
            coeff[i][v] = 1;
        }
    }

    /* Initialization of placement */
    for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
        /* Place every integer variable */
        for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
            int v = data.getNodeId(n);
            if (xSol[k][i][v] >= 1 - EPS){
                sectionNodes[i].push_back(v);
                coeff[i][v] = 0;
            }
        }
        /* If still empty, select some initial node based on the best x/availability ratio. */
        if (sectionNodes[i].empty()){
            int selectedNode = -1;
            double bestValue = -1.0;
            for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                int v = data.getNodeId(n);
                if ( (xSol[k][i][v] / data.getNode(v).getAvailability()) > bestValue){
                    bestValue = (xSol[k][i][v] / data.getNode(v).getAvailability());
                    selectedNode = v;
                }
            }
            sectionNodes[i].push_back(selectedNode);
            coeff[i][selectedNode] = 0;
        }
        /* Set initial section availability. */
        sectionAvailability[i] = 1.0 - data.getFailureProb(sectionNodes[i]);
    }
}

/* Greedly solves the separation problem associated with the availability constraints. */
void Callback::heuristicSeparationOfAvailibilityConstraints(const Context &context, const IloNum3DMatrix& xSol)
{
    /* Check VNF placement availability for each demand */
    for (int k = 0; k < data.getNbDemands(); k++){
        
        /* Declare auxiliary structures. */
        std::vector< std::vector<int> > coeff;          // the variable coefficient in the constraint
        std::vector< std::vector<int> > sectionNodes;   // the set of nodes placed in each section
        std::vector< double > sectionAvailability;      // the availability assoaciated with the placement
        
        initiateHeuristic(k, coeff, sectionNodes, sectionAvailability, xSol);

        double chainAvailability = data.getChainAvailability(sectionAvailability);
        const double REQUIRED_AVAIL = data.getDemand(k).getAvailability(); 
        
        if (chainAvailability < REQUIRED_AVAIL){
            std::vector< std::vector<double> > deltaAvailability;
            deltaAvailability.resize(data.getDemand(k).getNbVNFs());
            for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
                deltaAvailability[i].resize(xSol[k][i].size());
            }

            bool STOP = false;
            while (!STOP){
                computeDeltaAvailability(REQUIRED_AVAIL, deltaAvailability, sectionAvailability, coeff);  
                int nextSection = -1;
                int nextNode = -1;
                double bestRatio = -1.0;

                /* Search for next vnf to include on placement without satifying the chain availability. */
                for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
                    for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                        int v = data.getNodeId(n);
                        if ((chainAvailability + deltaAvailability[i][v] < REQUIRED_AVAIL) && ((xSol[k][i][v] + EPS/deltaAvailability[i][v]) > bestRatio)){
                            bestRatio = (xSol[k][i][v]/deltaAvailability[i][v]);
                            nextSection = i;
                            nextNode = v;
                        }
                    }
                }
                /* If a vnf is found, include it. */
                if ((nextSection != -1) && (nextNode != -1)){
                    chainAvailability += deltaAvailability[nextSection][nextNode];
                    sectionAvailability[nextSection] = (1.0 - ((1.0 - sectionAvailability[nextSection])*(1.0 - data.getNode(nextNode).getAvailability())));
                    coeff[nextSection][nextNode] = 0;
                    sectionNodes[nextSection].push_back(nextNode);
                }
                /* If not, stop */
                else{
                    STOP = true;
                }
            }
            
            double lhs = 0.0;
            for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
                for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                    int v = data.getNodeId(n);
                    lhs += (coeff[i][v]*xSol[k][i][v]);
                }
            }

            if (lhs < 1){
                IloExpr expr(env);
                for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
                    for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                        int v = data.getNodeId(n);
                        if (coeff[i][v] == 1){
                            expr += x[k][i][v];
                        }
                    }
                }
                std::string name = "availabilityCut";

                IloRange cut(env, 1, expr, IloInfinity, name.c_str());
                //std::cout << "Adding user cut: " << std::endl;
                context.addUserCut(cut, IloCplex::UseCutFilter, IloFalse);
                expr.end();
                incrementAvailabilityCutsHeuristic();
            }
        }
    }
}

void Callback::computeDeltaAvailability(const double CHAIN_AVAIL, std::vector< std::vector<double> >& deltaAvail, const std::vector< double >& sectionAvail, const std::vector< std::vector<int> >& coeff){
    for (unsigned int i = 0; i < sectionAvail.size(); i++){
        for (unsigned int v = 0; v < coeff[i].size(); v++){
            /* If node is already placed, forbid inclusion */
            if (coeff[i][v] == 0){
                deltaAvail[i][v] = 10.0;
            }
            else{
                double newSectionAvail = (1.0 - ((1.0 - sectionAvail[i])*(1.0 - data.getNode(v).getAvailability())));
                double newChainAvail = (CHAIN_AVAIL / sectionAvail[i])*newSectionAvail;
                deltaAvail[i][v] = newChainAvail - CHAIN_AVAIL;
            }
        }
    }
}

void Callback::addLazyConstraints(const Context &context)
{
    try {
        /* Get current integer solution */
        IloNum3DMatrix xSol = getIntegerSolution(context); 

        /* Check VNF placement availability for each demand */
        for (int k = 0; k < data.getNbDemands(); k++){
            
            /* Compute sections availability and sort them by increasing order */
            std::vector<MapAvailability> sectionAvailability = getAvailabilitiesOfSections(k, xSol);
            std::sort(sectionAvailability.begin(), sectionAvailability.end(), compareAvailability);

            /* Find smallest subset of sections violating the SFC availability. */
            const double REQUIRED_AVAIL = data.getDemand(k).getAvailability(); 
            double chainAvailability = 1.0;
            int index = 0;
            int nbSelectedSections = 0;
            while ((chainAvailability >= REQUIRED_AVAIL) && (index < data.getDemand(k).getNbVNFs())){
                chainAvailability *= sectionAvailability[index].availability;
                nbSelectedSections++;
                index++;
            }
            /* If such subset is found, add lazy constraint. */
            if (chainAvailability < REQUIRED_AVAIL){
                //std::cout << "\t SFC: " << k << ", Availability:" << chainAvailability << ", Requested availability: " << data.getDemand(k).getAvailability() <<  std::endl;
                //std::cout << "Reject candidate solution with " << nbSelectedSections << "sections selected out of " << data.getDemand(k).getNbVNFs() << ".  " << std::endl;
                
                /* Try to lift the separating inequality */
                lift(xSol[k], REQUIRED_AVAIL, sectionAvailability, nbSelectedSections);

                /* Build inequality. */
                IloExpr exp(env);
                for (int s = 0; s < nbSelectedSections; ++s){
                    int i = sectionAvailability[s].section;
                    for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                        int v = data.getNodeId(n);
                        if (xSol[k][i][v] < 1 - EPS){
                            exp += x[k][i][v];
                        }
                    }
                }
                IloRange cut(env, 1.0, exp, IloInfinity);
                //std::cout << "Adding lazy constraint: " << cut << std::endl;
                context.rejectCandidate(cut);
                exp.end();
                incrementLazyConstraints();
            }
        }
    }
    catch (...) {
        throw;
    }
}

void Callback::lift(IloNumMatrix& xSol, const double& availabilityRequired, std::vector<Callback::MapAvailability>& sectionAvailability, const int& nbSections){
    
    //std::cout << "LIFTING:" << std::endl;
    for (int s = 0; s < nbSections; ++s){
        int i = sectionAvailability[s].section;
        for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
            int v = data.getNodeId(n);
            /* If the i-th vnf is not placed on node v */
            if (xSol[i][v] < 1 - EPS){
                /* Compute the availability obtained if a i-th vnf was placed on node v*/
                double futureAvailability = 1.0;
                double futureAvailabilityOfSection = sectionAvailability[s].availability;
                for (int j = 0; j < nbSections; ++j){
                    if (s == j){
                        double newFailureRate = (1.0 - sectionAvailability[j].availability)*(1.0 - data.getNode(v).getAvailability());
                        futureAvailabilityOfSection = (1.0 - newFailureRate);
                        futureAvailability *= futureAvailabilityOfSection;
                    }
                    else{
                        futureAvailability *= sectionAvailability[j].availability;
                    }
                }
                //std::cout << "\t Required:" << availabilityRequired << ", Future: " << futureAvailability << std::endl;
                /* If the availability would still be violated */
                if (futureAvailability < availabilityRequired){
                    /* Place vnf */
                    xSol[i][v] = 1;
                    sectionAvailability[s].availability = futureAvailabilityOfSection;
                    //std::cout << "\t Placing additional vnf..." << std::endl;
                }
            }
        }
    }
}

double Callback::getAvailabilityOfSection(const int& k, const int& i, const IloNum3DMatrix& xSol) const
{
    double failure_prob = 1.0;
    for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
        int v = data.getNodeId(n);
        if (xSol[k][i][v] >= 1 - EPS){
            failure_prob *= (1.0 - data.getNode(v).getAvailability());
        }
    }
    double availability = 1.0 - failure_prob;
    return availability;
}

/* Returns the availabilities of the sections of a SFC demand obtained from an integer solution. */
std::vector<Callback::MapAvailability> Callback::getAvailabilitiesOfSections (const int& k, const IloNum3DMatrix& xSol) const
{   
    std::vector<MapAvailability> sectionAvailability;
    for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
        MapAvailability entry;
        entry.section = i;
        entry.availability = getAvailabilityOfSection(k,i, xSol);
        sectionAvailability.push_back(entry);
    }
    return sectionAvailability;
}

IloNum3DMatrix Callback::getIntegerSolution(const Context &context) const
{
    /* Initialize solution */
    IloNum3DMatrix xSol;
    xSol.resize(data.getNbDemands());
    for (int k = 0; k < data.getNbDemands(); k++){
        xSol[k].resize(data.getDemand(k).getNbVNFs());
        for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
            xSol[k][i].resize(lemon::countNodes(data.getGraph()));
        }
    }

    /* Fill solution matrix */
    if (context.getId() == Context::Id::Candidate){
        if (context.isCandidatePoint()) {
            for (int k = 0; k < data.getNbDemands(); k++){
                for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
                    for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                        int v = data.getNodeId(n);
                        xSol[k][i][v] = context.getCandidatePoint(x[k][i][v]);
                    }
                }
            }
        }
        else{
            throw IloCplex::Exception(-1, "ERROR: Unbounded solution within callback !");
        }
    }
    else{
        throw IloCplex::Exception(-1, "ERROR: Trying to get integer solution while not in candidate context !");
    }

    /* Return solution matrix */
    return xSol;
}

IloNum3DMatrix Callback::getFractionalSolution(const Context &context) const
{
    /* Initialize solution */
    IloNum3DMatrix xSol;
    xSol.resize(data.getNbDemands());
    for (int k = 0; k < data.getNbDemands(); k++){
        xSol[k].resize(data.getDemand(k).getNbVNFs());
        for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
            xSol[k][i].resize(lemon::countNodes(data.getGraph()));
        }
    }

    /* Fill solution matrix */
    if (context.getId() == Context::Id::Relaxation){
        for (int k = 0; k < data.getNbDemands(); k++){
            for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
                for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
                    int v = data.getNodeId(n);
                    xSol[k][i][v] = context.getRelaxationPoint(x[k][i][v]);
                }
            }
        }
    }
    else{
       throw IloCplex::Exception(-1, "ERROR: Trying to get fractional solution while not in relaxation context !");
    }
    
    /* Return solution matrix */
    return xSol;
}


const bool Callback::isIntegerAssignment(const int& k, const IloNum3DMatrix& xSol) const{
    for (int i = 0; i < data.getDemand(k).getNbVNFs(); i++){
        for (NodeIt n(data.getGraph()); n != lemon::INVALID; ++n){
            int v = data.getNodeId(n);
            if ((xSol[k][i][v] >= EPS)  && (xSol[k][i][v] <= 1 - EPS)){
                return false;
            }
        }
    }
    return true;
}


void Callback::incrementLazyConstraints()
{
    thread_flag.lock();
    ++nbLazyConstraints;
    //std::cout << "Nb lazy constraints: " << nbLazyConstraints << std::endl;
    thread_flag.unlock();
}

void Callback::incrementAvailabilityCutsHeuristic()
{
    thread_flag.lock();
    ++nb_cuts_avail_heuristic;
    //std::cout << "Nb user cuts: " << nb_cuts_avail_heuristic << std::endl;
    thread_flag.unlock();
}

void Callback::incrementTime(const IloNum time)
{
    thread_flag.lock();
    timeAll += time;
    //std::cout << "Nb lazy constraints: " << nbLazyConstraints << std::endl;
    thread_flag.unlock();
}

bool compareAvailability(Callback::MapAvailability a, Callback::MapAvailability b)
{
    return (a.availability < b.availability);
}