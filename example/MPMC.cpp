#include <qinmo/tool.h> // <qinmo/tool/MPMCUnbounded.h>

int main()
{
    /// Bounded
    qinmo::MPMCBounded<int> queue(1);
    qinmo::println(queue.push(1));
    qinmo::println(queue.push(2));
    int temp = -1;
    qinmo::println(queue.pop(temp), " temp=", temp);
    qinmo::println(queue.pop(temp), " temp=", temp);


    // Unbounded
    qinmo::MPMCUnbounded<int> q(8);
    temp = 0;
    std::atomic<int> i{0};
    std::thread th1(
        [&]() -> void
        {
            while (true)
            {
                q.push(++i);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }
    );
    std::thread th2(
        [&]() -> void
        {
            while (true)
            {
                q.push(++i);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }
    );
    std::thread th3(
        [&]() -> void
        {
            while (true)
            {
                q.push(++i);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }
    );
    
    std::thread th4(
        [&]() -> void
        {
            qinmo::println("begin:");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            int temp = 0;
            while (true)
            {
                if (q.pop(temp))
                    qinmo::println(temp, " i=", i.load(std::memory_order_relaxed));
    
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    );
    
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    qinmo::println("main th begin:");
    int count = 0;
    while (true)
    {
        if (q.pop(count))
            qinmo::println(count, " i=", i.load(std::memory_order_relaxed));
        else
            qinmo::println("false");
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    th1.join();
    th2.join();
    th3.join();
    th4.join();

    /*
    output:

    1
    0
    1 temp=1
    0 temp=1
    begin:
    1 i=12
    2 i=12
    3 i=15
    4 i=18
    5 i=18
    6 i=21
    7 i=24
    8 i=24
    main th begin:
    9 i=27
    10 i=27
    11 i=27
    12 i=27
    13 i=30
    14 i=30
    15 i=30
    16 i=30
    17 i=30
    18 i=33
    19 i=33
    20 i=33
    21 i=33
    22 i=36
    23 i=36
    24 i=36
    25 i=36
    26 i=39
    27 i=39
    28 i=39
    29 i=39
    30 i=42
    31 i=42
    32 i=42
    33 i=42
    34 i=42
    35 i=45
    36 i=45
    37 i=45
    38 i=45
    39 i=48
    40 i=48
    41 i=48
    42 i=48
    43 i=48
    44 i=51
    45 i=51
    46 i=51
    47 i=51
    48 i=54
    49 i=54
    50 i=54
    51 i=54
    52 i=54
    53 i=57
    54 i=57
    56 i=57
    55 i=60
    57 i=60
    58 i=60
    59 i=60
    60 i=60
    61 i=63
    62 i=63
    63 i=63
    false
    64 i=66
    66 i=66
    65 i=66
    */
    return 0;
}