// #include <utils/Common.h>
#include <gbase/net/GClient.h>
#include <sstream>
#include <iostream>
#include <msg/message.h>

#include <memory> // for allocator, __shared_ptr_access
#include <string> // for char_traits, operator+, string, basic_string

#include "ftxui/component/captured_mouse.hpp"     // for ftxui
#include "ftxui/component/component.hpp"          // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"                     // for Ref

#include <memory> // for allocator, __shared_ptr_access
#include <string> // for char_traits, operator+, string, basic_string

#include "ftxui/component/captured_mouse.hpp"     // for ftxui
#include "ftxui/component/component.hpp"          // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"                     // for Ref

class TradingClient : public GClient
{
public:
    virtual void onResponse(const char *message) override
    {
        // specify what to do upon recieving response
        // std::cout << "onResponse::  " << message << std::endl;
    }
};

int main()
{
    using namespace ftxui;

    TradingClient client;
    client.connect("127.0.0.1", 9999);

    // The data:
    std::string instrumentID;
    std::string orderType;
    std::string orderQuantity;
    std::string price;
    bool readyToSend = false;

    // The basic input components:
    Component input_instrumentID = Input(&instrumentID, "Instrument ID");
    Component input_orderType = Input(&orderType, "Order Type");

    // The orderQuantity input component:
    Component input_orderQuantity = Input(&orderQuantity, "Order Quantity (int)");
    input_orderQuantity |= CatchEvent([&](Event event)
                                      { return event.is_character() && !std::isdigit(event.character()[0]); });

    // The phone number input component:
    // We are using `CatchEvent` to filter out non-digit characters.
    Component input_price = Input(&price, "Price");
    input_price |= CatchEvent([&](Event event)
                              { return event.is_character() && !std::isdigit(event.character()[0]); });

    // The tree of components. This defines how to navigate using the keyboard.
    auto buttons = Container::Horizontal({
        Button(
            "Reset", [&]
            { instrumentID = ""; orderType = ""; orderQuantity = ""; price = ""; }, ButtonOption::Animated(Color::Green)),
        Button(
            "Submit Order", [&]
            {
                if (readyToSend == false)
                {
                    return;
                }
                OrderMessage message{instrumentID, orderType == "SELL" ? 0 : 1, std::atoi(orderQuantity.c_str()), std::atoi(price.c_str())};

                std::stringstream oss;
                message.serialize(oss);

                // send to the server
                auto serializedString = oss.str();
                client.send(serializedString);
            }, ButtonOption::Animated(Color::Blue)),
    });

    // Modify the way to render them on screen:
    auto component_buttons = Renderer(buttons, [&]
                                      {
    if (buttons->ActiveChild() == buttons->ChildAt(1) &&
        orderType.empty() == false &&
        orderQuantity.empty() == false &&
        instrumentID.empty() == false &&
        price.empty() == false)
    {
        readyToSend = true;
            return vbox({
                vbox({
                    text("Order Info :- " + orderType + " " + orderQuantity + " of " + instrumentID + " at " + price),
                }) | border,
                buttons->Render(),
            });
    }
    else
    {
        readyToSend = false;
        return vbox({
            vbox({
                text("Order Info pending"),
            }) | border,
            buttons->Render(),
        });
    } });

    // The component tree:
    auto component = Container::Vertical({input_instrumentID,
                                          input_orderType,
                                          input_orderQuantity,
                                          input_price});

    // Tweak how the component tree is rendered:
    auto renderer = Renderer(component, [&]
                             { return vbox({hbox(text(" Intrument      : "), input_instrumentID->Render()),
                                            hbox(text(" Order Type     : "), input_orderType->Render()),
                                            hbox(text(" Order Quantity : "), input_orderQuantity->Render()),
                                            hbox(text(" Price          : "), input_price->Render())}) |
                                      border; });

    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(Container::Vertical({renderer, component_buttons}));
}