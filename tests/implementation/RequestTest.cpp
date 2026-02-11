#include <RequestTest.h>

void RequestTest::SetUp()
{
    backend = std::make_unique<BackendEnvironment>();
    backend->SetUp();
}

void RequestTest::TearDown()
{
    backend->TearDown();
    backend.reset();
}