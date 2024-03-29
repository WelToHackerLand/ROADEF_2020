#ifndef ACO_SOLUTION_VER2
#define ACO_SOLUTION_VER2

#include "../utils.cpp"
#include "../Problem_ver2.cpp"
#include "NLS_object_ver2.cpp"
#include "NLS_local_search.cpp"
#include "NLS_repair_procedure.cpp"
#include "NLS_destroy_procedure.cpp"

namespace ACO_solution {
    int nAnts, iLimit, R_per, C_per, limit_percent_chosen, numKeep;
    double Rho, Phe_max, Phe_min;

    void Assign_parameter(Problem_Instance &instance) {
        iLimit = 10000;
        limit_percent_chosen = 95;
        numKeep = 30;

        nAnts = instance.numInterventions;
        R_per = 70;
        C_per = 70;
        Rho = 0.95;
        Phe_max = 1.0;
        Phe_min = Phe_max / (instance.T * instance.numInterventions);
    }

    vector<vector<double> > prepare_obj1_cost(Problem_Instance &instance) {
        vector<vector<double> > obj1_cost;
        obj1_cost.resize( instance.numInterventions+1 );
        for (int i = 1; i <= instance.numInterventions; ++i) obj1_cost[i].resize(instance.tmax[i]+1, 0.0);

        for (int i = 1; i <= instance.numInterventions; ++i) 
        for (int start_Time = 1; start_Time <= instance.tmax[i]; ++start_Time) {
            if ( i + instance.delta[i][start_Time] > instance.T+1 ) continue;
 
            for (auto foo : instance.risk_list[i][start_Time]) {
                int s = foo.scenario, t = foo.time;
                double val = foo.cost;
                obj1_cost[i][start_Time] += val * (double) 1 / (double) instance.T / (double) instance.numScenarios[t];
            }
        }
        return obj1_cost;
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

    bool ACO_recreate(Problem_Instance &instance, NLS_object &obj, vector<int> Erase_list, 
        vector<vector<double> > &phe, double alpha, double beta, vector<vector<double> > &obj1_cost) {

        vector<vector<int> > canList;
        canList.resize( instance.numInterventions+1 );

        for (int i : Erase_list) {
            for (int start_Time = 1; start_Time <= instance.tmax[i]; ++start_Time) {
                if ( start_Time + instance.delta[i][start_Time] > instance.T+1 ) continue;
                if ( !obj.exclusionChecking(instance, i, start_Time) ) continue;

                bool ok = true;
                for (auto foo : instance.r_list[i][start_Time]) {
                    int c = foo.resource, t = foo.time;
                    double val = foo.cost;
                    if ( obj.r_ct[c][t] + val > instance.u[c][t] ) { ok = false; break; }
                }             
                if (ok) canList[i].push_back(start_Time);
            }
        }

        while ( Erase_list.size() ) {
                // for (int i : Erase_list) {
                //     cerr << i << "(" << canList[i].size() << "); "; 
                // }
                // cerr << '\n';

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
                return false;
            }

            priority_queue<pair<double, int> > heap;
            for (int t : canList[i]) {
                double total_score = obj1_cost[i][t];// * phe[i][t];
                heap.push( make_pair(total_score, t) );
                while ( (int) heap.size() > numKeep ) heap.pop();
            }   

            vector<pair<double, int> > V;
            double ORE_TOTAL = 0; 
            while ( heap.size() ) {
                int t = heap.top().second; heap.pop();

                int numAcceptedLB = 0, numViolatedUB = 0;
                double costLB = 0, costUB = 0;
                double oreLB = obj.LBResources_cost, oreUB = obj.UBResources_cost;
                obj.Insert_no_care_UB(instance, i, t, numAcceptedLB, numViolatedUB, costLB, costUB);

                double obj_score = obj.get_OBJ(instance);
                double LB_score = 0; //costLB * alpha;
                double UB_score = 0; //costUB * beta;
                double total_score = ( 1 / (obj_score - LB_score + UB_score) + 1 ) * phe[i][t];

                obj.Erase_no_care_UB(instance, i, t, numAcceptedLB, numViolatedUB);
                V.push_back( make_pair(total_score, t) );
                ORE_TOTAL += total_score;
            }

            double num = ORE_TOTAL * (double) ( rand() % 101 ) / 100.0;  
            int best_Time = V.back().second;
            for (int id = 0; id < (int) V.size(); ++id) { 
                num -= V[id].first;
                if ( num < 1e-6 ) { best_Time = V[id].second; break; }
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
                for (int start_Time : canList[i]) {
                    if ( start_Time + instance.delta[i][start_Time] > instance.T+1 ) continue;
                    if ( !obj.exclusionChecking(instance, i, start_Time) ) continue;

                    /// check resource constraint
                    bool ok = true;
                    for (auto foo : instance.r_list[i][start_Time]) {
                        int c = foo.resource, t = foo.time;
                        double val = foo.cost;
                        if ( obj.r_ct[c][t] + val > instance.u[c][t] ) { ok = false; break; }
                    }             
                    if (ok) next_gerenation.push_back(start_Time);
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

        /// create obj1_cost
        vector<vector<double> > obj1_cost = prepare_obj1_cost(instance);

        /// create random solution 
        NLS_object gBest; 
        bool flag_gBest = false;

        /// main algorithm
        clock_t startTime = clock();
        for (int loop = 1; loop <= iLimit; ++loop) {
            if ((double)(clock() - startTime) / CLOCKS_PER_SEC > timeLimit) break;

            cerr << "LOOP: #" << loop;
            if ( flag_gBest ) cerr << " --> " << gBest.get_OBJ(instance) << '\n';
            else cerr << " --> " << -1 << '\n';

            NLS_object iBest;
            bool flag_iBest = false;
            for (int ant = 1; ant <= nAnts; ++ant) {
                /// ruin
                vector<int> Erase_list;
                NLS_object obj;
                if (!flag_gBest) {  
                    for (int i = 1; i <= instance.numInterventions; ++i) Erase_list.push_back(i);
                    obj.Initialize(instance);
                }
                else {
                    Erase_list = ACO_ruin( instance, gBest, phe );
                    obj = gBest;
                    for (int i : Erase_list) {
                        int t = obj.Time_Start_Intervention[i], nAC = 0, nVL = 0;
                        obj.Erase_no_care_UB(instance, i, t, nAC, nVL);
                    }
                }

                /// recreate
                bool ok = ACO_recreate( instance, obj, Erase_list, phe, alpha, beta, obj1_cost );
                if (!ok) continue;

                //    cerr << "ANT: " << ant << " " << obj.get_OBJ(instance) << '\n';

                // cerr << "PRE_LS: " << obj.get_OBJ(instance) << '\n';
                // while ( NLS_local_search::Change_time_start_interventions_best(instance, obj, alpha, beta, 0.05) ) {
                //     cerr << "local search: " << obj.get_OBJ(instance) << '\n';
                // }
                // cerr << "\n\n\n";

                /// update iBest
                if ( !flag_iBest ) { iBest = obj; flag_iBest = true; }
                double iBest_score = iBest.getScore(instance, alpha, beta);
                double obj_score = obj.getScore(instance, alpha, beta);
                if ( iBest_score > obj_score + 1e-6 ) iBest = obj;
            }

                cerr << "IBEST = " << iBest.get_OBJ(instance) 
                    << " " << "LB_cost = " << iBest.LBResources_cost << " " 
                    << " " << "UB_cost = " << iBest.UBResources_cost << " "
                    << "found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';

            /// update gBest
            if ( !flag_iBest ) continue;
            while ( NLS_local_search::Change_time_start_interventions_best(instance, iBest, alpha, beta, 1) ) {
                cerr << "local search: " << iBest.get_OBJ(instance) << '\n';
            }

            if ( !flag_gBest ) { gBest = iBest; flag_gBest = true; }

            double iBest_score = iBest.getScore(instance, alpha, beta);
            double gBest_score = gBest.getScore(instance, alpha, beta);

            if (iBest.LBResources_cost < 1e-5 && iBest.UBResources_cost < 1e-5) {
                // assert( abs(iBest_score - iBest.get_OBJ(instance)) < 1e-5 );
                if ( abs(iBest_score - iBest.get_OBJ(instance)) > 1e-5 ) {
                    cerr << iBest_score << " " << iBest.get_OBJ(instance) << '\n';
                    cerr << abs(iBest_score - iBest.get_OBJ(instance)) << '\n';
                }
            }

            if ( gBest_score > iBest_score + 1e-6 ) {
                gBest = iBest;
                cerr << "new best solution = " << gBest.get_OBJ(instance) 
                    << " " << "LB_cost = " << gBest.LBResources_cost << " " 
                    << " " << "UB_cost = " << gBest.UBResources_cost << " "
                    << "found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
            }

            /// update phe
            if ( flag_iBest ) {
                for (int i = 1; i <= instance.numInterventions; ++i) {
                    int time_start = iBest.Time_Start_Intervention[i];
                    for (int t = 1; t <= instance.tmax[i]; ++t) 
                        if (t == time_start) phe[i][t] = phe[i][t] * Rho + Phe_max * (1-Rho);
                        else phe[i][t] = phe[i][t] * Rho + Phe_min * (1-Rho);
                }
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

#endif // ACO_SOLUTION_VER2