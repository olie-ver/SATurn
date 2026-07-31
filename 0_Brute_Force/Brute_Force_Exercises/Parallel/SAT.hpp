#include <atomic>
#include <string_view>
#include <optional>
#include <vector>
#include <mutex>

namespace saturn {
    class satsolver {
        public:
            satsolver(std::string_view input);
            bool solveCNF();

        private:
            void parse(std::string_view equation);
            void threadWorker(size_t low, size_t high);

            std::mutex vars_mutex;
            std::atomic_bool stop;
            int numVars;
            int numClauses;
            std::vector<std::vector<int>> clauses;
            std::optional<std::vector<bool>> vars;
    };
}