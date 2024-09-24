#include <iostream>
#include <map>
#include <type_traits>
#include <cassert>

template<typename K, typename V>
class interval_map {
    static_assert(std::is_copy_constructible<K>::value&& std::is_move_constructible<K>::value, "Key type K must support copy and move construction.");
    static_assert(std::is_copy_constructible<V>::value&& std::is_move_constructible<V>::value, "Value type V must support copy and move construction.");
    static_assert(std::is_copy_assignable<K>::value&& std::is_move_assignable<K>::value, "Key type K must support copy and move assignment.");
    static_assert(std::is_copy_assignable<V>::value&& std::is_move_assignable<V>::value, "Value type V must support copy and move assignment.");

    // Ensure K is less-than comparable
    static_assert(std::is_same<decltype(std::declval<K>() < std::declval<K>()), bool>::value, "Key type K must be less-than comparable.");

    friend void IntervalMapTest();  // Friend function for testing

    // Member variables
    V m_valBegin;            // Default value for keys less than the first key in m_map
    std::map<K, V> m_map;    // Internal map to store intervals

public:
    // Constructor
    interval_map(V const& default_value) : m_valBegin(default_value) {}

    // Function to assign values to intervals
    template<typename V_forward>
    void assign(K const& keyBegin, K const& keyEnd, V_forward&& val)
        requires (std::is_same<std::remove_cvref_t<V_forward>, V>::value) {
        // Check for invalid interval
        if (keyBegin >= keyEnd) return;

        // Use upper_bound to find the appropriate insert positions
        auto itLow = m_map.lower_bound(keyBegin);
        auto itHigh = m_map.lower_bound(keyEnd);

        // If the previous value is the same, we need to extend the interval
        if (itLow != m_map.begin()) {
            auto itPrev = std::prev(itLow);
            if (itPrev->second == val) {
                // Merge intervals by adjusting keyBegin
                m_map.erase(itLow, itHigh); // Remove old entries
                return; // No need to insert a new entry
            }
        }

        // Remove the existing range (itLow to itHigh)
        m_map.erase(itLow, itHigh);

        // Insert the new value at keyBegin
        m_map[keyBegin] = std::forward<V_forward>(val);

        // Restore the original value at keyEnd if it exists
        if (itHigh != m_map.end() && itHigh->first > keyEnd) {
            m_map[keyEnd] = itHigh->second;  // Set the end boundary value
        }

        // Ensure canonical representation: remove consecutive entries with the same value
        for (auto it = m_map.begin(); it != m_map.end(); ) {
            auto next_it = std::next(it);
            if (next_it != m_map.end() && it->second == next_it->second) {
                // Remove the next iterator if it has the same value
                m_map.erase(next_it);
            }
            else {
                // Move to the next entry
                it++;
            }
        }
    }

    // Access operator to get the value associated with a key
    V const& operator[](K const& key) const {
        auto it = m_map.upper_bound(key);
        if (it == m_map.begin()) {
            return m_valBegin;  // Return the default value for keys less than the first key
        }
        else {
            return (--it)->second;  // Return the value associated with the largest key <= key
        }
    }
};

// Function to test the interval_map class
void IntervalMapTest() {
    interval_map<int, char> imap('A');  // Create an interval map with default value 'A'

    // Assign 'B' to the interval [1, 3)
    imap.assign(1, 3, 'B');

    // Assign 'C' to the interval [5, 7)
    imap.assign(5, 7, 'C');

    // Test access before and after the intervals
    assert(imap[0] == 'A');  // Default value 'A' for keys < 1
    assert(imap[1] == 'B');  // 'B' from [1, 3)
    assert(imap[2] == 'B');  // 'B' from [1, 3)
    assert(imap[3] == 'A');  // Back to default 'A'
    assert(imap[5] == 'C');  // 'C' from [5, 7)
    assert(imap[6] == 'C');  // 'C' from [5, 7)
    assert(imap[7] == 'A');  // Back to default 'A'

    std::cout << "All tests passed!" << std::endl;
}

int main() {
    IntervalMapTest();  // Call the test function
    return 0;
}
