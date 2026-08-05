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
        std::cout << "created watched" << std::endl;

        //if createWatched found unit clauses, then they're on the trail
        if (trail.size() > 0) {
            //start a new decision level at the root
            trail_starts.push_back({0, false});

            //propagate and if it failed, we can't do anything about it
            if (!propagate()) {
                return false;
            }
        }
        std::cout << "starting solve loop" << std::endl;

        while (true) {
            std::cout << "in solve loop" << std::endl;
            for (TrailEntry t : trail) {
                std::cout << "lit: " << t.lit << ", level: " << t.level << ", reasonIdx: ";
                if (t.reasonIdx.has_value()) {
                    std::cout << *t.reasonIdx;
                } else {
                    std::cout << "decision";
                }
                std::cout << '\n';
            }

            std::cout << "end of trail print" << std::endl;

            for (DecisionLevel d : trail_starts) {
                std::cout << "starting idx: " << d.idx << ", triedFalse: " << d.triedFalse << '\n';
            }

            std::cout << "end of trail_starts print" << std::endl;
            //since root propagation has a decision level of 0, this decision level 
            //  must be at least 1
            size_t decisionLevel = std::max(trail_starts.size(), 1ul);

            std::cout << "decisionLevel: " << decisionLevel << std::endl;

            //try to find the first unassigned variable
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
                std::cout << "solved" << std::endl;
                return true;
            }

            std::cout << "first unassigned variable: " << *firstUnassigned << std::endl;

            //assign this variable to be True (okay because firstUnassigned must be positive => True)
            vars[*firstUnassigned - 1] = TRUE;

            std::cout << "assigned " << *firstUnassigned << " TRUE" << std::endl;

            //push back the first unassigned variable (assumed True), the decision level
            //  and nullopt to show that this was a decision, not propagation/inference
            trail.push_back({*firstUnassigned, decisionLevel, std::nullopt});

            std::cout << "added { var:" << *firstUnassigned << ", level: " << decisionLevel 
                << ", reason: std::nullopt } to trail" << std::endl;  

            //the newest decision level begins at the index of the last trail entry
            trail_starts.push_back({trail.size() - 1, false});

            std::cout << "newest trail start: { " << trail.size() - 1 << ", false }" <<  std::endl;

            //our propagation queue now starts at the latest decision
            qHead = trail_starts.back().idx;

            std::cout << "qHead points at index: " << qHead << std::endl; 

            bool succeeded = propagate();

            std::cout << "finished propagation" << std::endl;

            if (!succeeded) {
                std::cout << "propagation failed" << std::endl;
                while (trail_starts.size() > 0) {
                    DecisionLevel& decision = trail_starts.back();

                    std::cout << "target: " << decision.idx << std::endl;

                    std::cout << "old trail size: " << trail.size() << std::endl;
                    while (trail.size() > decision.idx + 1) {
                        int lit = trail.back().lit;
                        vars[std::abs(lit) - 1] = UNASSIGNED;
                        trail.pop_back();
                    }

                    std::cout << "new trail size: " << trail.size() << std::endl;

                    if (!decision.triedFalse) {
                        std::cout << "haven't tried false yet" << std::endl;
                        TrailEntry& entry = trail.back();

                        entry.lit = -entry.lit;
                        vars[std::abs(entry.lit) - 1] = FALSE;

                        //try the second branch
                        decision.triedFalse = true;

                        qHead = decision.idx;

                        std::cout << "qHead points to " << qHead << std::endl;

                        break;
                    } else {
                        std::cout << "decision failed, going back up one level" << std::endl;

                        int lit = trail.back().lit;
                        vars[std::abs(lit) - 1] = UNASSIGNED;
                        trail.pop_back();
                        trail_starts.pop_back();
                    }
                }

                // if (trail_starts.empty() || trail.back().level == 0) {
                //     std::cout << "root contradiction, returning false" << std::endl;
                //     return false;
                // }
                if (trail_starts.empty()) {
                    std::cout << "root contradiction, returning false" << std::endl;
                    return false;
                }

                continue;
            }

            assert(qHead <= trail.size());

            for (size_t i = qHead; i < trail.size(); ++i)
            {
                int lit = trail[i].lit;
                assert(vars[std::abs(lit)-1] != UNASSIGNED);
            }
        }
    }
}