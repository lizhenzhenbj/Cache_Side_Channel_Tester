#ifndef THESISDEV_EXECUTOR_H
#define THESISDEV_EXECUTOR_H

#include <cmath>
#include <cstdlib>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <random>

namespace thesis {
    std::mt19937_64 seed(time(NULL));
    std::uniform_int_distribution<int> rand1000(0, 1000);

    class optimal_data_finder {
    private:
        int **dataset;
        int **bkp_dataset;
        int *counts;
        int no_of_params;
        std::string program_path;
        std::string program;
        std::string results_filename;
        int execution_rounds;

        void backup_dataset() {
            int i, j;
            for (i = 0; i < no_of_params; i++)
                for (j = 0; j < counts[i]; j++)
                    bkp_dataset[i][j] = dataset[i][j];
        }

        void restore_dataset() {
            int i, j;
            for (i = 0; i < no_of_params; i++)
                for (j = 0; j < counts[i]; j++)
                    dataset[i][j] = bkp_dataset[i][j];
        }

        void run_program(int times) {
            int i, j;
            int temp;
            std::string command = "rm " + results_filename;
            system(command.data());
            std::string path = "cd " + program_path + " && ./";
            std::string param_list;
            for (j = 0; j < times; j++) {
                param_list = "";
                for (i = 0; i < no_of_params; i++) {
                    temp = dataset[i][rand1000(seed) % counts[i]];
                    param_list += " " + std::to_string(temp);
                }
                command = path + program + param_list;
                system(command.data());
            }
        }

    protected:
        virtual void randomize_dataset() {
            int i, j, k;
            int temp_rand;
            bool not_unique;
            for (i = 0; i < no_of_params; i++) {
                for (j = 0; j < counts[i]; j++) {
                    not_unique = true;
                    while(not_unique) {
                        temp_rand = rand1000(seed) % 256;
                        for (k = 0; k < j; k++)
                            if (temp_rand == dataset[i][k])
                                break;
                        not_unique = (k != j);
                    }
                    dataset[i][j] = temp_rand;
                }
            }
        }

        virtual void mutate_dataset(int factor) {
            int i, j;
            int flipper[] = {1, 2, 4, 8, 16, 32, 64, 128};
            int mutate_data;
            int mutate_bit;
            for (i = 0; i < no_of_params; i++) {
                for (j = 0; j < counts[i]; j++) {
                    mutate_data = rand1000(seed) % factor;
                    if (!mutate_data) {
                        mutate_bit = rand1000(seed) % 8;
                        dataset[i][j] ^= flipper[mutate_bit];
                    }
                }
            }
        }

        virtual long get_objective(std::string results_filename) {
            std::ifstream out_file;
            long output = 0;
            int i, j;
            long keys[execution_rounds][no_of_params], da[execution_rounds], lm[execution_rounds], sm[execution_rounds], dm[execution_rounds];
            out_file.open(results_filename, std::ios::in);
            for (i = 0; i < execution_rounds; i++) {
                for (j = 0; j < no_of_params; j++)
                    out_file >> keys[i][j];
                out_file >> da[i];
                out_file >> lm[i];
                out_file >> sm[i];
                out_file >> dm[i];
                for (j = 0; j < i; j++)
                    if (lm[i] == lm[j])
                        break;
                if (j == i)
                    output++;
            }
            return output;
        }

    public:

        optimal_data_finder(int no_of_params, int* counts, std::string program_path, std::string program, int execution_rounds, std::string results_filename) {
            this->no_of_params = no_of_params;
            this->counts = new int[no_of_params];
            dataset = new int*[no_of_params];
            bkp_dataset = new int*[no_of_params];
            for (int i = 0; i < no_of_params; i++) {
                this->counts[i] = counts[i];
                dataset[i] = new int[counts[i]];
                bkp_dataset[i] = new int[counts[i]];
            }
            this->program_path = program_path;
            this->program = program;
            this->results_filename = results_filename;
            this->execution_rounds = execution_rounds;
            randomize_dataset();
            backup_dataset();
        }

        void sim_ann(double t_init, double t_final, double alpha, int max_trials) {
            bool acceptable;
            double acceptance_prob;
            int trial;
            double t;
            long new_obj;
            std::string saveresults = "cp ";
            saveresults += results_filename + " data/best-results.dat";

            std::cout << "Initializing..." << std::flush;
            run_program(execution_rounds);
            dataset_to_file("data/best-dataset.csv");
            system(saveresults.data());
            long obj = get_objective(results_filename);
            std::cout << "Initial objective: " << obj << std::endl;
            for (t = t_init; t >= t_final; t *= alpha) {
                std::cout << "Temperature: " << t << std::endl;
                for (trial = 0; trial < max_trials; trial++) {
                    std::cout << "Trial #" << trial + 1 << ". " << std::flush;
                    mutate_dataset(5);
                    run_program(execution_rounds);
                    new_obj = get_objective(results_filename);
                    std::cout << "Objective: " << new_obj;
                    acceptable = new_obj > obj;
                    if (!acceptable && new_obj != obj) {
                        acceptance_prob = 0.8 * std::exp((new_obj - obj) / t);
                        acceptable = acceptance_prob > ((double)rand1000(seed) / 1000.0);
                    }
                    if (acceptable) {
                        dataset_to_file("data/best-dataset.csv");
                        system(saveresults.data());
                        backup_dataset();
                        obj = new_obj;
                        std::cout << " - Accepted";
                    }
                    else {
                        restore_dataset();
                    }
                    std::cout << std::endl;
                }
            }

        }

        void dataset_to_file(std::string filename) {
            int i, j;
            std::ofstream file;
            file.open(filename, std::ios::out);
            for (i = 0; i < no_of_params; i++) {
                file << dataset[i][0];
                for (j = 1; j < counts[i]; j++) {
                    file << "," << dataset[i][j];
                }
                file << "\n";
            }
            file.close();
        }
    };
}

#endif //THESISDEV_EXECUTOR_H
