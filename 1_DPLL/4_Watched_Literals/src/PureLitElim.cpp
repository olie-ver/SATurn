#include "SAT.hpp"

#include <unordered_map>

namespace saturn {
    std::pair<std::vector<int>, std::vector<int>> 
    satsolver::pureElim(std::vector<var>& assignment) {
        //map from literal to clause indices
        std::unordered_map<int, std::vector<int>> pure_to_clause;

        std::vector<int> pure;
        std::vector<int> pure_satisfied;

        for (size_t i = 0; i < numClauses; i++) {
            if (satisfied[i]) {
                continue;
            }

            const std::vector<int>& clause = clauses[i];

            for (size_t j = 0; j < clause.size(); j++) {
                pure_to_clause[clause[j]].push_back(i);
            }
        }

        for (const auto& [pure_lit, pure_clauses] : pure_to_clause) {
            int idx = std::abs(pure_lit) - 1;

            if (pure_to_clause.contains(-pure_lit) || assignment[idx] != UNASSIGNED) {
                //if this were a multi-pass algorithm, this would be more useful
                // pure_to_clause.erase(pure_lit);
                // pure_to_clause.erase(-pure_lit);
                continue;
            }

            if (pure_lit > 0) {
                assignment[idx] = TRUE;
            } else {
                assignment[idx] = FALSE;
            }

            for (size_t i = 0; i < pure_clauses.size(); i++) {
                satisfied[pure_clauses[i]] = true;
                pure_satisfied.push_back(pure_clauses[i]);
            }

            pure.push_back(pure_lit);
        }

        return std::pair{std::move(pure), std::move(pure_satisfied)};
    }
}