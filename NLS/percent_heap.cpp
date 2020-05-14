#ifndef PERCENT_HEAP
#define PERCENT_HEAP

#include "../Problem_ver2.cpp"

struct percent_heap {
    multiset<double> L, R;
    int sz_L;

    void Initalize(Problem_Instance &instance, int total_sz) {
        sz_L = ceil( instance.quantile * total_sz );
        for (int i = 1; i <= total_sz; ++i) 
            if (i <= sz_L) L.insert(0.0);
            else R.insert(0.0);
    }  

    void balance() {
        while ( (int) L.size() < sz_L && R.size() ) {
            auto it = R.begin();
            L.insert( *it );
            R.erase(it);
        }
        while ( (int) L.size() > sz_L ) {
            auto it = L.end(); --it;
            R.insert( *it );
            L.erase(it);
        }
    }

    void Insert(double val) { 
        if ( L.size() && val >= (*L.rbegin()) ) R.insert(val);
        else L.insert(val);
        balance();
    }

    void Erase(double val) {
        if ( L.empty() || val > (*L.rbegin() ) ) {
            auto it = R.lower_bound(val-1e-9);
            assert( it != R.end() && abs( (*it) - val ) <= 1e-6 );
            R.erase(it);
        }
        else {
            auto it = L.lower_bound(val-1e-9);
            assert( it != L.end() && abs( (*it) - val ) <= 1e-6 );
            L.erase(it);
        }
        balance();
    }

    void debug() {
        cerr << "PERCENT HEAP: " << sz_L << '\n';
        cerr << "L(" << L.size() << "): "; 
        for (auto val : L) cerr << val << " ";
        cerr << '\n';

        cerr << "R(" << R.size() << "): "; 
        for (auto val : R) cerr << val << " ";
        cerr << '\n';
    }

    void test() {
        sz_L = 5;
        vector<int> V;
        for (int i = 1; i<= 10; ++i) {
            int val = rand() % 31;
            V.push_back(val);
            cerr << val << " --> ";
            Insert(val);    
            debug();
        }
        
        cerr << "ERASE:\n";
        random_shuffle(V.begin(), V.end());
        for (int val : V) {
            cerr << val << " --> ";
            Erase(val);
            debug();
        }

        exit(0);
    }
};

#endif /// PERCENT_HEAP