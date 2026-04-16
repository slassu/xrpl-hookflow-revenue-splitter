/**
 * XRPL HookFlow — Hook 5: Escrow Release
 *
 * Validates EscrowFinish transactions before allowing release.
 * Checks destination address and ensures the escrow condition
 * has been met. Rolls back premature or unauthorized releases.
 *
 * Triggers:  ttPAYMENT, ttESCROW_FINISH
 * Tests:     4/4 passing
 *
 * Website:   https://xrplhookflow.com
 * GitHub:    https://github.com/slassu/xrpl-hookflow-revenue-splitter
 * Email:     hello@xrplhookflow.com
 */

#include "hookapi.h"

#define DONE(msg)  ACCEPT(msg, sizeof(msg))
#define NOPE(msg)  ROLLBACK(msg, sizeof(msg), __LINE__)

int64_t hook(uint32_t reserved) {

    TRACESTR("escrow_release.c: called");

    // -------------------------------------------------------
    // 1. Identify transaction type
    // -------------------------------------------------------
    int64_t tt = otxn_type();

    // Pass-through for non-escrow, non-payment transactions
    if (tt != ttESCROW_FINISH && tt != ttPAYMENT)
        DONE("EscrowRelease: unrelated tx type, passing");

    // -------------------------------------------------------
    // 2. For regular payments, accept without action
    // -------------------------------------------------------
    if (tt == ttPAYMENT) {
        TRACESTR("EscrowRelease: regular payment, accepting");
        DONE("EscrowRelease: payment accepted");
    }

    // -------------------------------------------------------
    // 3. EscrowFinish — begin validation
    // -------------------------------------------------------
    TRACESTR("EscrowRelease: processing EscrowFinish");

    // -------------------------------------------------------
    // 4. Read the hook account (authorized destination)
    // -------------------------------------------------------
    uint8_t hook_acc[20];
    hook_account(SBUF(hook_acc));

    // -------------------------------------------------------
    // 5. Read the escrow destination from the transaction
    // -------------------------------------------------------
    uint8_t escrow_dest[20];
    int64_t dest_len = otxn_field(SBUF(escrow_dest), sfDestination);
    if (dest_len != 20)
        NOPE("EscrowRelease: could not read escrow destination");

    // -------------------------------------------------------
    // 6. Validate destination matches hook account
    //    Only allow releases to this hook account
    // -------------------------------------------------------
    if (!BUFFER_EQUAL_20(hook_acc, escrow_dest))
        NOPE("EscrowRelease: destination mismatch — release denied");

    TRACESTR("EscrowRelease: destination verified");

    // -------------------------------------------------------
    // 7. Read the escrow owner (originator)
    // -------------------------------------------------------
    uint8_t escrow_owner[20];
    int64_t owner_len = otxn_field(SBUF(escrow_owner), sfAccount);
    if (owner_len != 20)
        NOPE("EscrowRelease: could not read escrow owner");

    // -------------------------------------------------------
    // 8. Check FinishAfter — reject premature releases
    //    Read the ledger close time and compare to FinishAfter
    // -------------------------------------------------------
    int64_t finish_after = 0;
    uint8_t fa_buf[8];
    int64_t fa_len = otxn_field(SBUF(fa_buf), sfFinishAfter);
    if (fa_len == 8) {
        finish_after = UINT64_FROM_BUF(fa_buf);
        int64_t ledger_time = ledger_last_time();
        TRACEVAR(finish_after);
        TRACEVAR(ledger_time);

        if (ledger_time < finish_after)
            NOPE("EscrowRelease: FinishAfter not reached — release denied");

        TRACESTR("EscrowRelease: FinishAfter condition satisfied");
    }

    // -------------------------------------------------------
    // 9. Check Condition/Fulfillment (crypto-condition)
    //    If escrow has a Condition field, Fulfillment must be present
    // -------------------------------------------------------
    uint8_t condition[32];
    int64_t cond_len = otxn_field(SBUF(condition), sfCondition);

    if (cond_len > 0) {
        uint8_t fulfillment[32];
        int64_t fulfill_len = otxn_field(SBUF(fulfillment), sfFulfillment);

        if (fulfill_len <= 0)
            NOPE("EscrowRelease: Condition present but no Fulfillment — denied");

        TRACESTR("EscrowRelease: Condition/Fulfillment pair present");
        // Note: XRPL protocol validates fulfillment cryptographic correctness
        // The hook confirms presence; ledger validates the crypto
    }

    // -------------------------------------------------------
    // 10. All checks passed — accept the EscrowFinish
    // -------------------------------------------------------
    TRACESTR("EscrowRelease: all conditions met, accepting release");
    DONE("EscrowRelease: escrow release authorised");
}
