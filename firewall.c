#define HAS_CALLBACK
#include <stdint.h>
#include "hookapi.h"

// Firewall Hook — Block payments from specific addresses
// Replace the blocked address below with the address you want to block

int64_t cbak(uint32_t reserved) { return 0; }

int64_t hook(uint32_t reserved)
{
    _g(1, 1);

    // Only fire on Payment transactions
    int64_t tt = otxn_type();
    if (tt != 0)
        accept(SBUF("Firewall: Not a payment"), 0);

    // Get the hook account
    unsigned char hook_accid[20];
    hook_account((uint32_t)hook_accid, 20);

    // Get the destination of this transaction
    uint8_t dest_accid[20];
    int32_t dest_len = otxn_field(SBUF(dest_accid), sfDestination);
    if (dest_len < 20)
        accept(SBUF("Firewall: No destination"), 0);

    // Only fire on INCOMING payments
    int is_incoming = 0;
    BUFFER_EQUAL(is_incoming, hook_accid, dest_accid, 20);
    if (!is_incoming)
        accept(SBUF("Firewall: Outgoing tx"), 0);

    // Get the sender address
    uint8_t sender_accid[20];
    int32_t sender_len = otxn_field(SBUF(sender_accid), sfAccount);
    if (sender_len < 20)
        accept(SBUF("Firewall: No sender"), 0);

    // Resolve blocked address to 20-byte account ID
    // CHANGE THIS to the address you want to block
    uint8_t blocked_accid[20];
    util_accid(
        SBUF(blocked_accid),
        SBUF("rBLOCKED_ADDRESS_HERE_REPLACE_ME")
    );

    // Check if sender matches blocked address
    int is_blocked = 0;
    BUFFER_EQUAL(is_blocked, sender_accid, blocked_accid, 20);

    if (is_blocked)
        rollback(SBUF("Firewall: Address blocked"), 1);

    accept(SBUF("Firewall: Payment allowed"), 0);
    return 0;
}
