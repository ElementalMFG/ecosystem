// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_storage_fs_core.c — host harness for the ss_storage self-heal mount
// FSM + wear-stat core.
//
// No Unity dependency: a tiny CHECK macro records failures. Built with
// -fsanitize=address,undefined so any out-of-bounds access aborts the run. Only
// ss_storage_fs_core.c is linked (ss_storage.c needs the IDF).

#include <stdint.h>
#include <stdio.h>

#include "ss_storage_fs_core.h"

static int g_checks = 0;
static int g_fails = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        g_checks++;                                                                                \
        if (!(cond)) {                                                                             \
            g_fails++;                                                                             \
            printf("  FAIL: %s (line %d)\n", #cond, __LINE__);                                     \
        }                                                                                          \
    } while (0)

// Drive the FSM the way the glue does: feed back a caller-supplied result for
// each MOUNT/FORMAT step until a terminal step. `mount_results` / `fmt_results`
// give the OK/ERR outcome for the Nth mount / format performed. Returns the
// terminal step; out-params report how many mounts/formats were performed.
static ss_fs_step_t run_fsm(const ss_fs_result_t* mount_results, const ss_fs_result_t* fmt_results,
                            int* out_mounts, int* out_formats)
{
    ss_fs_heal_state_t st;
    ss_fs_heal_init(&st);
    ss_fs_step_t step = SS_FS_STEP_MOUNT; // first action is always a mount
    int mounts = 0, formats = 0;

    for (int i = 0; i < 16; i++) { // hard cap: FSM must terminate well within this
        ss_fs_result_t res;
        if (step == SS_FS_STEP_MOUNT) {
            res = mount_results[mounts++];
        } else if (step == SS_FS_STEP_FORMAT) {
            res = fmt_results[formats++];
        } else {
            break; // terminal
        }
        step = ss_fs_heal_next(&st, step, res);
    }
    if (out_mounts) { *out_mounts = mounts; }
    if (out_formats) { *out_formats = formats; }
    return step;
}

int main(void)
{
    // 1. Happy path: first mount succeeds → DONE_OK, no format.
    {
        const ss_fs_result_t mounts[] = {SS_FS_R_OK};
        const ss_fs_result_t fmts[] = {SS_FS_R_OK};
        int nm = 0, nf = 0;
        CHECK(run_fsm(mounts, fmts, &nm, &nf) == SS_FS_STEP_DONE_OK);
        CHECK(nm == 1);
        CHECK(nf == 0);
        printf("case 1  happy path: 1 mount, 0 format → DONE_OK\n");
    }

    // 2. Corruption path: mount err → format ok → remount ok → DONE_OK.
    {
        const ss_fs_result_t mounts[] = {SS_FS_R_ERR, SS_FS_R_OK};
        const ss_fs_result_t fmts[] = {SS_FS_R_OK};
        int nm = 0, nf = 0;
        CHECK(run_fsm(mounts, fmts, &nm, &nf) == SS_FS_STEP_DONE_OK);
        CHECK(nm == 2); // one failed, one after-format success
        CHECK(nf == 1); // format tried exactly once
        printf("case 2  self-heal: mount-err → format → remount-ok → DONE_OK\n");
    }

    // 3. Unrecoverable A: format itself fails → FAILED, no 2nd mount.
    {
        const ss_fs_result_t mounts[] = {SS_FS_R_ERR, SS_FS_R_ERR};
        const ss_fs_result_t fmts[] = {SS_FS_R_ERR};
        int nm = 0, nf = 0;
        CHECK(run_fsm(mounts, fmts, &nm, &nf) == SS_FS_STEP_FAILED);
        CHECK(nm == 1); // only the initial mount was attempted
        CHECK(nf == 1);
        printf("case 3  format-err → FAILED (no remount)\n");
    }

    // 4. Unrecoverable B: mount err → format ok → 2nd mount err → FAILED.
    {
        const ss_fs_result_t mounts[] = {SS_FS_R_ERR, SS_FS_R_ERR};
        const ss_fs_result_t fmts[] = {SS_FS_R_OK};
        int nm = 0, nf = 0;
        CHECK(run_fsm(mounts, fmts, &nm, &nf) == SS_FS_STEP_FAILED);
        CHECK(nm == 2);
        CHECK(nf == 1); // format tried at most once even though both mounts failed
        printf("case 4  2nd-mount-err → FAILED; format tried once (bounded)\n");
    }

    // 5. Boundedness/format-once explicit: after a FORMAT the machine never
    //    hands out a second FORMAT regardless of results.
    {
        ss_fs_heal_state_t st;
        ss_fs_heal_init(&st);
        // MOUNT(err) → FORMAT
        CHECK(ss_fs_heal_next(&st, SS_FS_STEP_MOUNT, SS_FS_R_ERR) == SS_FS_STEP_FORMAT);
        // FORMAT(ok) → MOUNT
        CHECK(ss_fs_heal_next(&st, SS_FS_STEP_FORMAT, SS_FS_R_OK) == SS_FS_STEP_MOUNT);
        // MOUNT(err) again → FAILED (NOT another FORMAT)
        CHECK(ss_fs_heal_next(&st, SS_FS_STEP_MOUNT, SS_FS_R_ERR) == SS_FS_STEP_FAILED);
        CHECK(st.formatted == true);
        printf("case 5  format handed out at most once → bounded\n");
    }

    // 6. Fail-safe: NULL state, garbage result, terminal `last` → FAILED.
    {
        ss_fs_heal_state_t st;
        ss_fs_heal_init(&st);
        CHECK(ss_fs_heal_next(NULL, SS_FS_STEP_MOUNT, SS_FS_R_OK) == SS_FS_STEP_FAILED);
        CHECK(ss_fs_heal_next(&st, SS_FS_STEP_MOUNT, (ss_fs_result_t)99) == SS_FS_STEP_FAILED);
        CHECK(ss_fs_heal_next(&st, SS_FS_STEP_DONE_OK, SS_FS_R_OK) == SS_FS_STEP_FAILED);
        CHECK(ss_fs_heal_next(&st, SS_FS_STEP_FAILED, SS_FS_R_ERR) == SS_FS_STEP_FAILED);
        CHECK(ss_fs_heal_next(&st, (ss_fs_step_t)77, SS_FS_R_OK) == SS_FS_STEP_FAILED);
        ss_fs_heal_init(NULL); // no crash under ASan
        printf("case 6  fail-safe: NULL / garbage / terminal → FAILED\n");
    }

    // 7. ss_fs_step_is_terminal helper.
    {
        CHECK(ss_fs_step_is_terminal(SS_FS_STEP_DONE_OK));
        CHECK(ss_fs_step_is_terminal(SS_FS_STEP_FAILED));
        CHECK(!ss_fs_step_is_terminal(SS_FS_STEP_MOUNT));
        CHECK(!ss_fs_step_is_terminal(SS_FS_STEP_FORMAT));
        printf("case 7  is_terminal classifies steps\n");
    }

    // 8. fill_pct edges: 0, 50, 100, total==0, used>total.
    {
        CHECK(ss_fs_fill_pct((ss_fs_usage_t){.total_bytes = 1000, .used_bytes = 0}) == 0);
        CHECK(ss_fs_fill_pct((ss_fs_usage_t){.total_bytes = 1000, .used_bytes = 500}) == 50);
        CHECK(ss_fs_fill_pct((ss_fs_usage_t){.total_bytes = 1000, .used_bytes = 1000}) == 100);
        CHECK(ss_fs_fill_pct((ss_fs_usage_t){.total_bytes = 0, .used_bytes = 0}) == 100);
        CHECK(ss_fs_fill_pct((ss_fs_usage_t){.total_bytes = 0, .used_bytes = 42}) == 100);
        CHECK(ss_fs_fill_pct((ss_fs_usage_t){.total_bytes = 1000, .used_bytes = 5000}) == 100);
        // Large-value no-overflow sanity (used < total, near 2^40).
        CHECK(ss_fs_fill_pct((ss_fs_usage_t){.total_bytes = 1ULL << 40,
                                             .used_bytes = (1ULL << 40) / 4}) == 25);
        printf("case 8  fill_pct edges: 0/50/100, total==0→100, used>total→100\n");
    }

    // 9. free_bytes: normal, exact-full, over-full saturates at 0.
    {
        CHECK(ss_fs_free_bytes((ss_fs_usage_t){.total_bytes = 1000, .used_bytes = 300}) == 700);
        CHECK(ss_fs_free_bytes((ss_fs_usage_t){.total_bytes = 1000, .used_bytes = 1000}) == 0);
        CHECK(ss_fs_free_bytes((ss_fs_usage_t){.total_bytes = 1000, .used_bytes = 5000}) == 0);
        CHECK(ss_fs_free_bytes((ss_fs_usage_t){.total_bytes = 0, .used_bytes = 0}) == 0);
        printf("case 9  free_bytes: total-used, saturating at 0\n");
    }

    printf("\n%d checks, %d failures\n", g_checks, g_fails);
    return (g_fails != 0) ? 1 : 0;
}
