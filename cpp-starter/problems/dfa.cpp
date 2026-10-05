#include "dfa.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string to_upper(const std::string &s) {
  std::string result = s;
  for (char &c : result) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return result;
}

bool DFAProblem::load_from_file(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open input file: " << filename << std::endl;
    return false;
  }

  std::string line;

  if (!std::getline(file, line)) {
    std::cerr << "Error: Failed to read states" << std::endl;
    return false;
  }
  std::istringstream states_stream(line);
  std::string state;
  while (states_stream >> state) {
    states.push_back(state);
  }

  if (!std::getline(file, line)) {
    std::cerr << "Error: Failed to read alphabet" << std::endl;
    return false;
  }
  std::istringstream alphabet_stream(line);
  std::string symbol;
  while (alphabet_stream >> symbol) {
    alphabet.push_back(symbol);
  }

  if (!std::getline(file, line)) {
    std::cerr << "Error: Failed to read start state" << std::endl;
    return false;
  }
  start_state = line;
  // Trim leading/trailing whitespace
  start_state.erase(0, start_state.find_first_not_of(" \t\r\n"));
  start_state.erase(start_state.find_last_not_of(" \t\r\n") + 1);

  if (!std::getline(file, line)) {
    std::cerr << "Error: Failed to read final states" << std::endl;
    return false;
  }
  std::istringstream final_states_stream(line);
  std::string final_state;
  while (final_states_stream >> final_state) {
    final_states.insert(final_state);
  }

  while (std::getline(file, line)) {
    // Skip empty lines
    if (line.empty())
      continue;

    std::istringstream transition_stream(line);
    std::string current_state, symbol, next_state;

    // Parse: current_state input_symbol next_state
    // We expect exactly 3 space-separated tokens
    if (!(transition_stream >> current_state >> symbol >> next_state)) {
      // If parsing fails, skip this line and continue
      // This handles malformed or comment lines gracefully
      continue;
    }

    // Store the transition in our map
    // Key: pair of (current_state, symbol)
    // Value: next_state
    transitions[{current_state, symbol}] = next_state;
  }

  return true;
}

bool DFAProblem::accepts(const std::string &word) const {
  // Start at the start state
  // We use an integer representation for simplicity:
  //   q0 -> 0, q1 -> 1, q2 -> 2
  // But we'll map using a lookup approach
  std::string current_state = start_state;

  // Process each character of the input word
  for (char c : word) {
    std::string symbol(1, c); // Convert char to string (e.g., '0' -> "0")

    // Look up the transition in our map
    auto it = transitions.find({current_state, symbol});

    // If no transition is defined for this (state, symbol) pair,
    // the DFA rejects the word immediately
    if (it == transitions.end()) {
      return false; // Dead state - no transition defined
    }

    // Move to the next state
    current_state = it->second;
  }

  // After processing all symbols, check if we're in an accepting state
  // The word is accepted if the final state is in the final_states set
  return final_states.count(current_state) > 0;
}

void DFAProblem::write_result(const std::string &output_filename,
                              const std::string &word, bool accepted) const {
  std::ofstream out_file(output_filename, std::ios::app); // Append mode
  if (!out_file.is_open()) {
    std::cerr << "Error: Cannot open output file: " << output_filename
              << std::endl;
    return;
  }

  out_file << to_upper(accepted ? "IGEN" : "NEM") << std::endl;
}

void DFAProblem::initialize_parser(cxxopts::Options &options) {
  options.add_options()("check", "Check word(s) (comma-separated)",
                        cxxopts::value<std::string>());
}

bool DFAProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
  return args.count("check") > 0;
}

int DFAProblem::run(const cxxopts::ParseResult &args) {
  // Step 1: Ensure input and output files are provided
  if (!args.count("input") || !args.count("output")) {
    std::cerr << "Error: --input and --output are required for DFAProblem"
              << std::endl;
    return 1;
  }

  std::string check_words = args["check"].as<std::string>();
  std::string input_file = args["input"].as<std::string>();
  std::string output_file = args["output"].as<std::string>();

  // Step 2: Initialize DFA state machine from the input file
  if (!load_from_file(input_file)) {
    return 1; // Error loading file, abort
  }

  // Step 3: Parse and process each comma-separated word
  std::istringstream check_stream(check_words);
  std::string word;
  bool first = true;

  // We read the check string word by word, splitting by commas (',')
  while (std::getline(check_stream, word, ',')) {

    // Step 4: Trim any leading or trailing whitespaces just in case
    size_t start = word.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
      continue; // Skip completely empty words
    size_t end = word.find_last_not_of(" \t\r\n");
    word = word.substr(start, end - start + 1);

    // Step 5: Simulate the word through the DFA
    bool acc = accepts(word);

    // Step 6: Write the result to the output file (appends automatically)
    write_result(output_file, word, acc);

    // Step 7: Print the result to the console for user feedback
    if (!first)
      std::cout << std::endl;
    first = false;
    std::cout << word << ": " << (acc ? "IGEN" : "NEM");
  }
  std::cout << std::endl;

  return 0; // Success
}
