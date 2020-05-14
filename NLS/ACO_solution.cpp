#ifndef ACO_SOLUTION
#define ACO_SOLUTION

#include "../utils.cpp"
#include "../Problem.cpp"
#include "NLS_object.cpp"
#include "NLS_local_search.cpp"
#include "NLS_repair_procedure.cpp"
#include "NLS_destroy_procedure.cpp"

namespace ACO_solution {
    int nAnts, iLimit, R_per, C_per;
    double Rho, Phe_max, Phe_min;

    void Assign_parameter(Problem_Instance &instance) {
        iLimit = 10000;

        nAnts = instance.numInterventions;
        R_per = 50;
        C_per = 60;
        Rho = 0.9;
        Phe_max = 1.0;
        Phe_min = Phe_max / (5 * instance.numInterventions);
    }

    void random_solution(Problem_Instance &instance, NLS_object &obj, double alpha, double beta) {
        obj.Initialize(instance);

        while (true) {
            vector<int> V;
            for (int i = 1; i <= instance.numInterventions; ++i) V.push_back(i);
            instance.Random_Priority_Descent_Delta_Sort(V);

            NLS_repair_procedure::process_version_2(instance, V, obj, alpha, beta);
            if (obj.numFailedIntervention == 0) break;
        }
    }

    vector<int> ACO_ruin(Problem_Instance &instance, NLS_object &obj, vector<vector<double> > &phe) {
        vector<pair<double, int> > V;
        vector<int> Erase_list;
        for (int i = 1; i <= instance.numInterventions; ++i) {
            int keep_percent = rand() % 101;
            if ( (double) keep_percent <= (double) R_per ) {
                int t = obj.Time_Start_Intervention[i];
                V.push_back( make_pair(phe[i][t], i) );
            }
            else Erase_list.push_back(i);
        }

        sort(V.begin(),V.end());
        reverse(V.begin(), V.end());

        int numKeep = (int) V.size() * C_per / 100;
        while ( (int) V.size() > numKeep ) {
            Erase_list.push_back( V.back().second );
            V.pop_back();
        }
        return Erase_list;
    }

    bool ACO_recreate(Problem_Instance &instance, NLS_object &obj, vector<int> Erase_list, double alpha, double beta) {
        vector<vector<int> > canList;
        canList.resize( instance.numInterventions+1 );

        for (int i : Erase_list) {
            for (int t = 1; t <= instance.tmax[i]; ++t) {
                if ( t + instance.delta[i][t] > instance.T+1 ) continue;
                if ( !obj.exclusionChecking(instance, i, t) ) continue;
            
                int nAC = 0, nVL = 0;
                double costLB = 0, costUB = 0;
                obj.Insert_no_care_UB(instance, i, t, nAC, nVL, costLB, costUB);

                if ( costUB < 1e-5 ) canList[i].push_back(t);
                obj.Erase_no_care_UB(instance, i, t, nAC, nVL);
            }
        }

        while ( Erase_list.size() ) {
            /// get intervention i with minimum size of canList
            vector<int> candidate;
            int min_size = (int) 1e9+7;
            for (int i : Erase_list) {
                if ( canList[i].size() < min_size ) {
                    candidate.clear();
                    candidate.push_back(i);
                    min_size = canList[i].size();
                }
                else if ( canList[i].size() == min_size ) candidate.push_back(i);
            }
            assert( min_size < (int) 1e9 );

            random_shuffle(candidate.begin(), candidate.end());
            int i = candidate[0];

            /// there is no time t suit for intervention i
            if ( canList[i].empty() ) {
                for (int t = 1; t <= instance.tmax[i]; ++t) {
                    if ( t + instance.delta[i][t] > instance.T+1 ) continue;
                    if ( !obj.exclusionChecking(instance, i, t) ) continue;
                    canList[i].push_back(t);
                }
                if ( canList[i].empty() ) {
                    cerr << "EXC TURN FOUND !!!\n";
                    return false;
                }
            }

            /// get best time for intervention i
            int best_Time = -1;
            double best_score = -1;
            for (int t : canList[i]) {
                int numAcceptedLB = 0, numViolatedUB = 0;
                double costLB = 0, costUB = 0;
                double oreLB = obj.LBResources_cost, oreUB = obj.UBResources_cost;
                obj.Insert_no_care_UB(instance, i, t, numAcceptedLB, numViolatedUB, costLB, costUB);

                double obj_score = obj.get_OBJ(instance);
                double LB_score = costLB * alpha;
                double UB_score = costUB * beta;
                double total_score = obj_score - LB_score + UB_score;

                if ( best_Time == -1 || best_score > total_score + 1e-6 ) {
                    best_score = total_score;
                    best_Time = t;
                }

                obj.Erase_no_care_UB(instance, i, t, numAcceptedLB, numViolatedUB);
            }   
            assert(best_Time != -1);

            int nAC = 0, nVL = 0;
            double costLB = 0, costUB = 0;
            obj.Insert_no_care_UB(instance, i, best_Time, nAC, nVL, costLB, costUB);

            /// erase i in Erase_list
            for (int id = 0; id < (int) Erase_list.size(); ++id) 
                if ( Erase_list[id] == i ) {
                    int sz = (int) Erase_list.size();
                    swap( Erase_list[id], Erase_list[sz-1] );
                    Erase_list.pop_back();
                    break;
                }

            /// update canList
            for (int i : Erase_list) {
                vector<int> next_gerenation;
                for (int t : canList[i]) {
                    if ( t + instance.delta[i][t] > instance.T+1 ) continue;
                    if ( !obj.exclusionChecking(instance, i, t) ) continue;
                
                    int nAC = 0, nVL = 0;
                    double costLB = 0, costUB = 0;
                    obj.Insert_no_care_UB(instance, i, t, nAC, nVL, costLB, costUB);
                    bool ok = obj.Insert(instance, i, t, nAC);
                    if ( ok ) next_gerenation.push_back(t);
                    obj.Erase_no_care_UB(instance, i, t, nAC, nVL);
                }
                canList[i] = next_gerenation;
            }
        }
        return true;
    }

    void process(Problem_Instance &instance, string outputFile, double timeLimit, double alpha, double beta) {
        cerr << "******* ACO VERSION: accept solution which violated upper_bound constraint *******\n";
	    cerr << "----------------ACO is working--------------------\n";

        /// Assign parameter
        Assign_parameter(instance);

        /// create phe array
        vector<vector<double> > phe;
        phe.resize( instance.numInterventions+1 );
        for (int i = 1; i <= instance.numInterventions; ++i) phe[i].resize( instance.tmax[i]+1, Phe_max );

        /// create random solution 
        NLS_object gBest; 
        random_solution(instance, gBest, alpha, beta);

        /// main algorithm
        clock_t startTime = clock();
        for (int loop = 1; loop <= iLimit; ++loop) {
            if ((double)(clock() - startTime) / CLOCKS_PER_SEC > timeLimit) break;

            cerr << "LOOP: #" << loop << '\n';

            NLS_object iBest;
            bool flag = false;
            for (int ant = 1; ant <= nAnts; ++ant) {
                /// ruin
                vector<int> Erase_list = ACO_ruin( instance, gBest, phe );
                NLS_object obj = gBest;
                for (int i : Erase_list) {
                    int t = obj.Time_Start_Intervention[i], nAC = 0, nVL = 0;
                    obj.Erase_no_care_UB(instance, i, t, nAC, nVL);
                }

                /// recreate
                bool ok = ACO_recreate( instance, obj, Erase_list, alpha, beta );
                if (!ok) continue;

                /// update iBest
                if ( !flag ) { iBest = obj; flag = true; }
                double iBest_score = iBest.getScore(instance, alpha, beta);
                double obj_score = obj.getScore(instance, alpha, beta);
                if ( iBest_score > obj_score + 1e-6 ) iBest = obj;
            }

            /// update gBest
            if (!flag) continue;
            double iBest_score = iBest.getScore(instance, alpha, beta);
            double gBest_score = gBest.getScore(instance, alpha, beta);
            if ( gBest_score > iBest_score ) {
                gBest = iBest;
                cerr << "new best solution = " << gBest.get_OBJ(instance) 
                    << " " << "LB_cost = " << gBest.LBResources_cost << " " 
                    << " " << "UB_cost = " << gBest.UBResources_cost << " "
                    << "found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
            }

            /// update phe
            for (int i = 1; i <= instance.numInterventions; ++i) {
                int time_start = iBest.Time_Start_Intervention[i];
                for (int t = 1; t <= instance.tmax[i]; ++t) 
                    if (t == time_start) phe[i][t] = phe[i][t] * Rho + Phe_max * (1-Rho);
                    else phe[i][t] = phe[i][t] * Rho + Phe_min * (1-Rho);
            }
        }

        cerr << "ACO FINISH TIME = " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
        cerr << "----------------NLS is done--------------------\n";

            // NLS_print_object::process(instance, best_obj, outputFile);
            if ( gBest.numFailedIntervention > 0 ) {
                cout << "SOME INTERVENTIONS ARE NOT SCHEDULED !!!\n";
                exit(0);
            }

            ofstream out(outputFile);
            for (int i = 1; i <= instance.numInterventions; ++i) {
                out << instance.Intervention_name[i] << " " << gBest.Time_Start_Intervention[i] << '\n';
            }

            cerr << "OBJ1 = " << gBest.obj1 << '\n';
            cerr << "OBJ2 = " << gBest.obj2 << '\n';
            cerr << "OBJ = " << gBest.get_OBJ(instance) << '\n';
            // LAG O? DA^Y QUA'
    }
}

#endif // ACO_SOLUTION