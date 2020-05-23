// all_LS_ver2 -> all ants
// dp_with_k_exchange -> iBest
// more dp_with_k_exchange -> gBest

#ifndef ACO_SOLUTION_VER2
#define ACO_SOLUTION_VER2

#include "../utils.cpp"
#include "../Problem_ver2.cpp"
#include "NLS_object_ver2.cpp"
#include "NLS_local_search.cpp"

namespace ACO_solution {
    int nAnts, iLimit, R_per, C_per, addRan, limit_percent_chosen, numKeep;
    double Rho, Phe_max, Phe_min;

    void Assign_parameter(Problem_Instance &instance) {
        iLimit = 10000;
        numKeep = 20;

        // nAnts = 2;

        nAnts = 20;
        R_per = 85;
        C_per = 85;
        addRan = 10;
        Rho = 0.5;
        Phe_max = 1.0;
        Phe_min = Phe_max / (5 * instance.T);

        cerr<<"nAnts						"<< nAnts <<"\n";
        cerr<<"R_per						"<< R_per <<"\n";
        cerr<<"C_per						"<< C_per <<"\n";
        cerr<<"rho   						"<< Rho <<"\n";
        cerr<<"numKeep						"<< numKeep <<"\n";
        cerr<<"Phe_max						"<< Phe_max <<"\n";
        cerr<<"Phe_min						"<< Phe_min <<"\n";

        cerr<<"instance.T					"<< instance.T <<"\n";
        cerr<<"instance.numInterventions		"<< instance.numInterventions <<"\n";
        cerr<<"instance.numResources 			"<< instance.numResources <<"\n\n";
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


    vector<int> ACO_ruin(Problem_Instance &instance, NLS_object &obj, vector<vector<double> > &phe) {
        vector<pair<double, int> > V;
        vector<int> Erase_list;
        //int addRan = rand() % 20;
        cerr << "with " << (R_per + addRan) * (C_per + addRan) / 100 << "% ";

        for (int i = 1; i <= instance.numInterventions; ++i) {
            int keep_percent = rand() % 101;
            if ( (double) keep_percent <= (double) R_per + addRan) {
                int t = obj.Time_Start_Intervention[i];
                V.push_back( make_pair(phe[i][t], i) );
            }
            else Erase_list.push_back(i);
        }

        sort(V.begin(),V.end());
        reverse(V.begin(), V.end());

        int numKeepInt = (int) V.size() * (C_per + addRan) / 100;
        while ( (int) V.size() > numKeepInt ) {
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
            for (int i : Erase_list) 
                if ( canList[i].size() < min_size ) 
                    min_size = canList[i].size();


            for (int i : Erase_list) 
                if ( canList[i].size() - min_size <= 3 )              
                    candidate.push_back(i); 

           	//cerr<<min_size << " " << candidate.size()<<"\n";

            assert( min_size < (int) 1e9 );

            random_shuffle(candidate.begin(), candidate.end());
            int i = candidate[0];

            /// there is no time t suit for intervention i
            if ( canList[i].empty() ) {
                return false;
                // for (int t = 1; t <= instance.tmax[i]; ++t) {
                //     if ( t + instance.delta[i][t] > instance.T+1 ) continue;
                //     if ( !obj.exclusionChecking(instance, i, t) ) continue;
                //     canList[i].push_back(t);
                // }
                // if ( canList[i].empty() ) {
                //     cerr << "EXC TURN FOUND !!!\n";
                //     return false;
                // }
            }

            priority_queue<pair<double, int> > heap;
            for (int t : canList[i]) {
                double total_score = obj1_cost[i][t] * (1.0 / phe[i][t]);
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
                //double total_score = 1 / (obj_score - LB_score + UB_score);

                obj.Erase_no_care_UB(instance, i, t, numAcceptedLB, numViolatedUB);
                V.push_back( make_pair(total_score, t) );
                ORE_TOTAL += total_score;
            }

            random_shuffle ( V.begin(), V.end() );

            double num = ORE_TOTAL * (double) ( rand() % 1000001 ) / 1000000.0;  
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

        ofstream log(outputFile + ".log");
        log<<"nAnts						"<< nAnts <<"\n";
        log<<"R_per						"<< R_per <<"\n";
        log<<"C_per						"<< C_per <<"\n";
        log<<"rho   						"<< Rho <<"\n";
        log<<"numKeep						"<< numKeep <<"\n";
        log<<"Phe_max						"<< Phe_max <<"\n";
        log<<"Phe_min						"<< Phe_min <<"\n";

        /// create phe array
        vector<vector<double> > phe;
        phe.resize( instance.numInterventions+1 );
        for (int i = 1; i <= instance.numInterventions; ++i) phe[i].resize( instance.tmax[i]+1, Phe_max );

        /// create obj1_cost
        vector<vector<double> > obj1_cost = prepare_obj1_cost(instance);

        /// create random solution 
        NLS_object gBest, uBest; 
        bool flag_gBest = false;
        int flag_uBest = 0;

        /// main algorithm
        clock_t startTime = clock();

        int lastLoop = 0;
        for (int loop = 1; loop <= iLimit; ++loop) {
            if ((double)(clock() - startTime) / CLOCKS_PER_SEC > timeLimit) break;


            NLS_object iBest;
            bool flag_iBest = false;
            for (int ant = 1; ant <= nAnts; ++ant) {
            	cerr << "\n\nant " << ant <<" ";
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

                cerr << " -> " << obj.get_OBJ(instance) << '\n';
                int cnt = 0;
                bool flag_ls = true;
                while ( (cnt < 20) && NLS_local_search::all_LS_ver2_for_each_ant(instance, obj, alpha, beta, 0.1)) {
                        // break;
                	cnt ++;
                	cerr << obj.get_OBJ(instance) << " - ";
                }

                //cnt = 0;
                //while ( (cnt < 20) && NLS_local_search::Change_time_start_interventions_best(instance, obj, alpha, beta, 0.1) ) { cnt ++;}
                //     cerr << "local search: " << obj.get_OBJ(instance) << '\n';
                
                // cerr << "\n\n\n";

                /// update iBest
                if ( !flag_iBest ) { iBest = obj; flag_iBest = true; }
                double iBest_score = iBest.getScore(instance, alpha, beta);
                double obj_score = obj.getScore(instance, alpha, beta);
                if ( iBest_score > obj_score + 1e-6 ) iBest = obj;
            }

            /// update gBest
            if ( !flag_iBest ) continue;
            cerr << "\n#" << loop << " --> " << iBest.get_OBJ(instance) << "\n";
            log << "\n#" << loop << " --> " << iBest.get_OBJ(instance) << "\n";

            cerr << "local search ... ";
            log << "local search ... ";
            int cnt = 0;
            bool flag_ls = true;
            int rand_ls;

            while ( (cnt < 20) && flag_ls ) {
                cnt++;
            	rand_ls = rand() % 100;
            	    
        	    if (rand_ls < 25) {
                        // continue;
					flag_ls = NLS_local_search::dp_with_k_exchange_ver3(instance, iBest, alpha, beta, 2, 7, 0.0001);
					cerr<<"(2, 7, ";
					log<<"(2, 7, ";
        	    }
				else if (rand_ls < 50) {
                        // continue;
					flag_ls = NLS_local_search::dp_with_k_exchange_ver3(instance, iBest, alpha, beta, 3, 3, 0.0001);
					cerr<<"(3, 3, ";
					log<<"(3, 3, ";
				}
				else if (rand_ls < 75) {
                        // continue;
					flag_ls = NLS_local_search::dp_with_k_exchange_ver3(instance, iBest, alpha, beta, 4, 2, 0.0001);
					cerr<<"(4, 2, ";
					log<<"(4, 2, ";
        	    }
				else {
                        // continue;
					flag_ls = NLS_local_search::dp_with_k_exchange_ver3(instance, iBest, alpha, beta, 8, 1, 0.0001);
					cerr<<"(8, 1, ";
					log<<"(8, 1, ";
				}

                cerr << iBest.get_OBJ(instance) <<") - ";
                log << iBest.get_OBJ(instance) <<") - ";
            }
            cerr << "\n";
            log << "\n";

                assert( iBest.numFailedIntervention == 0 );
                assert( iBest.LBResources_cost < 1e-6 );
                assert( iBest.UBResources_cost < 1e-6 );

            if ( flag_uBest == 0 ) 
            	{ uBest = iBest; flag_uBest = 1;}
            else {
            	flag_uBest++;
            	if (uBest.getScore(instance, alpha, beta) > iBest.getScore(instance, alpha, beta) + 1e-6)
            		uBest = iBest;
            }

            if ( !flag_gBest ) { gBest = iBest; flag_gBest = true; flag_uBest = 0;}

            if ( (flag_uBest > 5) || (gBest.getScore(instance, alpha, beta) > uBest.getScore(instance, alpha, beta) + 1e-6) ) {

            	cerr << "\napply more local search for uBest ("<<flag_uBest<<")...";
            	log << "\napply more local search for uBest ("<<flag_uBest<<")...";
            	flag_uBest = 0;
            	flag_ls = true;
           
            	while (flag_ls) {

            		flag_ls = NLS_local_search::opt2_version2(instance, uBest, alpha, beta, 30, 5, 0.0001);
					if (flag_ls) cerr<<"(b30, 10, " << uBest.get_OBJ(instance) <<") - ";;
					if (flag_ls) log<<"(b30, 10, " << uBest.get_OBJ(instance) <<") - ";;

	            	rand_ls = rand() % 100;
	            	    
	        	    if (rand_ls < 25) {
                            // continue;
						if (NLS_local_search::dp_with_k_exchange_ver3(instance, uBest, alpha, beta, 2, 25, 0.0001)) {
							flag_ls = true;
							cerr<<"(2, 25, " << uBest.get_OBJ(instance) <<") - ";
							log<<"(2, 25, " << uBest.get_OBJ(instance) <<") - ";
						}
	        	    }
					else if (rand_ls < 25) {
                            // continue;
						if (NLS_local_search::dp_with_k_exchange_ver3(instance, uBest, alpha, beta, 3, 7, 0.0001)) {
							flag_ls = true;
							cerr<<"(3, 7, " << uBest.get_OBJ(instance) <<") - ";
							log<<"(3, 7, " << uBest.get_OBJ(instance) <<") - ";
						}
					}
					else if (rand_ls < 25) {
                            // continue;
						if (NLS_local_search::dp_with_k_exchange_ver3(instance, uBest, alpha, beta, 4, 3, 0.0001)) {
							flag_ls = true;
							cerr<<"(4, 3, " << uBest.get_OBJ(instance) <<") - ";
							log<<"(4, 3, " << uBest.get_OBJ(instance) <<") - ";
						}
					}
					else {
                            // continue;
						if (NLS_local_search::dp_with_k_exchange_ver3(instance, uBest, alpha, beta, 11, 1, 0.0001)) {
							flag_ls = true;
							cerr<<"(11, 1, " << uBest.get_OBJ(instance) <<") - ";
							log<<"(11, 1, " << uBest.get_OBJ(instance) <<") - ";
						}
					}

                        // cerr << "#dm\n";
            	}

            	cerr << "\n";
            	log << "\n";
	        }

            if ( gBest.getScore(instance, alpha, beta) > uBest.getScore(instance, alpha, beta) + 1e-6 ) {
               
                flag_uBest = 0;
                gBest = uBest;
                iBest = uBest;
                addRan = 10;

                //while ( NLS_local_search::dp_Change_time_interventions_more_complex(instance, gBest, alpha, beta, 0.0001 )) {
                //    cerr << gBest.get_OBJ(instance) <<" ";
                //}

                cerr << "\nnew best solution = " << gBest.get_OBJ(instance) 
                    //<< " " << "LB_cost = " << gBest.LBResources_cost << " " 
                    //<< " " << "UB_cost = " << gBest.UBResources_cost << " "
                    << " found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';

                log << "\nnew best solution = " << gBest.get_OBJ(instance) 
                    << " found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
                
            	for (int i = 1; i <= instance.numInterventions; ++i) {
                	log << instance.Intervention_name[i] << " " << gBest.Time_Start_Intervention[i] << '\n';
            	}

            }
            else if (addRan > 0)
            	addRan = addRan - 1;

            cerr<< "time: " << (double)(clock() - startTime) / CLOCKS_PER_SEC;
            if ( flag_gBest ) cerr << " ----------------------------------------> " << gBest.get_OBJ(instance) << '\n';
            else cerr << " --> " << -1 << '\n';

            /// update phe
            if ( flag_iBest ) {
            	
                for (int i = 1; i <= instance.numInterventions; ++i) {
                    int time_start = iBest.Time_Start_Intervention[i];
                    
                    for (int t = 1; t <= instance.tmax[i]; ++t) {
                        if (t == time_start) phe[i][t] = phe[i][t] * Rho + Phe_max * (1-Rho);
                        else phe[i][t] = phe[i][t] * Rho + Phe_min * (1-Rho);
                        
                    }
                    
                }

                if (loop - lastLoop > 30) {
                	lastLoop = loop;
                	cerr <<"reset pheromone...\n";
                	addRan = 10;
                	for (int i = 1; i <= instance.numInterventions; ++i) 
                    	for (int t = 1; t <= instance.tmax[i]; ++t) 
                    		phe[i][t] = Phe_max;

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

            cerr <<setprecision(7) << fixed << "OBJ1 = " << gBest.obj1 << '\n';
            cerr <<setprecision(7) << fixed << "OBJ2 = " << gBest.obj2 << '\n';
            cerr <<setprecision(7) << fixed << "OBJ = " << gBest.get_OBJ(instance) << '\n';


            for (int i = 1; i <= instance.numInterventions; ++i) {
                log << instance.Intervention_name[i] << " " << gBest.Time_Start_Intervention[i] << '\n';
            }

            log <<setprecision(7) << fixed << "OBJ1 = " << gBest.obj1 << '\n';
            log <<setprecision(7) << fixed << "OBJ2 = " << gBest.obj2 << '\n';
            log <<setprecision(7) << fixed << "OBJ = " << gBest.get_OBJ(instance) << '\n';
            // LAG O? DA^Y QUA'
    }
}

#endif // ACO_SOLUTION_VER2