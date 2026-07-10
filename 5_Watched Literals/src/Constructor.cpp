#include "SAT.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace SATurn {
    SATSolver::SATSolver(std::string_view param) {
        if (std::filesystem::exists(param)) {
            //treat it as a file
            std::ifstream file(param.data());

            if (!file.is_open()) {
                std::cerr << "Error: Could not open the file!" << std::endl;
                std::abort();
            }

            std::streamsize size = std::filesystem::file_size(param.data());
            std::string content(size, '\0');

            if (file.read(&content[0], size)) {
                parse(std::string_view(content.data(), content.size()));
                file.close();
            } else {
                std::cerr << "Couldn't read file" << std::endl;
                std::abort();
            }
        } else {
            //treat it as an equation
            parse(param);
        }
    }

    void SATSolver::parse(std::string_view equation) {
        const char* begin = equation.data();
        const char* end = begin + equation.size();
        const char* ptr;

        size_t start = equation.find("p cnf");
        if (start == std::string::npos) {
            ptr = begin;

            bool reading_comment = *ptr == 'c';
            bool EOS = !reading_comment && *ptr == '%';

            std::vector<int> current_clause;
            size_t num_vars{};
            size_t num_clauses{};

            while (ptr < end && !EOS) {
                if (reading_comment) {
                    //skip to new line
                    while (ptr < end && *ptr != '\n') {
                        ++ptr;
                    }

                    //*ptr == '\n'
                    reading_comment = false;
                }

                int lit;
                auto [next, ec] = std::from_chars(ptr, end, lit);
                if (ec == std::errc{}) {
                    if (lit == 0) {
                        // if (!current_clause.empty()) {
                            clauses.push_back(std::move(current_clause));
                            current_clause.clear();
                            num_clauses++;
                        // }
                    } else {
                        num_vars = std::max(num_vars, static_cast<size_t>(std::abs(lit)));

                        current_clause.push_back(lit);
                    }

                    ptr = next;
                } else {
                    ++ptr;
                }

                reading_comment = ptr < end && *ptr == 'c';
                EOS = !reading_comment && *ptr == '%';
            }

            numVars = num_vars;
            numClauses = num_clauses;
        } else { //header exists
            ptr = begin + start;

            bool reading_comment = *ptr == 'c';
            bool EOS = !reading_comment && *ptr == '%';

            //assuming there is a header
            while (ptr < end) {
                if (reading_comment) {
                    //skip to new line
                    while (ptr < end && *ptr != '\n') {
                        ++ptr;
                    }

                    reading_comment = false;
                }

                int vars;
                auto [next, ec] = std::from_chars(ptr, end, vars);
                if (ec == std::errc{}) {
                    if (vars < 0) {
                        std::cerr << "Can't have a negative number of literals\n";
                        std::abort();
                    }

                    numVars = vars; 
                    ptr = next;
                    break;
                } else {
                    ++ptr;
                }
            }

            reading_comment = ptr < end && *ptr == 'c';

            while (ptr < end) {
                if (reading_comment) {
                    //skip to new line
                    while (ptr < end && *ptr != '\n') {
                        ++ptr;
                    }

                    //*ptr == '\n'
                    reading_comment = false;
                }

                int clauses;
                auto [next, ec] = std::from_chars(ptr, end, clauses);
                if (ec == std::errc{}) {
                    if (clauses < 0) {
                        std::cerr << "Can't have a negative number of clauses: " << clauses << '\n';
                        std::abort();
                    }

                    numClauses = clauses; 
                    ptr = next;
                    break;
                } else {
                    ++ptr;
                }
            }

            reading_comment = ptr < end && *ptr == 'c';
            EOS = !reading_comment && *ptr == '%';

            std::vector<int> current_clause;
            while (ptr < end && !EOS) {
                if (reading_comment) {
                    //skip to new line
                    while (ptr < end && *ptr != '\n') {
                        ++ptr;
                    }

                    //*ptr == '\n'
                    reading_comment = false;
                }

                int lit;
                auto [next, ec] = std::from_chars(ptr, end, lit);
                if (ec == std::errc{}) {
                    if (lit == 0) {
                        // if (!current_clause.empty()) {
                            clauses.push_back(std::move(current_clause));

                            if (clauses.size() > numClauses) {
                                std::cerr << "Number of clauses larger than from header" << std::endl;
                                std::abort();
                            }

                            current_clause.clear();
                        // }
                    } else {
                        if (std::abs(lit) > numVars) {
                            std::cerr << "Variable index is larger than number of variables: " 
                                << std::abs(lit) << std::endl;
                            std::abort();
                        }

                        current_clause.push_back(lit);
                    }

                    ptr = next;
                } else {
                    ++ptr;
                }

                reading_comment = ptr < end && *ptr == 'c';
                EOS = !reading_comment && *ptr == '%';
            }

            if (!current_clause.empty()) {
                clauses.push_back(std::move(current_clause));

                if (clauses.size() > numClauses) {
                    std::cerr << "Number of clauses larger than from header" << std::endl;
                    std::abort();
                }

                current_clause.clear();
            }
        }

        satisfied.resize(numClauses);
    }
}