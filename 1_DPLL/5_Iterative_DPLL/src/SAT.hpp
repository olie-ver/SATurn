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

            constexpr bool evals_false(int var) {
                int idx = std::abs(var) - 1;
                return var > 0 && vars[idx] == FALSE || var < 0 && vars[idx] == TRUE;
            };

            constexpr bool evals_true(int var) {
                int idx = std::abs(var) - 1;
                return var > 0 && vars[idx] == TRUE || var < 0 && vars[idx] == FALSE;
            }

            struct TrailEntry {
                //the literal being assigned (eg, 5 or -5)
                int lit;

                //the decision level that this entry is a part of
                size_t level;

                //the index of the clause for why it was assigned that way
                //reasonIdx.hasValue() == False => decision, else => propagation
                std::optional<size_t> reasonIdx;
            };

            struct DecisionLevel {
                size_t idx;
                bool triedFalse;
            };

            void parse(std::string_view equation);

            bool propagate();

            bool createWatched();

            int numVars;
            int numClauses;
            size_t qHead{};

            std::vector<std::vector<int>> clauses;
            std::vector<var> vars;

            //holds information about our decision trail
            std::vector<TrailEntry> trail;

            //each element is an index into trail where a new decision level starts
            std::vector<DecisionLevel> trail_starts;

            //map from variable to clause indices
            std::unordered_map<int, std::vector<size_t>> var_to_clause;
            //map from clause index to variable indices
            std::unordered_map<size_t, std::pair<size_t, size_t>> clause_to_var;
    };
}

#endif