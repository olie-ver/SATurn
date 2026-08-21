#include "SAT.hpp"

#include <algorithm>
#include <limits>
#include <unordered_set>
#include <cassert>
#include <iostream>

#define MAXUNASS 2

#define MAXINCREASE 1e100
#define RESET_INCREASE 1e-100

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

        for (size_t i = 0; i < numClauses; i++) {
            const std::vector<int>& clause = clauses[i];

            for (size_t j = 0; j < clause.size(); j++) {
                size_t idx = std::abs(clause[j]) - 1;

                //clause[j] < 0 = True = 1 => 2 * 1 - 1 = 1
                //clause[j] < 0 = False = 0 => 2 * 0 - 1 = -1
                polarity[idx] = 2 * (clause[j] < 0) - 1;
            }
        }

        size_t totalConflicts = 0;
        size_t curSize = numClauses;

        while (true) {
            //if the clause activity increase is too large, reset it so numbers don't 
            //  grow stupidly large
            if (activeInc >= MAXINCREASE) {
                for (size_t i = 0; i < cActivity.size(); i++) {
                    cActivity[i] *= RESET_INCREASE;
                }
                activeInc *= RESET_INCREASE;
            }

            if (clauses.size() > static_cast<size_t>(std::floor(curSize * 1.1))) {
                size_t numCanDelete = clauses.size() - curSize;
                
                std::vector<bool> remove(clauses.size() - numClauses, false);

                //create an array of indices 
                std::vector<size_t> candidates(clauses.size() - numClauses, 0);
                for (size_t i = 1; i < candidates.size(); i++) {
                    candidates[i] = i;
                }

                size_t cutoff = static_cast<size_t>(candidates.size() * 0.25);

                std::nth_element(candidates.begin(), candidates.begin() + cutoff, 
                    candidates.end(), 
                    [&](size_t a, size_t b) {
                        return cActivity[a] < cActivity[b];
                    }
                );

                double activityCutoff = cActivity[candidates[cutoff]];

                std::nth_element(candidates.begin(), candidates.begin() + cutoff,
                    candidates.end(),
                    [&](size_t a, size_t b) {
                        //smaller creationConflict => larger age
                        return creationConflict[a] < creationConflict[b];
                    }
                );

                size_t ageCutoff = creationConflict[candidates[cutoff]];

                std::nth_element(candidates.begin(), candidates.begin() + cutoff,
                    candidates.end(),
                    [&](size_t a, size_t b) {
                        return lbds[a] > lbds[b];
                    }
                );

                size_t lbdCutoff = lbds[candidates[cutoff]];

                for (size_t i = numClauses; i < clauses.size(); i++) {
                    bool lowActivity = cActivity[i] <= activityCutoff;
                    bool highLBD = lbds[i] >= lbdCutoff;
                    bool old = creationConflict[i] <= ageCutoff;

                    int badness = lowActivity + highLBD + old;

                    //we will mark the really bad clauses as ones to be deleted
                    remove[i - numClauses] = badness >= 2;
                }

                //numClauses = number of clauses in original equation
                for (size_t i = numClauses; i < clauses.size(); i++) {
                    //don't delete clauses with low LBD
                    if (lbds[i - numClauses] <= 2) {
                        remove[i - numClauses] = false;
                        continue;
                    }

                    if (remove[i - numClauses]) {
                        continue;
                    }

                    const std::vector<int>& clause = clauses[i];

                    size_t numUnassigned = 0;
                    for (size_t j = 0; j < clause.size(); j++) {
                        int idx = std::abs(clause[j]) - 1;
                        if (vars[idx] == UNASSIGNED) {
                            numUnassigned++;
                        }
                    }

                    // if (numUnassigned > MAXUNASS  && clause.size() > maxSize) {
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

                    //once we've deleted enough clauses, stop 
                    if (!numCanDelete) {
                        // std::cout << "deleted as much as possible" << std::endl; 
                        break;
                    }

                    //we're going to delete this clause, so just decrement up here
                    numCanDelete--;

                    size_t last = clauses.size() - 1;

                    // If we're deleting the last clause, easy.
                    if (idx == last) {
                        removeClauseFromWatchLists(idx);

                        clauses.pop_back();
                        clause_to_var.pop_back();

                        //remove its data from the clause VSIDS data structures
                        cActivity.pop_back();
                        creationConflict.pop_back();
                        lbds.pop_back();

                        continue;
                    }

                    // Remove idx from its watcher lists.
                    removeClauseFromWatchLists(idx);

                    // Move last -> idx.
                    std::swap(clauses[idx], clauses[last]);
                    std::swap(clause_to_var[idx], clause_to_var[last]);

                    std::swap(cActivity[idx], cActivity[last]);
                    std::swap(creationConflict[idx], creationConflict[last]);
                    std::swap(lbds[idx], lbds[last]);

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

                    cActivity.pop_back();
                    creationConflict.pop_back();
                    lbds.pop_back();
                }

                curSize = clauses.size();

                //Restart code:
                // std::cout << "restarting" << std::endl;
                trail.clear();
                trail_starts.resize(1);
                // var_to_trail.clear();
                qHead = 0;
                decisionLevel = 0;

                for (size_t i = 0; i < numVars; i++) {
                    vars[i] = UNASSIGNED;
                }

                propagate(decisionLevel);
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

                size_t aIdx = *firstUnassigned - 1;

                //polarity[aIdx] >= 0 = True = 0b01 = TRUE
                //polarity[aIdx] >= 0 = False = 0b00 = FALSE
                vars[aIdx] = polarity[aIdx] >= 0;

                int sign = (polarity[aIdx] >= 0) - (polarity[aIdx] < 0);

                assert(sign != 0);

                int aLit = sign * *firstUnassigned;

                //push back the first unassigned variable (assumed True), the decision level
                //  and nullopt to show that this was a decision, not propagation/inference
                trail.push_back({aLit, decisionLevel, std::nullopt});

                //Record the index in the trail where the first unassigned variable went
                var_to_trail[aIdx] = trail.size() - 1;

                //the newest decision level begins at the index of the last trail entry
                trail_starts.push_back(trail.size() - 1);

                //our propagation queue now starts at the latest decision
                qHead = trail_starts.back();
            }

            const std::optional<std::vector<int>>& contradiction = propagate(decisionLevel);

            //if there's a conflicting clause
            if (contradiction.has_value()) {  
                totalConflicts++;

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

                    size_t reasonIdx = *trail[trailIdx].reasonIdx;

                    //if the reason clause is a learned clause, update its activity
                    if (reasonIdx >= numClauses) {
                        cActivity[reasonIdx - numClauses] += activeInc;
                    }

                    const std::vector<int>& reason = clauses[reasonIdx];

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

                    //update a literal's polarity count
                    polarity[s_idx] += 2 * (lit < 0) - 1;
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

                //need to update the size of our clause activity data structures
                cActivity.emplace_back();
                creationConflict.emplace_back(totalConflicts);
                //add this learned clause's lbd to the lbd array
                lbds.emplace_back(lbd);

                //since all the propagation (on this decision level)
                //  has been undone, we can now initialize watched literals
                //  for this clause
                // assert(createWatched(clauses.size() - 1, decisionLevel));
                if (!createWatched(clauses.size() - 1, decisionLevel)) {
                    assert(decisionLevel == 0);
                    return false;
                }

                //bump up the activity increase after each conflict
                activeInc *= activeGrow;

                continue;
            }
        }
    }
}