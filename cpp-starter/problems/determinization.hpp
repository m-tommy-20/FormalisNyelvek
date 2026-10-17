// ===== exercise_2 (determinization) =====
#ifndef DETERMINIZATION_HPP
#define DETERMINIZATION_HPP

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "../problem.hpp"

class DeterminizationProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;

    bool is_chosen_problem(const cxxopts::ParseResult &args) override;

    int run(const cxxopts::ParseResult &args) override;

    bool load_from_file(const std::string &filename);

    void determinize();

    bool write_result(const std::string &output_filename) const;

    // NFA data
    std::vector<std::string> states;
    std::vector<std::string> alphabet;
    std::string start_state;
    std::set<std::string> final_states;
    // Key: (from_state, symbol), Value: all target states (NFA: possibly several)
    std::map<std::pair<std::string, std::string>, std::set<std::string>> transitions;

    // DFA data (built by determinize)
    // Each DFA state is a set of NFA states; index i corresponds to s<i>
    std::vector<std::set<std::string>> dfa_states;
    std::set<int> dfa_final_states;
    // Key: (dfa_state_index, symbol), Value: dfa_state_index
    std::map<std::pair<int, std::string>, int> dfa_transitions;
};

#endif // DETERMINIZATION_HPP
