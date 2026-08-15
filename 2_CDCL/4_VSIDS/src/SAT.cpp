#include "SAT.hpp"

#include <algorithm>
#include <limits>
#include <unordered_set>
#include <cassert>
#include <iostream>

namespace saturn {
    bool satsolver::solveCNF() {
        //if we can't create watched literals, this equation is unsatisfiable
        if (!createWatched()) {
            return false;
        }

        size_t decisionLevel = 0;

        //start a new decision level at the root
        trail_starts.push_back(0);

        //if createWatched found unit clauses, then they're on the trail
        if (trail.size() > 0) {
            //propagate and if it failed, we can't do anything about it
            if (propagate(decisionLevel).has_value()) {
                return false;
            }
        }

        while (true) {
            if (qHead == trail.size()) {
                decisionLevel++;

                std::optional<int> firstUnassigned;
                double maxScore = -1.0;
                for (size_t i = 0; i < numVars; i++) {
                    if (vars[i] == UNASSIGNED) {
                        if (activity[i] > maxScore) {
                            maxScore = activity[i];
                            //variable = idx + 1
                            firstUnassigned = i + 1;
                        }
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

                //Record the index in the trail where the first unassigned variable went
                var_to_trail[*firstUnassigned - 1] = trail.size() - 1;

                //the newest decision level begins at the index of the last trail entry
                trail_starts.push_back(trail.size() - 1);

                //our propagation queue now starts at the latest decision
                qHead = trail_starts.back();
            }

            const std::optional<std::vector<int>>& contradiction = propagate(decisionLevel);

            //if there's a conflicting clause
            if (contradiction.has_value()) {  
                numConflicts++;
                
                //we now want a reference instead of a copy
                const std::vector<int>& conflict = *contradiction;   

                size_t levelCount = 0;

                for (size_t i = 0; i < conflict.size(); i++) {
                    int var = conflict[i];
                    size_t idx = std::abs(var) - 1;

                    if (seen[idx]) {
                        continue;
                    }

                    seen[idx] = true;
                    seenIdx.push_back(idx);
                    seenPos[idx] = seenIdx.size() - 1;

                    delta[idx] = true;

                    size_t trailIdx = *var_to_trail[idx];

                    if (trail[trailIdx].decisionLevel == decisionLevel) {
                        levelCount++;
                    }
                }

                for (size_t trailIdx = trail.size(); trailIdx-- > 0;) {
                    const TrailEntry& entry = trail[trailIdx];
                    size_t idx = std::abs(entry.lit) - 1;

                    if (!seen[idx]) {
                        continue;
                    }

                    if (levelCount == 1) {
                        break;
                    }

                    const std::vector<int>& reason = clauses[*trail[trailIdx].reasonIdx];

                    for (size_t i = 0; i < reason.size(); i++) {
                        size_t r_idx = std::abs(reason[i]) - 1;

                        if (seen[r_idx]) {
                            continue;
                        }

                        seen[r_idx] = true;
                        seenIdx.push_back(r_idx);
                        seenPos[r_idx] = seenIdx.size() - 1;

                        size_t reasonTrailIdx = *var_to_trail[r_idx];

                        if (trail[reasonTrailIdx].decisionLevel == decisionLevel) {
                            levelCount++;
                        }
                    }

                    delta[idx] = true;

                    size_t pos = seenPos[idx];
                    size_t last = seenIdx.back();

                    seenIdx[pos] = last;
                    seenPos[last] = pos;

                    seenIdx.pop_back();
                    seen[idx] = false;
                    
                    levelCount--;
                }

                std::vector<int> learned;
                learned.reserve(seenIdx.size());

                size_t lbd = 0;

                std::vector<bool> decisionSeen;
                decisionSeen.resize(decisionLevel + 1);

                while (!seenIdx.empty()) {
                    size_t s_idx = seenIdx.back();

                    delta[s_idx] = true;

                    int lit = trail[*var_to_trail[s_idx]].lit;
                    learned.push_back(-lit);

                    seen[s_idx] = false;
                    seenIdx.pop_back();

                    size_t decisionL = trail[*var_to_trail[s_idx]].decisionLevel;

                    if (!decisionSeen[decisionL]) {
                        decisionSeen[decisionL] = true;
                        lbd++;
                    }
                }

                for (size_t i = 0; i < decisionSeen.size(); i++) {
                    if (decisionSeen[i]) {
                        lbd++;
                    }
                }

                if (learned.empty()) {
                    return false;
                }

                //find the second largest decision level
                size_t t_idx = 0;

                for (size_t i = 0; i < learned.size(); i++) {
                    assert(var_to_trail[std::abs(learned[i]) - 1].has_value());
                    size_t decision = trail[*var_to_trail[std::abs(learned[i]) - 1]].decisionLevel;
                    if (decision != decisionLevel) {
                        t_idx = std::max(t_idx, decision);
                    }
                }

                size_t backjumpTo = t_idx;

                //backtracking
                while (!trail.empty() && trail.back().decisionLevel > backjumpTo) {
                    size_t decision_level = trail_starts.back();

                    while (trail.size() > decision_level) {
                        int lit = trail.back().lit;
                        vars[std::abs(lit) - 1] = UNASSIGNED;
                        var_to_trail[std::abs(lit) - 1] = std::nullopt;
                        trail.pop_back();
                    }

                    trail_starts.pop_back();
                }

                decisionLevel = backjumpTo;

                //if we have no more decisions levels or we backtrack to the
                //  root level (which MUST be True), then it's unsatisfiable
                if (trail_starts.empty()) {
                    return false;
                }

                double alpha = 0.1;
                lbdema = (1 - alpha) * lbd + alpha * lbdema;
                double decayFac = 0.0;

                if (lbd > lbdema) {
                    decayFac = 0.75;
                } else {
                    decayFac = 0.99;
                }

                //update activity scores
                for (size_t i = 0; i < delta.size(); i++) {
                    if (delta[i]) {
                        activity[i] = (1 - decayFac) + decayFac * activity[i];
                    } else {
                        activity[i] = decayFac * activity[i];
                    }
                    delta[i] = false;
                }

                qHead = trail_starts.back();

                clauses.push_back(std::move(learned));

                clause_to_var.emplace_back();

                //since all the propagation (on this decision level)
                //  has been undone, we can now initialize watched literals
                //  for this clause
                // assert(createWatched(clauses.size() - 1, decisionLevel));
                if (!createWatched(clauses.size() - 1, decisionLevel)) {
                    assert(decisionLevel == 0);
                    return false;
                }

                //arbitrary values chosen for both conflict count and the decay factor
                // if (numConflicts == 10) {
                //     for (size_t i = 0; i < activity.size(); i++) {
                //         activity[i] *= 0.8;
                //     }
                //     numConflicts = 0;
                // }

                continue;
            }
        }
    }
}