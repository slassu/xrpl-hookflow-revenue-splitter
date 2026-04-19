# HookFlow — 8 Smart Contract Hooks for Xahau

> A production-tested library of 8 smart contract Hooks for the XRPL/Xahau ecosystem. Open source, MIT licensed, ready for mainnet.

**Website:** [xrplhookflow.com](https://xrplhookflow.com)
**Contact:** hello@xrplhookflow.com

---

## Mainnet Deployment — Hook 1 Live

**Hook 1 (Revenue Split) was deployed and verified on Xahau mainnet on April 18, 2026.**

| | |
|---|---|
| **Deploy TX** | [FB55D1FF…28463](https://xahau.xrplwin.com/tx/FB55D1FFCB7063A8E3CA642FCE484D3A716A93D5CC2C8CF60C5F6BA813028463) |
| **Hook hash** | `8927306F708A15EB1CEA1E4379CAB010C11EC13048D8FFAF29A3BE7B880C32F1` |
| **Hook account** | `rhCE4q2o7xLhHCi6qZU1LCmQaBtggx9u5t` |
| **Split destination** | `rfWWSP7i8Nu3nEuLxfM278tr2EBQ24yaW6` |
| **Deploy fee** | 1.003565 XAH (2,007-byte WASM) |
| **Flags used** | `1` (hsfOVERRIDE — unnecessary for fresh install; `hsfCanEmit` was missing, see below) |

**Test fire (same day):** Sent 2 XAH from test account → hook emitted 0.1 XAH (5%) to split destination within 1 ledger close. Both transactions confirmed `tesSUCCESS`.

Hooks 2–8 are production-tested on testnet (46/46 passes) and deploy on demand with purchase.

> **Note on flags:** The initial deploy used `Flags: 1` (`hsfOVERRIDE`), which is unnecessary for a fresh install and should be omitted. `hsfCanEmit` (`Flags: 4`) was not set — this flag should be included when a hook emits transactions. A corrected re-deploy with `Flags: 4` is planned. The hook emits correctly without it on the current Xahau version, but best practice is to set it explicitly.

---

## What this is

HookFlow is a library of 8 Hooks — small WebAssembly modules that install directly onto an XRPL/Xahau account and add smart-contract behaviour at the protocol level. Each Hook solves a specific real-world wallet automation problem.

All 8 Hooks were tested on Xahau testnet with 46 total on-chain transactions, all passing. They are ready for mainnet deployment today.

---

## The 8 Hooks

| # | Hook | What it does | Tests |
|---|------|--------------|-------|
| 01 | [Revenue Split](./revenue_split.c) | Automatically splits a % of every incoming payment to a second wallet | 8/8 ✅ |
| 02 | [Spending Limit](./spending_limit.c) | Caps daily outgoing spend at a configured amount | 8/8 ✅ |
| 03 | [Firewall](./firewall.c) | Blocks payments from specific sender addresses at protocol level | 5/5 ✅ |
| 04 | [Receipt Logger](./receipt_logger.c) | Permanently logs incoming payments to on-chain state — LAST + TOTAL | 5/5 ✅ |
| 05 | [Escrow Release](./escrow_release.c) | Validates escrow finish conditions before release is allowed | 4/4 ✅ |
| 06 | [Notification Bridge](./notification_bridge.c) | Emits a 1-drop ping on qualifying payments to a notification address | 5/5 ✅ |
| 07 | [Multi-Sig Gate](./multisig_gate.c) | Requires M-of-N approvals before outgoing transactions can settle | 5/5 ✅ |
| 08 | [Auto-Refund](./auto_refund.c) | Returns payments missing a required destination tag | 5/5 ✅ |

**Total: 46/46 tests passed on Xahau testnet.**

---

## Why HookFlow

**Protocol-enforced, not server-enforced.** Every Hook runs inside the Xahau validator consensus. No server can go offline, be hacked, or miss an event. If the ledger is live, the Hook executes.

**Zero infrastructure.** No backend, no watchers, no cron jobs. The ledger is the runtime.

**Open source from day one.** Every line of every Hook is public in this repository. You can audit before you install. No closed-source logic.

**Live on mainnet.** Xahau mainnet has supported Hooks since October 2023. These Hooks are not waiting on an amendment vote — they deploy today.

**One-person accountability.** HookFlow is built and supported by one developer (Steve Lassu, Canada). Small team, direct email support, no ticket queues.

---

## Installation Options

### Free (self-install)

- Clone this repo or download the `.c` file for the Hook you want
- Modify the parameters at the top of the file (wallet addresses, percentages, thresholds)
- Paste into the [Hooks Builder](https://hooks-builder.xrpl.org/develop) — it auto-compiles
- Deploy via SetHook transaction from your Xahau wallet
- You're on your own for support — the code is documented but there's no guarantee

### Standard tier (100 XRP or 1500 XAH per hook)

We do the above for you, plus:

- Configure the Hook with your specific parameters
- Pre-compile to `.wasm` so you don't need the toolchain
- Deliver a 12-page branded PDF installation guide
- Email support during setup
- Multi-hook discount: 80 XRP / 1200 XAH each at 3+ hooks
- 30-day bug fix guarantee

Contact **hello@xrplhookflow.com** to get started.

### Pro / White-Label (500+ XRP or 7500+ XAH custom)

Custom Hook logic to your specification, or the full 8-hook library deployed on one wallet. Includes security review, full source hand-off, and white-label / co-branding option.

---

## Quick Start (Free tier)

```bash
# 1. Clone the repo
git clone https://github.com/slassu/xrpl-hookflow-revenue-splitter.git
cd xrpl-hookflow-revenue-splitter

# 2. Open the Hook you want (e.g. Revenue Split)
open revenue_split.c

# 3. Compile via Hooks Builder (https://hooks-builder.xrpl.org/develop)
#    or locally: wasmcc revenue_split.c -o revenue_split.wasm

# 4. Deploy via SetHook transaction with your parameters as HookParameters:
#    DEST_ACC   = hex AccountID of split destination (20 bytes)
#    SPLIT_PCT  = single byte, e.g. 0x05 for 5%
#    MIN_DROPS  = 8-byte big-endian uint64, e.g. 0x00000000000F4240 for 1 XAH
#
#    Parameters are NOT set by editing #define values in the .c file.
#    They are passed in the SetHook transaction as HookParameters.
#
#    Recommended Flags for a fresh install:
#    Flags: 4  (hsfCanEmit — required if hook emits transactions)
#    DO NOT use Flags: 1 (hsfOVERRIDE) for a fresh install.

# 5. Sign with Xaman via xahau.xrplwin.com → Raw JSON Transaction Sender → done
```

Just follow the README for each `.c` file — each has specific configuration notes for its parameters.

---

## Hook 01 — Revenue Split

**File:** [`revenue_split.c`](./revenue_split.c) · **Status:** ✅ Live on Xahau mainnet · **Triggers:** `ttPAYMENT`

Automatically splits a configurable percentage of every incoming XAH payment and routes it to a designated destination wallet. The emission happens in the same ledger close as the incoming payment, meaning no delay, no server dependency — the split fires or the transaction rolls back.

**Real world:** A music producer receives royalty payments in XAH, wants 35% automatically sent to her co-producer. With this Hook: no spreadsheet, no manual transfer, no mistakes. The ledger does it.

**Parameters (set as `HookParameters` in the SetHook transaction — not `#define` values):**

| Key | Key (hex) | Value encoding | Example |
|-----|-----------|---------------|---------|
| `DEST_ACC` | `444553545F414343` | 20-byte AccountID | `476564...` (hex of destination r-address) |
| `SPLIT_PCT` | `53504C49545F504354` | 1-byte uint8 | `05` = 5% |
| `MIN_DROPS` | `4D494E5F44524F5053` | 8-byte big-endian uint64 | `00000000000F4240` = 1 XAH |

> **Note:** Parameters are passed at deploy time via `HookParameters` in the SetHook transaction. Editing `#define` values in the source has no effect on the deployed hook — the code reads configuration via `hook_param()` at runtime.

**Tested behaviours (testnet):**

| Test | Input | Result |
|------|-------|--------|
| 1 | Deploy hook to test account | ✅ tesSUCCESS |
| 2 | 100 XAH incoming, 5% split | ✅ 5 XAH emitted |
| 3 | 200 XAH incoming, 5% split | ✅ 10 XAH emitted |
| 4 | 2 XAH incoming, 5% split | ✅ 0.1 XAH emitted |
| 5 | 3×10 XAH rapid-fire | ✅ 3× 0.5 XAH emitted |
| 6 | 0.5 XAH (below minimum) | ✅ No emission |
| 7 | Outgoing TX (loop safety) | ✅ Ignored |
| 8 | Non-XRP token payment | ✅ Ignored |

---

## Hook 02 — Spending Limit

**File:** [`spending_limit.c`](./spending_limit.c) · **Status:** ✅ 8/8 tests · **Triggers:** `ttPAYMENT`

Enforces a configurable daily outgoing spend limit using Hook state storage. Transactions that would exceed the limit are rolled back at the protocol level — no server to monitor, no alert to miss.

**Real world:** A shared business wallet has a 1,000 XAH daily cap. An attacker attempts to drain 5,000 XAH at 2am. The hook reads the cumulative spend, sees it would breach the limit, and rolls back the transaction automatically.

**Tested behaviours (testnet):**

| Test | Input | Result |
|------|-------|--------|
| 1 | 100 XAH outgoing, within limit | ✅ Approved |
| 2 | 1,100 XAH outgoing, over limit | ✅ Blocked |
| 3 | 950 XAH outgoing after 100 XAH already spent | ✅ Blocked (cumulative) |
| 4 | Incoming payment | ✅ Ignored |
| 5 | Non-XRP token outgoing | ✅ Ignored |
| 6 | Day reset after 24h | ✅ Limit refreshed |
| 7 | State read + write across ledger closes | ✅ Verified |
| 8 | Loop safety (Hook ignores own emissions) | ✅ No re-trigger |

---

## Hook 03 — Firewall

**File:** [`firewall.c`](./firewall.c) · **Status:** ✅ 5/5 tests · **Triggers:** `ttPAYMENT`

Blocks incoming payments from a configured blacklist of sender addresses. Other senders pass through unaffected.

**Real world:** A known scam wallet keeps sending dust payments to your address. Add it to the Firewall. From that point on, every payment from that address is rejected by the ledger before it touches your balance — permanently, with zero maintenance.

**Tested behaviours (testnet):**

| Test | Input | Result |
|------|-------|--------|
| 1 | Blocked address sends payment | ✅ Blocked |
| 2 | Allowed address sends payment | ✅ Allowed |
| 3 | Outgoing payment | ✅ Ignored |
| 4 | Non-XRP token from blocked address | ✅ Ignored |
| 5 | Multiple blocked addresses | ✅ All blocked selectively |

---

## Hook 04 — Receipt Logger

**File:** [`receipt_logger.c`](./receipt_logger.c) · **Status:** ✅ 5/5 tests · **Triggers:** `ttPAYMENT`

Writes incoming payment details to on-chain Hook state storage — both the LAST payment (amount + timestamp) and a cumulative TOTAL across all payments since installation. Readable by anyone from the explorer Namespaces tab.

**Real world:** A freelancer invoices a client for 500 XAH. The client claims they paid. The freelancer's Receipt Logger has the amount, timestamp, and running total written permanently to on-chain state — verifiable by anyone, no login required.

**Tested behaviours (testnet):**

| Test | Description | Result |
|------|-------------|--------|
| 1 | Incoming 100 XAH payment | ✅ LAST updated |
| 2 | Incoming 50 XAH payment | ✅ LAST updated, TOTAL += 50 |
| 3 | Outgoing | ✅ Ignored |
| 4 | Non-XRP token | ✅ Ignored |
| 5 | LAST + TOTAL readable from Namespaces | ✅ Verified |

---

## Hook 05 — Escrow Release

**File:** [`escrow_release.c`](./escrow_release.c) · **Status:** ✅ 4/4 tests · **Triggers:** `ttPAYMENT`, `ttESCROW_FINISH`

Validates EscrowFinish transactions before funds can be released. Enforces destination matching, time-lock conditions, and crypto-condition fulfillment.

**Real world:** 10,000 XAH locked in escrow for a contractor. The contractor tries to finish the escrow early before delivering. The hook reads the FinishAfter timestamp, sees the condition isn't met, and rolls back the release automatically.

**Tested behaviours (testnet):**

| Test | Description | Result |
|------|-------------|--------|
| 1 | Deploy escrow_release.c | ✅ tesSUCCESS |
| 2 | Create escrow with valid condition + finish time | ✅ Created |
| 3 | Attempt premature EscrowFinish | ✅ Blocked |
| 4 | Valid EscrowFinish after condition met | ✅ Released |

---

## Hook 06 — Notification Bridge

**File:** [`notification_bridge.c`](./notification_bridge.c) · **Status:** ✅ 5/5 tests · **Triggers:** `ttPAYMENT`

Emits a 1-drop XAH ping to a configured notification address whenever an incoming payment meets a threshold. Any bot or wallet watching that address gets an immediate on-chain signal.

**Real world:** A DAO treasury receives a 5,000 XAH payment. A monitoring bot watching the notification address sees the 1-drop ping, looks up the originating TX, and posts an alert to the DAO Discord — all triggered by the ledger, no server polling.

**Tested behaviours (testnet):**

| Test | Description | Result |
|------|-------------|--------|
| 1 | Deploy notification_bridge.c | ✅ tesSUCCESS |
| 2 | Payment below threshold (0.05 XAH) | ✅ Silent |
| 3 | Payment at threshold (0.1 XAH) | ✅ Ping emitted |
| 4 | Payment above threshold (1 XAH) | ✅ Ping emitted |
| 5 | Loop safety — own outgoing ignored | ✅ Verified |

---

## Hook 07 — Multi-Sig Gate

**File:** [`multisig_gate.c`](./multisig_gate.c) · **Status:** ✅ 5/5 tests · **Triggers:** `ttPAYMENT`

Requires M-of-N signers to approve outgoing transactions on-chain. Signers approve by sending a payment to the hook account; the hook tracks the count in state and releases the transaction once the threshold is met.

**Real world:** A 3-person team shares a business wallet. They require 2-of-3 sign-off before any payment leaves. Two members send approval payments — the hook counts them, hits the threshold, and releases the outgoing transaction. The third member can't override it alone.

**Tested behaviours (testnet):**

| Test | Description | Result |
|------|-------------|--------|
| 1 | Deploy multisig_gate.c | ✅ tesSUCCESS |
| 2 | Outgoing TX with 0 approvals | ✅ Blocked |
| 3 | Signer 1 (Alice) sends approval | ✅ Recorded |
| 4 | Signer 2 (Charlie) sends approval | ✅ Recorded |
| 5 | Outgoing TX after 2-of-2 approvals | ✅ Released |

---

## Hook 08 — Auto-Refund

**File:** [`auto_refund.c`](./auto_refund.c) · **Status:** ✅ 5/5 tests · **Triggers:** `ttPAYMENT`

Automatically refunds incoming payments that are missing a required destination tag. Stops the #1 cause of lost exchange deposits worldwide.

**Real world:** A user sends 500 XAH to an exchange deposit address but forgets the destination tag. Without this hook, the funds land in the exchange's omnibus wallet and require a support ticket to recover. With the hook, 499.999 XAH is returned automatically in seconds.

**Tested behaviours (testnet):**

| Test | Description | Result |
|------|-------------|--------|
| 1 | Deploy auto_refund.c | ✅ tesSUCCESS |
| 2 | Payment WITH tag (5 XAH) | ✅ Accepted |
| 3 | Payment WITHOUT tag (5 XAH) | ✅ Refunded |
| 4 | Payment below minimum (0.5 XAH) | ✅ No refund |
| 5 | Loop safety — outgoing ignored | ✅ Verified |

---

## Tech Stack

- **Language:** C (Hook API v0)
- **Compiler:** Hooks Builder (hooks-builder.xrpl.org) or `wasmcc` / `clang`
- **Target:** Xahau Mainnet (Network ID 21337) — also compatible with Xahau Testnet (21338)
- **Wallet:** Xaman (recommended) — any SetHook-capable wallet works
- **Deploy:** Hooks Builder UI or Node.js scripts (`xahau` npm package)

---

## A note on testnet transaction links

Earlier versions of this README linked to individual transaction records on the Xahau testnet explorer. Testnet data is periodically pruned, and the original demo account (`rDiPi...`) has since been deleted on testnet — meaning those links no longer resolve.

**The test results themselves are documented above and are fully reproducible.** Anyone can clone this repo, deploy a hook to a fresh testnet wallet, and reproduce every test case.

**Hook 1 is live on mainnet.** Deploy TX and hook hash are in the Mainnet Deployment section at the top of this README. Testnet transaction links from earlier demo accounts no longer resolve due to pruning, but the test cases above are fully reproducible on any fresh testnet wallet.

---

## Contributing

Found a bug? Have a feature request? Open an issue or send a PR.

For custom hooks or commercial deployments, email **hello@xrplhookflow.com**.

---

## Support

- **Website:** [xrplhookflow.com](https://xrplhookflow.com)
- **Email:** hello@xrplhookflow.com
- **Customer FAQ:** 40-question PDF on the website

---

## License

MIT — use it, fork it, modify it. Just don't blame us if it eats your wallet.

---

## About

HookFlow is a product of **Sulphurtech International Services Ltd.**, incorporated in British Columbia, Canada. Built by Steve Lassu. Hook 1 live on Xahau mainnet since April 18, 2026.
