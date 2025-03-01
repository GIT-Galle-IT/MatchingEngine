#pragma once

#include "User.h"
#include <string>

namespace refdata::data
{
    class Issuer final
    {
        public:
            Issuer(int issuerId, int instrumentId, std::string issuerName):gIssuerId(issuerId), gInstrumentName(issuerName){};
            virtual ~Issuer() = default;

            int getInstrumentId() const { return gInstrumentId; }
            void setInstrumentId(int instrumentId) { gInstrumentId = instrumentId; }
            
        private:
            int gIssuerId;
            int gInstrumentId;
            std::string gInstrumentName;
        
    };
}