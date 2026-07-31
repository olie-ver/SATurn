#include "SAT.hpp"

#include <algorithm>

namespace saturn {
    bool satsolver::solveCNF() {
        for (size_t i = 0; i < (0b1 << numVars); i++) {
            bool solved = true;

            for (size_t j = 0; j < numClauses; j++) {
                const std::vector<int>& clause = clauses[j];

                bool satisfied_clause = false;
                for (size_t k = 0; k < clause.size(); k++) {
                    int var = clause[k];
                    int idx = std::abs(var) - 1;

                    if (var > 0) {
                        satisfied_clause |= i & (0b1 << idx);
                    } else {
                        satisfied_clause |= !(i & (0b1 << idx));
                    }
                }

                solved &= satisfied_clause;

                if (!solved) {
                    break;
                }
            }

            if (solved) {
                std::vector<bool> assignment;
                assignment.resize(numVars);

                for (size_t j = 0; j < numVars; j++) {
                    assignment[j] = i & (0b1 << j);
                }

                vars = std::move(assignment);

                return true;
            }
        }
        return false;
    }
}