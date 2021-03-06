#include "arq_in_unit_tests.h"
#include "arq_runtime_mock_plugin.h"
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cstring>

TEST_GROUP(recv) {};

namespace {

TEST(recv, invalid_params)
{
    void *p = nullptr;
    unsigned size;
    arq_t arq;
    CHECK_EQUAL(ARQ_ERR_INVALID_PARAM, arq_recv(nullptr, p, 1, &size));
    CHECK_EQUAL(ARQ_ERR_INVALID_PARAM, arq_recv(&arq, nullptr, 1, &size));
    CHECK_EQUAL(ARQ_ERR_INVALID_PARAM, arq_recv(&arq, p, 1, nullptr));
}

unsigned MockRecvWndRecv(arq__recv_wnd_t *rw, void *dst, unsigned dst_max)
{
    return mock().actualCall("arq__recv_wnd_recv").withParameter("rw", rw)
                                                  .withParameter("dst", dst)
                                                  .withParameter("dst_max", dst_max)
                                                  .returnUnsignedIntValue();
}

struct Fixture
{
    Fixture()
    {
        ARQ_MOCK_HOOK(arq__recv_wnd_recv, MockRecvWndRecv);
        arq.need_poll = ARQ_FALSE;
    }
    arq_t arq;
    char recv[20];
    unsigned recvd;
};

TEST(recv, need_poll)
{
    Fixture f;
    f.arq.need_poll = ARQ_TRUE;
    mock().ignoreOtherCalls();
    arq_err_t const e = arq_recv(&f.arq, f.recv, sizeof(f.recv), &f.recvd);
    CHECK_EQUAL(ARQ_ERR_POLL_REQUIRED, e);
}

TEST(recv, calls_wnd_recv)
{
    Fixture f;
    mock().expectOneCall("arq__recv_wnd_recv").withParameter("rw", &f.arq.recv_wnd)
                                              .withParameter("dst", (void *)f.recv)
                                              .withParameter("dst_max", sizeof(f.recv));
    arq_recv(&f.arq, f.recv, sizeof(f.recv), &f.recvd);
}

TEST(recv, wnd_recv_return_copies_into_output_size_param)
{
    Fixture f;
    mock().expectOneCall("arq__recv_wnd_recv").ignoreOtherParameters().andReturnValue(4321);
    arq_recv(&f.arq, f.recv, sizeof(f.recv), &f.recvd);
    CHECK_EQUAL(4321, f.recvd);
}

TEST(recv, returns_success)
{
    Fixture f;
    mock().expectOneCall("arq__recv_wnd_recv").ignoreOtherParameters();
    arq_err_t const rv = arq_recv(&f.arq, f.recv, sizeof(f.recv), &f.recvd);
    CHECK_EQUAL(ARQ_OK_COMPLETED, rv);
}

}

