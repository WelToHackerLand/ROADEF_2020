#ifndef PROBLEM_VER2
#define PROBLEM_VER2

// #include "testlib.h"
#include "utils.cpp"
#include<bits/stdc++.h>
#include "nlohmann/json.hpp"
using namespace std;

struct Problem_Instance {
    double alpha, quantile;
    int T, numResources, numInterventions;

    vector<vector<double> > l, u;
    vector<int> tmax, numScenarios;
    vector<vector<int> > delta;
    vector<int> total_delta;
    map<string, int> Map_name_Intervention, Map_name_Resource;
    vector<string> Intervention_name;
 
    struct dataRisk {
        int scenario, time;
        double cost;
        dataRisk() {};
        dataRisk(int scenario, int time, double cost) : scenario(scenario), time(time), cost(cost) {};
    };
    vector<vector<vector<dataRisk> > > risk_list;
    
    struct dataR {
        int resource, time;
        double cost;
        dataR() {};
        dataR(int resource, int time, double cost) : resource(resource), time(time), cost(cost) {};
    };
    vector<vector<vector<dataR> > > r_list;
    
    vector<vector<vector<pair<int, int> > > > Exclusion_list;
    vector<int> seasons_period[4];
    /// 0 is "all"
    /// 1 is "winter"
    /// 2 is "summer"
    /// 3 is "is"

	void read_Json_file(string instance_file) {
            cerr << "===============READING===================\n";

		std::ifstream ifs{ instance_file };
		auto info = nlohmann::json::parse(ifs);

        T = info["T"];
        alpha = info["Alpha"];
        quantile = info["Quantile"];
        numResources = info["Resources"].size();
        numInterventions = info["Interventions"].size();

        ////-----------------------------------
        /// import num scenario
        numScenarios.resize(T+1);
        int ttime = 0;
        for (auto num : info["Scenarios_number"]) numScenarios[++ttime] = num;

        ////-----------------------------------
        /// import season
        for (int val = 1; val <= T; ++val) seasons_period[0].push_back(val);
        for (string val : info["Seasons"]["winter"]) seasons_period[1].push_back( stoi(val) );
        for (string val : info["Seasons"]["summer"]) seasons_period[2].push_back( stoi(val) );
        for (string val : info["Seasons"]["is"]) seasons_period[3].push_back( stoi(val) );

        ////-----------------------------------
        l.resize(numResources+1);
        for (int i = 1; i <= numResources; ++i) l[i].resize(T+1);
        u.resize(numResources+1);
        for (int i = 1; i <= numResources; ++i) u[i].resize(T+1);

        int idResource = 0;
        for (auto foo : info["Resources"].items()) {
            ++idResource;

            auto name = foo.key();
            auto resource = foo.value();
            Map_name_Resource[name] = idResource;
            
            /// import minimum resource
            int current_time = 0;
            for (auto value : resource["min"]) l[idResource][++current_time] = value;
            
            /// import maximum resource
            current_time = 0;
            for (auto value : resource["max"]) u[idResource][++current_time] = value;

            /// assertion for sure
            assert(resource["max"].size() == T);
            assert(resource["min"].size() == T);
        }
        assert(idResource == numResources); 

        ////-----------------------------------
        tmax.resize(numInterventions+1);
        delta.resize(numInterventions+1);
        total_delta.resize(numInterventions+1);
        for (int i = 1; i <= numInterventions; ++i) delta[i].resize(T+1, 0);
        Intervention_name.resize(numInterventions+1);

        risk_list.resize(numInterventions+1);
        for (int i = 1; i <= numInterventions; ++i) risk_list[i].resize(T+1);
        
        r_list.resize(numInterventions+1);
        for (int i = 1; i <= numInterventions; ++i) r_list[i].resize(T+1);

        int idIntervention = 0;
        for (auto foo : info["Interventions"].items()) {
            ++idIntervention;

            auto name_intervention = foo.key();
            auto intervention = foo.value();
            Map_name_Intervention[name_intervention] = idIntervention;
            Intervention_name[idIntervention] = name_intervention;

            /// import tmax(i) - limit of time start of each intervention  
            string str_tmax = intervention["tmax"];
            tmax[idIntervention] = stoi(str_tmax);
            
            /// import delta(i,t) - duration of each intervention in each time
            int current_time = 0;
            for (auto val : intervention["Delta"]) {
                delta[idIntervention][++current_time] = val;
                total_delta[idIntervention] += (int) val;
            }
            assert(current_time == T);

            /// import workload(idResource, Time, idIntervention, start_Time) 
            for (auto cc : intervention["workload"].items()) {
                auto name_resource = cc.key();
                auto resource = cc.value();
                int idResource = Map_name_Resource[name_resource];
                
                for (auto Time_info : resource.items()) {
                    int Time = stoi( Time_info.key() );
                    for (auto start_Time_info : Time_info.value().items()) {
                        int start_Time = stoi( start_Time_info.key() );
                        double val = start_Time_info.value();
                        r_list[idIntervention][start_Time].push_back( dataR(idResource, Time, val) );
                    } 
                }
            }

            /// import risk(idScenario, Time, idIntervention, start_Time)
            ofstream test("output.txt");
            test << "#" << idIntervention << ": \n";

            for (auto Time_info : intervention["risk"].items()) {
                int Time = stoi(  Time_info.key() );
                for (auto start_Time_info : Time_info.value().items()) {
                    int start_Time = stoi( start_Time_info.key() );

                    int idScenario = 0;
                    for (auto val : start_Time_info.value()) {
                        ++idScenario;
                        risk_list[idIntervention][start_Time].push_back( dataRisk(idScenario, Time, val) );
                        //    test << start_Time << " " << idScenario << " " << Time << '\n';
                    }
                    assert(idScenario == numScenarios[Time]);
                }
            }
            // test.close();
        }
        assert(idIntervention == numInterventions); 

            //  for (int i = 1; i <= numInterventions; ++i) 
            // for (int t = 1; t <= tmax[i]; ++t) 
            //     cerr << r_list[i][t].size() <<  " " << risk_list[i][t].size() << '\n';

            // for (int t = 1; t <= T; ++t)
            //     cerr << numScenarios[t] << "\n";
            // exit(0);
            // cerr << risk_list[2][1].size() << '\n';
            // for (auto foo : risk_list[2][1]) {
            //     cerr << foo.scenario << " " << foo.time << " " << foo.cost << '\n';
            // }
            // exit(0);

        ////-----------------------------------
        /// import Exclusion
        Exclusion_list.resize(numInterventions+1);
        for (int i = 1; i <= numInterventions; ++i) Exclusion_list[i].resize(tmax[i]+1);

        for (auto E : info["Exclusions"]) {
            vector<string> V;
            for (string str : E) V.push_back(str);
            assert( (int) V.size() == 3 );

            int I1 = Map_name_Intervention[V[0]], I2 = Map_name_Intervention[V[1]];
            int season;
            if (V[2] == "full") season = 0;
            else if (V[2] == "winter") season = 1;
            else if (V[2] == "summer") season = 2;
            else if (V[2] == "is") season = 3;
            else {
                cerr << "Invalid season\n";
                exit(0);
            }

            for (int t : seasons_period[season]) {
                for (int st1 = 1; st1 <= min(t, tmax[I1]); ++st1) {
                    if ( st1 + delta[I1][st1] - 1 < t ) continue;
                    for (int st2 = 1; st2 <= min(t, tmax[I2]); ++st2) {
                        if ( st2 + delta[I2][st2] - 1 < t ) continue;
                        Exclusion_list[I1][st1].push_back( make_pair(I2, st2) );
                        Exclusion_list[I2][st2].push_back( make_pair(I1, st1) );
                    }
                } 
            }
        }
        
        for (int i = 1; i <= numInterventions; ++i) 
        for (int t = 1; t <= tmax[i]; ++t) {
            sort(Exclusion_list[i][t].begin(), Exclusion_list[i][t].end());
            Exclusion_list[i][t].resize(unique(Exclusion_list[i][t].begin(), Exclusion_list[i][t].end()) - Exclusion_list[i][t].begin());
        }   

            // ofstream test("output.txt");
            // for (int i = 1; i <= numInterventions; ++i) 
            //     for (int start_Time = 1; start_Time <= T; ++start_Time) {
            //         sort(risk_list[i][start_Time].begin(), risk_list[i][start_Time].end(), [] (dataRisk a, dataRisk b) {
            //             if (a.time != b.time) return a.time < b.time;
            //             return a.scenario < b.scenario;
            //         }); 

            //         test << "# " << i << " " << start_Time << '\n'; 
            //         for (int j = 0; j < (int) risk_list[i][start_Time].size()-1; ++j) {
            //             if (risk_list[i][start_Time][j].time == risk_list[i][start_Time][j+1].time) {
            //                 assert(risk_list[i][start_Time][j+1].scenario == risk_list[i][start_Time][j].scenario+1);
            //                 test << risk_list[i][start_Time][j+1].cost - risk_list[i][start_Time][j].cost << " "; 
            //             }
            //             else {
            //                 assert(risk_list[i][start_Time][j+1].time == risk_list[i][start_Time][j].time+1);
            //             }
            //         }
            //         test << '\n';
            //     }
            // test.close();
            // exit(0);

            cerr << "T_T: quantile = " << quantile << '\n';    
            cerr << "===============DONE_READING==============\n";
            // exit(0);
	}

    void Random_Priority_Descent_Delta_Sort(vector<int> &V) {
        sort(V.begin(), V.end(), [&] (int u, int v) {
            return total_delta[u] > total_delta[v];
        });

        for (int i = 0; i < (int) V.size(); ++i) {
            // int j = rnd.next(10000000) % (int) V.size();
            // int p = rnd.next(10000000) % 101;
            int j = rand() % (int) V.size();
            int p = rand() % 101;
            if (p <= 10) swap(V[i], V[j]);
        }
    }

    void Random_Priority_Ascent_Delta_Sort(vector<int> &V) {
        sort(V.begin(), V.end(), [&] (int u, int v) {
            return total_delta[u] < total_delta[v];
        });

        for (int i = 0; i < (int) V.size(); ++i) {
            // int j = rnd.next(10000000) % (int) V.size();
            // int p = rnd.next(10000000) % 101;
            int j = rand() % (int) V.size();
            int p = rand() % 101;
            if (p <= 10) swap(V[i], V[j]);
        }
    }

    void Random_Priority_Sort(vector<int> &V) {
        int option = rand() % 2;
        if (option == 0) Random_Priority_Ascent_Delta_Sort(V);
        else if (option == 1) Random_Priority_Descent_Delta_Sort(V);
        else {
            cerr << "Invalid sort option\n";
            exit(0);
        }
    }

    void Total_Random(vector<int> &V){
	    Utils::shuffle(V);
    }

    void Just_Shuffle(vector<int> &V){
        int option = Utils::integer_random_generator(0,3);

        if (option == 0) return Total_Random(V);
        else if (option == 1) return Random_Priority_Descent_Delta_Sort(V);
        else if (option == 2) return Random_Priority_Ascent_Delta_Sort(V);
        else {
            cerr << "Invalid destroy option\n";
            exit(0);
        }
    }

    void debug_parameter(string input) {
        // cerr << "T = " << T << '\n';
        // cerr << "Alpha = " << alpha << '\n';
        // cerr << "Quantile = " << quantile << '\n'; 
        // cerr << "Resources numbers = " << numResources << '\n';
        // cerr << "Intervention numbers = " << numInterventions << '\n';

        ofstream out("output.txt", std::ofstream::app);
        out << "INPUT: " << input << '\n';
        // out << "\tT = " << T << '\n';
        // out << "\tAlpha = " << alpha << '\n';
        // out << "\tQuantile = " << quantile << '\n'; 
        // out << "\tResources numbers = " << numResources << '\n';
        // out << "\tIntervention numbers = " << numInterventions << '\n';

        // vector<int> cnt;
        // cnt.resize(T+1);
        // for (dataExclusion foo : Exc) cnt[foo.t]++;
        
        // int max_cnt = cnt[1], min_cnt = cnt[1], sum_cnt = 0;
        // for (int i = 1; i <= T; ++i) {
        //     min_cnt = min(min_cnt, cnt[i]);
        //     max_cnt = max(max_cnt, cnt[i]);
        //     sum_cnt += cnt[i];
        // }

        // out << "\tMax Exclusion in one time = " << max_cnt << '\n';
        // out << "\tMin Exclusion in one time = " << min_cnt << '\n';
        // out << "\tAvg Exclusion in one time = " << (double) sum_cnt / (double) Exc.size() << '\n'; 

        int maxDelta = -1, minDelta = (int) 1e9 + 7, sumDelta = 0, cntDelta = 0;
        for (int i = 1; i <= numInterventions; ++i) for (int t = 1; t <= tmax[i]; ++t) {
            maxDelta = max(maxDelta, delta[i][t]);
            minDelta = min(minDelta, delta[i][t]);
            sumDelta += delta[i][t];
            ++cntDelta;
        }
        double avgDelta = (double) sumDelta / (double) cntDelta;

        out << "\tMax processing time = " << maxDelta << '\n';
        out << "\tMin processing time = " << minDelta << '\n';
        out << "\tAvg processing time = " << avgDelta << '\n';

        // out << '\n';

        // for (int i = 1; i <= numInterventions; ++i) for (int st = 1; st <= tmax[i]; ++st) {
        //     for (auto foo : r_list[i][st]) 
        // }
    }

    // void debug_risk_cost() {
    //     ifstream in("risk_cost.txt");
    //     while (true) {
    //         int s, t, i, st;
    //         double cost;
    //         in >> s >> t >> i >> st >> cost;
            
    //         if ( abs(cost - my-cost) > 1e-9 ) {
    //             cerr << "WRONG RISK COST\n";
    //             exit(0);
    //         } 
    //     }
    //     cerr << "ACCEPT RISK COST";
    //     exit(0);
    // }

    vector<vector<double> > obj1_cost;

    void prepare_obj1_cost() {
        obj1_cost.resize( numInterventions+1 );
        for (int i = 1; i <= numInterventions; ++i) obj1_cost[i].resize(tmax[i]+1, 0.0);

        for (int i = 1; i <= numInterventions; ++i)
        for (int start_Time = 1; start_Time <= tmax[i]; ++start_Time) {
            if ( i + delta[i][start_Time] > T+1 ) continue;

            for (auto foo : risk_list[i][start_Time]) {
                int s = foo.scenario, t = foo.time;
                double val = foo.cost;
                obj1_cost[i][start_Time] += val * (double) 1 / (double) T / (double) numScenarios[t];
            }
        }
    }
};

#endif // PROBLEM_VER2