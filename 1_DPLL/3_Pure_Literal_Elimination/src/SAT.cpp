#include "SAT.hpp"

#include <algorithm>

namespace saturn {
    bool satsolver::solveCNF() {
        std::vector<var> assignment;
        assignment.resize(numVars);
        satisfied.resize(numClauses);

        for (size_t i = 0; i < numVars; i++) {
            assignment[i] = UNASSIGNED;
        }

        //when we propagate at the root level, everything that we've marked 
        //  satisfied and found unit should NEVER be undone, so we don't 
        //  need to deal with a return value
        //and since there is no variable with label 0, passing it in just 
        //  means "find all clauses with length 1 and propagate from there."
        propagate(assignment, 0);

        //After we propagate, we want to eliminate any pure literals
        //  that may be left over
        pureElim(assignment);

        return solve(assignment, 0);
    }

    bool satsolver::solve(std::vector<var>& assignment, int level) {
        if (level == numVars - 1) {
            return evaluate(assignment);
        }

        //if this variable is learned, continue
        if (assignment[level] != UNASSIGNED) {
            return solve(assignment, level + 1);
        }

        assignment[level] = TRUE;

        const std::pair<std::vector<int>, std::vector<int>>& prop_result = propagate(assignment, level);
        const std::pair<std::vector<int>, std::vector<int>>& elim_result = pureElim(assignment);

        //try solving after propagation
        if (solve(assignment, level + 1)) {
            return true;
        }

        //backtrack
        const std::vector<int>& foundUnit = prop_result.first;
        const std::vector<int>& foundSatisfied = prop_result.second;

        for (size_t i = 0; i < foundUnit.size(); i++) {
            int var = foundUnit[i];
            int idx = std::abs(var) - 1;

            assignment[idx] = UNASSIGNED;
        }

        for (size_t i = 0; i < foundSatisfied.size(); i++) {
            satisfied[foundSatisfied[i]] = false;
        }

        const std::vector<int>& foundPure = elim_result.first;
        const std::vector<int>& foundPureSatisfied = elim_result.second;

        for (size_t i = 0; i < foundPure.size(); i++) {
            int var = foundPure[i];
            int idx = std::abs(var) - 1;

            assignment[idx] = UNASSIGNED;
        }

        for (size_t i = 0; i < foundPureSatisfied.size(); i++) {
            satisfied[foundPureSatisfied[i]] = false;
        }

        //try to solve with False instead of True
        assignment[level] = FALSE;

        const std::pair<std::vector<int>, std::vector<int>>& new_prop_result = propagate(assignment, level);
        const std::pair<std::vector<int>, std::vector<int>>& new_elim_result = pureElim(assignment);

        if (solve(assignment, level + 1)) {
            return true;
        }

        //backtrack
        const std::vector<int>& new_foundUnit = new_prop_result.first;
        const std::vector<int>& new_foundSatisfied = new_prop_result.second;

         for (size_t i = 0; i < new_foundUnit.size(); i++) {
            int var = new_foundUnit[i];
            int idx = std::abs(var) - 1;

            assignment[idx] = UNASSIGNED;
        }

        for (size_t i = 0; i < new_foundSatisfied.size(); i++) {
            satisfied[new_foundSatisfied[i]] = false;
        }

        const std::vector<int>& new_foundPure = new_elim_result.first;
        const std::vector<int>& new_foundPureSatisfied = new_elim_result.second;

        for (size_t i = 0; i < new_foundPure.size(); i++) {
            int var = new_foundPure[i];
            int idx = std::abs(var) - 1;

            assignment[idx] = UNASSIGNED;
        }

        for (size_t i = 0; i < new_foundPureSatisfied.size(); i++) {
            satisfied[new_foundPureSatisfied[i]] = false;
        }

        //undo this decision entirely before returning
        assignment[level] = UNASSIGNED;
        return false;
    }

    bool satsolver::evaluate(const std::vector<var>& assignment) {
        bool solved = true;
        for (size_t j = 0; j < numClauses; j++) {
            if (satisfied[j]) {
                continue;
            }
            const std::vector<int>& clause = clauses[j];

            bool satisfied_clause = false;
            for (size_t k = 0; k < clause.size(); k++) {
                int var = clause[k];

                int idx = std::abs(var) - 1;

                if (var > 0) {
                    satisfied_clause |= assignment[idx] == TRUE;
                } else {
                    satisfied_clause |= assignment[idx] == FALSE;
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