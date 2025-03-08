#include <iostream>
#include <vector>
#include <memory>

namespace gbase
{
    template<typename T>
    class MemoryPool final
    {
        public:
            MemoryPool(size_t poolSize):memPoolSize(poolSize), memBlocks(poolSize, {T{}, true})
            {
                std::cout << "Memory Pool created wih size" << memPoolSize << std::endl;
            }

            template<typename ...Args>
            T* allocate(Args... args)
            {
                auto nextFreeBlock = &(memBlocks[nextFreeMem]);
                T* obj = &(nextFreeBlock->object);
                obj = new T(args...);
                updateNextAvailIndex();
                return obj;
            }

            void deallocate(const T* obj)
            {
                const auto memDiff = reinterpret_cast<const char*>(obj) - reinterpret_cast<const char*>(&memBlocks[0]);
                memBlocks[memDiff].isFree = true;
            }

            virtual ~MemoryPool() = default;


        private:
            struct memBlock 
            {
                T object;
                bool isFree{true};
            };

            void updateNextAvailIndex()
            {
                nextFreeMem++;
            }


            std::vector<memBlock> memBlocks;
            size_t memPoolSize;
            size_t nextFreeMem; 
    };
}