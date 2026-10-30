#include <qinmo/tool.h> // <qinmo/tool/ObjectPool.h>

int main()
{
    using Type = int;
    using PoolType = qinmo::ObjectPool<Type>;

    /*
        method 1
    */
    // pool options
    qinmo::PoolOptions options;
    options.initSize = 2;
    options.maxSize = 8;
    options.maxIdle = 8;
    options.resetOnRelease = true;
    options.validateOnCreate = true;
    options.validateOnAcquire = true;
    options.validateOnRelease = true;
    options.allowReturnOnTimeout = true;
    // pool rule
    qinmo::PoolRules<Type> rules;
    rules.create = []() -> Type { return 0; };
    rules.close = [](Type& i) -> void { i = -1; };
    rules.reset = [](Type& i) -> void { i = 0; };
    rules.validate = [](Type& i) -> bool { return i != -1; };
    // create with options and rules
    std::shared_ptr<PoolType> p1 = PoolType::create(options, rules);


    /*
        method 2
    */
    {
        // creaet with builder
        std::shared_ptr<PoolType> p2 = PoolType::builder()
            .setInitSize(2)
            .setMaxSize(8)
            .setMaxIdle(8)
            .setResetOnRelease(true)
            .setValidateOnCreate(true)
            .setValidateOnAcquire(true)
            .setValidateOnRelease(true)
            .setAllowReturnOnTimeout(true)
            .setFuncCreate([](){ return 1; })
            .setFuncClose([](Type& i) -> void { i = -1; })
            .setFuncReset([](Type& i) -> void { i = 0; })
            .setFuncValidate([](Type& i) -> bool { return i != -1; })
            .build();
    } // auto release when the lifecycle ends


    auto pool = p1;

    // non-hlocking acquire obj
    auto obj1 = pool->try_acquire();
    qinmo::println(*obj1);

    // acquire obj with blocking
    auto obj2 = pool->acquire();
    qinmo::println(*obj2);

    // return null pointer on timeout
    auto obj3 = pool->acquire(std::chrono::milliseconds(10));
    qinmo::println(*obj3);

    /*
    output:

    0
    0
    0
    */

    return 0;
}