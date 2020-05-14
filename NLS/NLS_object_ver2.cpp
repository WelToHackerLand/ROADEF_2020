#ifndef NLS_OBJECT_VER_2
#define NLS_OBJECT_VER_2

#include "../Problem_ver2.cpp"
#include "percent_heap.cpp"

struct NLS_object {
    /// **NOTE** risk(t, s) instead of risk(s, t) 
    vector<vector<double> > risk_st;
    vector<double> risk_t;
    double obj1;

    vector<percent_heap> heap_Q;
    vector<vector<double> > Q;
    vector<double> Excess;
    double obj2;

    vector<vector<double> > r_ct;
    int numSatisfiedLBResources, numViolatedUBResources;
    double UBResources_cost, LBResources_cost;

    // vector<vector<int> > Interventions_at_time;
    vector<int> Time_Start_Intervention;
    int numFailedIntervention;

    double get_OBJ(Problem_Instance &instance) {
        double obj = instance.alpha * obj1 + (1 - instance.alpha) * obj2; 
        return obj;
    }

    double getScore(Problem_Instance &instance, double alpha, double beta) {
        double obj_cost = get_OBJ(instance);
        double LB_cost = LBResources_cost;
        double UB_cost = UBResources_cost;
        return obj_cost + alpha * LB_cost + beta * UB_cost;
    }

    void Initialize(Problem_Instance &instance) {
        int T = instance.T;
        obj1 = obj2 = 0;
        numSatisfiedLBResources = 0;
        numViolatedUBResources = 0;
        UBResources_cost = LBResources_cost = 0.0;

        risk_st.resize(T+1);
        for (int t = 1; t <= T; ++t) 
            risk_st[t].resize(instance.numScenarios[t]+1, 0);
        risk_t.resize(T+1, 0);

        heap_Q.resize(T+1);
        for (int i = 1; i <= T; ++i) heap_Q[i].Initalize(instance, instance.numScenarios[i]);

        Q.resize(T+1);
        for (int i = 1; i <= T; ++i) 
            Q[i].resize( instance.numScenarios[i], 0 );
        Excess.resize(T+1, 0);

        r_ct.resize(instance.numResources+1);
        for (int c = 1; c <= instance.numResources; ++c) {    
            r_ct[c].resize(T+1, 0);
            for (int t = 1; t <= T; ++t) {
                if ( instance.l[c][t] <= r_ct[c][t] ) numSatisfiedLBResources++;
                LBResources_cost += r_ct[c][t];
                assert( r_ct[c][t] == 0 );
                assert( r_ct[c][t] <= instance.u[c][t] );
            }
        }

        numFailedIntervention = instance.numInterventions;
        Time_Start_Intervention.resize(instance.numInterventions+1, 0);
    }

    bool Insert(Problem_Instance &instance, int i, int start_Time, double &increase_satisfied_LB) {
        if ( start_Time > instance.tmax[i] ) { 
            cerr << "ERROR!!!: start_Time > tmax(i)\n"; 
            assert(1 == 0); exit(0); 
        }

        // return false if violated constraint  a, r_ct[c][t] <= u[c][t]
        //                                      b, start_Time + delta[i][t] <= T+1          
        if (start_Time + instance.delta[i][start_Time] > instance.T+1) return false;    

        bool returnVal = true;
        for (auto foo : instance.r_list[i][start_Time]) {
            int c = foo.resource, t = foo.time;
            double val = foo.cost;
            if (r_ct[c][t] > instance.l[c][t]) increase_satisfied_LB += val; 
            
            r_ct[c][t] += val;

            if ( r_ct[c][t] - val < instance.l[c][t] && r_ct[c][t] >= instance.l[c][t] ) numSatisfiedLBResources++;
            if ( r_ct[c][t] > instance.u[c][t] ) return false;
        }
        assert(returnVal == true);
        if (!returnVal) return returnVal;

        int T = instance.T;

        for (auto foo : instance.risk_list[i][start_Time]) {
            int s = foo.scenario, t = foo.time;
            double val = foo.cost;

            double old_risk_st = risk_st[t][s];
            risk_st[t][s] += val; 
            risk_t[t] += val * (double) 1 / (double) instance.numScenarios[t];
            obj1 += val * (double) 1 / (double) T / (double) instance.numScenarios[t];

            bool cek_erase = false;
            for (int i = 0; i < (int) Q[t].size(); ++i) 
                if ( abs(Q[t][i]-old_risk_st) <= 1e-6 ) { 
                    Q[t].erase(Q[t].begin() + i); 
                    cek_erase = true;
                    break; 
                }

                if (!cek_erase) {
                    cerr << "??? " << old_risk_st << " " << risk_st[t][s] << '\n';
                    for (double x : Q[t]) cerr << x << " ";
                    cerr << '\n';
                    exit(0);
                }
            assert(cek_erase == true);

            bool cek_insert = false;
            for (int i = 0; i < (int) Q[t].size(); ++i) 
                if (Q[t][i] > risk_st[t][s]) {
                    cek_insert = true;
                    Q[t].insert(Q[t].begin() + i, risk_st[t][s]);
                    break;
                }
            if (!cek_insert) Q[t].push_back(risk_st[t][s]);

            for (int i = 0; i < (int) Q[t].size()-1; ++i) assert( Q[t][i] <= Q[t][i+1] );
            assert( Q[t].size() == instance.numScenarios[t] );

            int pos = ceil( instance.quantile * Q[t].size() ) - 1;
            double Q_quantile = Q[t][pos];

            obj2 -= Excess[t] * (double) 1 / (double) T;
            Excess[t] = max((double) 0, Q_quantile - risk_t[t]);
            obj2 += Excess[t] * (double) 1 / (double) T;
        }

        Time_Start_Intervention[i] = start_Time;
        numFailedIntervention--;
        return returnVal;
    }

    bool Erase(Problem_Instance &instance, int i, int start_Time) {
        if ( start_Time > instance.tmax[i] ) { 
            cerr << "ERROR!!!: start_Time > tmax(i)\n"; 
            assert(1 == 0); exit(0); 
        }

        // return false if violated constraint  a, r_ct[c][t] <= u[c][t]
        // return true if start_Time + delta[i][t] > instance.T + 1;        
        if (start_Time + instance.delta[i][start_Time] > instance.T+1) return true;     

        bool returnVal = true, type = true;
        for (auto foo : instance.r_list[i][start_Time]) {
            int c = foo.resource, t = foo.time;
            double val = foo.cost;
            
            if ( !type ) break;
            if ( r_ct[c][t] > instance.u[c][t] ) type = false;
            r_ct[c][t] -= val;

            if ( r_ct[c][t] + val >= instance.l[c][t] && r_ct[c][t] < instance.l[c][t] ) numSatisfiedLBResources--;
            if ( r_ct[c][t] > instance.u[c][t] ) returnVal = false;
        }
        if (!type) return true; 
        if (!returnVal) return false;

        int T = instance.T;
        for (auto foo : instance.risk_list[i][start_Time]) {
            int s = foo.scenario, t = foo.time;
            double val = foo.cost;

            double old_risk_st = risk_st[t][s];
            risk_st[t][s] -= val; 
            risk_t[t] -= val * (double) 1 / (double) instance.numScenarios[t];
            obj1 -= val * (double) 1 / (double) T / (double) instance.numScenarios[t];

            bool cek_erase = false;
            for (int i = 0; i < (int) Q[t].size(); ++i) 
                if ( abs(Q[t][i]-old_risk_st) <= 1e-6 ) { 
                    Q[t].erase(Q[t].begin() + i);
                    cek_erase = true; 
                    break; 
                } 
                
                if (!cek_erase) {
                    cerr << "??? " << old_risk_st << " " << risk_st[t][s] << '\n';
                    for (double x : Q[t]) cerr << x << " ";
                    cerr << '\n';
                    exit(0);
                }
            assert(cek_erase == true);      
                
            bool cek_insert = false;
            for (int i = 0; i < (int) Q[t].size(); ++i) 
                if (Q[t][i] > risk_st[t][s]) {
                    cek_insert = true;
                    Q[t].insert(Q[t].begin() + i, risk_st[t][s]);
                    break;
                }
            if (!cek_insert) Q[t].push_back(risk_st[t][s]); 

            for (int i = 0; i < (int) Q[t].size()-1; ++i) assert( Q[t][i] <= Q[t][i+1] );
            assert( Q[t].size() == instance.numScenarios[t] );

            int pos = ceil( instance.quantile * Q[t].size() ) - 1;
            double Q_quantile = Q[t][pos];

            obj2 -= Excess[t] * (double) 1 / (double) T;
            Excess[t] = max((double) 0, Q_quantile - risk_t[t]);
            obj2 += Excess[t] * (double) 1 / (double) T;
        }

        numFailedIntervention++;
        Time_Start_Intervention[i] = 0;
        assert(returnVal == true);
        return returnVal;
    }

    void Insert_no_care_UB(Problem_Instance &instance, int i, int start_Time, 
        int &numAcceptedLB, int &numViolatedUB, double &costLB, double &costUB) {
        if ( start_Time > instance.tmax[i] ) { 
            cerr << "ERROR!!!: start_Time > tmax(i) ===> " << start_Time << " " << instance.tmax[i] << '\n'; 
            assert(1 == 0); exit(0); 
        }

        for (auto foo : instance.r_list[i][start_Time]) {
            int c = foo.resource, t = foo.time;
            double val = foo.cost;

            r_ct[c][t] += val;
            if ( r_ct[c][t] - val < instance.l[c][t] && r_ct[c][t] >= instance.l[c][t] ) {
                numAcceptedLB++; 
                numSatisfiedLBResources++;
            }
            if ( r_ct[c][t] - val <= instance.u[c][t] && r_ct[c][t] > instance.u[c][t] ) {
                numViolatedUB++;
                numViolatedUBResources++;
            }

            LBResources_cost -= max( 0.0, instance.l[c][t] - (r_ct[c][t]-val) );
            LBResources_cost += max( 0.0, instance.l[c][t] - r_ct[c][t] );
            if ( r_ct[c][t]-val < instance.l[c][t] ) costLB += min( val, instance.l[c][t]-(r_ct[c][t]-val) );
            
            UBResources_cost -= max( 0.0, r_ct[c][t]-val - instance.u[c][t] );
            UBResources_cost += max( 0.0, r_ct[c][t] - instance.u[c][t] );
            if ( r_ct[c][t]-val <= instance.u[c][t] ) costUB += max( 0.0, r_ct[c][t]-instance.u[c][t] );
            else costUB += val;
        } 

        int T = instance.T;
        for (auto foo : instance.risk_list[i][start_Time]) {
            int s = foo.scenario, t = foo.time;
            double val = foo.cost;

            double old_risk_st = risk_st[t][s];
            risk_st[t][s] += val; 
            risk_t[t] += val * (double) 1 / (double) instance.numScenarios[t];
            obj1 += val * (double) 1 / (double) T / (double) instance.numScenarios[t];

            // bool cek_erase = false;
            // for (int i = 0; i < (int) Q[t].size(); ++i) {
            //     if ( abs(Q[t][i]-old_risk_st) <= 1e-6 ) { 
            //         Q[t].erase(Q[t].begin() + i); 
            //         cek_erase = true;
            //         break; 
            //     }
            // }

            //     if (!cek_erase) {
            //         cerr << "??? " << old_risk_st << " " << risk_st[t][s] << '\n';
            //         for (double x : Q[t]) cerr << x << " ";
            //         cerr << '\n';
            //         exit(0);
            //     }
            // assert(cek_erase == true);

            // bool cek_insert = false;
            // for (int i = 0; i < (int) Q[t].size(); ++i) 
            //     if (Q[t][i] > risk_st[t][s]) {
            //         cek_insert = true;
            //         Q[t].insert(Q[t].begin() + i, risk_st[t][s]);
            //         break;
            //     }
            // if (!cek_insert) Q[t].push_back(risk_st[t][s]);

            // for (int i = 0; i < (int) Q[t].size()-1; ++i) assert( Q[t][i] <= Q[t][i+1] );
            // assert( Q[t].size() == instance.numScenarios[t] );

            int pos = ceil( instance.quantile * Q[t].size() ) - 1;
            double Q_quantile = Q[t][pos];

            heap_Q[t].Erase(old_risk_st);
            heap_Q[t].Insert(risk_st[t][s]);

            double heap_Q_quantile = *(heap_Q[t].L.rbegin());
            // if ( abs(Q_quantile-heap_Q_quantile) > 1e-7 ) {
            //     for (double x : Q[t]) cerr << setprecision(2) << fixed << x << " ";
            //     cerr << '\n';
            //     heap_Q[t].debug();
            //     cerr << "??? " << heap_Q_quantile << " " << Q_quantile << '\n';
            //     cerr << "T_T: " << pos << '\n';
            //     exit(0);
            // }
            // assert( abs(Q_quantile-heap_Q_quantile) <= 1e-7 );

            obj2 -= Excess[t] * (double) 1 / (double) T;
            Excess[t] = max((double) 0, heap_Q_quantile - risk_t[t]);
            obj2 += Excess[t] * (double) 1 / (double) T;
        }

        Time_Start_Intervention[i] = start_Time;
        numFailedIntervention--;
    }
    void Erase_no_care_UB(Problem_Instance &instance, int i, int start_Time, int &numAcceptedLB, int& numViolatedUB) {
        if ( start_Time > instance.tmax[i] ) { 
            cerr << "ERROR!!!: start_Time > tmax(i)\n"; 
            assert(1 == 0); exit(0); 
        }

        for (auto foo : instance.r_list[i][start_Time]) {
            int c = foo.resource, t = foo.time;
            double val = foo.cost;

            r_ct[c][t] -= val;

            if ( r_ct[c][t] + val >= instance.l[c][t] && r_ct[c][t] < instance.l[c][t] ) {
                numAcceptedLB--; 
                numSatisfiedLBResources--;
            }
            if ( r_ct[c][t] + val > instance.u[c][t] && r_ct[c][t] <= instance.u[c][t] ) {
                numViolatedUB--;
                numViolatedUBResources--;
            }

            LBResources_cost -= max( 0.0, instance.l[c][t] - (r_ct[c][t]+val) );
            LBResources_cost += max( 0.0, instance.l[c][t] - r_ct[c][t] );
            
            UBResources_cost -= max( 0.0, r_ct[c][t]+val - instance.u[c][t] );
            UBResources_cost += max( 0.0, r_ct[c][t] - instance.u[c][t] );
        }

        int T = instance.T;
        for (auto foo : instance.risk_list[i][start_Time]) {
            int s = foo.scenario, t = foo.time;
            double val = foo.cost;

            double old_risk_st = risk_st[t][s];
            risk_st[t][s] -= val; 
            risk_t[t] -= val * (double) 1 / (double) instance.numScenarios[t];
            obj1 -= val * (double) 1 / (double) T / (double) instance.numScenarios[t];

            // bool cek_erase = false;
            // for (int i = 0; i < (int) Q[t].size(); ++i) 
            //     if ( abs(Q[t][i]-old_risk_st) <= 1e-6 ) { 
            //         Q[t].erase(Q[t].begin() + i);
            //         cek_erase = true; 
            //         break; 
            //     } 
                
            // if (!cek_erase) {
            //     cerr << "??? " << old_risk_st << " " << risk_st[t][s] << '\n';
            //     for (double x : Q[t]) cerr << x << " ";
            //     cerr << '\n';
            //     exit(0);
            // }
            // assert(cek_erase == true);      
                
            // bool cek_insert = false;
            // for (int i = 0; i < (int) Q[t].size(); ++i) 
            //     if (Q[t][i] > risk_st[t][s]) {
            //         cek_insert = true;
            //         Q[t].insert(Q[t].begin() + i, risk_st[t][s]);
            //         break;
            //     }
            // if (!cek_insert) Q[t].push_back(risk_st[t][s]); 

            // for (int i = 0; i < (int) Q[t].size()-1; ++i) assert( Q[t][i] <= Q[t][i+1] );
            // assert( Q[t].size() == instance.numScenarios[t] );

            int pos = ceil( instance.quantile * Q[t].size() ) - 1;
            double Q_quantile = Q[t][pos];

            heap_Q[t].Erase(old_risk_st);
            heap_Q[t].Insert(risk_st[t][s]);

            double heap_Q_quantile = *(heap_Q[t].L.rbegin());
            // if ( abs(Q_quantile-heap_Q_quantile) > 1e-7 ) {
            //     for (int x : Q[t]) cerr << setprecision(2) << fixed << x << " ";
            //     cerr << '\n';
            //     heap_Q[t].debug();
            //     cerr << "??? " << heap_Q_quantile << " " << Q_quantile << '\n';
            //     exit(0);
            // }
            // assert( abs(Q_quantile-heap_Q_quantile) <= 1e-7 );

            obj2 -= Excess[t] * (double) 1 / (double) T;
            Excess[t] = max((double) 0, heap_Q_quantile - risk_t[t]);
            obj2 += Excess[t] * (double) 1 / (double) T;
        }

        Time_Start_Intervention[i] = 0;
        numFailedIntervention++;
    }

    bool check_resource_constraint(Problem_Instance &instance, NLS_object &obj, int i, int start_Time) {
        for (auto foo : instance.r_list[i][start_Time]) {
            int c = foo.resource, t = foo.time;
            double val = foo.cost;
            if ( obj.r_ct[c][t] + val > instance.u[c][t] ) return false;
        }             
        return true;
    }

    void Insert_Only_Resources(Problem_Instance &instance, NLS_object &obj, int i, int start_Time) {
        if ( start_Time > instance.tmax[i] ) { 
            cerr << "ERROR!!!: start_Time > tmax(i)\n"; 
            assert(1 == 0); exit(0); 
        }

        for (auto foo : instance.r_list[i][start_Time]) {
            int c = foo.resource, t = foo.time;
            double val = foo.cost;
            r_ct[c][t] += val;

            LBResources_cost -= max( 0.0, instance.l[c][t] - (r_ct[c][t]-val) );
            LBResources_cost += max( 0.0, instance.l[c][t] - r_ct[c][t] );
            
            UBResources_cost -= max( 0.0, r_ct[c][t]-val - instance.u[c][t] );
            UBResources_cost += max( 0.0, r_ct[c][t] - instance.u[c][t] );
        } 
    }

    void Erase_Only_Resources(Problem_Instance &instance, NLS_object &obj, int i, int start_Time) {
        if ( start_Time > instance.tmax[i] ) { 
            cerr << "ERROR!!!: start_Time > tmax(i)\n"; 
            assert(1 == 0); exit(0); 
        }

        for (auto foo : instance.r_list[i][start_Time]) {
            int c = foo.resource, t = foo.time;
            double val = foo.cost;
            r_ct[c][t] -= val;

            LBResources_cost -= max( 0.0, instance.l[c][t] - (r_ct[c][t]+val) );
            LBResources_cost += max( 0.0, instance.l[c][t] - r_ct[c][t] );
            
            UBResources_cost -= max( 0.0, r_ct[c][t]+val - instance.u[c][t] );
            UBResources_cost += max( 0.0, r_ct[c][t] - instance.u[c][t] );
        }
    }

    bool exclusionChecking(Problem_Instance &instance, int i, int start_Time) {
        for (auto foo : instance.Exclusion_list[i][start_Time]) {
            int j = foo.first, t = foo.second;
            if ( Time_Start_Intervention[j] == t ) return false;
        }
        return true;
    }

    bool Randomized_Insert(Problem_Instance &instance, int i) {
		double best_score = 1e10;
		int best_start_Time = -1;

		for (int startTime = 1; startTime <= instance.tmax[i]; ++startTime) {
			// check exclusion constraint
			if ( !exclusionChecking(instance, i, startTime) ) continue;
			// check limited time step constraint
			if ( startTime + instance.delta[i][startTime] > instance.T+1 ) continue;

			// check UB
			for (auto foo : instance.r_list[i][startTime]) {
				int c = foo.resource, t = foo.time;
				double val = foo.cost;

				r_ct[c][t] += val;

				if ( r_ct[c][t] - val < instance.l[c][t] && r_ct[c][t] >= instance.l[c][t] ) numSatisfiedLBResources++;
				if ( r_ct[c][t] > instance.u[c][t] ) return false;
			}
			// check LB

			// check addition risk
			double add_obj_1 = instance.obj1_cost[i][startTime];
			double rnd = Utils::real_random_generator(0,1);
			if (rnd > 1 - 0.25 && best_start_Time != -1) continue;
			if (add_obj_1 < best_score){
				best_score = add_obj_1;
				best_start_Time = startTime;
			}
		}

		if (best_start_Time == -1) return false;
		else {
			//Proceed_Insert(instance, i, best_start_Time);
            int nAC = 0, nVL = 0;
            double costLB = 0, costUB = 0;
            Insert_no_care_UB(instance, i, best_start_Time, nAC, nVL, costLB, costUB);
			return true;
		}
        assert(1 == 0);
    }
};

#endif // NLS_OBJECT_VER_2