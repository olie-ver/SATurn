#include "SAT.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>

namespace saturn {
    bool satsolver::solveCNF() {
        vars.resize(numVars);

        for (size_t i = 0; i < numVars; i++) {
            vars[i] = UNASSIGNED;
        }

        //if we can't create watched literals, this equation is unsatisfiable
        if (!createWatched()) {
            return false;
        }

        //if createWatched found unit clauses, then they're on the trail
        if (trail.size() > 0) {
            //start a new decision level at the root
            trail_starts.push_back({0, false});

            //propagate and if it failed, we can't do anything about it
            if (!propagate(0)) {
                return false;
            }
        }

        while (true) {
            //since root propagation has a decision level of 0, this decision level 
            //  must be at least 1
            size_t decisionLevel = std::max(trail_starts.size(), 1ul);

            // std::cout << "decisionLevel: " << decisionLevel << std::endl;

            std::cout << qHead << " " << trail.size() << std::endl;

            if (qHead == trail.size()) {
                std::optional<int> firstUnassigned;
                for (size_t i = 0; i < numVars; i++) {
                    if (vars[i] == UNASSIGNED) {
                        //variable = idx + 1
                        firstUnassigned = i + 1;
                        break;
                    }
                }

                //if there is none => all assigned => satisfied
                if (!firstUnassigned.has_value()) {
                    return true;
                }

                //assign this variable to be True (okay because firstUnassigned must be positive => True)
                vars[*firstUnassigned - 1] = TRUE;

                //push back the first unassigned variable (assumed True), the decision level
                //  and nullopt to show that this was a decision, not propagation/inference
                trail.push_back({*firstUnassigned, decisionLevel, std::nullopt});

                //the newest decision level begins at the index of the last trail entry
                trail_starts.push_back({trail.size() - 1, false});

                //our propagation queue now starts at the latest decision
                qHead = trail_starts.back().idx;

                for (size_t i = qHead; i < trail.size(); i++) {
                    int lit = trail[i].lit;
                    int idx = std::abs(lit) - 1;

                    assert(vars[idx] != UNASSIGNED);
                }

                for (int v = 1; v <= numVars; v++) {
                    bool onTrail = false;

                    for (const auto& t : trail)
                        if (std::abs(t.lit) == v)
                            onTrail = true;

                    assert(onTrail == (vars[v-1] != UNASSIGNED));
                }

            }
            // assert(qHead == trail.size());
            //try to find the first unassigned variable
        
            bool succeeded = propagate(decisionLevel);

            if (!succeeded) {
                while (trail_starts.size() > 0) {
                    DecisionLevel& decision = trail_starts.back();

                    while (trail.size() > decision.idx + 1) {
                        int lit = trail.back().lit;
                        vars[std::abs(lit) - 1] = UNASSIGNED;
                        trail.pop_back();
                    }

                    if (!decision.triedFalse) {
                        TrailEntry& entry = trail.back();

                        entry.lit = -entry.lit;
                        vars[std::abs(entry.lit) - 1] = FALSE;

                        //try the second branch
                        decision.triedFalse = true;

                        qHead = decision.idx;
                        break;
                    } else {
                        int lit = trail.back().lit;
                        vars[std::abs(lit) - 1] = UNASSIGNED;
                        trail.pop_back();
                        trail_starts.pop_back();
                    }
                }

                //if we have no more decisions levels or we backtrack to the
                //  root level (which MUST be True), then it's unsatisfiable
                if (trail_starts.empty() || trail.back().level == 0) {
                    return false;
                }

                continue;
            }


            // //try to find the first unassigned variable
            // std::optional<int> firstUnassigned;
            // for (size_t i = 0; i < numVars; i++) {
            //     if (vars[i] == UNASSIGNED) {
            //         //variable = idx + 1
            //         firstUnassigned = i + 1;
            //         break;
            //     }
            // }

            // //if there is none => all assigned => satisfied
            // if (!firstUnassigned.has_value()) {
            //     return true;
            // }

            // //assign this variable to be True (okay because firstUnassigned must be positive => True)
            // vars[*firstUnassigned - 1] = TRUE;

            // //push back the first unassigned variable (assumed True), the decision level
            // //  and nullopt to show that this was a decision, not propagation/inference
            // trail.push_back({*firstUnassigned, decisionLevel, std::nullopt});

            // //the newest decision level begins at the index of the last trail entry
            // trail_starts.push_back({trail.size() - 1, false});

            // //our propagation queue now starts at the latest decision
            // qHead = trail_starts.back().idx;
        }
    }
}