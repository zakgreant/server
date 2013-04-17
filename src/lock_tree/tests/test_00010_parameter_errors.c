/* We are going to test whether create and close properly check their input. */

#include "test.h"

static DBT _key;
DBT* key;
u_int32_t max_locks = 1000;
toku_ltm* ltm = NULL;

static void do_range_test(int (*acquire)(toku_lock_tree*, DB*, TXNID,
                                         const DBT*,
                                         const DBT*)) {
    int r;
    toku_lock_tree* lt  = NULL;
    DB*             db  = (DB*)1;
    TXNID           txn = (TXNID)1;  // Fake.
    DBT _key_l  = _key;
    DBT _key_r  = _key;
    DBT* key_l  = &_key_l;
    DBT* key_r  = &_key_r;
    {
        r = toku_lt_create(&lt, dbpanic, ltm,
                           get_compare_fun_from_db,
                           toku_malloc, toku_free, toku_realloc);
        CKERR(r);
        assert(lt);

        if (acquire == toku_lt_acquire_range_write_lock) {
            r = acquire(lt,  db,  txn,  key_l, key_r);
            CKERR2(r, ENOSYS);
        }


        r = acquire(NULL,   db,  txn,  key_l,  key_r);
        CKERR2(r, EINVAL);
        r = acquire(lt,     db,  txn,  NULL,   key_r);
        CKERR2(r, EINVAL);
        r = acquire(lt,     db,  txn,  key_l,  NULL);
        CKERR2(r, EINVAL);

        /* left > right tests. */
        const DBT* inf       =              toku_lt_infinity;
        const DBT* ninf      =              toku_lt_neg_infinity;
        r = acquire(lt,     db,  txn,  inf,    key_r);
        CKERR2(r, EDOM);
        r = acquire(lt,     db,  txn,  key_l,  ninf);
        CKERR2(r, EDOM);
        r = acquire(lt,     db,  txn,  inf,    ninf);
        CKERR2(r, EDOM);

        /* Cleanup. */
        r = toku_lt_close(lt);
        CKERR(r);

        lt = NULL;
    }
}

static void do_point_test(int (*acquire)(toku_lock_tree*, DB*, TXNID,
                                         const DBT*)) {
    int r;
    toku_lock_tree* lt  = NULL;
    TXNID           txn = (TXNID)1;  // Fake.
    DB* db = (DB*)0x1;

    lt = NULL;

    /* Point read tests. */
    key  = &_key;
    {
        r = toku_lt_create(&lt, dbpanic, ltm,
                           get_compare_fun_from_db,
                           toku_malloc, toku_free, toku_realloc);
        CKERR(r);
        assert(lt);

        r = toku_lt_unlock(NULL, (TXNID)1);
        CKERR2(r, EINVAL);

        r = acquire(NULL, db, txn,  key);
        CKERR2(r, EINVAL);

        r = acquire(lt,   db, txn,  NULL);
        CKERR2(r, EINVAL);

        /* Cleanup. */
        r = toku_lt_close(lt);
        CKERR(r);

        lt = NULL;
    }
}

int main(int argc, const char *argv[]) {
    parse_args(argc, argv);

    int r;
    toku_lock_tree* lt  = NULL;

    r = toku_ltm_create(NULL, max_locks, dbpanic,
                        get_compare_fun_from_db,
                        toku_malloc, toku_free, toku_realloc);
        CKERR2(r, EINVAL);
        assert(ltm == NULL);
    r = toku_ltm_create(&ltm, 0,         dbpanic,
                        get_compare_fun_from_db,
                        toku_malloc, toku_free, toku_realloc);
        CKERR2(r, EINVAL);
        assert(ltm == NULL);
    r = toku_ltm_create(&ltm, max_locks, dbpanic,
                        get_compare_fun_from_db,
                        NULL,        toku_free, toku_realloc);
        CKERR2(r, EINVAL);
        assert(ltm == NULL);
    r = toku_ltm_create(&ltm, max_locks, dbpanic,
                        get_compare_fun_from_db,
                        toku_malloc, NULL,      toku_realloc);
        CKERR2(r, EINVAL);
        assert(ltm == NULL);
    r = toku_ltm_create(&ltm, max_locks, dbpanic,
                        get_compare_fun_from_db,
                        toku_malloc, toku_free, NULL);
        CKERR2(r, EINVAL);
        assert(ltm == NULL);

    /* Actually create it. */
    r = toku_ltm_create(&ltm, max_locks, dbpanic,
                        get_compare_fun_from_db,
                        toku_malloc, toku_free, toku_realloc);
    CKERR(r);
    assert(ltm);

    r = toku_ltm_set_max_locks(NULL, max_locks);
        CKERR2(r, EINVAL);
    r = toku_ltm_set_max_locks(ltm,  0);
        CKERR2(r, EINVAL);
    r = toku_ltm_set_max_locks(ltm,  max_locks);
        CKERR(r);

    u_int32_t get_max = 73; //Some random number that isn't 0.
    r = toku_ltm_get_max_locks(NULL, &get_max);
        CKERR2(r, EINVAL);
        assert(get_max == 73);
    r = toku_ltm_get_max_locks(ltm,  NULL);
        CKERR2(r, EINVAL);
        assert(get_max == 73);
    r = toku_ltm_get_max_locks(ltm,  &get_max);
        CKERR(r);
        assert(get_max == max_locks);

    /* create tests. */
    {
        r = toku_lt_create(NULL, dbpanic, ltm,
                           get_compare_fun_from_db,
                           toku_malloc, toku_free, toku_realloc);
        CKERR2(r, EINVAL);

        r = toku_lt_create(&lt,  NULL,    ltm,
                           get_compare_fun_from_db,
                           toku_malloc, toku_free, toku_realloc);
        CKERR2(r, EINVAL);

        r = toku_lt_create(&lt,  dbpanic, NULL,
                           get_compare_fun_from_db,
                           toku_malloc, toku_free, toku_realloc);
        CKERR2(r, EINVAL);

        r = toku_lt_create(&lt,  dbpanic, ltm,
                           NULL,
                           toku_malloc, toku_free, toku_realloc);
        CKERR2(r, EINVAL);

        r = toku_lt_create(&lt,  dbpanic, ltm,
                           get_compare_fun_from_db,
                           NULL,        toku_free, toku_realloc);
        CKERR2(r, EINVAL);
        r = toku_lt_create(&lt,  dbpanic, ltm,
                           get_compare_fun_from_db,
                           toku_malloc, NULL,      toku_realloc);
        CKERR2(r, EINVAL);
        r = toku_lt_create(&lt,  dbpanic, ltm,
                           get_compare_fun_from_db,
                           toku_malloc, toku_free, NULL);
        CKERR2(r, EINVAL);
    }

    /* Close tests. */
    r = toku_lt_close(NULL);
    CKERR2(r, EINVAL);

    do_point_test(toku_lt_acquire_read_lock);
    do_point_test(toku_lt_acquire_write_lock);

    do_range_test(toku_lt_acquire_range_read_lock);
    do_range_test(toku_lt_acquire_range_write_lock);

    toku_ltm_close(ltm);
    return 0;
}
