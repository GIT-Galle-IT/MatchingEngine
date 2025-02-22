#include <iostream>
#include <vector>
#include <memory>

namespace gbase
{
    template<typename T>
    class MemoryPool final
    {
        public:
            MemoryPool(size_t poolSize):memPoolSize(poolSize), memBlock(poolSize, {T{}, true})
            {
                std::cout << "Memory Pool created wih size" << memPoolSize << std::endl;
            }

            template<typename ...Args>
            T* allocate(Args... args)
            {
                auto nextFreeBlock = &(memBlocks[nextFreeMem]);
                T* obj = &(nextFreeBlock->object);
                obj = new(object) T(args...);
                updateNextAvaIndex();
                return obj;
            }

            void deallocate(const T* obj)
            {
                const auto memDiff = std::reinterpret_cast<const memBlock>(obj) - &memBlocks[0];
                memBlocks[memDiff].isFree == true;
            }

            virtual ~MemoryPool() = default;


        private:

            updateNextAvaIndex();
            std::vector<memBlock> memBlocks;
            size_t memPoolSize;
            size_t nextFreeMem; 

            struct memBlock 
            {
                T object;
                bool isFree{true};
            }
    };
}