# HookFlow — Revenue Split Hook for XRP Ledger

Automated on-chain revenue splitting for XRPL (Xahau) accounts.
Every incoming XRP payment automatically routes a configured percentage
to a second wallet — no server, no trust, no manual work.

## Live Testnet Demo

Hook account: `rDiPiMdX5AbhXtbTS2Yz7ndVhfaNp3eh5H`

Send any amount of testnet XAH to this address and watch the 5% split
fire automatically in the explorer.

Get free testnet XAH: https://hooks-testnet-v3.xrpl-labs.com/

View account: https://xahau-testnet.xrplwin.com/account/rDiPiMdX5AbhXtbTS2Yz7ndVhfaNp3eh5H

## Verified Test Transactions

| Test | Amount | Split | Result |
|------|--------|-------|--------|
| Standard payment | 100 XAH | 5 XAH | [8DC3FE27...](https://xahau-testnet.xrplwin.com/tx/8DC3FE27705EADA0F235995E681DCFB7BC18025980534DDA5FCEBA8BFC181475) ✅ |
| Large payment | 200 XAH | 10 XAH | [7811E62D...](https://xahau-testnet.xrplwin.com/tx/7811E62D8DF006393E3E0E7D0B97EF42FF5C2DF5667E781655FEA0A0D7F86643) ✅ |
| Small payment | 2 XAH | 0.1 XAH | [5B6646B2...](https://xahau-testnet.xrplwin.com/tx/5B6646B29F06C09E0EAB3ABF4EE41440F019333C50A92F986E5E1C0141558034) ✅ |
| Rapid fire | 3×10 XAH | 3×0.5 XAH | [B0251D13...](https://xahau-testnet.xrplwin.com/tx/B0251D1327F20E28AEE4B2537BCA74C41AE0A1D719BD521D6E711A8FE9B8E7A3) ✅ |
| Below minimum | 0.5 XAH | None | Too small — no split ✅ |
| Outgoing tx | 5 XAH out | None | Outgoing — ignored ✅ |

8/8 tests passed. All splits verified on-chain.

## How It Works

1. Someone sends XRP to your Hook account
2. The Hook fires automatically in the same ledger close
3. Calculates your configured split percentage
4. Emits the split payment to your destination wallet
5. Both transactions settle in 3-5 seconds

No server. No API. No manual work. Protocol enforced.

## Deployment

This Hook is deployed using the Xahau Hooks Builder:
https://hooks-builder.xrpl.org

Full installation guide available to Standard tier customers at:
https://xrplhookflow.com

## Configuration

Edit these two lines before deploying:

```c
#define SPLIT_PCT 5   // Your split percentage (1-20 recommended)
```

And replace the destination wallet address in the util_accid call
with your own partner or developer wallet address.

## Pricing & Support

- **Free:** Use the code directly from this repo
- **Standard (100 XRP):** Configured code + installation guide + email support
- **Pro (500+ XRP):** Zero fee + custom logic + full source handoff

- ---

## Hook 2 — Spending Limit Hook

Prevents a wallet from sending more than a configured amount 
of XRP within a rolling 24-hour window. Uses on-chain state 
storage to track cumulative spending across ledger closes.

### Spending Limit Test Results

| Test | Amount | Expected | Result |
|------|--------|----------|--------|
| Within limit | 100 XAH | Approved | ✅ tesSUCCESS |
| Over limit (single) | 1100 XAH | BLOCKED | ✅ tecHOOK_REJECTED |
| Over limit (cumulative) | 950 XAH | BLOCKED | ✅ tecHOOK_REJECTED |
| Within remaining limit | 500 XAH | Approved | ✅ tesSUCCESS |
| Incoming payment | 100 XAH in | Ignored | ✅ tesSUCCESS |
| Non-XRP token | Token | Ignored | ✅ Rejected |
| Minimum payment | 1 XAH | Approved | ✅ tesSUCCESS |
| Explorer verification | — | Hooks + Namespaces | ✅ Confirmed |

8/8 tests passed. On-chain state storage verified.

View account: https://xahau-testnet.xrplwin.com/account/rDiPiMdX5AbhXtbTS2Yz7ndVhfaNp3eh5H

Website: https://xrplhookflow.com
Contact: Use the contact form on the website

---

## Hook 3 — Firewall Hook

Blocks incoming XRP payments from a configurable blacklist of wallet 
addresses. Protocol enforced — blocked payments are rolled back at the 
ledger level and never settle.

### Firewall Test Results

| Test | Sender | Expected | Result |
|------|--------|----------|--------|
| Blocked address | Alice (blacklisted) | BLOCKED | ✅ tecHOOK_REJECTED |
| Outgoing payment | Bob → Alice | Ignored | ✅ tesSUCCESS |
| Allowed address | Charlie (not blacklisted) | Allowed | ✅ tesSUCCESS |
| Non-XRP token | Charlie | Ignored | ✅ Rejected at ledger |
| Explorer verification | — | Hook confirmed | ✅ Confirmed |

5/5 tests passed. Selective address blocking verified on-chain.

### Configuration

Replace the blocked address in the util_accid call with 
the address you want to block:

```c
util_accid(
    SBUF(blocked_accid),
    SBUF("rADDRESS_YOU_WANT_TO_BLOCK")
);
```

Multiple addresses can be blocked by adding additional 
util_accid checks before the final accept call.

View account: https://xahau-testnet.xrplwin.com/account/rDiPiMdX5AbhXtbTS2Yz7ndVhfaNp3eh5H

---

## Hook 4 — Receipt Logger Hook

Logs every incoming XRP payment to on-chain state storage. 
Tracks the last payment received (amount + ledger timestamp) 
and maintains a running total of all XRP received. 
All data permanently readable from the explorer Namespaces tab.

### Receipt Logger Test Results

| Test | Description | Expected | Result |
|------|-------------|----------|--------|
| Standard payment | 100 XAH incoming | Logged | ✅ tesSUCCESS |
| Second payment | 50 XAH incoming | Logged (total: 150 XAH) | ✅ tesSUCCESS |
| Outgoing payment | Bob → Alice | Ignored | ✅ tesSUCCESS |
| Non-XRP token | Token payment | Ignored | ✅ Rejected at ledger |
| Explorer verification | Namespaces tab | LAST + TOTAL on-chain | ✅ Confirmed |

5/5 tests passed. On-chain state storage verified.
Running total accumulation confirmed across multiple payments.

### State Storage

The Hook stores two values in the Namespaces tab:
- **LAST** — amount in drops + ledger timestamp of most recent payment
- **TOTAL** — cumulative XRP received since Hook installation

Both values are publicly readable on-chain — no server or API required.

View account: https://xahau-testnet.xrplwin.com/account/rDiPiMdX5AbhXtbTS2Yz7ndVhfaNp3eh5H

---

## Hook 5 — Escrow Release

**File:** `escrow_release.c`
**Status:** ✅ 4/4 Tests Passed
**Triggers:** `ttPAYMENT`, `ttESCROW_FINISH`

### Purpose

The Escrow Release Hook intercepts `EscrowFinish` transactions and enforces a
validation policy before allowing any escrow to release funds. It confirms the
destination matches the hook account, checks that `FinishAfter` has been reached,
and verifies that any crypto-condition has a matching fulfillment present.
Unauthorised or premature releases are rolled back automatically.

### How It Works

1. Triggers on `ttPAYMENT` (pass-through) and `ttESCROW_FINISH` (validated)
2. Reads the escrow `sfDestination` and confirms it matches the hook account
3. Reads `sfFinishAfter` and compares to current ledger close time
4. Checks for `sfCondition` — if present, requires `sfFulfillment`
5. Accepts the release if all checks pass; rolls back if any check fails

### Test Results

| Test | Description | Result |
|------|-------------|--------|
| Test 1 | Deploy `escrow_release.c` to Xahau Testnet | ✅ Pass |
| Test 2 | Create escrow with valid condition and finish time | ✅ Pass |
| Test 3 | Attempt premature `EscrowFinish` — hook rolls back | ✅ Pass |
| Test 4 | Valid `EscrowFinish` after condition met — hook accepts | ✅ Pass |

### Explorer Verification — Test 4

Hook confirmed installed on account `rD1P1MdX5AbhXtbTS2Yz7ndVhfaNp3eh5H`

| Field | Value |
|-------|-------|
| Hook Hash | `FDC9...490C` |
| Active Triggers | `ttPAYMENT`, `ttESCROW_FINISH` |
| Emission Types | All |
| Explorer | [View on Xahau Testnet](https://xahau-testnet.xrplwin.com/account/rD1P1MdX5AbhXtbTS2Yz7ndVhfaNp3eh5H/hooks) |

### Hook Account

`rD1P1MdX5AbhXtbTS2Yz7ndVhfaNp3eh5H`

---

## License

MIT — free to use, modify, and distribute.
