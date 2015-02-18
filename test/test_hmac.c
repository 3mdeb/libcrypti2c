#include <check.h>
#include "../libcryptoauth.h"


int fill_random(uint8_t *ptr, const int len)
{
    int rc = -1;
    int fd = open("/dev/urandom");
    size_t num = 0;

    if (fd < 0)
        return rc;

    while (num < len)
    {
        rc = read(fd, ptr + num, len - num);
        if (rc < 0)
        {
            return rc;
        }
        else
        {
            num += rc;
        }
    }

    close (fd);

    return len;
}

START_TEST(test_hmac)
{

    uint8_t key [32];
    uint8_t challenge [32];

    struct lca_octet_buffer k_buf;
    struct lca_octet_buffer c_buf;
    struct lca_octet_buffer result;


    ck_assert_int_eq(fill_random(key, sizeof(key)), sizeof(key));
    ck_assert_int_eq(fill_random(challenge, sizeof(challenge)), sizeof(challenge));

    k_buf.ptr = key;
    k_buf.len = sizeof(key);

    c_buf.ptr = challenge;
    c_buf.len = sizeof(challenge);

    result = perform_soft_hmac_256_defaults(c_buf, k_buf, 0);

    ck_assert_int_eq(result.len, 32);

    // Verify the result
    ck_assert(lca_verify_hmac_defaults(c_buf, result, k_buf, 0));

    // Try to verify the key, which should fail
    ck_assert(!lca_verify_hmac_defaults(c_buf, c_buf, k_buf, 0));

    // Now let's sign the hmac

    gcry_sexp_t ecc;

    ck_assert(lca_gen_soft_keypair (&ecc));

    struct lca_octet_buffer signature;

    signature = lca_soft_sign(&ecc, result);




}
END_TEST

START_TEST(test_hmac_key_slot)
{

    uint8_t key [32];
    uint8_t challenge [32];
    int i, z;

    struct lca_octet_buffer k_buf;
    struct lca_octet_buffer c_buf;
    struct lca_octet_buffer result;


    ck_assert_int_eq(fill_random(key, sizeof(key)), sizeof(key));
    ck_assert_int_eq(fill_random(challenge, sizeof(challenge)), sizeof(challenge));

    k_buf.ptr = key;
    k_buf.len = sizeof(key);

    c_buf.ptr = challenge;
    c_buf.len = sizeof(challenge);

    for (i=0; i < 16; i++)
    {
        result = perform_soft_hmac_256_defaults(c_buf, k_buf, i);

        ck_assert_int_eq(result.len, 32);

        // Verify the result
        ck_assert(lca_verify_hmac_defaults(c_buf, result, k_buf, i));

        // Try to verify the key, which should fail
        if (i == 0)
            z = 15;
        else
            z = i - 1;
        ck_assert(!lca_verify_hmac_defaults(c_buf, result, k_buf, z));
    }
}
END_TEST

Suite * hmac_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("HMAC");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_hmac);
    tcase_add_test(tc_core, test_hmac_key_slot);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = hmac_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
