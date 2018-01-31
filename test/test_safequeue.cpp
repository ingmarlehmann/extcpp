#include <gtest/gtest.h>
#include <extcpp/safequeue.hpp>
#include <future>

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class TestEvent
{
public:
    TestEvent(int a, int b) : a_(a), b_(b) {}
    TestEvent() {}

    bool operator==(const TestEvent& rhs) const
    {
        return (this->a_ == rhs.a_ && this->b_ == rhs.b_);
    }

public:
    int a_{4};
    int b_{5};
};

class Fixture :
        public ::testing::Test
{
public:
    // Test interface
    Fixture() {};
    virtual ~Fixture() override {}

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(Fixture, st_construct)
{
    extcpp::SafeQueue<int> queue;
    extcpp::SafeQueue<TestEvent> queue2;
}

TEST_F(Fixture, st_construct_destruct)
{
    {
        extcpp::SafeQueue<int> queue;
    }
    {
        extcpp::SafeQueue<TestEvent> queue;
    }
}

TEST_F(Fixture, st_push_pod)
{
    extcpp::SafeQueue<int> queue;

    const int NUM_ELEMENTS = 1000;
    for(int i=0; i<NUM_ELEMENTS; ++i)
    {
        queue.push(i);
    }

    ASSERT_EQ(queue.size(), NUM_ELEMENTS);
}

TEST_F(Fixture, st_pop_pod)
{
    extcpp::SafeQueue<int> queue;

    const int NUM_ELEMENTS = 1000;
    for(int i=0; i<NUM_ELEMENTS; ++i)
    {
        queue.push(i);
    }

    ASSERT_EQ(queue.size(), NUM_ELEMENTS);

    for(int i=0; i<NUM_ELEMENTS; ++i)
    {
        ASSERT_EQ(queue.pop(), i);
    }

    ASSERT_EQ(queue.size(), 0);
}

TEST_F(Fixture, st_push_user_type)
{
    extcpp::SafeQueue<TestEvent> queue;

    const int NUM_ELEMENTS = 1000;
    for(int i=0; i<NUM_ELEMENTS; ++i)
    {
        queue.push(TestEvent(i, 0));
    }

    ASSERT_EQ(queue.size(), NUM_ELEMENTS);
}

TEST_F(Fixture, st_pop_user_type)
{
    extcpp::SafeQueue<TestEvent> queue;

    const int NUM_ELEMENTS = 1000;
    for(int i=0; i<NUM_ELEMENTS; ++i)
    {
        queue.push(TestEvent(i, 0));
    }

    ASSERT_EQ(queue.size(), NUM_ELEMENTS);

    for(int i=0; i<NUM_ELEMENTS; ++i)
    {
        TestEvent ev(i, 0);
        ASSERT_EQ(queue.pop(), ev);
    }

    ASSERT_EQ(queue.size(), 0);
}

TEST_F(Fixture, st_ensure_pop_block)
{
    extcpp::SafeQueue<int> queue;

    auto cb = [&queue](){queue.pop();};
    auto future = std::async(std::launch::async, cb);
    auto status = future.wait_for(std::chrono::seconds(2));
    ASSERT_EQ(status, std::future_status::timeout);

    queue.push(1);
}

TEST_F(Fixture, mt_push_pod)
{
    extcpp::SafeQueue<int> queue;

    auto push_n_items = [&queue](int start_value, int num_entries)
    {
        for(int i=start_value; i<num_entries; ++i)
        {
            queue.push(i);
        }
    };

    auto future_t1 = std::async(std::launch::async, push_n_items,     0, 20000);
    auto future_t2 = std::async(std::launch::async, push_n_items, 20000, 40000);
    auto future_t3 = std::async(std::launch::async, push_n_items, 40000, 60000);
    auto future_t4 = std::async(std::launch::async, push_n_items, 60000, 80000);

    future_t1.get();
    future_t2.get();
    future_t3.get();
    future_t4.get();

    ASSERT_EQ(queue.size(), 80000);
}

TEST_F(Fixture, mt_pop_pod)
{
    extcpp::SafeQueue<int> queue;

    std::vector<int> values;
    values.reserve(80000);

    for(int i=0; i<80000; ++i)
    {
        values.push_back(i);
    }

    auto push_n_items = [&queue](int start_value, int num_items)
    {
        for(int i=start_value; i<num_items; ++i)
        {
            queue.push(i);
        }
    };

    auto pop_n_items = [&queue, &values](int num_items)
    {
        for(int i=0; i<num_items; ++i)
        {
            int value = queue.pop();
            int size = queue.size();
            auto iter = std::find(std::begin(values), std::end(values), value);
            ASSERT_NE(iter, values.end());
        }
    };

    auto future_t1 = std::async(std::launch::async, push_n_items,     0, 20000);
    auto future_t2 = std::async(std::launch::async, push_n_items, 20000, 40000);
    auto future_t3 = std::async(std::launch::async, push_n_items, 40000, 60000);
    auto future_t4 = std::async(std::launch::async, push_n_items, 60000, 80000);

    auto future_t5 = std::async(std::launch::async, pop_n_items, 40000);
    auto future_t6 = std::async(std::launch::async, pop_n_items, 40000);

    future_t1.get();
    future_t2.get();
    future_t3.get();
    future_t4.get();
    future_t5.get();
    future_t6.get();

    ASSERT_EQ(queue.size(), 0);
}

TEST_F(Fixture, mt_pop_user_type)
{
    extcpp::SafeQueue<TestEvent> queue;

    std::vector<TestEvent> values;
    values.reserve(80000);

    for(int i=0; i<80000; ++i)
    {
        values.push_back(TestEvent(i, 0));
    }

    auto push_n_items = [&queue](int start_value, int num_items)
    {
        for(int i=start_value; i<num_items; ++i)
        {
            queue.push(TestEvent(i, 0));
        }
    };

    auto pop_n_items = [&queue, &values](int num_items)
    {
        for(int i=0; i<num_items; ++i)
        {
            TestEvent value = queue.pop();

            // throw this in to increase the chaces of exposing potential threading related bugs.
            int size = queue.size();             auto iter = std::find(std::begin(values), std::end(values), value);

            ASSERT_NE(iter, values.end());
        }
    };

    auto future_t1 = std::async(std::launch::async, push_n_items,     0, 20000);
    auto future_t2 = std::async(std::launch::async, push_n_items, 20000, 40000);
    auto future_t3 = std::async(std::launch::async, push_n_items, 40000, 60000);
    auto future_t4 = std::async(std::launch::async, push_n_items, 60000, 80000);

    auto future_t5 = std::async(std::launch::async, pop_n_items, 40000);
    auto future_t6 = std::async(std::launch::async, pop_n_items, 40000);

    future_t1.get();
    future_t2.get();
    future_t3.get();
    future_t4.get();
    future_t5.get();
    future_t6.get();

    ASSERT_EQ(queue.size(), 0);
}
