#include <iostream>
#include <cstring>
#include <cassert>
#include <unistd.h>

#include "net/Buffer.h"

#define EXPECT_TRUE(expr)                                                    \
    do {                                                                     \
        if (!(expr)) {                                                       \
            std::cerr << "[FAILED] " << #expr                                \
                      << "  (" << __FILE__ << ":" << __LINE__ << ")\n";       \
            return EXIT_FAILURE;                                             \
        }                                                                    \
    } while (0)

#define EXPECT_EQ(a, b)                                                      \
    do {                                                                     \
        auto _a = (a);                                                       \
        auto _b = (b);                                                       \
        if (!(_a == _b)) {                                                   \
            std::cerr << "[FAILED] " << #a << " != " << #b                   \
                      << "\nExpected: " << _b                                \
                      << "\nActual  : " << _a                                \
                      << "\nLine    : " << __LINE__                          \
                      << std::endl;                                          \
            return EXIT_FAILURE;                                             \
        }                                                                    \
    } while (0)

int TestInitialState()
{
    std::cout << "Running TestInitialState..." << std::endl;

    Buffer buf;

    EXPECT_EQ(buf.readableBytes(), 0u);
    EXPECT_EQ(buf.prependableBytes(), Buffer::kCheapPrepend);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize);

    std::cout << "[PASS]\n";
}

int TestAppend()
{
    std::cout << "Running TestAppend..." << std::endl;

    Buffer buf;

    const char* msg = "Hello Buffer";

    buf.append(msg, strlen(msg));

    EXPECT_EQ(buf.readableBytes(), strlen(msg));
    EXPECT_TRUE(std::string(buf.peek(), strlen(msg)) == "Hello Buffer");

    std::cout << "[PASS]\n";
}

int TestRetrieve()
{
    std::cout << "Running TestRetrieve..." << std::endl;

    Buffer buf;

    buf.append("abcdef", 6);

    buf.retrieve(2);

    EXPECT_EQ(buf.readableBytes(), 4u);
    EXPECT_TRUE(std::string(buf.peek(), 4) == "cdef");

    std::cout << "[PASS]\n";
}

int TestRetrieveString()
{
    std::cout << "Running TestRetrieveString..." << std::endl;

    Buffer buf;

    buf.append("abcdef", 6);

    auto s = buf.retrieveString(3);

    EXPECT_TRUE(s == "abc");

    EXPECT_EQ(buf.readableBytes(), 3u);

    EXPECT_TRUE(std::string(buf.peek(), 3) == "def");

    std::cout << "[PASS]\n";
}

int TestRetrieveAllString()
{
    std::cout << "Running TestRetrieveAllString..." << std::endl;

    Buffer buf;

    buf.append("network", 7);

    auto s = buf.retrieveAllString();

    EXPECT_TRUE(s == "network");

    EXPECT_EQ(buf.readableBytes(), 0u);

    EXPECT_EQ(buf.prependableBytes(), Buffer::kCheapPrepend);

    std::cout << "[PASS]\n";
}

int TestExpand()
{
    std::cout << "Running TestExpand..." << std::endl;

    Buffer buf(8);

    std::string big(5000, 'A');

    buf.append(big.data(), big.size());

    EXPECT_EQ(buf.readableBytes(), big.size());

    EXPECT_TRUE(std::string(buf.peek(), big.size()) == big);

    std::cout << "[PASS]\n";
}

int TestReadFd()
{
    std::cout << "Running TestReadFd..." << std::endl;

    int pipefd[2];

    EXPECT_TRUE(pipe(pipefd) == 0);

    const char* msg = "Hello Pipe";

    write(pipefd[1], msg, strlen(msg));

    Buffer buf;

    int savedErrno = 0;

    ssize_t n = buf.readFd(pipefd[0], &savedErrno);

    EXPECT_EQ(n, (ssize_t)strlen(msg));

    EXPECT_EQ(buf.readableBytes(), strlen(msg));

    EXPECT_TRUE(std::string(buf.peek(), strlen(msg)) == msg);

    close(pipefd[0]);
    close(pipefd[1]);

    std::cout << "[PASS]\n";
}

// int main()
// {
//     std::cout << "==============================" << std::endl;
//     std::cout << "       Buffer Unit Test       " << std::endl;
//     std::cout << "==============================" << std::endl;
//
//     TestInitialState();
//
//     TestAppend();
//
//     TestRetrieve();
//
//     TestRetrieveString();
//
//     TestRetrieveAllString();
//
//     TestExpand();
//
//     TestReadFd();
//
//     std::cout << std::endl;
//     std::cout << "==============================" << std::endl;
//     std::cout << " ALL TESTS PASSED SUCCESSFULLY" << std::endl;
//     std::cout << "==============================" << std::endl;
//
//     return EXIT_SUCCESS;
// }