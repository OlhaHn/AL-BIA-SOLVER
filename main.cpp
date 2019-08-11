#include "includes.h"
#include "settings.h"
#include "variable.h"
#include "reader.h"
#include "sat_class.h"
#include <fstream>
/*
    Returns:
    0 -> instance is unsatisfable
    -1 -> decision variable was not found
    > 0 -> decision varible

*/
bool last_decision_true_is_better = false;
bool march_true_is_better = false;

int look_ahead(SATclass& instance, int depth);

int double_lookahead(SATclass& instance, SATclass& old_instance) {
    #if DOUBLE_LOOKAHEAD >= 1
        auto binary_clauses = instance.new_binary_clauses.size();
        //std::cout << binary_clauses << ' ' << old_instance.trigger << '\n'; 
        if(binary_clauses >= old_instance.trigger) {
            auto next_look_ahed_result = look_ahead(instance, 2);
            if (next_look_ahed_result == 0) {
                #if DOUBLE_LOOKAHEAD == 3
                    old_instance.trigger = old_instance.start_tigger;
                #endif
                return 0;
            } else {
                #if DOUBLE_LOOKAHEAD == 3 || DOUBLE_LOOKAHEAD == 4
                    old_instance.trigger = binary_clauses;
                #endif
            }
        }
    #endif
    return 1;
}

double count_crh(SATclass& old_instace, SATclass& instance) {
    double result = 0;
    for(auto clause_hash: instance.reducted_clauses) {
        if(instance.formula.find(clause_hash) != instance.formula.end()) {
            result += powers[instance.get_clause_size(clause_hash)];
        } else {
            result += 1; // binary clause;
        }
        
    }
    return result;
}

void recount_weights(SATclass& instance, SATclass& new_instace) {
    for(auto hash: new_instace.reducted_clauses) {
        double new_coeff = powers[new_instace.get_clause_size(hash)];
        double old_coeff = powers[instance.get_clause_size(hash)];
        double update = new_coeff - old_coeff;
        for(auto literal: new_instace.formula[hash]) {
            new_instace.literal_weights[literal] += update;
        }
    }
}

double count_wbh(SATclass& instance, SATclass& new_instace) {
    recount_weights(instance, new_instace);
    double wbh = 0;
    for(auto i: new_instace.reducted_clauses) {
        if(new_instace.get_clause_size(i) == 2) {
            for(auto literal: new_instace.formula[i]) {
                wbh += new_instace.literal_weights[-1*literal];
            }
        }
    }
    return wbh;
}

double count_bsh(SATclass& instance, SATclass& new_instace) {
    recount_weights(instance, new_instace);
    double bsh = 0;
    for(auto i: new_instace.reducted_clauses) {
        if(new_instace.get_clause_size(i) == 2) {
            double result = 1;
            for(auto literal: new_instace.formula[i]) {
                result *= new_instace.literal_weights[-1*literal];
            }
            bsh += result;
        }
    }
    return bsh;
}

double count_bsrh(SATclass& instance, SATclass& new_instace) {
    recount_weights(instance, new_instace);
    
    double total_size = 0;
    double coeff_sum = 0;
    for(auto i: new_instace.reducted_clauses) {
        for(auto literal: new_instace.formula[i]) {
            coeff_sum += new_instace.literal_weights[-1*literal];
        }
        total_size += new_instace.formula[i].size();
    }
    double normalization = coeff_sum/total_size;
    double bsrh = 0;
    for(auto i: new_instace.reducted_clauses) {
        auto& clause = new_instace.formula[i];
        double temp_res = powers[clause.size()];
        for(auto literal: clause) {
            temp_res *= new_instace.literal_weights[-1*literal]/normalization;
        }
        bsrh += temp_res;
    }
    return bsrh;
}


double decision_heuristic(SATclass& instance, SATclass& true_instace, SATclass& false_instance) {
    #if DIFF_HEURISTIC == 0
    auto function_pointer = count_crh;
    #elif DIFF_HEURISTIC == 1
    auto function_pointer = count_wbh;
    #elif DIFF_HEURISTIC == 2
    auto function_pointer = count_bsh;
    #elif DIFF_HEURISTIC == 3
    auto function_pointer = count_bsrh;
    #endif
    double true_result = function_pointer(instance, true_instace);
    double false_result = function_pointer(instance, false_instance);
    #if DIRECTION_HEURISTIC == 1
    last_decision_true_is_better = true_result > false_result;
    #endif
    return true_result*false_result;
}


bool kcnfs_direction(SATclass& instance, int decision_variable) {
    return instance.literal_count[decision_variable] > instance.literal_count[-1*decision_variable];
}

bool march_direction() {
    return !march_true_is_better;
}

bool posit_direction(SATclass& instance, int decision_variable) {
    return instance.literal_weights[decision_variable] < instance.literal_weights[-1*decision_variable];
}

bool get_direction_heuristic_val(SATclass& instance, int decision_variable) {
    #if DIRECTION_HEURISTIC == 0
    return kcnfs_direction(instance, decision_variable);
    #elif DIRECTION_HEURISTIC == 1
    return march_direction();
    #elif DIRECTION_HEURISTIC == 2
    return posit_direction(instance, decision_variable);
    #elif DIRECTION_HEURISTIC == 3
    return true;
    #endif
}

void prepare_binary_clauses(SATclass& instance, SATclass& after_propagation, int variable, bool value) {
    int literal = value ? -1*variable : variable;
    auto& clauses = instance.binary_clauses[literal];

    for(auto var: after_propagation.implicated_variables) {
        auto next_literal = after_propagation.variables[var].value ? var : -1*var;
        instance.formula[instance.number_of_all_clauses] = {literal, next_literal};
        clauses.insert(std::make_pair(next_literal, instance.number_of_all_clauses));
        instance.binary_clauses[next_literal].insert(std::make_pair(literal, instance.number_of_all_clauses));
        instance.number_of_all_clauses += 1;
    }
}

int look_ahead(SATclass& instance, int depth) {
    #if PRESELECT_HEURISTIC == 0
    auto preselect = instance.preselect_propz();
    #else
    auto preselect = instance.preselect_cra();
    #endif
    int selected_var = -1;
    double decision_heuristic_value = -100;
    for(auto i: preselect) {
        if(instance.variables[i].value == -1) {
            #if AUTARKY_REASONING != 0
                if(instance.literal_count[i] == 0) { // positive literal does not appear
                    instance.propagation(i, 0);
                    continue;
                } else if(instance.literal_count[-1*i] == 0) { // negative literal does not apper
                    instance.propagation(i, 1);
                    continue;
                }
            #endif

            auto result_of_true_instance = instance;
            auto result_of_false_instance = instance;

            bool true_propagation = result_of_true_instance.propagation(i, 1);
            #if LOCAL_LEARNING == 1
                prepare_binary_clauses(instance, result_of_true_instance, i, true);
            #endif
            bool false_propagation = result_of_false_instance.propagation(i, 0);
            #if LOCAL_LEARNING == 1
                prepare_binary_clauses(instance, result_of_false_instance, i, false);
            #endif
            if(!true_propagation && !false_propagation) {
                return 0;
            } else if(!true_propagation) {
                instance = result_of_false_instance;
                #if DOUBLE_LOOKAHEAD > 0
                    if(depth == 0) {
                        auto new_look_ahead_result = double_lookahead(instance, instance);
                        if (new_look_ahead_result == 0) {
                            return 0;
                        }
                    }
                #endif
            } else if(!false_propagation) {
                instance = result_of_true_instance;
                #if DOUBLE_LOOKAHEAD > 0
                    if(depth == 0) {
                        auto new_look_ahead_result = double_lookahead(instance, instance);
                        if (new_look_ahead_result == 0) {
                            return 0;
                        }
                    }
                #endif
            } else {

                #if DOUBLE_LOOKAHEAD > 0
                    if(depth == 0) {
                        auto double_for_true = double_lookahead(result_of_true_instance, instance);
                        auto double_for_false = double_lookahead(result_of_false_instance, instance);
                        if(double_for_true == 0 && double_for_false == 0) {
                            return 0;
                        }
                    }
                #endif

                auto new_decision = decision_heuristic(instance, result_of_true_instance, result_of_false_instance);
                if(new_decision > decision_heuristic_value) {
                    decision_heuristic_value = new_decision;
                    selected_var = i;
                    #if DIRECTION_HEURISTIC == 1
                        march_true_is_better = last_decision_true_is_better;
                    #endif
                }
            }
        }
    }
    #if DOUBLE_LOOKAHEAD == 4
        if(instance.trigger > 0) {
            instance.trigger--;
        }
    #endif

    if(selected_var > 0 && instance.variables[selected_var].value != -1) {
        return -1;
    }
    return selected_var;
}


bool dpll(SATclass instance) {
    instance.decision_level += 1;
    if (instance.is_satisfied()) {
        return true;
    } else {
        int decision_variable = look_ahead(instance, 0);
        if (decision_variable == 0) {
            return false;
        } else if (decision_variable == -1) {
            return dpll(instance);
        } else {
            auto instance_copy = instance;
            bool value = get_direction_heuristic_val(instance, decision_variable);
            bool propagarion_res = instance.propagation(decision_variable, value);
            if(propagarion_res && dpll(instance)) {
                return true;
            } else {
                propagarion_res = instance_copy.propagation(decision_variable, !value);
                return propagarion_res && dpll(instance_copy);
            }
        }
    }
}

int main() {

    //std::ifstream in("/Users/marek/Desktop/licencjat_in_progress/input/jnh/jnh1.cnf");
    //std::cin.rdbuf(in.rdbuf());

    auto formula = std::unordered_map<int, std::unordered_set<int>>();
    auto variables = std::unordered_map<int, Variable>();
    auto unasigned_variables = std::unordered_set<int>();
    auto binary_clauses = std::unordered_map<int, PairsSet>();
    auto literal_wieghts = std::unordered_map<int, double>();
    auto literal_count = std::unordered_map<int, int>(); 
    int number_of_clauses = 0;

    read_input(formula, variables, unasigned_variables, binary_clauses, number_of_clauses, literal_wieghts, literal_count);
    auto sat_instance = SATclass(unasigned_variables, variables, formula, binary_clauses, number_of_clauses, literal_wieghts, literal_count);

    #if DOUBLE_LOOKAHEAD == 1
        sat_instance.trigger = 65;
    #elif DOUBLE_LOOKAHEAD == 2 || DOUBLE_LOOKAHEAD == 3
        sat_instance.trigger = 0.17*sat_instance.variables.size();
        sat_instance.start_tigger = 0.17*sat_instance.variables.size();
    #elif DOUBLE_LOOKAHEAD == 4
        sat_instance.trigger = 0;
    #endif

    auto start = std::clock();
    auto result = dpll(sat_instance);
    auto duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout << duration << ';';
    //std::cout << "Result: " << result << " " << "duration " << duration  << '\n';

    return 0;
}
