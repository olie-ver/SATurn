#include "SAT.hpp"

#include <algorithm>

namespace saturn {
    bool satsolver::solveCNF() {
        std::vector<var> assignment;
        assignment.resize(numVars);

        for (size_t i = 0; i < numVars; i++) {
            assignment[i] = UNASSIGNED;
        }

        propagate(assignment);

        return solve(assignment, 0);
    }

    bool satsolver::solve(std::vector<var>& assignment, int level) {
        if (level == numVars - 1) {
            return evaluate(assignment);
        }

        //fresh copy of the current set of clauses
        std::vector<std::vector<int>> backupClauses = clauses;

        //backup copy of the assignment
        std::vector<var> backupAssignment = assignment;

        if (assignment[level] == UNASSIGNED) {
            assignment[level] = TRUE;

            //possibly modifies the current set of clauses
            propagate(assignment);
        }

        if (solve(assignment, level + 1)) {
            return true;
        }

        //restore the clauses to before the propagation
        clauses = backupClauses;
        assignment = backupAssignment;
        

        if (assignment[level] == UNASSIGNED) {
            assignment[level] = FALSE;

            //possibly modify them again
            propagate(assignment);
        }

        if (solve(assignment, level + 1)) {
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