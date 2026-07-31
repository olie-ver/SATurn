#include "SAT.hpp"

#include <algorithm>

namespace saturn {
    bool satsolver::solveCNF() {
        for (size_t i = 0; i < 0b1 << numVars; i++) {
            bool solved = true;
            for (size_t j = 0; j < numClauses; j++) {
                const std::vector<int>& clause = clauses[j];

                bool satisfied_clause = false;
                for (size_t k = 0; k < clause.size(); k++) {
                    if (clause[k] > 0) {
                        satisfied_clause |= i & (0b1 << numVars);
                    } else {
                        satisfied_clause |= !(i & (0b1 << numVars));
                    }
                }

                solved &= satisfied_clause;

                if (!solved) {
                    break;
                }
            }

            if (solved) {
                (*vars).reserve(numVars);
                
                for (size_t j = 0; j < numVars; j++) {
                    (*vars)[j] = i & (0b1 << j);
                }

                return true;
            }
        }
        return false;
    }
}