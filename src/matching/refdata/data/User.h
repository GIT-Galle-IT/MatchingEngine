#pragma once

#include <string>
#include "Issuer.h"

namespace refdata::data
{
    class User
    {
        public:
            User(int userId, std::string userName):gUserId(userId), gUserName(userName){}
            virtual ~User() = default;

            int getUserId() const { return gUserId; }
            void setUserId(int userId) { gUserId = userId; }

            std::string getUserName() const { return gUserName; }
            void setUserName(const std::string &userName) { gUserName = userName; }


        private:
            int gUserId;
            std::string gUserName;
    };
}