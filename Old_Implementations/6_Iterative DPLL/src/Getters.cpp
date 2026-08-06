#include "SAT.hpp"

#include <iostream>
#include <stdexcept>

namespace SATurn {
    const size_t SATSolver::getNumVars() const {
        return numVars;
    }

    const size_t SATSolver::getNumClauses() const {
        return numClauses;
    }

    const std::vector<std::vector<int>>& SATSolver::getClauses() const {
        return clauses;
    }

    const std::vector<SATSolver::lit>& SATSolver::getSolution() const {
        return assignment;
    }

    void SATSolver::printSolution() {
        if (try_solved) {
            std::cout << "Solution:\n"; 
            for (size_t i = 0; i < assignment.size(); i++) {
                std::cout << "Var " << i + 1 << ": ";
                if (assignment[i]) {
                    std::cout << "true";
                } else {
                    std::cout << "false";
                }
                std::cout << std::endl;
            }
        } else {
            throw std::logic_error{"Invalid state. Call solveCNF() and try again"};
        }
    }
}