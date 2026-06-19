#include <testpp/testpp.hpp>

#include "../src/SAT.hpp"

TEST(Constructor_headerless, simple) {
    SATurn::SATSolver solver("tests/no_header/simple.cnf");
    EXPECT_EQ(4, solver.getNumVars());
    EXPECT_EQ(1, solver.getNumClauses());
    std::vector<int> clause{1, 2, 3, 4};
    EXPECT_ORDERED_EQ(solver.getClauses()[0], clause);
}

TEST(Constructor_headerless, cut) {
SATurn::SATSolver solver("tests/no_header/comment_between_clause.cnf");
    EXPECT_EQ(2, solver.getNumVars());
    EXPECT_EQ(1, solver.getNumClauses());
    std::vector<int> clause{1, 2};
    EXPECT_ORDERED_EQ(solver.getClauses()[0], clause);
}

TEST(Constructor, string_input) {
    SATurn::SATSolver solver(std::string_view("1 2 3"));
    EXPECT_EQ(3, solver.getNumVars());
    EXPECT_EQ(1, solver.getNumClauses());
    std::vector<int> clause{1, 2, 3};
    EXPECT_ORDERED_EQ(solver.getClauses()[0], clause);
}

TEST(Constructor, simple_cnf) {
    SATurn::SATSolver solver("tests/header/simple.cnf");
    const size_t numVars = solver.getNumVars();
    EXPECT_EQ(numVars, 1);

    const size_t numClauses = solver.getNumClauses();
    ASSERT_EQ(numClauses, 1);

    const std::vector<std::vector<int>>& clauses = solver.getClauses();
    ASSERT_EQ(numClauses, clauses.size());
    std::vector<int> clause{1};
    ASSERT_ORDERED_EQ(clause, clauses[0]);
}

TEST(Constructor, medium_cnf) {
    SATurn::SATSolver solver("tests/header/medium.cnf");
    const size_t numVars = solver.getNumVars();
    ASSERT_EQ(numVars, 4);

    const size_t numClauses = solver.getNumClauses();
    ASSERT_EQ(numClauses, 6);

    const std::vector<std::vector<int>>& clauses = solver.getClauses();
    ASSERT_EQ(clauses.size(), numClauses);

    std::vector<std::vector<int>> cnfClauses{{1, 2}, {3, 4}, {1, -3}, {2, -4}, {-2, 3}, {-1, 4}};
    ASSERT_ORDERED_EQ(cnfClauses, clauses);
}