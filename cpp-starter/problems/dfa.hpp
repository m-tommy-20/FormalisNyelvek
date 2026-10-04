#ifndef DFA_HPP
#define DFA_HPP

#include "../problem.hpp"
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

class DFAProblem : public Problem {
public:
  void initialize_parser(cxxopts::Options &options) override;
  bool is_chosen_problem(const cxxopts::ParseResult &args) override;
  int run(const cxxopts::ParseResult &args) override;

  bool load_from_file(const std::string &filename);

  bool accepts(const std::string &word) const;

  void write_result(const std::string &output_filename, const std::string &word,
                    bool accepted) const;

  std::vector<std::string> states;

  std::vector<std::string> alphabet;

  std::string start_state;

  std::set<std::string> final_states;

  std::map<std::pair<std::string, std::string>, std::string> transitions;
};

#endif // DFA_HPP
