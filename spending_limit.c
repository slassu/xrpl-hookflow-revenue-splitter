#define HAS_CALLBACK
#include <stdint.h>
#include "hookapi.h"

// Maximum XRP allowed per time window (in drops)
// 1000 XRP = 1000000000 drops
#define LIMIT_DROPS 1000000000LL

// Time window in seconds (86400 = 24 hours)
#define TIME_WINDOW 86400LL

int64_t cbak(uint32_t reserved) { return 0; }

int64_t hook(uint32_t reserved)
{
    _g(1, 1);

    // Only fire on outgoing Payment transactions
    int64_t tt = otxn_type();
    if (tt != 0)
        accept(SBUF("Not a payment"), 0);

    // Only fire on OUTGOING payments
    unsigned char hook_accid[20];
    hook_account((uint32_t)hook_accid, 20);

    uint8_t src_accid[20];
    otxn_field(SBUF(src_accid), sfAccount);

    int is_outgoing = 0;
    BUFFER_EQUAL(is_outgoing, hook_accid, src_accid, 20);
    if (!is_outgoing)
        accept(SBUF("SpendLimit: Incoming tx"), 0);

    // Read the outgoing amount
    unsigned char amount_buffer[48];
    int64_t amount_len = otxn_field(SBUF(amount_buffer), sfAmount);

    if (amount_len != 8)
        accept(SBUF("SpendLimit: Not XRP"), 0);

    int64_t drops = AMOUNT_TO_DROPS(amount_buffer);
    if (drops <= 0)
        accept(SBUF("SpendLimit: Invalid amount"), 0);

    // Get current ledger time
    int64_t current_time = ledger_last_time();

    // Read stored window start time from state
    uint8_t window_key[5] = {0x57, 0x49, 0x4E, 0x44, 0x4F}; // WINDO
    uint8_t window_buf[8];
    int64_t window_len = state(SBUF(window_buf), SBUF(window_key));

    int64_t window_start = 0;
    if (window_len == 8)
    {
        for (int i = 0; i < 8; i++)
            window_start = (window_start << 8) | window_buf[i];
    }

    // Read stored spent amount from state
    uint8_t spent_key[5] = {0x53, 0x50, 0x45, 0x4E, 0x54}; // SPENT
    uint8_t spent_buf[8];
    int64_t spent_len = state(SBUF(spent_buf), SBUF(spent_key));

    int64_t spent_so_far = 0;
    if (spent_len == 8)
    {
        for (int i = 0; i < 8; i++)
            spent_so_far = (spent_so_far << 8) | spent_buf[i];
    }

    // Check if time window has expired - reset if so
    if (current_time > window_start + TIME_WINDOW)
    {
        window_start = current_time;
        spent_so_far = 0;
    }

    // Check if this payment would exceed the limit
    int64_t new_total = spent_so_far + drops;
    if (new_total > LIMIT_DROPS)
        rollback(SBUF("SpendLimit: Limit exceeded"), 1);

    // Update state - store new window start
    uint8_t new_window[8];
    int64_t ws = window_start;
    for (int i = 7; i >= 0; i--) {
        new_window[i] = ws & 0xFF;
        ws >>= 8;
    }
    state_set(SBUF(new_window), SBUF(window_key));

    // Update state - store new spent amount
    uint8_t new_spent[8];
    int64_t ns = new_total;
    for (int i = 7; i >= 0; i--) {
        new_spent[i] = ns & 0xFF;
        ns >>= 8;
    }
    state_set(SBUF(new_spent), SBUF(spent_key));

    accept(SBUF("SpendLimit: Payment approved"), 0);
    return 0;
}
