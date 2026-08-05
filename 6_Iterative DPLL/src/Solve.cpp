#include "SAT.hpp"

#include <algorithm>
#include <iostream>

namespace SATurn {
    bool SATSolver::solveCNF() {
        try_solved = true;
        assignment.resize(numVars);
        varData.resize(numVars);

        for (size_t i = 0; i < assignment.size(); i++) {
            assignment[i] = Unassigned;
        }

        if (!create_watched()) {
            return false;
        }

        while (true) {
            const std::optional<std::vector<int>>& delta = propagate();

            // !delta.has_value() => conflict
            if (!delta.has_value()) {
                //try to backtrack
                while (decisions.size() > 0) {
                    DecisionLevel& decision_level = decisions.back();

                    while (trail.size() > decision_level.level_start + 1)
                    {
                        int lit = trail.back().lit;
                        assignment[std::abs(lit) - 1] = Unassigned;
                        // varData[std::abs(lit) - 1].trail_index = -1;
                        trail.pop_back();
                    }

                    if (!decision_level.tried_second_branch) {
                        TrailEntry& decision = trail.back();
                        decision.lit = -decision.lit;
                        assignment[std::abs(decision.lit) - 1] = False;

                        //try the second branch
                        decision_level.tried_second_branch = true;

                        qhead = decision_level.level_start;
                        break;
                    } else {
                        int lit = trail.back().lit;
                        assignment[std::abs(lit) - 1] = Unassigned;
                        // varData[std::abs(lit) - 1].trail_index = -1;
                        trail.pop_back();
                        decisions.pop_back();
                    }
                }

                if (decisions.empty()) {
                    return false;
                }

                continue;
            }

            //check if all are assigned
            size_t first_unassigned = 0;
            for (size_t i = 0; i < assignment.size(); i++) {
                if (assignment[i] == Unassigned) {
                    first_unassigned = i + 1;
                    break;
                }
            }

            //if all are assigned, return true
            if (!first_unassigned) {
                return true;
            }

            //otherwise, make a new decision
            assignment[first_unassigned - 1] = True;
            trail.push_back({static_cast<int>(first_unassigned), nullptr});
            // varData[first_unassigned - 1].trail_index = trail.size() - 1;
            decisions.push_back({trail.size() - 1, false});
            //mark the newest decision as the starting point for propagation
            qhead = trail.size() - 1;
        }
    }
}