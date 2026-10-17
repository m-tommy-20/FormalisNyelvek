// ===== exercise_2 (determinization) =====
#include "determinization.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>

void DeterminizationProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("det", "Determinize the automaton");
}

bool DeterminizationProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
    return args.count("det") > 0;
}

bool DeterminizationProblem::load_from_file(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open input file: " << filename << std::endl;
        return false;
    }

    std::string line;

    // First line: states
    if (!std::getline(file, line)) {
        std::cerr << "Error: Failed to read states" << std::endl;
        return false;
    }
    std::istringstream states_stream(line);
    std::string state;
    while (states_stream >> state) {
        states.push_back(state);
    }

    // Second line: alphabet
    if (!std::getline(file, line)) {
        std::cerr << "Error: Failed to read alphabet" << std::endl;
        return false;
    }
    std::istringstream alphabet_stream(line);
    std::string symbol;
    while (alphabet_stream >> symbol) {
        alphabet.push_back(symbol);
    }

    // Third line: start state
    if (!std::getline(file, line)) {
        std::cerr << "Error: Failed to read start state" << std::endl;
        return false;
    }
    start_state = line;
    start_state.erase(0, start_state.find_first_not_of(" \t\r\n"));
    start_state.erase(start_state.find_last_not_of(" \t\r\n") + 1);

    // Fourth line: final states
    if (!std::getline(file, line)) {
        std::cerr << "Error: Failed to read final states" << std::endl;
        return false;
    }
    std::istringstream final_states_stream(line);
    std::string final_state;
    while (final_states_stream >> final_state) {
        final_states.insert(final_state);
    }

    // Basic validation of the declared elements
    auto is_known_state = [this](const std::string &s) {
        return std::find(states.begin(), states.end(), s) != states.end();
    };
    auto is_known_symbol = [this](const std::string &s) {
        return std::find(alphabet.begin(), alphabet.end(), s) != alphabet.end();
    };

    if (!is_known_state(start_state)) {
        std::cerr << "Error: Start state '" << start_state
                  << "' is not among the declared states" << std::endl;
        return false;
    }
    for (const std::string &f : final_states) {
        if (!is_known_state(f)) {
            std::cerr << "Error: Final state '" << f
                      << "' is not among the declared states" << std::endl;
            return false;
        }
    }

    // Remaining lines: transitions (from symbol to)
    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        std::istringstream transition_stream(line);
        std::string from, sym, to;

        if (!(transition_stream >> from >> sym >> to)) {
            std::cerr << "Error: Malformed transition line: " << line << std::endl;
            return false;
        }

        // Validate that the referenced states and symbol are declared
        if (!is_known_state(from)) {
            std::cerr << "Error: Unknown state '" << from
                      << "' in transition: " << line << std::endl;
            return false;
        }
        if (!is_known_symbol(sym)) {
            std::cerr << "Error: Unknown symbol '" << sym
                      << "' in transition: " << line << std::endl;
            return false;
        }
        if (!is_known_state(to)) {
            std::cerr << "Error: Unknown state '" << to
                      << "' in transition: " << line << std::endl;
            return false;
        }

        transitions[{from, sym}].insert(to);
    }

    return true;
}

void DeterminizationProblem::determinize() {
    // Subset construction.
    // dfa_states[i] holds the set of NFA states behind DFA state s<i>.
    // Discovery is breadth-first, which fixes the s0, s1, ... numbering.

    std::map<std::set<std::string>, int> subset_to_index;
    std::queue<int> work;

    std::set<std::string> start_subset = {start_state};
    subset_to_index[start_subset] = 0;
    dfa_states.push_back(start_subset);
    work.push(0);

    while (!work.empty()) {
        int current = work.front();
        work.pop();

        for (const std::string &sym : alphabet) {
            // Union of all NFA targets reachable with this symbol
            std::set<std::string> target;
            for (const std::string &nfa_state : dfa_states[current]) {
                auto it = transitions.find({nfa_state, sym});
                if (it != transitions.end()) {
                    target.insert(it->second.begin(), it->second.end());
                }
            }

            // No transition at all: leave it out of the output
            if (target.empty())
                continue;

            auto known = subset_to_index.find(target);
            int next_index;
            if (known == subset_to_index.end()) {
                next_index = static_cast<int>(dfa_states.size());
                subset_to_index[target] = next_index;
                dfa_states.push_back(target);
                work.push(next_index);
            } else {
                next_index = known->second;
            }

            dfa_transitions[{current, sym}] = next_index;
        }
    }

    // A DFA state is final if its subset contains any NFA final state
    for (int i = 0; i < static_cast<int>(dfa_states.size()); ++i) {
        for (const std::string &nfa_state : dfa_states[i]) {
            if (final_states.count(nfa_state) > 0) {
                dfa_final_states.insert(i);
                break;
            }
        }
    }
}

bool DeterminizationProblem::write_result(const std::string &output_filename) const {
    std::ofstream out_file(output_filename); // Truncate: fresh result every run
    if (!out_file.is_open()) {
        std::cerr << "Error: Cannot open output file: " << output_filename << std::endl;
        return false;
    }

    // State list: s0 s1 ... in ascending order
    for (int i = 0; i < static_cast<int>(dfa_states.size()); ++i) {
        if (i > 0)
            out_file << " ";
        out_file << "s" << i;
    }
    out_file << "\n";

    // Alphabet, in the order given by the input file
    for (int i = 0; i < static_cast<int>(alphabet.size()); ++i) {
        if (i > 0)
            out_file << " ";
        out_file << alphabet[i];
    }
    out_file << "\n";

    // Start state
    out_file << "s0\n";

    // Final states, ascending
    bool first = true;
    for (int i : dfa_final_states) {
        if (!first)
            out_file << " ";
        first = false;
        out_file << "s" << i;
    }
    out_file << "\n";

    // Transitions: source state ascending, then symbol in alphabet order
    // (one line per existing transition, missing ones are simply skipped)
    for (int i = 0; i < static_cast<int>(dfa_states.size()); ++i) {
        for (const std::string &sym : alphabet) {
            auto it = dfa_transitions.find({i, sym});
            if (it == dfa_transitions.end())
                continue;
            out_file << "s" << i << " " << sym << " s" << it->second << "\n";
        }
    }

    return true;
}

int DeterminizationProblem::run(const cxxopts::ParseResult &args) {
    if (!args.count("input") || !args.count("output")) {
        std::cerr << "Error: --input and --output are required for --det" << std::endl;
        return 1;
    }

    std::string input_file = args["input"].as<std::string>();
    std::string output_file = args["output"].as<std::string>();

    if (!load_from_file(input_file)) {
        return 1;
    }

    determinize();

    if (!write_result(output_file)) {
        return 1;
    }

    return 0;
}
