#include "SAT.hpp"

#include <algorithm>

namespace saturn {
    bool satsolver::solveCNF() {
        std::vector<var> assignment;
        assignment.reserve(numVars);

        return solve(assignment);
    }

    bool satsolver::solve(std::vector<var>& assignment) {
        if (assignment.size() == numVars) {
            return evaluate(assignment);
        }

        //backup copy of the assignment
        std::vector<var> backupAssignment = assignment;

        assignment.push_back(TRUE);

        //fresh copy of the current set of clauses
        std::vector<std::vector<int>> backupClauses;
        backupClauses = clauses;

        //possibly modifies the current set of clauses
        propagate(assignment);

        if (solve(assignment)) {
            return true;
        }

        assignment = backupAssignment;
        assignment.push_back(FALSE);

        //restore the clauses to before the propagation
        clauses = backupClauses;

        //possibly modify them again
        propagate(assignment);

        if (solve(assignment)) {
            return true;
        }

        //restore them to what they were before we backtrack and try again later
        clauses = backupClauses;

        assignment = backupAssignment;
        return false;
    }

    bool satsolver::evaluate(const std::vector<var>& assignment) {
        bool solved = true;
        for (size_t j = 0; j < numClauses; j++) {
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