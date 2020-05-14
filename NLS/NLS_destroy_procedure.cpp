#ifndef NLS_DESTROY_PROCEDURE
#define NLS_DESTROY_PROCEDURE

// #include "../testlib.h"
#include "../Problem_ver2.cpp"
#include "NLS_object_ver2.cpp"
#include "../utils.cpp"

namespace NLS_destroy_procedure {
    const double max_percentage = 30;
    const int numOptions = 3;

    vector<int> destroy_random_vertices(Problem_Instance &instance, NLS_object &obj, int max_percentage) {
        vector<int> notAdded, Added;

        for (int i = 1; i <= instance.numInterventions; ++i) 
            if (!obj.Time_Start_Intervention[i]) notAdded.push_back(i);
            else Added.push_back(i);

        // shuffle(notAdded.begin(), notAdded.end());
        // shuffle(Added.begin(), Added.end());
        random_shuffle(notAdded.begin(), notAdded.end());
        random_shuffle(Added.begin(), Added.end());

        // int percentage = rnd.next(100000000) % max_percentage;
        int percentage = rand() % max_percentage;
        int numTake = (int) Added.size() * percentage / 100;
        for (int i = 0; i < (int) numTake; ++i) {
            int u = Added[i];
            notAdded.push_back(u);
            obj.Erase(instance, u, obj.Time_Start_Intervention[u]);
        }       

        return notAdded;
    }

    vector<int> destroy_time_period(Problem_Instance &instance, NLS_object &obj, int max_percentage) {
        // int percentage = (int) rnd.next(10000000) % max_percentage;
        // int start_time_destroy = (int) rnd.next(1000000) % instance.T + 1;
        // int length_time_destroy = (int) rnd.next(1000000) % (instance.T - start_time_destroy + 1);
        int percentage = (int) rand() % max_percentage;
        int start_time_destroy = (int) rand() % instance.T + 1;
        int length_time_destroy = (int) rand() % (instance.T - start_time_destroy + 1);
        
        vector<int> notAdded, Added;
        for (int i = 1; i <= instance.numInterventions; ++i) {
            if (!obj.Time_Start_Intervention[i]) {
                notAdded.push_back(i);
                continue;
            }

            int l1 = obj.Time_Start_Intervention[i], r1 = l1 + instance.delta[i][l1] - 1;
            int l2 = start_time_destroy, r2 = l2 + length_time_destroy - 1;
            if (r1 < l2 || r2 < l1) continue;
            Added.push_back(i);
        }

        // shuffle(Added.begin(), Added.end());
        // shuffle(notAdded.begin(), notAdded.end());
        random_shuffle(notAdded.begin(), notAdded.end());
        random_shuffle(Added.begin(), Added.end());
        
        int numTake = (int) Added.size() * percentage / 100;
        for (int i = 0; i < numTake; ++i) {
            int u = Added[i];
            notAdded.push_back(u);
            obj.Erase(instance, u, obj.Time_Start_Intervention[u]);
        }
        return notAdded;
    }

    vector<int> destroy_time_with_high_risk(Problem_Instance &instance, NLS_object &obj, int max_percentage) {
        // int percentage = rnd.next(10000000) % max_percentage;
        int percentage = rand() % max_percentage;

        vector<int> notAdded;
        vector<pair<double, int> > Added;
        for (int i = 1; i <= instance.numInterventions; ++i) {
            if (!obj.Time_Start_Intervention[i]) {
                notAdded.push_back(i);
                continue;
            }
            int t = obj.Time_Start_Intervention[i];
            double score = obj.risk_t[t] + obj.Excess[t];
            Added.push_back(make_pair(score, i));
        }

        // shuffle(notAdded.begin(), notAdded.end());
        random_shuffle(notAdded.begin(), notAdded.end());
        sort(Added.begin(), Added.end());
        reverse(Added.begin(), Added.end());

        int numTake = (int) Added.size() * percentage / 100;
        for (int i = 0; i < numTake; ++i) {
            int u = Added[i].second;
            notAdded.push_back(u);
            obj.Erase(instance, u, obj.Time_Start_Intervention[u]);
        }
        return notAdded;
    }

    vector<int> process(Problem_Instance &instance, NLS_object &obj, int max_percentage, int option = -1) {
        // int option = rnd.next(10000000) % numOptions;
        if (option < 0) option = rand() % numOptions;

        if (option == 0) return destroy_random_vertices(instance, obj, max_percentage);
        else 
        if (option == 1) return destroy_time_period(instance, obj, max_percentage);
        else 
        if (option == 2) return destroy_time_with_high_risk(instance, obj, max_percentage);
        else {
            cerr << "Invalid option\n";
            exit(0);
        } 
    }

    vector<int> destroy_random_vertices_version_2(Problem_Instance &instance, NLS_object &obj, int max_percentage) {
        vector<int> notAdded, Added;

        for (int i = 1; i <= instance.numInterventions; ++i) 
            if (!obj.Time_Start_Intervention[i]) notAdded.push_back(i);
            else Added.push_back(i);

	Utils::shuffle(notAdded);
	Utils::shuffle(Added);

	int percentage = Utils::integer_random_generator(1, max_percentage);
        int numTake = (int) Added.size() * percentage / 100;
        for (int i = 0; i < (int) numTake; ++i) {
            int u = Added[i];
            notAdded.push_back(u);
            int nothing1 = 0, nothing2 = 0;
            obj.Erase_no_care_UB(instance, u, obj.Time_Start_Intervention[u], nothing1, nothing2);
        }       

        return notAdded;
    }

    vector<int> destroy_time_period_version_2(Problem_Instance &instance, NLS_object &obj, int max_percentage) {
	int percentage = (int) Utils::integer_random_generator(1, max_percentage);;
	int start_time_destroy = Utils::integer_random_generator(0, instance.T); + 1;
	int length_time_destroy = (int) Utils::integer_random_generator(1, instance.T - start_time_destroy + 1);
        
        vector<int> notAdded, Added;
        for (int i = 1; i <= instance.numInterventions; ++i) {
            if (!obj.Time_Start_Intervention[i]) {
                notAdded.push_back(i);
                continue;
            }

            int l1 = obj.Time_Start_Intervention[i], r1 = l1 + instance.delta[i][l1] - 1;
            int l2 = start_time_destroy, r2 = l2 + length_time_destroy - 1;
            if (r1 < l2 || r2 < l1) continue;
            Added.push_back(i);
        }

	Utils::shuffle(notAdded);
	Utils::shuffle(Added);
        
        int numTake = (int) Added.size() * percentage / 100;
        for (int i = 0; i < numTake; ++i) {
            int u = Added[i];
            notAdded.push_back(u);

            int nothing1 = 0, nothing2 = 0;
            obj.Erase_no_care_UB(instance, u, obj.Time_Start_Intervention[u], nothing1, nothing2);
        }
        return notAdded;
    }

    vector<int> destroy_time_with_high_risk_version_2(Problem_Instance &instance, NLS_object &obj, int max_percentage) {
        int percentage = rand() % max_percentage;

        vector<int> notAdded;
        vector<pair<double, int> > Added;
        for (int i = 1; i <= instance.numInterventions; ++i) {
            if (!obj.Time_Start_Intervention[i]) {
                notAdded.push_back(i);
                continue;
            }
            int t = obj.Time_Start_Intervention[i];
            double score = obj.risk_t[t] + obj.Excess[t];
            Added.push_back(make_pair(score, i));
        }

        random_shuffle(notAdded.begin(), notAdded.end());
        sort(Added.begin(), Added.end());
        reverse(Added.begin(), Added.end());

        int numTake = (int) Added.size() * percentage / 100;
        for (int i = 0; i < numTake; ++i) {
            int u = Added[i].second;
            notAdded.push_back(u);

            int nothing1 = 0, nothing2 = 0;
            obj.Erase_no_care_UB(instance, u, obj.Time_Start_Intervention[u], nothing1, nothing2);
        }
        return notAdded;
    }

    vector<int> destroy_intervention_with_pre_high_score(Problem_Instance &instance, NLS_object &obj, int max_percentage, double alpha, double beta) {
        vector<pair<double, int> > ls;
        vector<int> V;
        for (int i = 1; i <= instance.numInterventions; ++i) {
            if ( !obj.Time_Start_Intervention[i] ) {
                V.push_back(i);
                continue;
            }

            int nAC = 0, nVL = 0, start_Time = obj.Time_Start_Intervention[i];
            double not1 = 0, not2 = 0;
            obj.Erase_no_care_UB(instance, i, start_Time, nAC, nVL); 
            double score = obj.getScore(instance, alpha, beta);
            ls.push_back( make_pair(score, i) );

            obj.Insert_no_care_UB(instance, i, start_Time, nAC, nVL, not1, not2);
            assert(nAC == 0);
            assert(nVL == 0);
        }

        sort(ls.begin(), ls.end());
        reverse(ls.begin(), ls.end());

        int percentage = rand() % max_percentage;
        int numTake = percentage * (int) ls.size() / 100;
        for (int id = 0; id < numTake; ++id) {
            int i = ls[id].second;
            assert( obj.Time_Start_Intervention[i] > 0 );
            int nAC = 0, nVL = 0, start_Time = obj.Time_Start_Intervention[i];
            obj.Erase_no_care_UB(instance, i, start_Time, nAC, nVL); 
            V.push_back(i);
        }

        random_shuffle(V.begin(), V.end());

        return V;
    }

    vector<int> MingAnhDestroy(Problem_Instance &instance, NLS_object &obj, int max_percentage) {
        int percentage = rand() % max_percentage;

        vector<int> ans;
        vector<vector<int> > V;
        V.resize(instance.T+1);

        for (int i = 1; i <= instance.numInterventions; ++i) 
            if (!obj.Time_Start_Intervention[i]) ans.push_back(i);
            else {
                int t = obj.Time_Start_Intervention[i];                
                V[t].push_back(i);
            }
        random_shuffle(ans.begin(), ans.end());

        vector<pair<double, int> > lsTime;
        for (int t = 1; t <= instance.T; ++t) {
            double score = 0;
            for (int c = 1; c <= instance.numResources; ++c) score += max( 0.0, obj.r_ct[c][t] - instance.u[c][t] );
            lsTime.push_back( make_pair(score, t) );
        }
        sort(lsTime.begin(), lsTime.end());

        int numTake = ( instance.numInterventions - (int) ans.size() ) * percentage / 100;
        for (auto foo : lsTime) {
            if (numTake <= 0) break;

            int t = foo.second;
            for (int i : V[t]) {
                --numTake;
                ans.push_back(i);
                int numAC = 0, numVL = 0, start_Time = obj.Time_Start_Intervention[i];
                obj.Erase_no_care_UB(instance, i, start_Time, numAC, numVL);
            }
        }

        return ans;
    }

    vector<int> process_version_2(Problem_Instance &instance, NLS_object &obj, int max_percentage, int option = -1) {
	if (option == -1)
	    option = Utils::integer_random_generator(0,2);

        if (option == 0) return destroy_random_vertices_version_2(instance, obj, max_percentage);
        else 
        if (option == 1) return destroy_time_period_version_2(instance, obj, max_percentage);
        else 
        if (option == 2) return destroy_time_with_high_risk_version_2(instance, obj, max_percentage);
        else 
        if (option == 3) return MingAnhDestroy(instance, obj, max_percentage);
        else {
            cerr << "Invalid destroy option\n";
            exit(0);
        }
    }
}

#endif // NLS_DESTROY_PROCEDURE
