#pragma once

#ifndef SAT_H
#define SAT_H

#include <bitset>
#include <string_view>
#include <optional>
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

            // //The three states a variable can have
            static constexpr var FALSE{0b00};
            static constexpr var TRUE{0b01};
            static constexpr var UNASSIGNED{0b10};

            void parse(std::string_view equation);
            bool solve(std::vector<var>& assignment, int level);
            bool evaluate(const std::vector<var>& assignment);

            std::pair<std::vector<int>, std::vector<int>> 
                propagate(std::vector<var>& assignment, int level);

            std::pair<std::vector<int>, std::vector<int>> 
                pureElim(std::vector<var>& assignment);

            int numVars;
            int numClauses;
            std::vector<std::vector<int>> clauses;
            std::vector<bool> satisfied;
            std::optional<std::vector<var>> vars;
    };
}

#endif