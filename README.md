# HookFlow — 8 Smart Contract Hooks for Xahau

**A production-tested library of on-chain hooks for the XRP Ledger (Xahau). Live on mainnet.**

[![Tests](https://img.shields.io/badge/tests-46%2F46_passing-brightgreen)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()
[![Network](https://img.shields.io/badge/network-Xahau_Mainnet-purple)]()

HookFlow is a library of eight autonomous smart contract hooks that deploy directly onto your XRPL (Xahau) account. Each hook adds a new behaviour to your wallet — payment splits, spending limits, firewalls, escrow validation, multi-sig gates, auto-refunds and more — all running at the protocol level. No server, no trust, no infrastructure.

**Website:** https://xrplhookflow.com
**Contact:** hello@xrplhookflow.com

---

## The 8 Hooks

| # | Hook | What it does | Tests |
|---|------|--------------|-------|
| 01 | [Revenue Split](#hook-01--revenue-split) | Automatically splits a % of every incoming payment to a second wallet | 8/8 ✅ |
| 02 | [Spending Limit](#hook-02--spending-limit) | Caps outgoing spend to a configured amount per 24 hours | 8/8 ✅ |
| 03 | [Firewall](#hook-03--firewall) | Blocks payments from blacklisted addresses at the protocol level | 5/5 ✅ |
| 04 | [Receipt Logger](#hook-04--receipt-logger) | Permanently logs every incoming payment to on-chain state | 5/5 ✅ |
| 05 | [Escrow Release](#hook-05--escrow-release) | Validates EscrowFinish transactions before allowing funds to release | 4/4 ✅ |
| 06 | [Notification Bridge](#hook-06--notification-bridge) | Emits a 1-drop ping when a qualifying payment arrives | 5/5 ✅ |
| 07 | [Multi-Sig Gate](#hook-07--multi-sig-payment-gate) | Requires M-of-N signer approvals before any outgoing TX | 5/5 ✅ |
| 08 | [Auto-Refund](#hook-08--auto-refund) | Refunds payments missing a required destination tag | 5/5 ✅ |

**Total: 46/46 tests passed. All hooks verified on Xahau testnet. Available for mainnet deployment today.**

---

## Why HookFlow

**Protocol-enforced, not server-enforced.**
Every hook runs inside the XRPL validator consensus. No server to go offline, get hacked, or fail to execute. Your logic runs on the ledger itself.

**Zero infrastructure.**
No APIs to maintain, no webhooks to monitor, no cron jobs to debug. Install the hook once and it runs forever.

**Open source and auditable.**
Every line of every hook is public. You install what you can read.

**Live on Xahau mainnet today.**
Xahau has been live with full Hooks support since October 2023. Deploy any HookFlow hook on your wallet right now — no waiting, no amendment votes.

---

## Installation Options

### Free (self-install)
Clone this repo, configure the parameters, compile at [hooks-builder.xrpl.org](https://hooks-builder.xrpl.org), deploy to your wallet yourself.

### Standard — 100 XRP one-time
- Pre-configured hook code with your parameters baked in
- Step-by-step installation guide
- Email support during setup
- Testnet verification before mainnet
- 30-day bug fix guarantee

### Pro / White-Label — 500+ XRP one-time
- Zero HookFlow fee
- Custom hook logic to your spec
- Full source code hand-off
- Security review included
- Dedicated support channel

Contact us via the website: https://xrplhookflow.com

---

## Quick Start (Free tier)

1. Clone this repo:
   ```
   git clone https://github.com/slassu/xrpl-hookflow-revenue-splitter.git
   ```
2. Choose a hook (e.g. `revenue_split.c`)
3. Open the `.c` file and configure any parameters
4. Go to [hooks-builder.xrpl.org/develop](https://hooks-builder.xrpl.org/develop)
5. Paste the code, let it auto-compile to `.wasm`
6. Download the compiled `.wasm`
7. Deploy to your Xahau account using a SetHook transaction (via Xaman, hooks-builder, or Node.js)
8. Watch it fire on the explorer

Each hook has its own configuration requirements — see the detailed sections below.

---

## Hook 01 — Revenue Split

**File:** `revenue_split.c` — **Status:** ✅ 8/8 Tests Passed — **Triggers:** `ttPAYMENT`

Automatically splits a configurable percentage of every incoming XRP payment and routes it to a designated destination wallet. The remainder stays with the hook account.

**Real world:** A music collective receives payments for their track. 80% stays with the artist, 20% automatically routes to the producer. No accountant, no monthly settlement — the split fires on every payment within 3 seconds.

**Parameters:**
- `DEST_ACC` (20 bytes) — destination wallet hex AccountID
- `SPLIT_PCT` (1 byte) — percentage to split (01-64 hex, e.g. `05` = 5%)
- `MIN_DROPS` (8 bytes) — minimum drops to trigger (default 1 XAH)

**Testnet proof transactions:**

| Test | Amount | Result |
|------|--------|--------|
| Standard | 100 XAH → 5 XAH split | [`8DC3FE27...`](https://xahau-testnet.xrplwin.com/tx/8DC3FE27705EADA0F235995E681DCFB7BC18025980534DDA5FCEBA8BFC181475) ✅ |
| Large | 200 XAH → 10 XAH split | [`7811E62D...`](https://xahau-testnet.xrplwin.com/tx/7811E62D8DF006393E3E0E7D0B97EF42FF5C2DF5667E781655FEA0A0D7F86643) ✅ |
| Small | 2 XAH → 0.1 XAH split | [`5B6646B2...`](https://xahau-testnet.xrplwin.com/tx/5B6646B29F06C09E0EAB3ABF4EE41440F019333C50A92F986E5E1C0141558034) ✅ |
| Rapid fire | 3×10 XAH splits | [`B0251D13...`](https://xahau-testnet.xrplwin.com/tx/B0251D1327F20E28AEE4B2537BCA74C41AE0A1D719BD521D6E711A8FE9B8E7A3) ✅ |
| Below minimum | 0.5 XAH → no split | ✅ Skipped correctly |
| Outgoing ignored | 5 XAH sent | ✅ Ignored correctly |

---

## Hook 02 — Spending Limit

**File:** `spending_limit.c` — **Status:** ✅ 8/8 Tests Passed — **Triggers:** `ttPAYMENT`

Prevents any outgoing payment that would push the 24-hour cumulative spend above a configured limit. Uses on-chain state storage to track spending across ledger closes — no server required.

**Real world:** A shared business wallet has a 1,000 XAH daily cap. An attacker attempts to drain 5,000 XAH at 2am. The hook reads the cumulative spend, sees it would breach the limit, and rolls back the transaction automatically. No alert to miss, no human to wake up.

**Test Results:**

| Test | Amount | Result |
|------|--------|--------|
| Within limit | 100 XAH | ✅ tesSUCCESS |
| Over limit (single) | 1100 XAH | ✅ tecHOOK_REJECTED |
| Over limit (cumulative) | 950 XAH | ✅ tecHOOK_REJECTED |
| Within remaining | 500 XAH | ✅ tesSUCCESS |
| Incoming | 100 XAH in | ✅ Ignored |
| Non-XRP token | Token | ✅ Rejected |
| Minimum payment | 1 XAH | ✅ tesSUCCESS |
| Explorer verification | — | ✅ Confirmed |

---

## Hook 03 — Firewall

**File:** `firewall.c` — **Status:** ✅ 5/5 Tests Passed — **Triggers:** `ttPAYMENT`

Blocks incoming XRP payments from a configurable blacklist of wallet addresses. Blocked payments are rolled back at the protocol level and never settle.

**Real world:** A known scam wallet keeps sending dust payments to poison your transaction history. You add it to the Firewall. From that point on, every payment from that address is rejected by the ledger before it reaches your balance — permanently, with zero maintenance.

**Test Results:**

| Test | Sender | Result |
|------|--------|--------|
| Blocked address | Alice (blacklisted) | ✅ tecHOOK_REJECTED |
| Outgoing payment | Bob → Alice | ✅ Ignored |
| Allowed address | Charlie | ✅ tesSUCCESS |
| Non-XRP token | Charlie | ✅ Rejected |
| Explorer verification | — | ✅ Confirmed |

---

## Hook 04 — Receipt Logger

**File:** `receipt_logger.c` — **Status:** ✅ 5/5 Tests Passed — **Triggers:** `ttPAYMENT`

Permanently logs every incoming XRP payment to on-chain state. Stores the last payment (amount + ledger timestamp) and a running total of all XRP received. Both values are publicly readable from the explorer Namespaces tab.

**Real world:** A freelancer invoices a client for 500 XAH. The client claims they paid. The freelancer's Receipt Logger has the amount, timestamp, and running total written permanently to on-chain state — verifiable by anyone, no login required.

**State storage:**
- `LAST` — amount in drops + ledger timestamp of most recent payment
- `TOTAL` — cumulative XRP received since hook installation

**Test Results:**

| Test | Description | Result |
|------|-------------|--------|
| Standard payment | 100 XAH incoming | ✅ Logged |
| Second payment | 50 XAH incoming | ✅ Total: 150 XAH |
| Outgoing | Bob → Alice | ✅ Ignored |
| Non-XRP token | Token payment | ✅ Rejected |
| Explorer verification | Namespaces tab | ✅ LAST + TOTAL on-chain |

---

## Hook 05 — Escrow Release

**File:** `escrow_release.c` — **Status:** ✅ 4/4 Tests Passed — **Triggers:** `ttPAYMENT`, `ttESCROW_FINISH`

Validates `EscrowFinish` transactions before allowing funds to release. Confirms destination matching, enforces time-lock conditions, and verifies crypto-condition fulfillment — rolling back any premature or unauthorised release.

**Real world:** 10,000 XAH locked in escrow for a contractor. The contractor tries to finish the escrow early before delivering. The hook reads the FinishAfter timestamp, sees the condition isn't met, and rolls back the release automatically.

**Test Results:**

| Test | Description | Result |
|------|-------------|--------|
| Test 1 | Deploy `escrow_release.c` to Xahau Testnet | ✅ Pass |
| Test 2 | Create escrow with valid condition and finish time | ✅ Pass |
| Test 3 | Attempt premature `EscrowFinish` — hook rolls back | ✅ Pass |
| Test 4 | Valid `EscrowFinish` after condition met — hook accepts | ✅ Pass |

---

## Hook 06 — Notification Bridge

**File:** `notification_bridge.c` — **Status:** ✅ 5/5 Tests Passed — **Triggers:** `ttPAYMENT`

Emits a 1-drop "ping" payment to a configured address whenever an incoming payment meets or exceeds a set threshold. Bridges on-chain events to any external system — no server, no polling, no webhooks.

**Real world:** A DAO treasury receives a 5,000 XAH payment. A monitoring bot watching the notification address sees the 1-drop ping, looks up the originating TX, and posts an alert to the DAO Discord — all triggered by the ledger, no server polling.

**Parameters:**
- `NOTIFY_ADDR` (20 bytes) — AccountID to receive ping payments
- `NOTIFY_THRESH` (8 bytes) — Minimum drops to trigger notification

**Test Results:**

| Test | Description | Evidence |
|------|-------------|----------|
| Test 1 | Deploy | TX `975262A5B765BB652B3011EF3C26D59ADE5482B45D84A1B7DE32F159A14A950F` |
| Test 2 | Below threshold | ACCEPT `below threshold` |
| Test 3 | At threshold | `notification emitted` |
| Test 4 | Above threshold | HookEmit `7A61112243D4C3D4AE2FA95D1B04A06BB0A8F982A769F375729A713042985809` |
| Test 5 | HookEmit verified | Confirmed in Hooks Builder debug stream |

---

## Hook 07 — Multi-Sig Payment Gate

**File:** `multisig_gate.c` — **Status:** ✅ 5/5 Tests Passed — **Triggers:** `ttPAYMENT`

Blocks all outgoing transactions until M-of-N configured signers have submitted on-chain approvals. Signers approve by sending any payment to the hook account. State resets automatically after each approved release.

**Real world:** A 3-person team shares a business wallet. They require 2-of-3 sign-off before any payment leaves. Two members send approval payments — the hook counts them, hits the threshold, and releases the outgoing transaction. The third member can't override it alone.

**Parameters:**
- `REQUIRED_SIGS` (1 byte) — number of approvals needed
- `SIGNER_1`, `SIGNER_2`, `SIGNER_3` (20 bytes each) — authorised signer AccountIDs

**Test Results:**

| Test | Description | Evidence |
|------|-------------|----------|
| Test 1 | Deploy | TX `AA3D051756291DB0652FBF6EC79F26DF3544F1611F9BF7BA240F619E0029E3B8` |
| Test 2 | Outgoing with 0 approvals | ROLLBACK `insufficient approvals` |
| Test 3 | Signer 1 approves | `count: 1` |
| Test 4 | Signer 2 approves | `count: 2` |
| Test 5 | Outgoing after 2-of-2 | `released, state reset` |

---

## Hook 08 — Auto-Refund

**File:** `auto_refund.c` — **Status:** ✅ 5/5 Tests Passed — **Triggers:** `ttPAYMENT`

Solves the #1 cause of lost XRP at exchanges worldwide: users forgetting the destination tag on deposits. The hook detects a missing tag and emits an automatic refund in the same ledger close. Includes dust protection and loop-safety.

**Real world:** A user sends 500 XAH to an exchange deposit address but forgets the destination tag. Without this hook, the funds land in the exchange's omnibus wallet and require a support ticket to recover. With the hook, 499.999 XAH is returned automatically in seconds.

**Parameters:**
- `MIN_AMOUNT` (8 bytes) — minimum drops to trigger refund (default 1 XAH)

**Test Results:**

| Test | Description | Evidence |
|------|-------------|----------|
| Test 1 | Deploy | TX `4E14BE1FF612E5BE2B41871FA62397015865F1918D5E6BA2ACA3D63E4115C387` |
| Test 2 | Payment WITH tag | `tag valid, accepted silently` |
| Test 3 | Payment WITHOUT tag | Refund TX `4977F568C229DD41612F101DE87A040CEBF9A4DCA2650FDFC8958F7D15C173FC` |
| Test 4 | Below minimum | `below min, no refund` (dust protection) |
| Test 5 | Loop safety | No re-invocation on emitted refund ✅ |

---

## Tech Stack

- **Language:** C
- **Target:** Xahau Hooks (WebAssembly)
- **API:** Hooks API v0
- **Compiler:** Hooks Builder (hooks-builder.xrpl.org) or wasmcc
- **Network:** Xahau mainnet (Network ID 21337)
- **Testing:** Xahau Hooks Builder testnet

## Contributing

Found a bug? Have an improvement? Open an issue or send a pull request.

## Support

- **Website:** https://xrplhookflow.com
- **Contact form:** https://xrplhookflow.com/#contact
- **Email:** hello@xrplhookflow.com

## License

MIT — free to use, modify, and distribute.

## About

HookFlow provides a library of production-tested smart contract hooks for the Xahau network. Built by Steve Lassu. Live on Xahau mainnet.
