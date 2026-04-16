/**
 * HookFlow — Hook 7: Multi-Sig Payment Gate
 *
 * Blocks ALL outgoing transactions unless M-of-N configured
 * signers have submitted approvals via incoming payments.
 *
 * Hook Parameters:
 *   REQUIRED_SIGS  (1 byte)   — M: number of approvals needed
 *   SIGNER_1       (20 bytes) — AccountID of signer 1
 *   SIGNER_2       (20 bytes) — AccountID of signer 2
 *   SIGNER_3       (20 bytes) — AccountID of signer 3
 *
 * Website: https://xrplhookflow.com
 * GitHub:  https://github.com/slassu/xrpl-hookflow-revenue-splitter
 */

#include "hookapi.h"

#define GUARD(maxiter) _g(__LINE__, (maxiter)+1)

int64_t hook(uint32_t reserved) {

    GUARD(1);
    TRACESTR("MultiSigGate: called.");

    // ── Identify direction ────────────────────────────────────
    uint8_t hook_acc[20];
    uint8_t otxn_acc[20];
    hook_account(SBUF(hook_acc));
    otxn_field(SBUF(otxn_acc), sfAccount);

    int64_t is_outgoing = BUFFER_EQUAL_20(hook_acc, otxn_acc);

    // ── INCOMING — check if sender is an authorised signer ───
    if (!is_outgoing) {

        uint8_t signer1[20], signer2[20], signer3[20];
        uint8_t k1[] = "SIGNER_1";
        uint8_t k2[] = "SIGNER_2";
        uint8_t k3[] = "SIGNER_3";

        int64_t s1_set = hook_param(SBUF(signer1), k1, 8);
        int64_t s2_set = hook_param(SBUF(signer2), k2, 8);
        int64_t s3_set = hook_param(SBUF(signer3), k3, 8);

        // Trace param lengths so we can debug
        TRACEVAR(s1_set);
        TRACEVAR(s2_set);
        TRACEVAR(s3_set);

        int64_t is_s1 = (s1_set == 20) && BUFFER_EQUAL_20(otxn_acc, signer1);
        int64_t is_s2 = (s2_set == 20) && BUFFER_EQUAL_20(otxn_acc, signer2);
        int64_t is_s3 = (s3_set == 20) && BUFFER_EQUAL_20(otxn_acc, signer3);

        TRACEVAR(is_s1);
        TRACEVAR(is_s2);
        TRACEVAR(is_s3);

        if (!is_s1 && !is_s2 && !is_s3) {
            accept("MultiSigGate: not a signer.", 27, 0);
            return 0;
        }

        // Check if already voted
        uint8_t voted = 0;

        if (is_s1) {
            uint8_t vk[] = "S1_VOTED";
            int64_t sr = state(SBUF(&voted), vk, 8);
            if (sr >= 0 && voted == 1) {
                accept("MultiSigGate: S1 already voted.", 31, 0);
                return 0;
            }
            voted = 1;
            state_set(SBUF(&voted), vk, 8);
            TRACESTR("MultiSigGate: S1 approval recorded.");
        } else if (is_s2) {
            uint8_t vk[] = "S2_VOTED";
            int64_t sr = state(SBUF(&voted), vk, 8);
            if (sr >= 0 && voted == 1) {
                accept("MultiSigGate: S2 already voted.", 31, 0);
                return 0;
            }
            voted = 1;
            state_set(SBUF(&voted), vk, 8);
            TRACESTR("MultiSigGate: S2 approval recorded.");
        } else if (is_s3) {
            uint8_t vk[] = "S3_VOTED";
            int64_t sr = state(SBUF(&voted), vk, 8);
            if (sr >= 0 && voted == 1) {
                accept("MultiSigGate: S3 already voted.", 31, 0);
                return 0;
            }
            voted = 1;
            state_set(SBUF(&voted), vk, 8);
            TRACESTR("MultiSigGate: S3 approval recorded.");
        }

        // Increment count
        uint8_t count = 0;
        uint8_t ck[] = "APPROVAL_COUNT";
        int64_t cr = state(SBUF(&count), ck, 14);
        if (cr < 0) count = 0;
        count++;
        state_set(SBUF(&count), ck, 14);
        TRACEVAR(count);

        accept("MultiSigGate: approval recorded.", 32, 0);
        return 0;
    }

    // ── OUTGOING — check approval count ──────────────────────
    TRACESTR("MultiSigGate: outgoing tx detected.");

    uint8_t req_buf[1];
    uint8_t rk[] = "REQUIRED_SIGS";
    uint8_t required = 2;
    int64_t rp = hook_param(SBUF(req_buf), rk, 13);
    if (rp == 1) required = req_buf[0];

    uint8_t count = 0;
    uint8_t ck[] = "APPROVAL_COUNT";
    int64_t cr = state(SBUF(&count), ck, 14);
    if (cr < 0) count = 0;

    TRACEVAR(count);
    TRACEVAR(required);

    if (count < required) {
        rollback("MultiSigGate: insufficient approvals.", 37, 1);
        return 0;
    }

    // Reset state
    uint8_t zero = 0;
    uint8_t ck2[] = "APPROVAL_COUNT";
    uint8_t vk1[] = "S1_VOTED";
    uint8_t vk2[] = "S2_VOTED";
    uint8_t vk3[] = "S3_VOTED";
    state_set(SBUF(&zero), ck2, 14);
    state_set(SBUF(&zero), vk1, 8);
    state_set(SBUF(&zero), vk2, 8);
    state_set(SBUF(&zero), vk3, 8);

    TRACESTR("MultiSigGate: approved — state reset, tx released.");
    accept("MultiSigGate: released.", 23, 0);
    return 0;
}
