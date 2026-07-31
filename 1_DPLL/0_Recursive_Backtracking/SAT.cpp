#include "SAT.hpp"

#include <algorithm>

namespace saturn {
    bool satsolver::solveCNF() {
        std::vector<bool> assignment;
        assignment.reserve(numVars);

        return solve(assignment);
    }

    bool satsolver::solve(std::vector<bool>& assignment) {
        if (assignment.size() == numVars) {
            return evaluate(assignment);
        }

        assignment.push_back(true);

        if (solve(assignment)) {
            return true;
        }

        assignment.pop_back();
        assignment.push_back(false);

        if (solve(assignment)) {
            return true;
        }

        assignment.pop_back();
        return false;
    }

    bool satsolver::evaluate(const std::vector<bool>& assignment) {
        bool solved = true;
        for (size_t j = 0; j < numClauses; j++) {
            const std::vector<int>& clause = clauses[j];

            bool satisfied_clause = false;
            for (size_t k = 0; k < clause.size(); k++) {
                int var = clause[k];

                int idx = std::abs(var) - 1;

                if (var > 0) {
                    satisfied_clause |= assignment[idx];
                } else {
                    satisfied_clause |= !assignment[idx];
                }
            }
            solved &= satisfied_clause;
        }

        if (solved) {
            vars = std::move(assignment);
            return true;
        }
        return false;
    }
}