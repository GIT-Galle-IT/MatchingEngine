#pragma once

#include "RefDataLibrary.h"

namespace refdata::impl
{
    class RefDataManager
    {
        public:
            RefDataManager();
            ~RefDataManager(){delete gRefDataLibrary;}

            void init();
            bool onData();

        private:
            RefDataLibrary* gRefDataLibrary;

    };
}