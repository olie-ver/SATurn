#include "SAT.hpp"

#include <algorithm>
#include <unordered_set>
#include <cassert>

namespace saturn {
    bool satsolver::solveCNF() {
        vars.resize(numVars);
        var_to_trail.resize(numVars);

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
            trail_starts.push_back({0, true});

            //propagate and if it failed, we can't do anything about it
            if (!propagate()) {
                return false;
            }
        }

        while (true) {
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
                trail.push_back({*firstUnassigned, std::nullopt});

                //Record the index in the trail where the first unassigned variable went
                var_to_trail[*firstUnassigned - 1] = trail.size() - 1;

                //the newest decision level begins at the index of the last trail entry
                trail_starts.push_back({trail.size() - 1, false});

                //our propagation queue now starts at the latest decision
                qHead = trail_starts.back().idx;
            }

            const std::optional<std::vector<int>>& contradiction = propagate();

            //if there's a conflicting clause
            if (contradiction.has_value()) {
                //we want a copy instead of a reference since we'll be mutating the conflict 
                //  clause into a learned clause
                std::vector<int> conflict = *contradiction;

                bool propagated_found = true;
                while (true) {   
                    propagated_found = false;
                    size_t idx = 0;
                    //find the most recently propagated literal in the clause
                    for (size_t i = 0; i < conflict.size(); i++) {
                        int var = conflict[i];
                        int v_idx = std::abs(var) - 1;

                        size_t pos = *var_to_trail[v_idx];
                        
                        //if the trail at the trail index for this variable was propagated
                        if (trail[pos].reasonIdx.has_value()) {
                            propagated_found = true;
                            idx = std::max(idx, static_cast<size_t>(pos));
                        }
                    }

                    if (!propagated_found) {
                        break;
                    }

                    //we're resolving on this lit
                    int lit = trail[idx].lit;

                    const std::vector<int>& reason = clauses[*trail[idx].reasonIdx];

                    //insert the reason clause into the end of conflict clause
                    conflict.insert(conflict.end(), reason.begin(), reason.end());

                    //remove the lit we're resolving on
                    for (size_t i = 0; i < conflict.size();) {
                        if (std::abs(conflict[i]) == std::abs(lit)) {
                            std::swap(conflict[i], conflict.back());
                            conflict.pop_back();
                        } else {
                            i++;
                        }
                    }

                    //remove all duplicates
                    std::unordered_set<int> duplicates;
                    duplicates.reserve(conflict.size());
                    for (size_t i = 0; i < conflict.size();) {
                        if (duplicates.contains(conflict[i])) {
                            std::swap(conflict[i], conflict.back());
                            conflict.pop_back();
                        } else {
                            duplicates.insert(conflict[i]);
                            i++;
                        }
                    }
                }

                //backtracking
                while (trail_starts.size() > 0) {
                    DecisionLevel& decision = trail_starts.back();

                    while (trail.size() > decision.idx + 1) {
                        int lit = trail.back().lit;
                        vars[std::abs(lit) - 1] = UNASSIGNED;
                        var_to_trail[std::abs(lit) - 1] = std::nullopt;
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
                        var_to_trail[std::abs(lit) - 1] = std::nullopt;
                        trail.pop_back();
                        trail_starts.pop_back();
                    }
                }

                //if we have no more decisions levels or we backtrack to the
                //  root level (which MUST be True), then it's unsatisfiable
                if (trail_starts.empty()) {
                    return false;
                }

                //at this point, we should now have a clause containing
                //  only decision literals
                clauses.push_back(std::move(conflict));

                //since all the propagation (on this decision level)
                //  has been undone, we can now initialize watched literals
                //  for this clause
                assert(createWatched(clauses.size() - 1));

                continue;
            }
        }
    }
}