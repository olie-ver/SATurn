#include "SAT.hpp"

#include <algorithm>
#include <iostream>

namespace SATurn {
    bool SATSolver::solveCNF() {
        std::vector<bool> solution(numVars);
        unsigned long long iterations = 0b1 << numVars;

        bool solution_found = false;

        for (unsigned long long curIter = 0; curIter < iterations && !solution_found; curIter++) {
            bool solved = true;
            //set the current combination of bits*
            for (size_t idx = 0; idx < numVars; idx++) {
                solution[idx] = curIter & (1 << idx);
            }
            
            for (size_t i = 0; i < clauses.size(); i++) {
                bool true_clause = false;
                //evaluate whether or not the current clause evaluates to true
                for (size_t j = 0; j < clauses[i].size(); j++) {
                    int val = clauses[i][j];
                    int idx = std::abs(val) - 1;
                    if (val < 0) {
                        true_clause |= !solution[idx];
                    } else {
                        true_clause |= solution[idx];
                    }
                }
                //check if the whole equation evaluates to true
                solved &= true_clause;
            }
            solution_found = solved;
        }

        //move the current solution to our literal assignment
        lits = std::move(solution);
        solution.clear();

        return solution_found;
    }
}

//*curIter is a binary number of numVars bits from 0b0...0 to 0b1...1
//  the loop basically takes that binary number and stores the nth bit into the nth boolean
//  setting it to be either 1 or 0 (true or false), thus that code does generate every
//  possible boolean combination