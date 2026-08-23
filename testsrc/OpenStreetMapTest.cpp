#include <gtest/gtest.h>
#include "OpenStreetMap.h"
#include "StringDataSource.h"

// Basic sanity check: two nodes, one way.
// Verifies counts, NodeByIndex, WayByIndex bounds, NodeByID, ID(), Location().
TEST(OSMTest, SimpleFiles){
    auto OSMDataSource = std::make_shared< CStringDataSource >("<?xml version='1.0' encoding='UTF-8'?>\n"
                                                                "<osm version=\"0.6\" generator=\"osmconvert 0.8.5\">\n"
	                                                            "  <node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>\n"
	                                                            "  <node id=\"2\" lat=\"38.5\" lon=\"-121.8\"/>\n"
                                                                "  <way id=\"100\">\n"
		                                                        "    <nd ref=\"258592863\"/>\n"
		                                                        "    <nd ref=\"4399281377\"/>\n"
                                                                "  </way>\n"
                                                                "</osm>"
                                                                );
    auto OSMReader = std::make_shared< CXMLReader >(OSMDataSource);
    COpenStreetMap OpenStreetMap(OSMReader);

    EXPECT_EQ(OpenStreetMap.NodeCount(),2);
    EXPECT_EQ(OpenStreetMap.WayCount(),1);
    EXPECT_NE(OpenStreetMap.NodeByIndex(0),nullptr);
    EXPECT_NE(OpenStreetMap.NodeByIndex(1),nullptr);
    EXPECT_EQ(OpenStreetMap.NodeByIndex(2),nullptr);
    EXPECT_NE(OpenStreetMap.WayByIndex(0),nullptr);
    EXPECT_EQ(OpenStreetMap.WayByIndex(1),nullptr);
    auto TempNode = OpenStreetMap.NodeByIndex(0);
    ASSERT_NE(TempNode,nullptr);
    EXPECT_EQ(TempNode, OpenStreetMap.NodeByID(TempNode->ID()));
    EXPECT_EQ(TempNode->ID(),1);
    EXPECT_EQ(TempNode->Location(),std::make_pair(38.5,-121.7));
}

// NodeByIndex returns nodes in CSV insertion order.
// NodeByID must return the same pointer as NodeByIndex for the same node.
TEST(OSMTest, NodeByIndexAndID){
    auto Src = std::make_shared< CStringDataSource >(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"10\" lat=\"1.0\" lon=\"2.0\"/>"
        "  <node id=\"20\" lat=\"3.0\" lon=\"4.0\"/>"
        "  <node id=\"30\" lat=\"5.0\" lon=\"6.0\"/>"
        "</osm>");
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    ASSERT_EQ(Map.NodeCount(), 3);
    EXPECT_EQ(Map.NodeByIndex(0)->ID(), 10);
    EXPECT_EQ(Map.NodeByIndex(1)->ID(), 20);
    EXPECT_EQ(Map.NodeByIndex(2)->ID(), 30);
    EXPECT_EQ(Map.NodeByIndex(0), Map.NodeByID(10));
    EXPECT_EQ(Map.NodeByIndex(1), Map.NodeByID(20));
    EXPECT_EQ(Map.NodeByIndex(2), Map.NodeByID(30));
}

// NodeByIndex returns nullptr for out-of-range index.
TEST(OSMTest, NodeByIndexOutOfRange){
    auto Src = std::make_shared< CStringDataSource >(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"0.0\" lon=\"0.0\"/>"
        "</osm>");
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    EXPECT_EQ(Map.NodeByIndex(1), nullptr);
    EXPECT_EQ(Map.NodeByIndex(99), nullptr);
}

// NodeByID returns nullptr when the ID does not exist.
TEST(OSMTest, NodeByIDNotFound){
    auto Src = std::make_shared< CStringDataSource >(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"0.0\" lon=\"0.0\"/>"
        "</osm>");
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    EXPECT_EQ(Map.NodeByID(999), nullptr);
}

// Location returns the correct (lat, lon) pair for each node.
TEST(OSMTest, NodeLocation){
    auto Src = std::make_shared< CStringDataSource >(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>"
        "  <node id=\"2\" lat=\"-33.9\" lon=\"151.2\"/>"
        "</osm>");
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    ASSERT_EQ(Map.NodeCount(), 2);
    EXPECT_EQ(Map.NodeByIndex(0)->Location(), std::make_pair(38.5,  -121.7));
    EXPECT_EQ(Map.NodeByIndex(1)->Location(), std::make_pair(-33.9,  151.2));
}

// Two ways in insertion order; WayByID returns the same pointer.
TEST(OSMTest, WayByIndexAndID){
    auto Src = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"0.0\" lon=\"0.0\"/>"
        "  <node id=\"2\" lat=\"1.0\" lon=\"2.0\"/>"
        "  <way id=\"10\">"
        "    <nd ref=\"1\"/>"
        "    <nd ref=\"2\"/>"
        "    <tag k=\"highway\" v=\"residential\"/>"
        "  </way>"
        "  <way id=\"20\">"
        "    <nd ref=\"2\"/>"
        "    <tag k=\"name\" v=\"Charles Drive\"/>"
        "  </way>"
        "</osm>"
    );
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    ASSERT_EQ(Map.WayCount(), 2u);

    // Insertion order by index
    auto Way_1 = Map.WayByIndex(0);
    auto Way_2 = Map.WayByIndex(1);
    ASSERT_NE(Way_1, nullptr);
    ASSERT_NE(Way_2, nullptr);

    EXPECT_EQ(Way_1->ID(), 10u);
    EXPECT_EQ(Way_2->ID(), 20u);

    // WayByID returns the exact same pointers
    EXPECT_EQ(Way_1, Map.WayByID(10));
    EXPECT_EQ(Way_2, Map.WayByID(20));

    // Way_1 (id=10) has highway; Way_2 (id=20) has name
    EXPECT_TRUE(Way_1->HasAttribute("highway"));
    EXPECT_EQ(Way_1->GetAttribute("highway"), "residential");
    EXPECT_TRUE(Way_2->HasAttribute("name"));
    EXPECT_EQ(Way_2->GetAttribute("name"), "Charles Drive");
}

// WayByIndex returns nullptr for out-of-range index.
TEST(OSMTest, WayByIndexOutOfRange){
    auto Src = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"0.0\" lon=\"0.0\"/>"
        "  <way id=\"10\"><nd ref=\"1\"/></way>"
        "</osm>"
    );
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    ASSERT_EQ(Map.WayCount(), 1u);
    EXPECT_NE(Map.WayByIndex(0), nullptr);
    EXPECT_EQ(Map.WayByIndex(1), nullptr);
    EXPECT_EQ(Map.WayByIndex(99), nullptr);
}

// WayByID returns nullptr when the ID does not exist.
TEST(OSMTest, WayByIDNotFound){
    auto Src = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"0.0\" lon=\"0.0\"/>"
        "  <way id=\"2000\"><nd ref=\"10\"/></way>"
        "</osm>"
    );
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    EXPECT_EQ(Map.WayByID(999), nullptr);
}

// NodeCount() and GetNodeID() for a way; out-of-range returns default/invalid ID.
TEST(OSMTest, WayNodeRefs){
    auto Src = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"10.0\" lon=\"20.0\"/>"
        "  <node id=\"2\" lat=\"30.0\" lon=\"40.0\"/>"
        "  <node id=\"3\" lat=\"50.0\" lon=\"60.0\"/>"
        "  <way id=\"100\">"
        "    <nd ref=\"1\"/>"
        "    <nd ref=\"3\"/>"
        "    <nd ref=\"2\"/>"
        "  </way>"
        "</osm>"
    );
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    auto Way = Map.WayByID(100);
    ASSERT_NE(Way, nullptr);
    ASSERT_EQ(Way->NodeCount(), 3u);

    EXPECT_EQ(Way->GetNodeID(0), 1u);
    EXPECT_EQ(Way->GetNodeID(1), 3u);
    EXPECT_EQ(Way->GetNodeID(2), 2u);

    // If your framework defines a specific InvalidNodeID, use it here.
    // EXPECT_EQ(Way->GetNodeID(3), TNodeID{});
    // EXPECT_EQ(Way->GetNodeID(999), TNodeID{});
}

// A way that forms a loop (repeated node refs allowed and preserved).
TEST(OSMTest, WayWithRepeatedNodeRef){
    auto Src = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"0.0\" lon=\"0.0\"/>"
        "  <node id=\"2\" lat=\"1.0\" lon=\"1.0\"/>"
        "  <way id=\"42\">"
        "    <nd ref=\"1\"/>"
        "    <nd ref=\"2\"/>"
        "    <nd ref=\"1\"/>"  
        "  </way>"
        "</osm>"
    );
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    auto Way = Map.WayByID(42);
    ASSERT_NE(Way, nullptr);
    ASSERT_EQ(Way->NodeCount(), 3u);

    EXPECT_EQ(Way->GetNodeID(0), 1u);
    EXPECT_EQ(Way->GetNodeID(1), 2u);
    EXPECT_EQ(Way->GetNodeID(2), 1u); 
}

// File with only nodes; no ways should be present.
TEST(OSMTest, NoWays){
    auto Src = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"0.0\" lon=\"0.0\"/>"
        "  <node id=\"2\" lat=\"0.5\" lon=\"0.5\"/>"
        "</osm>"
    );
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    EXPECT_EQ(Map.WayCount(), 0u);
    EXPECT_EQ(Map.WayByIndex(0), nullptr);
    EXPECT_EQ(Map.WayByID(123), nullptr);
}

// way attributes should be preserving insertion order 
TEST(OSMTest, WayAttributes){
    auto Src = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "  <node id=\"1\" lat=\"10.0\" lon=\"20.0\"/>"
        "  <node id=\"2\" lat=\"10.1\" lon=\"20.1\"/>"
        "  <way id=\"7\">"
        "    <nd ref=\"1\"/>"
        "    <nd ref=\"2\"/>"
        "    <tag k=\"highway\" v=\"service\"/>"
        "    <tag k=\"name\" v=\"Back Alley\"/>"
        "  </way>"
        "</osm>"
    );
    COpenStreetMap Map(std::make_shared<CXMLReader>(Src));

    auto Way = Map.WayByID(7);
    ASSERT_NE(Way, nullptr);

    EXPECT_TRUE(Way->HasAttribute("highway"));
    EXPECT_EQ(Way->GetAttribute("highway"), "service");

    EXPECT_TRUE(Way->HasAttribute("name"));
    EXPECT_EQ(Way->GetAttribute("name"), "Back Alley");

    EXPECT_EQ(Way->AttributeCount(), 2u);
    EXPECT_EQ(Way->GetAttributeKey(0), "highway");
    EXPECT_EQ(Way->GetAttributeKey(1), "name");
    EXPECT_EQ(Way->GetAttributeKey(2), ""); // out of range should return empty 
}