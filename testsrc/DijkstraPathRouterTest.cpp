#include <gtest/gtest.h>
#include "DijkstraPathRouter.h"

TEST(DijkstraPathRouter, SimpleTest){
    CDijkstraPathRouter PathRouter;

    EXPECT_EQ(PathRouter.VertexCount(),0);
    auto VertexID = PathRouter.AddVertex(std::string("foo"));
    EXPECT_EQ(PathRouter.VertexCount(),1);
    auto VetextTag = std::any_cast<std::string>(PathRouter.GetVertexTag(VertexID));
    EXPECT_EQ(VetextTag,"foo");
    
}

TEST(DijkstraPathRouter, ShortestPath){
    CDijkstraPathRouter PathRouter;
    /*
                5   
       A ---> B --->C
          4  2|  ___^
              V / 1
              D
    
    */
    auto VertexA = PathRouter.AddVertex(std::string("A"));
    auto VertexB = PathRouter.AddVertex(std::string("B"));
    auto VertexC = PathRouter.AddVertex(std::string("C"));
    auto VertexD = PathRouter.AddVertex(std::string("D"));
    EXPECT_EQ(PathRouter.VertexCount(),4);
    EXPECT_TRUE(PathRouter.AddEdge(VertexA,VertexB,4.0));
    EXPECT_TRUE(PathRouter.AddEdge(VertexB,VertexC,5.0));
    EXPECT_TRUE(PathRouter.AddEdge(VertexB,VertexD,2.0));
    EXPECT_TRUE(PathRouter.AddEdge(VertexD,VertexC,1.0));
    std::vector<CPathRouter::TVertexID> Path;
    EXPECT_EQ(PathRouter.FindShortestPath(VertexA,VertexC,Path), 7.0);
    std::vector<CPathRouter::TVertexID> ExpectedPath{VertexA,VertexB,VertexD,VertexC};
    EXPECT_EQ(Path,ExpectedPath);
}

TEST(DijkstraPathRouter, NoPathExists){
    CDijkstraPathRouter PathRouter;
    auto VertexA = PathRouter.AddVertex(std::string("A"));
    auto VertexB = PathRouter.AddVertex(std::string("B"));
    // no edges added — disconnected graph
    std::vector<CPathRouter::TVertexID> Path;
    EXPECT_EQ(PathRouter.FindShortestPath(VertexA,VertexB,Path), CPathRouter::NoPathExists);
}

TEST(DijkstraPathRouter, NegativeWeightRejected){
    CDijkstraPathRouter PathRouter;
    auto VertexA = PathRouter.AddVertex(std::string("A"));
    auto VertexB = PathRouter.AddVertex(std::string("B"));
    EXPECT_FALSE(PathRouter.AddEdge(VertexA,VertexB,-1.0));
    EXPECT_EQ(PathRouter.VertexCount(),2);
}

TEST(DijkstraPathRouter, InvalidVertexTag){
    CDijkstraPathRouter PathRouter;
    auto Tag = PathRouter.GetVertexTag(999);
    EXPECT_FALSE(Tag.has_value());
}

TEST(DijkstraPathRouter, BidirectionalEdge){
    CDijkstraPathRouter PathRouter;
    auto VertexA = PathRouter.AddVertex(std::string("A"));
    auto VertexB = PathRouter.AddVertex(std::string("B"));
    EXPECT_TRUE(PathRouter.AddEdge(VertexA,VertexB,3.0,true));
    std::vector<CPathRouter::TVertexID> Path;
    EXPECT_EQ(PathRouter.FindShortestPath(VertexB,VertexA,Path), 3.0);
    std::vector<CPathRouter::TVertexID> ExpectedPath{VertexB,VertexA};
    EXPECT_EQ(Path,ExpectedPath);
}

TEST(DijkstraPathRouter, SrcEqualsDestPath){
    CDijkstraPathRouter PathRouter;
    auto VertexA = PathRouter.AddVertex(std::string("A"));
    std::vector<CPathRouter::TVertexID> Path;
    EXPECT_EQ(PathRouter.FindShortestPath(VertexA,VertexA,Path), 0.0);
    EXPECT_EQ(Path.size(), 1);
    EXPECT_EQ(Path[0], VertexA);
}