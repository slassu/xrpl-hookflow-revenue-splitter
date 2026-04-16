/**
 * HookFlow — Hook 8: Auto-Refund
 *
 * Automatically refunds incoming XRP payments that are missing
 * a required DestinationTag. Solves the #1 cause of lost XRP
 * deposits at exchanges: users forgetting the destination tag.
 *
 * Hook Parameters:
 *   REFUND_MODE  (1 byte)  — 01 = require DestinationTag
 *   MIN_AMOUNT   (8 bytes) — minimum drops to bother refunding
 *
 * Website: https://xrplhookflow.com
 * GitHub:  https://github.com/slassu/xrpl-hookflow-revenue-splitter
 */

#include "hookapi.h"

#define GUARD(maxiter) _g(__LINE__, (maxiter)+1)

int64_t hook(uint32_t reserved) {

    GUARD(1);
    TRACESTR("AutoRefund: called.");

    // ── 1. Only act on Payment transactions ──────────────────
    if (otxn_type() != ttPAYMENT) {
        accept("AutoRefund: not a payment.", 26, 0);
        return 0;
    }

    // ── 2. Ignore outgoing (our own refunds) ─────────────────
    uint8_t hook_acc[20];
    uint8_t otxn_acc[20];
    hook_account(SBUF(hook_acc));
    otxn_field(SBUF(otxn_acc), sfAccount);

    if (BUFFER_EQUAL_20(hook_acc, otxn_acc)) {
        accept("AutoRefund: outgoing, skip.", 27, 0);
        return 0;
    }

    // ── 3. Read incoming XRP amount ──────────────────────────
    uint8_t amt_buf[8];
    int64_t amt_len = otxn_field(SBUF(amt_buf), sfAmount);
    if (amt_len != 8) {
        accept("AutoRefund: IOU, skip.", 22, 0);
        return 0;
    }
    int64_t drops = (int64_t)(UINT64_FROM_BUF(amt_buf) & 0x3FFFFFFFFFFFFFFFULL);
    TRACEVAR(drops);

    // ── 4. Read MIN_AMOUNT parameter ─────────────────────────
    uint8_t min_buf[8];
    uint8_t min_key[] = "MIN_AMOUNT";
    int64_t min_amount = 1000000; // default: 1 XAH
    int64_t mp = hook_param(SBUF(min_buf), min_key, 10);
    if (mp == 8)
        min_amount = (int64_t)UINT64_FROM_BUF(min_buf);
    TRACEVAR(min_amount);

    // ── 5. Skip refund if below minimum (dust protection) ────
    if (drops < min_amount) {
        accept("AutoRefund: below min, no refund.", 33, 0);
        return 0;
    }

    // ── 6. Check for DestinationTag ──────────────────────────
    uint8_t tag_buf[4];
    int64_t tag_len = otxn_field(SBUF(tag_buf), sfDestinationTag);

    if (tag_len == 4) {
        // Tag present — payment is valid, accept silently
        TRACESTR("AutoRefund: tag present, accepting.");
        accept("AutoRefund: tag valid.", 22, 0);
        return 0;
    }

    // ── 7. No tag — refund the sender ────────────────────────
    TRACESTR("AutoRefund: no tag, refunding.");

    // Reserve emission
    etxn_reserve(1);

    // Reserve some drops for the fee, refund the rest
    int64_t refund_amount = drops - 1000; // keep 1000 drops for fee buffer
    if (refund_amount < 1) {
        accept("AutoRefund: refund too small.", 29, 0);
        return 0;
    }

    // Build refund payment
    uint8_t txn[PREPARE_PAYMENT_SIMPLE_SIZE];
    PREPARE_PAYMENT_SIMPLE(txn, refund_amount, otxn_acc, 0, 0);

    uint8_t emithash[32];
    int64_t emit_result = emit(SBUF(emithash), SBUF(txn));

    if (emit_result < 0) {
        TRACEVAR(emit_result);
        accept("AutoRefund: emit failed.", 24, 0);
        return 0;
    }

    TRACESTR("AutoRefund: refund emitted.");
    accept("AutoRefund: refunded.", 21, 0);
    return 0;
}
