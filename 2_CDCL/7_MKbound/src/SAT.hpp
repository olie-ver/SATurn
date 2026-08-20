#pragma once

#ifndef SAT_H
#define SAT_H

#include <algorithm>
#include <bitset>
#include <optional>
#include <queue>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace saturn {
    class satsolver {
        public:
            satsolver(std::string_view input);
            bool solveCNF();

        private:
            //we need 2 bits to represent a variable's assignment, 
            // and I'll be damned if I use 8 bits instead of 2.
            typedef std::bitset<2> var;

            //The three states a variable can have
            static constexpr var FALSE{0b00};
            static constexpr var TRUE{0b01};
            static constexpr var UNASSIGNED{0b10};

            /// @brief Evaluates whether or not a variable evaluates to False
            /// @param var a variable
            /// @return True iff the passed in variable would evaluate to False
            ///             False otherwise
            constexpr bool evals_false(int var) {
                int idx = std::abs(var) - 1;
                return var > 0 && vars[idx] == FALSE || var < 0 && vars[idx] == TRUE;
            };

            /// @brief Maps a variable to an index in var_to_clause
            /// @param var a variable
            /// @return an index in var_to_clause 
            constexpr size_t var_to_widx(int lit) {
                size_t var = std::abs(lit) - 1;
                return 2 * var + (lit < 0);
            };

            struct TrailEntry {
                //the literal being assigned (eg, 5 or -5)
                int lit;

                //In order to implement backjumping, we need to formalize decision levels now
                size_t decisionLevel;

                //the index of the clause for why it was assigned that way
                //reasonIdx.hasValue() == False => decision, else => propagation
                std::optional<size_t> reasonIdx;
            };

            void parse(std::string_view equation);

            std::optional<std::vector<int>> propagate(size_t decisionLevel);

            bool createWatched();

            bool createWatched(size_t index, size_t decisionLevel);

            //helpers:
            void removeFromWatchList(std::vector<size_t>& watchList, size_t clauseIdx);
            void removeClauseFromWatchLists(size_t clauseIdx);
            void replaceWatchIndex(size_t oldIdx, size_t newIdx);

            size_t numVars{};
            size_t numClauses{};
            size_t maxSize{};
            size_t qHead{};

            std::vector<std::vector<int>> clauses;
            std::vector<var> vars;

            std::vector<double> activity;
            uint numConflicts{};

            //index i is whether or not we've seen variable i + 1 in clause resolution
            std::vector<bool> seen;
            std::vector<bool> delta;
            double lbdema{};

            //index i is an index into seen
            std::vector<size_t> seenIdx;

            //index i is the position of an index in seenIdx
            std::vector<size_t> seenPos;

            //holds information about our decision trail
            std::vector<TrailEntry> trail;

            //each element is an index into trail where a new decision level starts
            std::vector<size_t> trail_starts;

            //This is a map where var_data[i] = the index in the trail that variable i 
            //  appears in
            std::vector<std::optional<size_t>> var_to_trail;

            //map from variable to clause indices where 
            //  var_to_clause[var_to_widx(var)] = the vector of indices that var is watching
            std::vector<std::vector<size_t>> var_to_clause;

            //map from clause index to variable indices where
            //  clause_to_var[i] = a pair of indices to literals that are watching clause i
            std::vector<std::pair<size_t, size_t>> clause_to_var;
    };
}

#endif