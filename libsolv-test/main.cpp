#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solver.h>
#include <solv/queue.h>
#include <solv/solvable.h>
#include <solv/solverdebug.h>
#include <iostream>

int main() {
    // Create a new dependency pool
    Pool *pool = pool_create();

    // Enable debug printing
    pool_setdebuglevel(pool, 2);  // 1 = minimal, 2 = verbose

    // Create an empty system repo (represents installed packages)
    Repo *systemRepo = repo_create(pool, "@System");
    repo_internalize(systemRepo);  // Empty system

    // Create a custom repo with packages A, B, C
    Repo *repo = repo_create(pool, "myrepo");

    // Add package A
    Id idA = repo_add_solvable(repo);
    Solvable *solvA = pool_id2solvable(pool, idA);
    solvA->name = pool_str2id(pool, "A", 1);
    solvA->evr = pool_str2id(pool, "1.0", 1);
    solvA->arch = ARCH_NOARCH;
    solvable_add_deparray(solvA, SOLVABLE_REQUIRES, pool_str2id(pool, "B", 1), 0);
    solvable_add_deparray(solvA, SOLVABLE_PROVIDES, solvA->name, 0);

    // Add package B
    Id idB = repo_add_solvable(repo);
    Solvable *solvB = pool_id2solvable(pool, idB);
    solvB->name = pool_str2id(pool, "B", 1);
    solvB->evr = pool_str2id(pool, "1.0", 1);
    solvB->arch = ARCH_NOARCH;
    solvable_add_deparray(solvB, SOLVABLE_REQUIRES, pool_str2id(pool, "C", 1), 0);
    solvable_add_deparray(solvB, SOLVABLE_PROVIDES, solvB->name, 0);

    // Add package C
    Id idC = repo_add_solvable(repo);
    Solvable *solvC = pool_id2solvable(pool, idC);
    solvC->name = pool_str2id(pool, "C", 1);
    solvC->evr = pool_str2id(pool, "1.0", 1);
    solvC->arch = ARCH_NOARCH;
    solvable_add_deparray(solvC, SOLVABLE_PROVIDES, solvC->name, 0);

    // Finalize (internalize) the repo
    repo_internalize(repo);

    // Define the job: Install package A by name
    Queue job;
    queue_init(&job);
    Id nameA = pool_str2id(pool, "A", 1);
    queue_push2(&job, SOLVER_INSTALL | SOLVER_SOLVABLE_NAME, nameA);

    // Create and run solver
    Solver *solver = solver_create(pool);
    int result = solver_solve(solver, &job);

    if (result != 0) {
        std::cerr << "Dependency resolution failed.\n";
    }

    // Create and inspect the transaction
    Transaction *trans = solver_create_transaction(solver);
    for (int i = 0; i < trans->steps.count; ++i) {
        Id p = trans->steps.elements[i];
        Solvable *s = pool_id2solvable(pool, p);
        std::cout << "Install: " << pool_id2str(pool, s->name)
                  << "-" << pool_id2str(pool, s->evr) << "\n";
    }

    // Clean up
    transaction_free(trans);
    solver_free(solver);
    queue_free(&job);
    pool_free(pool);

    return 0;
}
