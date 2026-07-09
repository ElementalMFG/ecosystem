// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 SS-SP Project Contributors
//
// Host gtest wrapper for the HAL conformance suite (S-03-022). Three concerns:
//   1. The builtin vector matrix (CONFORMANCE_SPEC.md §3) passes against the
//      pure-C mock env with zero diffs — a mock/vector drift oracle.
//   2. ss_conf_diff_format emits the frozen grammar byte-for-byte (asserted
//      against literal strings).
//   3. A deliberately non-conforming env produces the expected CONF-FAIL diff
//      line — the harness self-check that "failures produce actionable diffs".

#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

extern "C" {
#include "ss_hal_conformance.h"
#include "ss_hal_mock_env.h"
}

namespace
{

// ---- 1. builtin vectors pass against the mock ------------------------------

TEST(HalConformance, BuiltinVectorsPassAgainstMock)
{
    uint32_t n = 0u;
    const ss_conf_vector_t* vecs = ss_conf_builtin_vectors(&n);
    ASSERT_NE(vecs, nullptr);
    ASSERT_GE(n, 1u);

    ss_hal_mock_ctx_t ctx;
    ss_conf_env_t env;
    ss_hal_mock_env_init(&env, &ctx);

    ss_conf_result_t res;
    int32_t failed = ss_conf_run(vecs, n, &env, &res);

    // Surface every emitted diff so a regression names exactly what diverged.
    if (failed != 0) {
        const uint32_t shown =
            (ctx.diff_count < SS_HAL_MOCK_MAX_DIFFS) ? ctx.diff_count : SS_HAL_MOCK_MAX_DIFFS;
        for (uint32_t i = 0u; i < shown; ++i) { ADD_FAILURE() << ctx.diff_lines[i]; }
    }

    EXPECT_EQ(failed, 0);
    EXPECT_EQ(res.vectors_skipped, 0u); // mock claims all five domain caps
    EXPECT_EQ(res.diffs_emitted, 0u);
    EXPECT_EQ(res.vectors_run, res.vectors_passed);
    EXPECT_EQ(res.vectors_run, n);
}

// ---- 2. diff formatter grammar (frozen, byte-for-byte) ---------------------

TEST(HalConformance, DiffFormatMatchesFrozenGrammarRet)
{
    // The example from ss_hal_conformance.h.
    const ss_conf_diff_t d = {SS_CONF_DOM_POWER,
                              "wake-timer-arg-validation",
                              1u,
                              SS_CONF_OP_PWR_WAKE_TIMER_SET,
                              SS_CONF_FIELD_RET,
                              0x102u,
                              0x0u};
    const char* expected = "CONF-FAIL dom=power vec=wake-timer-arg-validation step=1 "
                           "op=PWR_WAKE_TIMER_SET field=ret expected=0x00000102 actual=0x00000000";

    char buf[SS_CONF_DIFF_LINE_MAX];
    uint32_t n = ss_conf_diff_format(&d, buf, sizeof(buf));
    EXPECT_STREQ(buf, expected);
    EXPECT_EQ(n, static_cast<uint32_t>(std::strlen(expected)));
}

TEST(HalConformance, DiffFormatCoversStateAuxExecAndDomains)
{
    char buf[SS_CONF_DIFF_LINE_MAX];

    const ss_conf_diff_t d_state = {SS_CONF_DOM_LORA,
                                    "rx-start-stop",
                                    2u,
                                    SS_CONF_OP_LORA_RX_STOP,
                                    SS_CONF_FIELD_STATE,
                                    0x1u,
                                    0x2u};
    ss_conf_diff_format(&d_state, buf, sizeof(buf));
    EXPECT_STREQ(buf, "CONF-FAIL dom=lora vec=rx-start-stop step=2 op=LORA_RX_STOP "
                      "field=state expected=0x00000001 actual=0x00000002");

    const ss_conf_diff_t d_aux = {
        SS_CONF_DOM_BLE,   "scan-lifecycle", 1u,  SS_CONF_OP_BLE_SCAN_START,
        SS_CONF_FIELD_AUX, 0x10000u,         0x0u};
    ss_conf_diff_format(&d_aux, buf, sizeof(buf));
    EXPECT_STREQ(buf, "CONF-FAIL dom=ble vec=scan-lifecycle step=1 op=BLE_SCAN_START "
                      "field=aux expected=0x00010000 actual=0x00000000");

    const ss_conf_diff_t d_exec = {SS_CONF_DOM_WIFI,   "lifecycle", 0u,  SS_CONF_OP_WIFI_INIT,
                                   SS_CONF_FIELD_EXEC, 0x1u,        0x0u};
    ss_conf_diff_format(&d_exec, buf, sizeof(buf));
    EXPECT_STREQ(buf, "CONF-FAIL dom=wifi vec=lifecycle step=0 op=WIFI_INIT "
                      "field=exec expected=0x00000001 actual=0x00000000");
}

TEST(HalConformance, DiffFormatNullAndZeroCapTolerated)
{
    char buf[8];
    EXPECT_EQ(ss_conf_diff_format(nullptr, buf, sizeof(buf)), 0u);
    const ss_conf_diff_t d = {SS_CONF_DOM_POWER, "x", 0u, SS_CONF_OP_PWR_INIT,
                              SS_CONF_FIELD_RET, 0u,  0u};
    EXPECT_EQ(ss_conf_diff_format(&d, nullptr, 16u), 0u);
    EXPECT_EQ(ss_conf_diff_format(&d, buf, 0u), 0u);

    // Truncation stays NUL-terminated and within cap.
    uint32_t n = ss_conf_diff_format(&d, buf, sizeof(buf));
    EXPECT_EQ(buf[sizeof(buf) - 1u], '\0');
    EXPECT_LT(n, sizeof(buf));
    EXPECT_EQ(std::strncmp(buf, "CONF-FA", 7), 0);
}

// ---- 3. negative self-check: non-conforming env -> actionable diff ---------

struct NegCtx {
    std::vector<std::string> lines;
};

bool neg_reset(void* /*ctx*/, const ss_conf_vector_t* /*vec*/)
{
    return true;
}

// Always reports OK/zero — but the vector demands INVALID_ARG, so it diverges.
bool neg_exec(void* /*ctx*/, const ss_conf_step_t* /*step*/, ss_conf_actual_t* out)
{
    out->ret = SS_CONF_RET_OK;
    out->state = 0u;
    out->aux = 0u;
    return true;
}

void neg_emit(void* ctx, const char* line)
{
    static_cast<NegCtx*>(ctx)->lines.emplace_back(line);
}

TEST(HalConformance, NonConformingEnvProducesActionableDiff)
{
    static const ss_conf_step_t steps[] = {
        {SS_CONF_OP_PWR_WAKE_TIMER_SET,
         0u,
         0u,
         0u,
         nullptr,
         {SS_CONF_RET_ERR_INVALID_ARG, 0u, 0u, SS_CONF_CHECK_RET}},
    };
    static const ss_conf_vector_t vec = {SS_CONF_DOM_POWER, "neg-selfcheck", 0u, steps, 1u};

    NegCtx nctx;
    ss_conf_env_t env = {neg_reset, neg_exec, neg_emit, &nctx};

    ss_conf_result_t res;
    int32_t failed = ss_conf_run(&vec, 1u, &env, &res);

    EXPECT_EQ(failed, 1); // exactly one vector failed
    EXPECT_EQ(res.diffs_emitted, 1u);
    ASSERT_EQ(nctx.lines.size(), 1u);
    EXPECT_EQ(nctx.lines[0], "CONF-FAIL dom=power vec=neg-selfcheck step=0 op=PWR_WAKE_TIMER_SET "
                             "field=ret expected=0x00000102 actual=0x00000000");
}

// ---- runner guard rails ----------------------------------------------------

TEST(HalConformance, RunnerRejectsNullEnv)
{
    uint32_t n = 0u;
    const ss_conf_vector_t* vecs = ss_conf_builtin_vectors(&n);
    EXPECT_EQ(ss_conf_run(vecs, n, nullptr, nullptr), SS_CONF_E_ARG);
}

} // namespace
