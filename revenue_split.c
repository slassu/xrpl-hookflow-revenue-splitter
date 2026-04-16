/**
 * HookFlow — Hook 1: Revenue Split
 *
 * Automatically splits a configurable percentage of every
 * incoming XRP payment and emits it to a designated destination
 * wallet. The remainder stays with the hook account.
 *
 * Hook Parameters:
 *   DEST_ACC   (20 bytes) — AccountID to receive the split
 *   SPLIT_PCT  (1 byte)   — Percentage to split (1-100)
 *   MIN_DROPS  (8 bytes)  — Minimum drops to trigger split
 *
 * Website: https://xrplhookflow.com
 * GitHub:  https://github.com/slassu/xrpl-hookflow-revenue-splitter
 */

#include "hookapi.h"

#define GUARD(maxiter) _g(__LINE__, (maxiter)+1)

int64_t hook(uint32_t reserved) {

    GUARD(1);
    TRACESTR("RevSplit: called.");

    // ── 1. Only act on Payment transactions ──────────────────
    if (otxn_type() != ttPAYMENT) {
        accept("RevSplit: not a payment.", 24, 0);
        return 0;
    }

    // ── 2. Ignore outgoing (our own split emissions) ─────────
    uint8_t hook_acc[20];
    uint8_t otxn_acc[20];
    hook_account(SBUF(hook_acc));
    otxn_field(SBUF(otxn_acc), sfAccount);

    if (BUFFER_EQUAL_20(hook_acc, otxn_acc)) {
        accept("RevSplit: outgoing, skip.", 25, 0);
        return 0;
    }

    // ── 3. Read incoming XRP amount ──────────────────────────
    uint8_t amt_buf[8];
    int64_t amt_len = otxn_field(SBUF(amt_buf), sfAmount);
    if (amt_len != 8) {
        accept("RevSplit: IOU, skip.", 20, 0);
        return 0;
    }
    int64_t drops = (int64_t)(UINT64_FROM_BUF(amt_buf) & 0x3FFFFFFFFFFFFFFFULL);
    TRACEVAR(drops);

    // ── 4. Read MIN_DROPS parameter (default: 1 XAH) ─────────
    uint8_t min_buf[8];
    uint8_t min_key[] = "MIN_DROPS";
    int64_t min_drops = 1000000;
    int64_t mp = hook_param(SBUF(min_buf), min_key, 9);
    if (mp == 8)
        min_drops = (int64_t)UINT64_FROM_BUF(min_buf);
    TRACEVAR(min_drops);

    // ── 5. Skip if below minimum (avoid splitting dust) ──────
    if (drops < min_drops) {
        accept("RevSplit: below minimum.", 24, 0);
        return 0;
    }

    // ── 6. Read SPLIT_PCT parameter (default: 5%) ────────────
    uint8_t pct_buf[1];
    uint8_t pct_key[] = "SPLIT_PCT";
    uint8_t split_pct = 5;
    int64_t pp = hook_param(SBUF(pct_buf), pct_key, 9);
    if (pp == 1)
        split_pct = pct_buf[0];

    // Clamp to sane range
    if (split_pct == 0 || split_pct > 100) {
        accept("RevSplit: invalid pct, skip.", 28, 0);
        return 0;
    }
    TRACEVAR(split_pct);

    // ── 7. Read DEST_ACC parameter ───────────────────────────
    uint8_t dest_acc[20];
    uint8_t dest_key[] = "DEST_ACC";
    int64_t dp = hook_param(SBUF(dest_acc), dest_key, 8);
    if (dp != 20) {
        accept("RevSplit: DEST_ACC not set.", 27, 0);
        return 0;
    }

    // ── 8. Calculate split amount ────────────────────────────
    int64_t split_amount = (drops * (int64_t)split_pct) / 100;
    if (split_amount < 1) {
        accept("RevSplit: split too small.", 26, 0);
        return 0;
    }
    TRACEVAR(split_amount);

    // ── 9. Reserve emission slot ─────────────────────────────
    etxn_reserve(1);

    // ── 10. Build and emit split payment ─────────────────────
    uint8_t txn[PREPARE_PAYMENT_SIMPLE_SIZE];
    PREPARE_PAYMENT_SIMPLE(txn, split_amount, dest_acc, 0, 0);

    uint8_t emithash[32];
    int64_t emit_result = emit(SBUF(emithash), SBUF(txn));

    if (emit_result < 0) {
        TRACEVAR(emit_result);
        accept("RevSplit: emit failed.", 22, 0);
        return 0;
    }

    TRACESTR("RevSplit: split emitted.");
    accept("RevSplit: done.", 15, 0);
    return 0;
}
