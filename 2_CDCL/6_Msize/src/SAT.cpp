#include "SAT.hpp"

#include <algorithm>
#include <limits>
#include <unordered_set>
#include <cassert>
#include <iostream>

#define RESET 10
#define MAXUNASS 2

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

        size_t curSize = numClauses;

        while (true) {
            if (clauses.size() > static_cast<size_t>(std::floor(curSize * 1.1))) {
                std::vector<bool> remove(clauses.size() - numClauses, false);

                //numClauses = number of clauses in original equation
                for (size_t i = numClauses; i < clauses.size(); i++) {
                    const std::vector<int>& clause = clauses[i];

                    size_t numUnassigned = 0;
                    for (size_t j = 0; j < clause.size(); j++) {
                        int idx = std::abs(clause[j]) - 1;
                        if (vars[idx] == UNASSIGNED) {
                            numUnassigned++;
                        }
                    }

                    if (numUnassigned > MAXUNASS) {
                        remove[i - numClauses] = true;
                    }
                }

                for (size_t i = 0; i < trail.size(); i++) {
                    //invalidate clauses that should be deleted, but are in the trail
                    if (trail[i].reasonIdx.has_value()) {
                        size_t reason = *trail[i].reasonIdx;

                        if (reason >= numClauses) {
                            remove[reason - numClauses] = false;
                        }
                    }
                }

                for (size_t idx = clauses.size(); idx-- > numClauses;) {
                    if (!remove[idx - numClauses]) {
                        continue;
                    }

                    size_t last = clauses.size() - 1;

                    // If we're deleting the last clause, easy.
                    if (idx == last) {
                        removeClauseFromWatchLists(idx);

                        clauses.pop_back();
                        clause_to_var.pop_back();

                        continue;
                    }

                    // Remove idx from its watcher lists.
                    removeClauseFromWatchLists(idx);

                    // Move last -> idx.
                    std::swap(clauses[idx], clauses[last]);
                    std::swap(clause_to_var[idx], clause_to_var[last]);

                    // Update watcher lists: last -> idx.
                    replaceWatchIndex(last, idx);

                    for (size_t i = 0; i < trail.size(); i++) {
                        if (trail[i].reasonIdx.has_value()) {
                            size_t& reasonIdx = *trail[i].reasonIdx;
                            if (reasonIdx == last) {
                                reasonIdx = idx;
                            }
                        }
                    }

                    clauses.pop_back();
                    clause_to_var.pop_back();
                }

                curSize = clauses.size();

                numConflicts = 0;
            }


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

                    assert(trail[trailIdx].reasonIdx.has_value());
                    assert(*trail[trailIdx].reasonIdx < clauses.size());

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

                continue;
            }
        }
    }
}