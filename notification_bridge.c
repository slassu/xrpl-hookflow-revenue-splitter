/**
 * HookFlow — Hook 6: Notification Bridge
 *
 * Emits a 1-drop "ping" payment to a configured notification address
 * whenever an incoming XRP payment meets or exceeds a set threshold.
 *
 * Hook Parameters:
 *   NOTIFY_ADDR   (20 bytes) — AccountID to receive ping payments
 *   NOTIFY_THRESH (8 bytes)  — Minimum drops to trigger notification
 *
 * Website: https://xrplhookflow.com
 * GitHub:  https://github.com/slassu/xrpl-hookflow-revenue-splitter
 */

#include "hookapi.h"

#define GUARD(maxiter) _g(__LINE__, (maxiter)+1)

int64_t hook(uint32_t reserved) {

    GUARD(1); // required — ensures _g is imported by hooks cleaner

    TRACESTR("NotifBridge: called.");

    // ── 1. Only act on Payment transactions ──────────────────
    if (otxn_type() != ttPAYMENT) {
        accept("NotifBridge: not a payment.", 27, 0);
        return 0;
    }

    // ── 2. Ignore outgoing payments ──────────────────────────
    uint8_t hook_acc[20];
    uint8_t otxn_acc[20];

    hook_account(SBUF(hook_acc));
    otxn_field(SBUF(otxn_acc), sfAccount);

    if (BUFFER_EQUAL_20(hook_acc, otxn_acc)) {
        accept("NotifBridge: outgoing, skip.", 28, 0);
        return 0;
    }

    // ── 3. Read incoming XRP amount in drops ─────────────────
    uint8_t amt_buf[8];
    int64_t amt_len = otxn_field(SBUF(amt_buf), sfAmount);
    if (amt_len != 8) {
        accept("NotifBridge: IOU, skip.", 23, 0);
        return 0;
    }
    int64_t drops = (int64_t)(UINT64_FROM_BUF(amt_buf) & 0x3FFFFFFFFFFFFFFFULL);
    TRACEVAR(drops);

    // ── 4. Read NOTIFY_THRESH parameter ──────────────────────
    uint8_t thresh_buf[8];
    uint8_t thresh_key[] = "NOTIFY_THRESH";
    int64_t threshold = 0;
    int64_t plen = hook_param(SBUF(thresh_buf), thresh_key, 13);
    if (plen == 8)
        threshold = (int64_t)UINT64_FROM_BUF(thresh_buf);
    TRACEVAR(threshold);

    // ── 5. Skip if below threshold ───────────────────────────
    if (threshold > 0 && drops < threshold) {
        accept("NotifBridge: below threshold.", 29, 0);
        return 0;
    }

    // ── 6. Read NOTIFY_ADDR parameter ────────────────────────
    uint8_t notify_addr[20];
    uint8_t addr_key[] = "NOTIFY_ADDR";
    int64_t addr_len = hook_param(SBUF(notify_addr), addr_key, 11);
    if (addr_len != 20) {
        accept("NotifBridge: NOTIFY_ADDR not set.", 33, 0);
        return 0;
    }

    // ── 7. Reserve emission slot ─────────────────────────────
    etxn_reserve(1);

    // ── 8. Build and emit 1-drop notification payment ────────
    uint8_t txn[PREPARE_PAYMENT_SIMPLE_SIZE];
    PREPARE_PAYMENT_SIMPLE(txn, 1, notify_addr, 0, 0);

    uint8_t emithash[32];
    int64_t emit_result = emit(SBUF(emithash), SBUF(txn));

    if (emit_result < 0) {
        TRACEVAR(emit_result);
        accept("NotifBridge: emit failed.", 25, 0);
        return 0;
    }

    TRACESTR("NotifBridge: notification emitted.");
    accept("NotifBridge: done.", 18, 0);
    return 0;
}
