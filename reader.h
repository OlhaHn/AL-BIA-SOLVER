#ifndef READER_H
#define READER_H

std::unordered_map<int, double> powers = std::unordered_map<int, double>();

void read_input(std::unordered_map<int, std::unordered_set<int>>& formula, std::unordered_map<int, Variable>& variables,
                std::unordered_set<int>& unsigned_variables, std::unordered_map<int, PairsSet>& binary_clauses, int& number_of_clauses);

void prepare_variables(int number, std::unordered_map<int, Variable>& variables, std::unordered_set<int>& unsigned_variables,
                       std::unordered_map<int, PairsSet>& binary_clauses);

void prepare_variables(int number, std::unordered_map<int, Variable>& variables, std::unordered_set<int>& unsigned_variables,
                       std::unordered_map<int, PairsSet>& binary_clauses) {
    for(int i=1; i<=number; i++) {
        variables[i] = {-1, -1, {}};
        binary_clauses[i] = {};
        binary_clauses[-1*i] = {};
        unsigned_variables.insert(i);
    }
}

void fill_power_arrays_for_heustics(int max_size) {

    powers[1] = 1;
    powers[2] = 1;
    powers[4] = 0.05;
    powers[5] = 0.01;
    powers[6] = 0.003;
    for(int i=7; i<=max_size; i++) {
        powers[i] = 20.4514*pow(0.218673, i);
    }
}

void read_input(std::unordered_map<int, std::unordered_set<int>>& formula, std::unordered_map<int, Variable>& variables,
                std::unordered_set<int>& unsigned_variables, std::unordered_map<int, PairsSet>& binary_clauses, int& number_of_clauses) {

    std::string line;
    // skip comments
    while (std::getline(std::cin, line)) {
        if (line[0] == 'p') {
            break;
        }
    }

    std::istringstream iss(line);
    std::vector<std::string> info(std::istream_iterator<std::string>{iss},
                                  std::istream_iterator<std::string>());

    int variables_number = std::stoi(*(info.end() - 2));
    number_of_clauses = std::stoi(*(info.end() - 1));
    prepare_variables(variables_number, variables, unsigned_variables, binary_clauses);
    int max_clause_size = 0;
    for(int i=0; i<number_of_clauses; i++) {
        std::string input;
        std::getline(std::cin, input);
        std::stringstream stream(input);

        std::unordered_set<int> clause = std::unordered_set<int>();

        int n;
        stream >> n;
        int clause_size = 0;
        while(n) {
            clause.insert(n);;
            stream >> n;
            clause_size++;
        }

        if(clause_size > max_clause_size) {
            max_clause_size = clause_size;
        }

        if(clause_size == 2) { // binary clause
            auto it = clause.begin();
            auto first_literal = *it; it++;
            auto second_literal = *it;

            binary_clauses[first_literal].insert(std::make_pair(second_literal, i));
            binary_clauses[second_literal].insert(std::make_pair(first_literal, i));
        } else {
            formula[i] = clause;
            for(auto var: clause) {
                variables[abs(var)].clauses.insert(i);
            }
        }
    }

    fill_power_arrays_for_heustics(max_clause_size);

}

#endif
