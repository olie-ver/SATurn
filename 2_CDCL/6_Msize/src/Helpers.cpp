#include "SAT.hpp"

namespace saturn {
    void satsolver::removeFromWatchList(std::vector<size_t>& watchList, size_t clauseIdx) {
        for (size_t i = 0; i < watchList.size(); i++) {
            if (watchList[i] == clauseIdx) {
                watchList[i] = watchList.back();
                watchList.pop_back();
                return;
            }
        }
    }

    void satsolver::removeClauseFromWatchLists(size_t clauseIdx) {
        auto [w1, w2] = clause_to_var[clauseIdx];

        int lit1 = clauses[clauseIdx][w1];
        int lit2 = clauses[clauseIdx][w2];

        size_t var1 = var_to_widx(lit1);
        size_t var2 = var_to_widx(lit2);

        removeFromWatchList(var_to_clause[var1], clauseIdx);

        if (var2 != var1) {
            removeFromWatchList(var_to_clause[var2], clauseIdx);
        }
    }

    void satsolver::replaceWatchIndex(size_t oldIdx, size_t newIdx) {
        // The clause now located at newIdx is the clause that
        // used to be located at oldIdx.
        auto [w1, w2] = clause_to_var[newIdx];

        int lit1 = clauses[newIdx][w1];
        int lit2 = clauses[newIdx][w2];

        size_t var1 = var_to_widx(lit1);
        size_t var2 = var_to_widx(lit2);

        // Update first watch list.
        for (size_t& clauseIdx : var_to_clause[var1]) {
            if (clauseIdx == oldIdx) {
                clauseIdx = newIdx;
            }
        }

        // Update second watch list.
        if (var2 != var1) {
            for (size_t& clauseIdx : var_to_clause[var2]) {
                if (clauseIdx == oldIdx) {
                    clauseIdx = newIdx;
                }
            }
        }
    }
}