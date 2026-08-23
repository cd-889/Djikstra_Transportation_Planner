#include <gtest/gtest.h>
#include "CSVBusSystem.h"
#include "StringDataSource.h"

// Basic sanity check: two stops, one route with two stops.
// Verifies StopCount, StopByIndex, StopByID, ID(), and NodeID().
TEST(CSVBusSystem, SimpleFiles){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,123\n"
                                                                "2,124");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1\n"
                                                                "A,2");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    
    CCSVBusSystem BusSystem(StopReader,RouteReader);
    
    EXPECT_EQ(BusSystem.StopCount(),2);
    auto StopObj = BusSystem.StopByIndex(0);
    EXPECT_NE(StopObj,nullptr);
    StopObj = BusSystem.StopByIndex(1);
    EXPECT_NE(StopObj,nullptr);
    StopObj = BusSystem.StopByID(1);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),1);
    EXPECT_EQ(StopObj->NodeID(),123);
    StopObj = BusSystem.StopByID(2);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),2);
    EXPECT_EQ(StopObj->NodeID(),124);
}

// Verifies RouteCount and RouteByName, including stop order within each route.
TEST(CSVBusSystem, RouteCountAndRouteByName){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,100\n"
                                                                "2,200\n"
                                                                "3,300");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1\n"
                                                                "A,2\n"
                                                                "B,2\n"
                                                                "B,3");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    CCSVBusSystem BusSystem(StopReader,RouteReader);

    EXPECT_EQ(BusSystem.RouteCount(),2);
    auto RouteA = BusSystem.RouteByName("A");
    ASSERT_NE(RouteA,nullptr);
    EXPECT_EQ(RouteA->Name(),"A");
    EXPECT_EQ(RouteA->StopCount(),2);
    EXPECT_EQ(RouteA->GetStopID(0),1);
    EXPECT_EQ(RouteA->GetStopID(1),2);
    auto RouteB = BusSystem.RouteByName("B");
    ASSERT_NE(RouteB,nullptr);
    EXPECT_EQ(RouteB->Name(),"B");
    EXPECT_EQ(RouteB->StopCount(),2);
    EXPECT_EQ(RouteB->GetStopID(0),2);
    EXPECT_EQ(RouteB->GetStopID(1),3);
}

// Verifies that RouteByIndex returns routes in CSV insertion order.
TEST(CSVBusSystem, RouteByIndex){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,100\n"
                                                                "2,200");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "First,1\n"
                                                                "First,2\n"
                                                                "Second,2\n"
                                                                "Second,1");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    CCSVBusSystem BusSystem(StopReader,RouteReader);

    auto R0 = BusSystem.RouteByIndex(0);
    ASSERT_NE(R0,nullptr);
    EXPECT_EQ(R0->Name(),"First");
    EXPECT_EQ(R0->StopCount(),2);
    auto R1 = BusSystem.RouteByIndex(1);
    ASSERT_NE(R1,nullptr);
    EXPECT_EQ(R1->Name(),"Second");
    EXPECT_EQ(R1->StopCount(),2);
    EXPECT_EQ(R1->GetStopID(0),2);
    EXPECT_EQ(R1->GetStopID(1),1);
}

// StopByIndex must return nullptr for any index >= StopCount.
TEST(CSVBusSystem, StopByIndexOutOfRange){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,100");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    CCSVBusSystem BusSystem(StopReader,RouteReader);

    EXPECT_EQ(BusSystem.StopByIndex(1),nullptr);
    EXPECT_EQ(BusSystem.StopByIndex(99),nullptr);
}

// StopByID must return nullptr when the ID does not exist.
TEST(CSVBusSystem, StopByIDNotFound){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,100");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    CCSVBusSystem BusSystem(StopReader,RouteReader);

    EXPECT_EQ(BusSystem.StopByID(999),nullptr);
}

// RouteByIndex must return nullptr for any index >= RouteCount.
TEST(CSVBusSystem, RouteByIndexOutOfRange){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,100");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    CCSVBusSystem BusSystem(StopReader,RouteReader);

    EXPECT_EQ(BusSystem.RouteByIndex(1),nullptr);
    EXPECT_EQ(BusSystem.RouteByIndex(10),nullptr);
}

// RouteByName must return nullptr for an unknown name and for an empty string.
TEST(CSVBusSystem, RouteByNameNotFound){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,100");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    CCSVBusSystem BusSystem(StopReader,RouteReader);

    EXPECT_EQ(BusSystem.RouteByName("Nonexistent"),nullptr);
    EXPECT_EQ(BusSystem.RouteByName(""),nullptr);
}

// GetStopID must return InvalidStopID when the index is past the end of the route.
TEST(CSVBusSystem, RouteGetStopIDOutOfRange){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,100");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    CCSVBusSystem BusSystem(StopReader,RouteReader);

    auto Route = BusSystem.RouteByName("A");
    ASSERT_NE(Route,nullptr);
    EXPECT_EQ(Route->GetStopID(1),CBusSystem::InvalidStopID);
    EXPECT_EQ(Route->GetStopID(99),CBusSystem::InvalidStopID);
}

// A stop ID may appear more than once in a route (e.g. a loop).
// StopCount should reflect the total entries including duplicates.
TEST(CSVBusSystem, RouteWithRepeatedStop){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,100\n"
                                                                "2,200");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "Loop,1\n"
                                                                "Loop,2\n"
                                                                "Loop,1");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    CCSVBusSystem BusSystem(StopReader,RouteReader);

    auto Route = BusSystem.RouteByName("Loop");
    ASSERT_NE(Route,nullptr);
    EXPECT_EQ(Route->StopCount(),3);
    EXPECT_EQ(Route->GetStopID(0),1);
    EXPECT_EQ(Route->GetStopID(1),2);
    EXPECT_EQ(Route->GetStopID(2),1);
}

// Edge case: exactly one stop and one route with one stop.
TEST(CSVBusSystem, SingleStopSingleRoute){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "42,999");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource,',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "Solo,42");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource,',');
    CCSVBusSystem BusSystem(StopReader,RouteReader);

    EXPECT_EQ(BusSystem.StopCount(),1);
    EXPECT_EQ(BusSystem.RouteCount(),1);
    auto Stop = BusSystem.StopByIndex(0);
    ASSERT_NE(Stop,nullptr);
    EXPECT_EQ(Stop->ID(),42);
    EXPECT_EQ(Stop->NodeID(),999);
    auto Route = BusSystem.RouteByIndex(0);
    ASSERT_NE(Route,nullptr);
    EXPECT_EQ(Route->Name(),"Solo");
    EXPECT_EQ(Route->StopCount(),1);
    EXPECT_EQ(Route->GetStopID(0),42);
}
