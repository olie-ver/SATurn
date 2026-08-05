#include "SAT.hpp"

#include <algorithm>

namespace saturn {
    bool satsolver::createWatched() {
        for (size_t i = 0; i < numClauses; i++) {
            const std::vector<int>& clause = clauses[i];

            if (clause.size() >= 2) {
                var_to_clause[clause[0]].push_back(i);
                var_to_clause[clause[1]].push_back(i);
                
                clause_to_var[i] = {0, 1};
            } else if (clause.size() == 1) {
                int var = clause[0];
                var_to_clause[var].push_back(i);

                clause_to_var[i] = {0, 0};
                //we want to be able to immediately perform unit propagation if
                //  there are unit clauses
                unitProp.push(var);
                
                int idx = std::abs(var) - 1;
                if (var > 0) {
                    vars[idx] = TRUE;
                } else {
                    vars[idx] = FALSE;
                }

            } else {
                return false;
            }
        }

        return true;
    }
}