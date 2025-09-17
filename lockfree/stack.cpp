#include <thread>
#include <atomic>


template<typename T>
class lock_free_stack {
private:
    struct node {
        T data;
        node* next;

        node(const T& data_) : data(data_) {}
    };

    std::atomic<node*> head_;
    static_assert(std::atomic<node*>::is_always_lock_free);
public:
    void push(const T& data) {
        node* const new_node = new node(data);
        new_node->next = head_.load();
        while(!head_.compare_exchange_weak(new_node->next, new_node));
    }
};

int main() {
    lock_free_stack<int> s;
    std::thread t1([&s]() {
        s.push(4);
        s.push(5);
    });
    std::thread t2([&s]() {
        s.push(6);
        s.push(7);
    });


    t1.join();
    t2.join();
    return 0;
}










