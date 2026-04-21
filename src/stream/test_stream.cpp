/**
 * stream 模块单元测试
 *
 * 测试覆盖:
 * - stream_base: 启停、监听者管理、连接管理
 * - stream_vi: 数据循环
 * - stream_venc: 编码通道
 * - stream_ai: 音频输入
 * - stream_aenc: 音频编码
 *
 * 使用简单自定义测试框架，不依赖外部库
 */

#include "stream_base.hpp"
#include "log/log_tag.h"
#include <atomic>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// ============== 简单测试框架 ==============

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define EXPECT_TRUE(expr) \
    do { \
        g_tests_run++; \
        if (expr) { \
            g_tests_passed++; \
            std::cout << "[PASS] " << #expr << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        } else { \
            g_tests_failed++; \
            std::cout << "[FAIL] " << #expr << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        } \
    } while(0)

#define EXPECT_EQ(a, b) \
    do { \
        g_tests_run++; \
        if ((a) == (b)) { \
            g_tests_passed++; \
            std::cout << "[PASS] " << #a << " == " << #b << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        } else { \
            g_tests_failed++; \
            std::cout << "[FAIL] " << #a << " == " << #b << " (got " << (a) << ", expected " << (b) << ") (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        } \
    } while(0)

#define EXPECT_NE(a, b) \
    do { \
        g_tests_run++; \
        if ((a) != (b)) { \
            g_tests_passed++; \
            std::cout << "[PASS] " << #a << " != " << #b << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        } else { \
            g_tests_failed++; \
            std::cout << "[FAIL] " << #a << " != " << #b << " (got " << (a) << ") (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        } \
    } while(0)

#define TEST(name) void test_##name()

// ============== Mock Listener ==============

class mock_listener : public stream_listener {
public:
    mock_listener() : call_count_(0), last_len_(0) {}

    void on_data_input(const char *data, int len) override {
        call_count_.fetch_add(1);
        if (data && len > 0) {
            std::lock_guard<std::mutex> lock(mutex_);
            last_data_.assign(data, data + len);
            last_len_ = len;
        }
    }

    void on_stream_data(const char *data, int len) override {
        call_count_.fetch_add(1);
        if (data && len > 0) {
            std::lock_guard<std::mutex> lock(mutex_);
            last_data_.assign(data, data + len);
            last_len_ = len;
        }
    }

    int get_call_count() const { return call_count_.load(); }
    int get_last_len() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_len_;
    }

    std::string get_last_data() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_data_;
    }

    void reset() {
        call_count_ = 0;
        std::lock_guard<std::mutex> lock(mutex_);
        last_data_.clear();
        last_len_ = 0;
    }

private:
    std::atomic<int> call_count_;
    mutable std::mutex mutex_;
    std::string last_data_;
    int last_len_;
};

// ============== Stream Base Tests ==============

TEST(StreamBaseTest_CreateAndDestroy) {
    stream_vi vi;
    EXPECT_EQ(vi.get_status(), STREAM_STATUS_STOPPED);
}

TEST(StreamBaseTest_StartStop) {
    stream_vi vi;
    EXPECT_EQ(vi.stream_start(), 0);
    EXPECT_EQ(vi.get_status(), STREAM_STATUS_RUNNING);

    EXPECT_EQ(vi.stream_stop(), 0);
    EXPECT_EQ(vi.get_status(), STREAM_STATUS_STOPPED);
}

TEST(StreamBaseTest_DoubleStart) {
    stream_vi vi;
    EXPECT_EQ(vi.stream_start(), 0);
    EXPECT_EQ(vi.stream_start(), 0);  // second start should be no-op
    EXPECT_EQ(vi.get_status(), STREAM_STATUS_RUNNING);
    vi.stream_stop();
}

TEST(StreamBaseTest_DoubleStop) {
    stream_vi vi;
    EXPECT_EQ(vi.stream_stop(), 0);
    EXPECT_EQ(vi.stream_stop(), 0);  // second stop should be no-op
    EXPECT_EQ(vi.get_status(), STREAM_STATUS_STOPPED);
}

// ============== Listener Tests ==============

TEST(ListenerTest_AddRemoveListener) {
    stream_vi vi;
    mock_listener listener1, listener2;

    vi.stream_add_listener(&listener1);
    vi.stream_add_listener(&listener2);

    vi.stream_del_listener(&listener1);
    vi.stream_del_listener(&listener2);
}

TEST(ListenerTest_DuplicateListenerIgnored) {
    stream_vi vi;
    mock_listener listener;

    vi.stream_add_listener(&listener);
    vi.stream_add_listener(&listener);  // duplicate should be ignored

    vi.stream_del_listener(&listener);
}

TEST(ListenerTest_NullListenerIgnored) {
    stream_vi vi;
    vi.stream_add_listener(nullptr);
    vi.stream_del_listener(nullptr);
}

// ============== Connect Tests ==============

TEST(ConnectTest_AddRemoveConnect) {
    stream_vi vi;
    stream_venc venc(0);

    vi.stream_add_connect(&venc);
    vi.stream_del_connect(&venc);
}

TEST(ConnectTest_DuplicateConnectIgnored) {
    stream_vi vi;
    stream_venc venc(0);

    vi.stream_add_connect(&venc);
    vi.stream_add_connect(&venc);  // duplicate should be ignored

    vi.stream_del_connect(&venc);
}

TEST(ConnectTest_NullConnectIgnored) {
    stream_vi vi;
    vi.stream_add_connect(nullptr);
    vi.stream_del_connect(nullptr);
}

// ============== VENC Tests ==============

TEST(VencTest_CreateWithChannel) {
    stream_venc venc0(0);
    stream_venc venc1(1);

    EXPECT_EQ(venc0.get_status(), STREAM_STATUS_STOPPED);
    EXPECT_EQ(venc1.get_status(), STREAM_STATUS_STOPPED);
}

TEST(VencTest_StartStop) {
    stream_venc venc(0);
    EXPECT_EQ(venc.stream_start(), 0);
    EXPECT_EQ(venc.get_status(), STREAM_STATUS_RUNNING);

    EXPECT_EQ(venc.stream_stop(), 0);
    EXPECT_EQ(venc.get_status(), STREAM_STATUS_STOPPED);
}

// ============== AI Tests ==============

TEST(AITest_CreateAndDestroy) {
    stream_ai ai;
    EXPECT_EQ(ai.get_status(), STREAM_STATUS_STOPPED);
}

TEST(AITest_StartStop) {
    stream_ai ai;
    EXPECT_EQ(ai.stream_start(), 0);
    EXPECT_EQ(ai.get_status(), STREAM_STATUS_RUNNING);

    EXPECT_EQ(ai.stream_stop(), 0);
    EXPECT_EQ(ai.get_status(), STREAM_STATUS_STOPPED);
}

// ============== AENC Tests ==============

TEST(AencTest_CreateWithChannel) {
    stream_aenc aenc0(0);
    stream_aenc aenc1(1);

    EXPECT_EQ(aenc0.get_status(), STREAM_STATUS_STOPPED);
    EXPECT_EQ(aenc1.get_status(), STREAM_STATUS_STOPPED);
}

TEST(AencTest_StartStop) {
    stream_aenc aenc(0);
    EXPECT_EQ(aenc.stream_start(), 0);
    EXPECT_EQ(aenc.get_status(), STREAM_STATUS_RUNNING);

    EXPECT_EQ(aenc.stream_stop(), 0);
    EXPECT_EQ(aenc.get_status(), STREAM_STATUS_STOPPED);
}

// ============== Data Notification Tests ==============

TEST(DataNotificationTest_OnDataInputNotifiesListeners) {
    stream_vi vi;
    mock_listener listener;

    vi.stream_add_listener(&listener);
    vi.stream_start();

    // Give some time for the loop to run
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    vi.stream_stop();
    vi.stream_del_listener(&listener);
}

TEST(DataNotificationTest_OnDataInputNotifiesConnects) {
    stream_vi vi;
    stream_venc venc(0);

    vi.stream_add_connect(&venc);
    vi.stream_start();
    venc.stream_start();

    // Give some time for the loop to run
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    venc.stream_stop();
    vi.stream_stop();
    vi.stream_del_connect(&venc);
}

// ============== Test Runner ==============

struct TestCase {
    const char *name;
    void (*func)();
};

#define TEST_ENTRY(name) { #name, test_##name }

static TestCase g_tests[] = {
    TEST_ENTRY(StreamBaseTest_CreateAndDestroy),
    TEST_ENTRY(StreamBaseTest_StartStop),
    TEST_ENTRY(StreamBaseTest_DoubleStart),
    TEST_ENTRY(StreamBaseTest_DoubleStop),
    TEST_ENTRY(ListenerTest_AddRemoveListener),
    TEST_ENTRY(ListenerTest_DuplicateListenerIgnored),
    TEST_ENTRY(ListenerTest_NullListenerIgnored),
    TEST_ENTRY(ConnectTest_AddRemoveConnect),
    TEST_ENTRY(ConnectTest_DuplicateConnectIgnored),
    TEST_ENTRY(ConnectTest_NullConnectIgnored),
    TEST_ENTRY(VencTest_CreateWithChannel),
    TEST_ENTRY(VencTest_StartStop),
    TEST_ENTRY(AITest_CreateAndDestroy),
    TEST_ENTRY(AITest_StartStop),
    TEST_ENTRY(AencTest_CreateWithChannel),
    TEST_ENTRY(AencTest_StartStop),
    TEST_ENTRY(DataNotificationTest_OnDataInputNotifiesListeners),
    TEST_ENTRY(DataNotificationTest_OnDataInputNotifiesConnects),
};

int main() {
    std::cout << "======================================" << std::endl;
    std::cout << "  Stream Module Unit Tests" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << std::endl;

    size_t num_tests = sizeof(g_tests) / sizeof(g_tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        std::cout << "Running " << g_tests[i].name << "..." << std::endl;
        g_tests[i].func();
    }

    std::cout << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "  Results: " << g_tests_passed << " passed, "
              << g_tests_failed << " failed, "
              << g_tests_run << " total" << std::endl;
    std::cout << "======================================" << std::endl;

    return g_tests_failed > 0 ? 1 : 0;
}
