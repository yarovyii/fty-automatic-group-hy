#include "lib/jobs/resolve.h"
#include "db.h"
#include "lib/jobs/create.h"
#include "lib/jobs/remove.h"
#include "lib/storage.h"
#include <catch2/catch.hpp>

class Group : public fty::Group
{
public:
    Group() = default;

    Group(fty::Group&& other)
        : fty::Group(other)
    {
    }

    ~Group() override
    {
        remove();
    }

    Group create()
    {
        fty::job::Create           create;
        fty::commands::create::Out created;
        REQUIRE_NOTHROW(create.run(*this, created));
        return Group(std::move(created));
    }

    void remove()
    {
        if (id.hasValue()) {
            fty::job::Remove remove;

            fty::commands::remove::In  in;
            fty::commands::remove::Out out;
            in.append(id);
            REQUIRE_NOTHROW(remove.run(in, out));
            REQUIRE(out.size() == 1);
            CHECK(out[0][fty::convert<std::string>(id.value())] == "Ok");
            clear();
        }
    }

    fty::commands::resolve::Out resolve()
    {
        fty::job::Resolve resolve;

        fty::commands::resolve::In  in;
        fty::commands::resolve::Out out;

        in.id = id;
        REQUIRE_NOTHROW(resolve.run(in, out));

        return out;
    }
};


TEST_CASE("Resolve by name")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      ext-name : srv11
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
                      ext-name : srv21
            )");

        Group group;
        group.name          = "ByName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.field = fty::Group::Fields::Name;

        //"Contains"
        {
            cond.value = "srv";
            cond.op    = fty::Group::ConditionOp::Contains;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv21");
        }

        //"Is"
        {
            cond.value = "srv11";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        //"IsNot"
        {
            cond.value = "srv11";
            cond.op    = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv21");
        }

        //"Not exists"
        {
            cond.value = "wtf";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 0);
        }

        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by InternalName")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
            )");

        Group group;
        group.name          = "ByInternalName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.field = fty::Group::Fields::InternalName;

        // Contains
        {
            cond.value = "srv";
            cond.op    = fty::Group::ConditionOp::Contains;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv21");
        }

        // Is
        {
            cond.value = "srv11";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        // IsNot
        {
            cond.value = "srv11";
            cond.op    = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 3);
            CHECK(info[0].name == "datacenter");
            CHECK(info[1].name == "datacenter1");
            CHECK(info[2].name == "srv21");
        }

        // Not exists
        {
            cond.value = "wtf";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 0);
        }

        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by location")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      ext-name : srv11
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
                      ext-name : srv21
            )");

        Group group;
        group.name          = "ByLocation";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.field = fty::Group::Fields::Location;

        //"Contains"
        {
            cond.value = "datacenter";
            cond.op    = fty::Group::ConditionOp::Contains;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv21");
        }

        // "Is"
        {
            cond.value = "datacenter";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        //"IsNot"
        {
            cond.value = "datacenter";
            cond.op    = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv21");
        }

        //"Not exists"
        {
            cond.value = "wtf";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 0);
        }


        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by type")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      ext-name : srv11
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
                      ext-name : srv21
            )");

        Group group;
        group.name          = "ByType";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.field = fty::Group::Fields::Type;

        // Contains
        {
            cond.value = "device";
            cond.op    = fty::Group::ConditionOp::Contains;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv21");
        }

        // Is
        {
            cond.value = "device";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv21");
        }

        // IsNot
        {
            cond.value = "device";
            cond.op    = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "datacenter");
            CHECK(info[1].name == "datacenter1");
        }

        /// Not exists
        {
            cond.value = "wtf";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();
            REQUIRE(info.size() == 0);
        }

        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by subtype")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      ext-name : srv11
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
                      ext-name : srv21
            )");

        Group group;
        group.name          = "BySubType";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.field = fty::Group::Fields::SubType;

        // Contains
        {
            cond.value = "server";
            cond.op    = fty::Group::ConditionOp::Contains;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv21");
        }

        // Is
        {
            cond.value = "server";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv21");
        }

        // IsNot
        {
            cond.value = "server";
            cond.op    = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "datacenter");
            CHECK(info[1].name == "datacenter1");
        }

        // Not exists
        {
            cond.value = "wtf";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();
            REQUIRE(info.size() == 0);
        }

        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by hostname")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      ext-name : srv11
                      attrs:
                          hostname.1 : localhost
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
                      ext-name : srv21
            )");

        Group group;
        group.name          = "ByHostName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.field = fty::Group::Fields::HostName;

        // Contains
        {
            cond.value = "local";
            cond.op    = fty::Group::ConditionOp::Contains;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        // Is
        {
            cond.value = "localhost";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        // IsNot
        {
            cond.value = "localhost";
            cond.op    = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv21");
        }

        // Not exists
        {
            cond.value = "wtf";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 0);
        }

        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by contact")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      attrs:
                          device.contact : dim
                    - type     : Server
                      name     : srv12
                      attrs:
                          contact_email  : dim@eaton.com
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
            )");

        Group group;
        group.name          = "ByContact";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.field = fty::Group::Fields::Contact;

        // Contains
        {
            cond.value = "dim";
            cond.op    = fty::Group::ConditionOp::Contains;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv12");
        }

        // Is
        {
            cond.value = "dim";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        // Is not
        {
            cond.value = "dim";
            cond.op    = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 4);
            CHECK(info[0].name == "datacenter");
            CHECK(info[1].name == "srv12");
            CHECK(info[2].name == "datacenter1");
            CHECK(info[3].name == "srv21");
        }

        // Not exists
        {
            cond.value = "wtf";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 0);
        }

        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by ip address")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      attrs:
                          ip.1 : 127.0.0.1
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
                      attrs:
                          ip.1 : 192.168.0.1
            )");

        Group group;
        group.name          = "ByIpAddress";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.field = fty::Group::Fields::IPAddress;

        // Contains
        {
            cond.value = "127.0";
            cond.op    = fty::Group::ConditionOp::Contains;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        // Is
        {
            cond.value = "127.0.0.1";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        // Is/mask
        {
            cond.value = "127.0.*";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        // Is/few
        {
            cond.value = "127.0.*|192.168.*";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv21");
        }

        // Is not
        {
            cond.value = "127.0.0.1";
            cond.op    = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv21");
        }

        // Not exists
        {
            cond.value = "wtf";
            cond.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 0);
        }

        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by name and location")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      ext-name : srv11
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
                      ext-name : srv21
            )");

        Group group;
        group.name          = "ByName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        {
            auto& var1  = group.rules.conditions.append();
            auto& cond1 = var1.reset<fty::Group::Condition>();
            cond1.field = fty::Group::Fields::Name;

            auto& var2  = group.rules.conditions.append();
            auto& cond2 = var2.reset<fty::Group::Condition>();
            cond2.field = fty::Group::Fields::Location;
        }

        // Contains
        {
            auto& cond1 = group.rules.conditions[0].get<fty::Group::Condition>();
            cond1.value = "srv";
            cond1.op    = fty::Group::ConditionOp::Contains;

            auto& cond2 = group.rules.conditions[1].get<fty::Group::Condition>();
            cond2.value = "datacenter";
            cond2.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv11");
        }

        // Is
        {
            auto& cond1 = group.rules.conditions[0].get<fty::Group::Condition>();
            cond1.value = "srv21";
            cond1.op    = fty::Group::ConditionOp::Is;

            auto& cond2 = group.rules.conditions[1].get<fty::Group::Condition>();
            cond2.value = "datacenter1";
            cond2.op    = fty::Group::ConditionOp::Is;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "srv21");
        }

        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by name and location/mutlty")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      ext-name : srv11
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
                      ext-name : srv21
                    - type     : Server
                      name     : srv22
                      ext-name : srv22
            )");

        Group group;
        group.name          = "ByName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var1  = group.rules.conditions.append();
        auto& cond1 = var1.reset<fty::Group::Condition>();
        cond1.op    = fty::Group::ConditionOp::Is;
        cond1.value = "datacenter1";
        cond1.field = fty::Group::Fields::Location;

        auto& var2  = group.rules.conditions.append();
        auto& rules = var2.reset<fty::Group::Rules>();
        {
            rules.groupOp = fty::Group::LogicalOp::Or;

            auto& scond1 = rules.conditions.append().reset<fty::Group::Condition>();
            scond1.op    = fty::Group::ConditionOp::Is;
            scond1.field = fty::Group::Fields::Name;
            scond1.value = "srv21";

            auto& scond2 = rules.conditions.append().reset<fty::Group::Condition>();
            scond2.op    = fty::Group::ConditionOp::Is;
            scond2.field = fty::Group::Fields::Name;
            scond2.value = "srv22";
        }

        auto g    = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "srv21");
        CHECK(info[1].name == "srv22");
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by hostname vm")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Connector
                  name : connector
                - type : InfraService
                  name : infra
                - type : Hypervisor
                  name : hypervisor
                  attrs:
                      hostName : hypo
                      address  : "[/127.0.0.1,]"
                - type : Hypervisor
                  name : hypervisor1
                  attrs:
                      hostName : hypo1
                      address  : "[/192.168.0.1,]"
                - type : VirtualMachine
                  name : vm1
                - type : VirtualMachine
                  name : vm2
                - type : VirtualMachine
                  name : vm3
            links:
                - dest : vm1
                  src  : hypervisor
                  type : vmware.esxi.hosts.vm
                - dest : vm2
                  src  : hypervisor
                  type : vmware.esxi.hosts.vm
                - dest : vm3
                  src  : hypervisor1
                  type : vmware.esxi.hosts.vm
                - dest : hypervisor
                  src  : infra
                  type : vmware.vcenter.monitors.esxi
                - dest : hypervisor1
                  src  : infra
                  type : vmware.vcenter.monitors.esxi
                - dest : infra
                  src  : connector
                  type : vmware.connected.to.vcenter
            )");

        Group group;
        group.name          = "ByHostNameVm";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var1  = group.rules.conditions.append();
        auto& cond1 = var1.reset<fty::Group::Condition>();

        // Contains
        {
            cond1.op    = fty::Group::ConditionOp::Contains;
            cond1.value = "hypo";
            cond1.field = fty::Group::Fields::HostName;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 3);
            CHECK(info[0].name == "vm1");
            CHECK(info[1].name == "vm2");
            CHECK(info[2].name == "vm3");
        }

        // Is
        {
            cond1.op    = fty::Group::ConditionOp::Is;
            cond1.value = "hypo";
            cond1.field = fty::Group::Fields::HostName;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "vm1");
            CHECK(info[1].name == "vm2");
        }

        // Is not
        {
            cond1.op    = fty::Group::ConditionOp::IsNot;
            cond1.value = "hypo1";
            cond1.field = fty::Group::Fields::HostName;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "vm1");
            CHECK(info[1].name == "vm2");
        }

        // Wrong
        {
            cond1.op    = fty::Group::ConditionOp::Is;
            cond1.value = "wtf";
            cond1.field = fty::Group::Fields::HostName;

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 0);
        }

    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by ip address vm")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Connector
                  name : connector
                - type : InfraService
                  name : infra
                - type : Hypervisor
                  name : hypervisor
                  attrs:
                      hostName : hypo
                      address  : "[/127.0.0.1,]"
                - type : Hypervisor
                  name : hypervisor1
                  attrs:
                      hostName : hypo1
                      address  : "[/192.168.0.1,]"
                - type : VirtualMachine
                  name : vm1
                - type : VirtualMachine
                  name : vm2
                - type : VirtualMachine
                  name : vm3
            links:
                - dest : vm1
                  src  : hypervisor
                  type : vmware.esxi.hosts.vm
                - dest : vm2
                  src  : hypervisor
                  type : vmware.esxi.hosts.vm
                - dest : vm3
                  src  : hypervisor1
                  type : vmware.esxi.hosts.vm
                - dest : hypervisor
                  src  : infra
                  type : vmware.vcenter.monitors.esxi
                - dest : hypervisor1
                  src  : infra
                  type : vmware.vcenter.monitors.esxi
                - dest : infra
                  src  : connector
                  type : vmware.connected.to.vcenter
            )");

        Group group;
        group.name          = "ByIpAddressVm";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var1  = group.rules.conditions.append();
        auto& cond1 = var1.reset<fty::Group::Condition>();
        cond1.field = fty::Group::Fields::IPAddress;

        // Contains
        {
            cond1.op    = fty::Group::ConditionOp::Contains;
            cond1.value = "127.0";

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "vm1");
            CHECK(info[1].name == "vm2");
        }

        // Is
        {
            cond1.op    = fty::Group::ConditionOp::Is;
            cond1.value = "127.0.*";

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "vm1");
            CHECK(info[1].name == "vm2");
        }

        // Is
        {
            cond1.op    = fty::Group::ConditionOp::Is;
            cond1.value = "192.168.*";

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "vm3");
        }

        // Is not
        {
            cond1.op    = fty::Group::ConditionOp::IsNot;
            cond1.value = "127.0.*";

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "vm3");
        }

        // Wrong
        {
            cond1.op    = fty::Group::ConditionOp::Is;
            cond1.value = "wtf";

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 0);
        }

    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

TEST_CASE("Resolve by hosted by")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Connector
                  name : connector
                - type : InfraService
                  name : infra
                - type : Hypervisor
                  name : hypervisor
                - type : Hypervisor
                  name : hypervisor1
                - type : VirtualMachine
                  name : vm1
                - type : VirtualMachine
                  name : vm2
                - type : VirtualMachine
                  name : vm3
            links:
                - dest : vm1
                  src  : hypervisor
                  type : vmware.esxi.hosts.vm
                - dest : vm2
                  src  : hypervisor
                  type : vmware.esxi.hosts.vm
                - dest : vm3
                  src  : hypervisor1
                  type : vmware.esxi.hosts.vm
                - dest : hypervisor
                  src  : infra
                  type : vmware.vcenter.monitors.esxi
                - dest : hypervisor1
                  src  : infra
                  type : vmware.vcenter.monitors.esxi
                - dest : infra
                  src  : connector
                  type : vmware.connected.to.vcenter
            )");

        Group group;
        group.name          = "ByHostedBy";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var1  = group.rules.conditions.append();
        auto& cond1 = var1.reset<fty::Group::Condition>();
        cond1.field = fty::Group::Fields::HostedBy;

        // Contains
        {
            cond1.op    = fty::Group::ConditionOp::Contains;
            cond1.value = "hypervisor";

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 3);
            CHECK(info[0].name == "vm1");
            CHECK(info[1].name == "vm2");
            CHECK(info[2].name == "vm3");
        }

        // Is
        {
            cond1.op    = fty::Group::ConditionOp::Is;
            cond1.value = "hypervisor";

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 2);
            CHECK(info[0].name == "vm1");
            CHECK(info[1].name == "vm2");
        }

        // Is not
        {
            cond1.op    = fty::Group::ConditionOp::IsNot;
            cond1.value = "hypervisor";

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "vm3");
        }

        // Wrong
        {
            cond1.op    = fty::Group::ConditionOp::Is;
            cond1.value = "wtf";

            auto g    = group.create();
            auto info = g.resolve();

            REQUIRE(info.size() == 0);
        }

    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}

struct GroupsData : pack::Node
{
    struct Condition : pack::Node
    {
        pack::String field = FIELD("field");
        pack::String op    = FIELD("operator");
        pack::String value = FIELD("value");

        using pack::Node::Node;
        META(Condition, field, op, value);
    };

    struct Item : pack::Node
    {
        pack::String                name  = FIELD("name");
        pack::String                op    = FIELD("operator");
        pack::ObjectList<Condition> rules = FIELD("rules");

        using pack::Node::Node;
        META(Item, name, op, rules);
    };


    pack::ObjectList<Item> items = FIELD("items");

    using pack::Node::Node;
    META(GroupsData, items);

    std::vector<Group> groups;
    GroupsData(const std::string& data);

    Group& group(const std::string name, uint64_t id = 0, bool numExist = false);
};

GroupsData::GroupsData(const std::string& data)
{
    if (auto ret = pack::yaml::deserialize(data, *this); !ret) {
        throw std::runtime_error(ret.error());
    }

    for (const auto& item : items) {
        groups.emplace_back(Group());
        auto& g = groups.back();
        g.name  = item.name.value();

        {
            fty::Group::LogicalOp opEnum;
            std::stringstream     ss;
            ss << item.op.value();
            ss >> opEnum;
            g.rules.groupOp = opEnum;
        }

        int it = 0;
        for (const auto& condition : item.rules) {
            auto& group = g.rules.conditions.append();
            auto& cond  = group.reset<fty::Group::Condition>();

            {
                fty::Group::Fields fieldEnum;
                std::stringstream  ss;
                ss << condition.field.value();
                ss >> fieldEnum;
                cond.field = fieldEnum;
            }

            {
                fty::Group::ConditionOp condOpEnum;
                std::stringstream       ss;
                ss << condition.op.value();
                ss >> condOpEnum;
                cond.op = condOpEnum;
            }

            cond       = g.rules.conditions[it++].get<fty::Group::Condition>();
            cond.value = condition.value;
        }
    }
}

Group& GroupsData::group(const std::string name, uint64_t id, bool numExist)
{
    for (auto& groupIn : groups) {
        if (groupIn.name.value() == name) {
            if (numExist) {
                for (auto& condition : groupIn.rules.conditions) {
                    auto& cond = condition.get<fty::Group::Condition>();
                    if (cond.field == fty::Group::Fields::Group) {
                        cond.value = fty::convert<std::string, uint64_t>(id);
                        std::cerr << cond.value.value() << std::endl;
                    }
                }
            }
            return groupIn;
        }
    }
    throw std::runtime_error("Group does not exist");
}

TEST_CASE("Resolve by Group")
{
    try {
        fty::SampleDb db(R"(
            items:
                - type : Datacenter
                  name : datacenter
                  items:
                    - type     : Server
                      name     : srv11
                      ext-name : srv11
                - type : Datacenter
                  name : datacenter1
                  items:
                    - type     : Server
                      name     : srv21
                      ext-name : srv21
                - type : Datacenter
                  name : datacenter2
                  items:
                    - type     : Server
                      name     : serv
                      ext-name : serv
                    - type     : Server
                      name     : servsrv
                      ext-name : servsrv
            )");

        GroupsData allGroups(R"(
            items:
              - name : ByName
                operator : AND
                rules : 
                  - field : name
                    operator : CONTAINS
                    value : srv
              - name : WithLink
                operator : AND
                rules : 
                  - field : name
                    operator : IS
                    value : serv
                  - field : group
                    operator : IS
                    value : ByName
              - name : LinkTmp
                operator : AND
                rules : 
                  - field : name
                    operator : CONTAINS
                    value : srv21
            )");

        auto groupName = allGroups.group("ByName").create();

        // And operator | Contains
        {
            auto g    = allGroups.group("WithLink", groupName.id.value(), true).create();
            auto info = g.resolve();
            REQUIRE(info.size() == 0);
        }

        // IsNot in group
        {
            auto& group = allGroups.group("WithLink", groupName.id.value(), true);
            auto& cond  = group.rules.conditions[0].get<fty::Group::Condition>();
            cond.op     = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();
            REQUIRE(info.size() == 3);
            CHECK(info[0].name == "srv11");
            CHECK(info[1].name == "srv21");
            CHECK(info[2].name == "servsrv");
        }

        // IsNot group
        {
            auto& group = allGroups.group("WithLink", groupName.id.value(), true);

            auto& condName = group.rules.conditions[0].get<fty::Group::Condition>();
            condName.op    = fty::Group::ConditionOp::Contains;

            auto& condGroupLink = group.rules.conditions[1].get<fty::Group::Condition>();
            condGroupLink.op    = fty::Group::ConditionOp::IsNot;

            auto g    = group.create();
            auto info = g.resolve();
            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "serv");
        }

        // IsNot with two linked groups
        {
            auto groupTmp = allGroups.group("LinkTmp").create();

            auto& group = allGroups.group("WithLink", groupName.id.value(), true);

            auto& condName = group.rules.conditions[0].get<fty::Group::Condition>();
            condName.value = "s";
            condName.op    = fty::Group::ConditionOp::Contains;

            auto& condGroupLink = group.rules.conditions[1].get<fty::Group::Condition>();
            condGroupLink.op    = fty::Group::ConditionOp::IsNot;

            // second link create
            auto& groupGroupLink = group.rules.conditions.append();
            auto& condGroupLink1 = groupGroupLink.reset<fty::Group::Condition>();
            condGroupLink1.field = fty::Group::Fields::Group;

            condGroupLink1       = group.rules.conditions[2].get<fty::Group::Condition>();
            condGroupLink1.value = fty::convert<std::string, uint64_t>(groupTmp.id.value());
            condGroupLink1.op    = fty::Group::ConditionOp::IsNot;

            auto g1   = group.create();
            auto info = g1.resolve();

            REQUIRE(info.size() == 1);
            CHECK(info[0].name == "serv");
        }

        groupName.remove();
        CHECK(fty::Storage::clear());
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    }
}
