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

            void parse(std::string_view equation);
            bool solve(int level);

            std::optional<std::vector<int>> propagate(int var);

            bool createWatched();

            int numVars;
            int numClauses;
            std::vector<std::vector<int>> clauses;
            std::vector<var> vars;
            std::queue<int> unitProp;

            //map from variable to clause indices
            std::unordered_map<int, std::vector<size_t>> var_to_clause;
            //map from clause index to variable indices
            std::unordered_map<size_t, std::pair<size_t, size_t>> clause_to_var;
    };
}

#endif