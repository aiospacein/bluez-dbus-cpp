//
// Copyright Audiofile LLC 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//

#include <bluez-dbus-cpp/bluez.h>
#include <bluez-dbus-cpp/GenericCharacteristic.h>
#include <bluez-dbus-cpp/ReadOnlyCharacteristic.h>
#include "SerialCharacteristic.h"
#include <bluez-dbus-cpp/proxy/AgentManager1_proxy.h>
#include <bluez-dbus-cpp/adaptor/Agent1_adaptor.h>
#include <iostream>
#include <signal.h>

using namespace org::bluez;

constexpr const char* BLUEZ_SERVICE = "org.bluez";
constexpr const char* DEVICE0 = "/org/bluez/hci0";

static int sig = 0;
static void sig_callback(int signum)
{
    exit(signum);
}

// ---- Agent Implementation -----------------------------------------------------------------------------------
class BluetoothAgent : public sdbus::AdaptorInterfaces<org::bluez::Agent1_adaptor>
{
public:
    BluetoothAgent(sdbus::IConnection &connection, const std::string &path)
        : AdaptorInterfaces(connection, sdbus::ObjectPath(path))
    {
        registerAdaptor();
    }

    std::string RequestPinCode(const sdbus::ObjectPath &device) override
    {
        // std::cout << "RequestPinCode: " << device << std::endl;
        return "0000"; // or any fixed PIN you want
        // throw sdbus::Error("org.bluez.Error.Rejected", "Not supported");
    }

    void DisplayPinCode(const sdbus::ObjectPath &device, const std::string &pincode) override
    {
        std::cout << "DisplayPinCode: " << device << ", code: " << pincode << std::endl;
    }

    void RequestConfirmation(const sdbus::ObjectPath &device, const uint32_t &passkey) override
    {
        std::cout << "[Agent] Auto-confirming passkey: " << passkey << std::endl;
        // std::cout << "[Agent] RequestConfirmation passkey: " << passkey << std::endl;
        // throw sdbus::Error("org.bluez.Error.Rejected", "No input/output available");
    }

    void RequestAuthorization(const sdbus::ObjectPath &device) override
    {
        std::cout << "RequestAuthorization: " << device << std::endl;
    }

    uint32_t RequestPasskey(const sdbus::ObjectPath &device) override
    {
        // std::cout << "[Agent] RequestPasskey from device: " << device << std::endl;
        return 123456; // Or return your fixed passkey if needed
        // throw sdbus::Error("org.bluez.Error.Rejected", "Not supported");
    }

    void DisplayPasskey(const sdbus::ObjectPath &device, const uint32_t &passkey, const uint16_t &entered) override
    {
        std::cout << "[Agent] DisplayPasskey for device: " << device << ", passkey: " << passkey << ", entered: " << entered << std::endl;
    }

    void AuthorizeService(const sdbus::ObjectPath &device, const std::string &uuid) override
    {
        std::cout << "AuthorizeService: " << device << ", uuid: " << uuid << std::endl;
    }

    void Cancel() override
    {
        std::cout << "Cancel" << std::endl;
    }

    void Release() override
    {
        std::cout << "Agent released." << std::endl;
    }
};

// Helper subclass to expose constructor
class AgentManager1Proxy : public org::bluez::AgentManager1_proxy
{
public:
    // Publicly inherit the base constructor
    explicit AgentManager1Proxy(sdbus::IProxy &proxy)
        : org::bluez::AgentManager1_proxy(proxy) {}
};

int main(int argc, char *argv[])
{
    if( signal(SIGINT, sig_callback) == SIG_ERR )
        std::cerr << std::endl << "error registering signal handler" << std::endl;

    constexpr const char* APP_PATH = "/org/bluez/example";
    constexpr const char* ADV_PATH = "/org/bluez/example/advertisement1";
    constexpr const char* AGENT_PATH = "/org/bluez/agent";
    constexpr const char* NAME = "STM_BLE";

    std::shared_ptr<IConnection> connection{ std::move( sdbus::createSystemBusConnection() ) };

    // ---- Adapter Info -----------------------------------------------------------------------------------------------

    {
        Adapter1 adapter1{ *connection, BLUEZ_SERVICE, DEVICE0 };

        adapter1.Powered( true );
        adapter1.Discoverable( true );
        adapter1.Pairable( true );
        adapter1.Alias( NAME );

        std::cout << "Found adapter '" << DEVICE0 << "'" << std::endl;
        std::cout << "  Name: " << adapter1.Name() << std::endl;
        std::cout << "  Address: " << adapter1.Address() << " type: " << adapter1.AddressType() << std::endl;
        std::cout << "  Powered: " << adapter1.Powered() << std::endl;
        std::cout << "  Discoverable: " << adapter1.Discoverable() << std::endl;
        std::cout << "  Pairable: " << adapter1.Pairable() << std::endl;
    }

    std::cout << std::endl;

    BluetoothAgent agent{ *connection, AGENT_PATH };
    auto proxy = sdbus::createProxy(*connection, BLUEZ_SERVICE, "/org/bluez");
    AgentManager1Proxy agentMgr(*proxy);

    agentMgr.RegisterAgent(AGENT_PATH, "NoInputNoOutput");
    agentMgr.RequestDefaultAgent(AGENT_PATH);

    GattManager1 gattMgr{ connection, BLUEZ_SERVICE, DEVICE0 };
    auto app =  std::make_shared<GattApplication1>( connection, APP_PATH );
    auto srv1 = std::make_shared<GattService1>( app, "deviceinfo", "180A" );
    ReadOnlyCharacteristic::createFinal( srv1, "2A24", NAME ); // model name
    ReadOnlyCharacteristic::createFinal( srv1, "2A25", "333-12345678-888" ); // serial number
    ReadOnlyCharacteristic::createFinal( srv1, "2A26", "1.0.1" ); // fw rev
    ReadOnlyCharacteristic::createFinal( srv1, "2A27", "rev A" ); // hw rev
    ReadOnlyCharacteristic::createFinal( srv1, "2A28", "5.0" ); // sw rev
    ReadOnlyCharacteristic::createFinal( srv1, "2A29", "ACME Inc." ); // manufacturer

    auto srv2 = std::make_shared<GattService1>( app, "serial", "368a3edf-514e-4f70-ba8f-2d0a5a62bc8c" );
    SerialCharacteristic::create( srv2, connection, "de0a7b0c-358f-4cef-b778-8fe9abf09d53" )
        .finalize();

    auto register_app_callback = [](const sdbus::Error* error)
    {
        if( error == nullptr )
        {
            std::cout << "Application registered." << std::endl;
        }
        else
        {
            std::cerr << "Error registering application " << error->getName() << " with message " << error->getMessage() << std::endl;
        }
    };

    gattMgr.RegisterApplicationAsync( app->getPath(), {} )
        .uponReplyInvoke(register_app_callback);

    // ---- Advertising ------------------------------------------------------------------------------------------------

    auto mgr = std::make_shared<LEAdvertisingManager1>( connection, BLUEZ_SERVICE, DEVICE0 );
    std::cout << "LEAdvertisingManager1" << std::endl;
    std::cout << "  ActiveInstances: " << mgr->ActiveInstances() << std::endl;
    std::cout << "  SupportedInstances: " << mgr->SupportedInstances() << std::endl;
    {
        std::cout << "  SupportedIncludes: ";
        auto includes = mgr->SupportedIncludes();
        for( auto include : includes )
        {
            std::cout << "\"" << include <<"\",";
        }
        std::cout << std::endl;
    }

    auto register_adv_callback = [](const sdbus::Error* error) -> void
    {
        if( error == nullptr )
        {
            std::cout << "Advertisement registered." << std::endl;
        }
        else
        {
            std::cerr << "Error registering advertisment " << error->getName() << " with message " << error->getMessage() << std::endl;
        }
    };

    auto ad = LEAdvertisement1::create( *connection, ADV_PATH )
        .withLocalName( NAME )
        .withServiceUUIDs( std::vector{ std::string{"368a3edf-514e-4f70-ba8f-2d0a5a62bc8c"} } )
        .withIncludes( std::vector{ std::string{ "tx-power" }} )
        .onReleaseCall( [](){ std::cout << "advertisement released" << std::endl; } )
        .registerWith( mgr, register_adv_callback );

    std::cout << "Loading complete." << std::endl;

    // connection->enterProcessingLoopAsync();
    connection->enterEventLoopAsync();

    bool run = true;
    while( run) {
        char cmd;
        std::cout << "commands:" << std::endl;
        std::cout << "  q      quit" << std::endl;
        std::cout << "$> ";
        std::cin >> cmd;

        switch(cmd)
        {
            case 'q':
                run = false;
        }
    } 
}
