#include <testpp/testpp.hpp>
#include <filesystem>

#include "../src/SAT.hpp"

TEST(Constructor, messed_up) {
    SATurn::SATSolver solver("tests/header/messed_up.cnf");
    ASSERT_EQ(solver.getNumVars(), 1);
    ASSERT_EQ(solver.getNumClauses(), 1);
    ASSERT_EQ(solver.getNumClauses(), solver.getClauses().size());
    std::vector<int> clause{1};
    ASSERT_ORDERED_EQ(solver.getClauses()[0], clause);
}

TEST(Constructor, last_clause) {
    SATurn::SATSolver solver("tests/header/last_clause.cnf");
    ASSERT_EQ(solver.getNumVars(), 1);
    ASSERT_EQ(solver.getNumClauses(), 1);
    ASSERT_EQ(solver.getNumClauses(), solver.getClauses().size());
    std::vector<int> clause{1};
    ASSERT_ORDERED_EQ(solver.getClauses()[0], clause);
}

TEST(Constructor, number_comment) {
    SATurn::SATSolver solver("tests/header/number_comment.cnf");
    ASSERT_EQ(solver.getNumVars(), 1);
    ASSERT_EQ(solver.getNumClauses(), 2);
    ASSERT_EQ(solver.getNumClauses(), solver.getClauses().size());
    std::vector<std::vector<int>> clause{{1}, {-1}};
    ASSERT_ORDERED_EQ(solver.getClauses(), clause);
}

TEST(Constructor, uf20_91) {
    for (const auto& path : std::filesystem::directory_iterator("tests/uf20-91")) {
        SATurn::SATSolver solver(path.path().c_str());
        ASSERT_EQ(solver.getNumVars(), 20);
        ASSERT_EQ(solver.getNumClauses(), 91);
        ASSERT_EQ(solver.getClauses().size(), 91);
        for (const auto& clause : solver.getClauses()) {
            ASSERT_EQ(clause.size(), 3);
        }
    }
}

TEST(Constructor, headerless_uf20_91) {
    for (const auto& path : std::filesystem::directory_iterator("tests/no_header/uf20-91")) {
        SATurn::SATSolver solver(path.path().c_str());
        ASSERT_EQ(solver.getNumVars(), 20);
        ASSERT_EQ(solver.getNumClauses(), 91);
        ASSERT_EQ(solver.getClauses().size(), 91);
        for (const auto& clause : solver.getClauses()) {
            ASSERT_EQ(clause.size(), 3);
        }
    }
}