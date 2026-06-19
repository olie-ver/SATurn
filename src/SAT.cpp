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
        // see if the header is there
        size_t start = equation.find("p cnf");

        //create the start pointer
        const char* ptr = equation.data() + start;

        //create the end pointer to be one character past the actual end of the input
        const char* end = ptr + equation.size() + 1;

        //ensure we're not currently reading a comment
        bool reading_comment = *ptr == 'c';
        if (start == std::string::npos) {
            std::cout << "no header\n";

            size_t num_vars = 0;
            size_t num_clauses = 0;
            std::vector<int> current_clause;

            while (ptr < end) {
                int lit;
                if (reading_comment) {
                    std::cout << "reading comment\n";
                    //skip to next newline
                    while (*ptr != '\n') {
                        ++ptr;
                    }
                    //we are no longer reading the comment
                    std::cout << "skipped comment\n";
                    reading_comment = false;
                } else {
                    auto [next, ec] = std::from_chars(ptr, end, lit);
                    if (ec == std::errc{}) {
                        if (lit == 0) {
                            //push back the clause
                            num_clauses++;
                            clauses.push_back(std::move(current_clause));
                            current_clause.clear();
                        } else {
                            //num_vars = max(num_vars, abs(lit))
                            num_vars = std::max(num_vars, static_cast<size_t>(std::abs(lit)));
                            //add the current lit to the clause
                            current_clause.push_back(lit);
                        }

                        ptr = next;
                    } else {
                        ++ptr;
                    }
                    reading_comment = *ptr == 'c';
                }
            }

            //if the current_clause isn't empty => we were reading in 
            //  a new clause up to end of file
            if (!current_clause.empty()) {
                num_clauses++;
                clauses.push_back(std::move(current_clause));
                current_clause.clear();
            }

            //assign our fields
            numVars = num_vars;
            numClauses = num_clauses;
        } else {
            //header found => first two numbers are: number of literals, number of clauses
            std::cout << "header found\n";
            //get the number of variables
            while (ptr < end) {
                if (reading_comment) {
                    std::cout << "reading comment\n";
                    //skip to next newline
                    while (*ptr != '\n') {
                        ++ptr;
                    }
                    std::cout << "skipped comment\n";
                    //we are no longer reading the comment
                    reading_comment = false;
                } else {
                    int lit;
                    auto [next, ec] = std::from_chars(ptr, end, lit);
                    if (ec == std::errc{}) {
                        if (lit < 0) {
                            std::cerr << "Can't have a negative number of literals\n";
                            std::abort();
                        }  

                        numVars = lit;
                        //move ptr to the position of the place where from_chars() stopped parsing
                        ptr = next;
                        //break out of this loop
                        break;
                    }
                    else {
                        //increment ptr to move to the next character
                        ++ptr;
                    }
                }
            }

            reading_comment = *ptr == 'c';  

            //get the number of clauses
            while (ptr < end) {
                if (reading_comment) {
                    std::cout << "reading comment\n";
                    //skip to next newline
                    while (*ptr != '\n') {
                        ++ptr;
                    }
                    std::cout << "skipped comment\n";
                    //we are no longer reading the comment
                    reading_comment = false;
                } else {
                    int lit;
                    auto [next, ec] = std::from_chars(ptr, end, lit);
                    if (ec == std::errc{}) {
                        if (lit < 0) {
                            std::cerr << "Can't have a negative number of clauses\n";
                            std::abort();
                        }  
                        numClauses = lit;

                        //move ptr to the position of the place where from_chars() stopped parsing
                        ptr = next;
                        break;
                    }
                    else {
                        //increment ptr to move to the next character
                        ++ptr;
                    }
                }
            } 

            reading_comment = *ptr == 'c';

            //create a temp clause
            std::vector<int> current_clause;
            while (ptr < end) {
                if (reading_comment) {
                    std::cout << "reading comment\n";
                    while (*ptr != 'c') {
                        ++ptr;
                    }
                    std::cout << "skipped comment\n";
                    reading_comment = false;
                } else {
                    //create a temp literal/variable
                    int lit;
                
                    //next position, error code = first numeric character sequence between ptr and end
                    auto [next, ec] = std::from_chars(ptr, end, lit);

                    //if there is no error code
                    if (ec == std::errc{}) {
                        //if lit == 0
                        if (lit == 0) {
                            //our current clause has concluded by DIMACS CNF convention
                            //  so move its data into the SAT solver's clauses
                            clauses.push_back(std::move(current_clause));

                            //clear the current clause so we can reuse it
                            current_clause.clear();
                        }
                        else {
                            //otherwise, our current clause hasn't concluded,
                            //  so add lit to our current clause
                            current_clause.push_back(lit);
                        }
                        //move ptr to the position of the place where from_chars() stopped parsing
                        ptr = next;
                    }
                    else {
                        //increment ptr to move to the next character
                        ++ptr;
                    }
                    reading_comment = *ptr == 'c';
                }
            }
        }
    }

    const size_t SATSolver::getNumVars() const {
        return numVars;
    }

    const size_t SATSolver::getNumClauses() const {
        return numClauses;
    }

    const std::vector<std::vector<int>>& SATSolver::getClauses() const {
        return clauses;
    }
}