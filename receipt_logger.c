#define HAS_CALLBACK
#include <stdint.h>
#include "hookapi.h"

// Receipt Logger Hook
// Logs every incoming XRP payment to on-chain state
// Tracks last payment details and running total received

int64_t cbak(uint32_t reserved) { return 0; }

int64_t hook(uint32_t reserved)
{
    _g(1, 1);

    // Only fire on Payment transactions
    int64_t tt = otxn_type();
    if (tt != 0)
        accept(SBUF("Receipt: Not a payment"), 0);

    // Get the hook account
    unsigned char hook_accid[20];
    hook_account((uint32_t)hook_accid, 20);

    // Get the destination
    uint8_t dest_accid[20];
    int32_t dest_len = otxn_field(SBUF(dest_accid), sfDestination);
    if (dest_len < 20)
        accept(SBUF("Receipt: No destination"), 0);

    // Only fire on INCOMING payments
    int is_incoming = 0;
    BUFFER_EQUAL(is_incoming, hook_accid, dest_accid, 20);
    if (!is_incoming)
        accept(SBUF("Receipt: Outgoing tx"), 0);

    // Read the incoming amount
    unsigned char amount_buffer[48];
    int64_t amount_len = otxn_field(SBUF(amount_buffer), sfAmount);
    if (amount_len != 8)
        accept(SBUF("Receipt: Not XRP"), 0);

    int64_t drops = AMOUNT_TO_DROPS(amount_buffer);
    if (drops <= 0)
        accept(SBUF("Receipt: Invalid amount"), 0);

    // Get ledger timestamp
    int64_t ledger_time = ledger_last_time();

    // Store last payment receipt in state
    uint8_t state_key[4] = {0x4C, 0x41, 0x53, 0x54}; // LAST
    uint8_t receipt[16];

    int64_t d = drops;
    for (int i = 7; i >= 0; i--) {
        receipt[i] = d & 0xFF;
        d >>= 8;
    }

    int64_t lt = ledger_time;
    for (int i = 7; i >= 0; i--) {
        receipt[8 + i] = lt & 0xFF;
        lt >>= 8;
    }

    state_set(SBUF(receipt), SBUF(state_key));

    // Update running total
    uint8_t total_key[5] = {0x54, 0x4F, 0x54, 0x41, 0x4C}; // TOTAL
    uint8_t total_buf[8];
    int64_t total_len = state(SBUF(total_buf), SBUF(total_key));

    int64_t running_total = 0;
    if (total_len == 8)
    {
        for (int i = 0; i < 8; i++)
            running_total = (running_total << 8) | total_buf[i];
    }

    running_total += drops;

    uint8_t new_total[8];
    int64_t nt = running_total;
    for (int i = 7; i >= 0; i--) {
        new_total[i] = nt & 0xFF;
        nt >>= 8;
    }
    state_set(SBUF(new_total), SBUF(total_key));

    accept(SBUF("Receipt: Logged"), 0);
    return 0;
}
