#include "SAT.hpp"

#include <iostream>

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

    const std::optional<std::vector<bool>>& SATSolver::getSolution() const {
        return lits;
    }

    void SATSolver::printSolution() {
        if (lits.has_value()) {
            std::cout << "Solution:\n"; 
            for (size_t i = 0; i < lits->size(); i++) {
                std::cout << "Var " << i + 1 << ": ";
                if ((*lits)[i]) {
                    std::cout << "true";
                } else {
                    std::cout << "false";
                }
                std::cout << std::endl;
            }
        } else {
            std::cout << "Invalid state. Call solveCNF() and try again" << std::endl;
        }
    }
}