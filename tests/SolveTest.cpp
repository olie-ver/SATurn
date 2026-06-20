#include <testpp/testpp.hpp>

#include "../src/SAT.hpp"

TEST(solve, simple_cnf) {
    SATurn::SATSolver solver("tests/header/simple.cnf");
    ASSERT_TRUE(solver.solveCNF());
    std::vector<bool> lits{true};

    ASSERT_TRUE(solver.getSolution().has_value());
    ASSERT_ORDERED_EQ(*solver.getSolution(), lits);
}

TEST(solve, medium_cnf) {
    SATurn::SATSolver solver("tests/header/medium.cnf");
    ASSERT_TRUE(solver.solveCNF());

    std::vector<bool> lits{true, true, true, true};
    ASSERT_TRUE(solver.getSolution().has_value());
    EXPECT_ORDERED_EQ(*solver.getSolution(), lits);
    solver.printSolution();
}