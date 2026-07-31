#include <string_view>
#include <optional>
#include <vector>

namespace saturn {
    class satsolver {
        public:
            satsolver(std::string_view input);
            bool solveCNF();

        private:
            void parse(std::string_view equation);
            int numVars;
            int numClauses;
            std::vector<std::vector<int>> clauses;
            std::optional<std::vector<bool>> vars;
    };
}