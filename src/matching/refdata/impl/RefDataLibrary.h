#pragma once

#include <unordered_map> 

#include "matching/refdata/data/User.h"
#include "matching/refdata/data/Instrument.h"
#include "matching/refdata/data/Issuer.h"

namespace refdata::impl
{
    class RefDataLibrary
    {
        public:
            RefDataLibrary();
            virtual ~RefDataLibrary() = default;

            void init();
            bool onData();

            void addUser();
            void addIssuer();
            void addInstrument();

            refdata::data::User* getUser();
            refdata::data::Instrument* getInstrument();
            refdata::data::Issuer* getIssuer();

            std::unordered_map<int, refdata::data::Issuer*> getIssuers() const { return gIssuers; }
            std::unordered_map<int, refdata::data::Instrument*> getInstrumnets() const { return gInstrumnets; }
            std::unordered_map<int, refdata::data::User*> getUsers() const { return gUsers; }

        private:
            std::unordered_map<int, refdata::data::User*> gUsers;
            std::unordered_map<int, refdata::data::Issuer*> gIssuers;
            std::unordered_map<int, refdata::data::Instrument*> gInstrumnets;

    };
}