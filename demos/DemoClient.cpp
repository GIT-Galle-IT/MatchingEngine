// #include <utils/Common.h>
#include <gbase/net/GClient.h>
#include <sstream>
#include <iostream>
#include "message.pb.cc"

int globalIndex = 0;
class DemoClient : public GClient
{
public:
    virtual void onResponse(const char *message) override
    {
        // specify what to do upon recieving response
        std::cout << "onResponse::  " << message << std::endl;
    }
};

// This function fills in a Person message based on user input.
void addPeopleToAddressBook(demos::Person* person) {
  person->set_id(globalIndex++);
  person->set_name("John Doe");
  person->set_email("johndoe@gmail.com");
  demos::Person::PhoneNumber* phone_number = person->add_phones();
  phone_number->set_number("+1 99 9884566");
  phone_number->set_type(demos::Person::PHONE_TYPE_WORK);
}

int main()
{
    DemoClient client;
    client.connect("127.0.0.1", 9999);
    demos::AddressBook addressBook;
    while (true)
    {
        addPeopleToAddressBook(addressBook.add_people());
        std::string buf = addressBook.SerializeAsString();

        std::cout << addressBook.people()[0].id() << std::endl;
        std::cout << addressBook.people()[0].name() << std::endl;
        std::cout << addressBook.people()[0].email() << std::endl;
        std::cout << addressBook.people()[0].phones()[0].number() << std::endl;

        // addressBook.clear_people();

        // send to the server
        client.send(buf);

        // close connection
        sleep(1);
    }
    google::protobuf::ShutdownProtobufLibrary();
    client.closeConnection();
    return 0;
}