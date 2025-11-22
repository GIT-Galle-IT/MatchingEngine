#include <logging/gLog.h>
#include <new>

#define __VERSION_1_OBJECT_POOL__

namespace gbase
{
#ifdef __VERSION_1_OBJECT_POOL__
    template <typename T, size_t size = 1>
    class ObjectPool
    {
        typedef T Object;
        typedef size_t pool_size;

    private:
        pool_size free_pool_size = size - 1;
        Object *pool = nullptr;

    public:
        ObjectPool()
        {
            pool = new (std::malloc(sizeof(Object) * size)) T;
        };

        Object *allocate()
        {
            free_pool_size--;
            return pool + free_pool_size + 1;
        }

        void release()
        {
            delete pool;
        }
    };
#endif

} // namespace gbase
