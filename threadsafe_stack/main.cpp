#include <iostream>
#include <stack>
#include <exception>
#include <future>
#include <vector>
#include <unistd.h>

struct empty_stack: std::exception
{
    const char * what() const noexcept override
    {
        return "empty stack";
    }
};

template<typename T>
class threadsafe_stack
{
public:
    threadsafe_stack()
    {
    }
    
    threadsafe_stack(threadsafe_stack const & other)
    {
        std::lock_guard<std::mutex> lock = { other.m_mutex };
        m_data = other.m_data;
    }
    
    auto operator =(threadsafe_stack const & ) -> threadsafe_stack & = delete;
    
    auto push(T new_value) -> void
    {
        std::lock_guard<std::mutex> lock { m_mutex };
        m_data.push(std::move(new_value));
    }
    
    auto pop() -> std::shared_ptr<T>
    {
        std::lock_guard<std::mutex> lock { m_mutex };
        if ( m_data.empty() ) {
            throw empty_stack();
        }
        
        std::shared_ptr<T> const result = { std::make_shared<T>(std::move(m_data.top())) };
        m_data.pop();
        
        return result;
    }
    
    auto pop(T & value) -> void
    {
        std::lock_guard<std::mutex> lock { m_mutex };
        if ( m_data.empty() ) {
            throw empty_stack();
        }
        
        value = std::move(m_data.top());
        m_data.pop();
    }
    
    auto empty() const -> bool
    {
        std::lock_guard<std::mutex> lock { m_mutex };
        return m_data.empty();
    }
    
private:
    mutable std::mutex m_mutex;
    std::stack<T> m_data;
};

int main(int argc, const char * argv[]) {
    
    threadsafe_stack<int> stack;
    
    std::vector<std::future<void>> results;
    
    results.push_back(
    std::async(std::launch::async, [&stack] {
        for (;;) {
            stack.push(10);
        }
    }));
    
    results.push_back(std::async(std::launch::async,[&stack] {
        for (;;) {
            stack.pop();
        }
    }));
    
    
    sleep(10);
    
    std::cout << "Hello, World!\n";
    return 0;
}
