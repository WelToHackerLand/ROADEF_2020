#ifndef CONFIG
#define CONFIG

namespace config {
    string input, test;
    bool found_input = false;

    string output;
    bool found_output = false;

    double time_limit = 300.0;
    bool found_time_limit = false;

    int max_iter = 50;
    bool found_max_iter = false;

    void parse_arguments(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            string key = argv[i];

            if (key == "--input") {
                string value = argv[++i];
                input = value;
                found_input = true;
            }
            else
            if (key == "--output") {
                string value = argv[++i];
                output = value;
                found_output = true;
            }
            else 
            if (key == "--time-limit") {
                string value = argv[++i];
                time_limit = stof(value);
                found_time_limit = true;
            }
            else 
            if (key == "--max-iter") {
                string value = argv[++i];
                max_iter = stoi(value);
                found_max_iter = true;
            }
            else {
                cerr << "Invalid argument !!!";
                exit(0);
            }
        }

        if (!found_input) {
            cerr << "Input is required !!!\n";
            exit(0);
        }
        if (!found_output) {
            cerr << "Warning: Output is missing !!!!\n";
        }
        if (!found_time_limit) {
            cerr << "Warning: time_limit default = 300.0s\n";
        }
        if (!found_max_iter) {
            cerr << "Warning: max_iter default = 50\n";
        }

        /// test 
        for (int i = (int) input.length()-1; i >= 0; --i) {
            if (input[i] == '/') break;
            test += input[i];
        }
        reverse(test.begin(), test.end());
    }
}

#endif // CONFIG