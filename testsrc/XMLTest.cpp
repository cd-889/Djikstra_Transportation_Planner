#include <gtest/gtest.h>
#include "XMLReader.h"
#include "XMLWriter.h"
#include "StringDataSource.h"
#include "StringDataSink.h"

TEST(XMLReader, SimpleTest){
    std::string XMLString = "<tag></tag>";
    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(XMLString);
    CXMLReader Reader(DataSource);
    SXMLEntity TempEntity;
    EXPECT_TRUE(Reader.ReadEntity(TempEntity));
    EXPECT_EQ(TempEntity.DType,SXMLEntity::EType::StartElement);
    EXPECT_EQ(TempEntity.DNameData,"tag");
    EXPECT_TRUE(TempEntity.DAttributes.empty());
    EXPECT_TRUE(Reader.ReadEntity(TempEntity));
    EXPECT_EQ(TempEntity.DType,SXMLEntity::EType::EndElement);
    EXPECT_EQ(TempEntity.DNameData,"tag");
    EXPECT_TRUE(TempEntity.DAttributes.empty());
}


TEST(XMLReader, CharDataTest){
    std::string XMLString = "<tag attr1=\"val1\">Some Data</tag>";
    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(XMLString);
    CXMLReader Reader(DataSource);
    SXMLEntity TempEntity;
    EXPECT_TRUE(Reader.ReadEntity(TempEntity));
    EXPECT_EQ(TempEntity.DType,SXMLEntity::EType::StartElement);
    EXPECT_EQ(TempEntity.DNameData,"tag");
    ASSERT_EQ(TempEntity.DAttributes.size(),1);
    EXPECT_EQ(TempEntity.AttributeValue("attr1"),"val1");
    EXPECT_TRUE(Reader.ReadEntity(TempEntity));
    EXPECT_EQ(TempEntity.DType,SXMLEntity::EType::CharData);
    EXPECT_EQ(TempEntity.DNameData,"Some Data");
    EXPECT_TRUE(Reader.ReadEntity(TempEntity));
    EXPECT_EQ(TempEntity.DType,SXMLEntity::EType::EndElement);
    EXPECT_EQ(TempEntity.DNameData,"tag");
    EXPECT_TRUE(TempEntity.DAttributes.empty());
}

TEST(XMLWriterTest, SimpleElementTest){
    auto sink = std::make_shared<CStringDataSink>();
    CXMLWriter writer(sink);
    SXMLEntity start;
    start.DType = SXMLEntity::EType::StartElement;
    start.DNameData = "tag";
    EXPECT_TRUE(writer.WriteEntity(start));
    SXMLEntity end;
    end.DType = SXMLEntity::EType::EndElement;
    end.DNameData = "tag";
    EXPECT_TRUE(writer.WriteEntity(end));
    EXPECT_TRUE(writer.Flush());
    EXPECT_EQ(sink->String(), "<tag></tag>");
}

TEST(XMLWriterTest, ElementWithAttributeAndCharDataTest){
    auto sink = std::make_shared<CStringDataSink>();
    CXMLWriter writer(sink);
    SXMLEntity start;
    start.DType = SXMLEntity::EType::StartElement;
    start.DNameData = "tag";
    start.SetAttribute("attr1", "val1");
    EXPECT_TRUE(writer.WriteEntity(start));
    SXMLEntity text;
    text.DType = SXMLEntity::EType::CharData;
    text.DNameData = "Some Data";
    EXPECT_TRUE(writer.WriteEntity(text));
    SXMLEntity end;
    end.DType = SXMLEntity::EType::EndElement;
    end.DNameData = "tag";
    EXPECT_TRUE(writer.WriteEntity(end));
    EXPECT_TRUE(writer.Flush());
    EXPECT_EQ(sink->String(), "<tag attr1=\"val1\">Some Data</tag>");
}

TEST(XMLEntityTest, AttributeExistsTest){
    SXMLEntity e;
    e.SetAttribute("Name", "John"); 
    EXPECT_TRUE(e.AttributeExists("Name")); 
    EXPECT_FALSE(e.AttributeExists("age")); 

}

TEST(XMLEntityTest, EmptyName){
    SXMLEntity e; 
    EXPECT_FALSE(e.SetAttribute("", "val")); 
}

TEST(XMLEntityTest, UpdatingExistingAttribute)
{
    SXMLEntity e; 
    EXPECT_TRUE(e.SetAttribute("Name", "John"));
    EXPECT_EQ(e.AttributeValue("Name"), "John"); 

    EXPECT_TRUE(e.SetAttribute("Name", "Julia")); 
    EXPECT_EQ(e.AttributeValue("Name"), "Julia"); 
}

TEST(XMLWriterTest, CharDataEscapingTest){
    auto sink = std::make_shared<CStringDataSink>();
    CXMLWriter writer(sink);

    SXMLEntity start;
    start.DType = SXMLEntity::EType::StartElement;
    start.DNameData = "tag";
    writer.WriteEntity(start);

    SXMLEntity text;
    text.DType = SXMLEntity::EType::CharData;
    text.DNameData = "& < > \" '";
    writer.WriteEntity(text);

    SXMLEntity end;
    end.DType = SXMLEntity::EType::EndElement;
    end.DNameData = "tag";
    writer.WriteEntity(end);

    writer.Flush();

    EXPECT_EQ(
        sink->String(),
        "<tag>&amp; &lt; &gt; &quot; &apos;</tag>"
    );
}

TEST(XMLWriterTest, AttributeEscapingTest){
    auto sink = std::make_shared<CStringDataSink>();
    CXMLWriter writer(sink);

    SXMLEntity e;
    e.DType = SXMLEntity::EType::StartElement;
    e.DNameData = "tag";
    e.SetAttribute("attr", "&<>\"'");
    writer.WriteEntity(e);

    SXMLEntity end;
    end.DType = SXMLEntity::EType::EndElement;
    end.DNameData = "tag";
    writer.WriteEntity(end);

    writer.Flush();

    EXPECT_EQ(
        sink->String(),
        "<tag attr=\"&amp;&lt;&gt;&quot;&apos;\"></tag>"
    );
}

TEST(XMLWriterTest, CompleteElementTest){
    auto sink = std::make_shared<CStringDataSink>();
    CXMLWriter writer(sink);

    SXMLEntity e;
    e.DType = SXMLEntity::EType::CompleteElement;
    e.DNameData = "tag";
    e.SetAttribute("a", "1");

    EXPECT_TRUE(writer.WriteEntity(e));
    writer.Flush();

    EXPECT_EQ(sink->String(), "<tag a=\"1\"/>");
}

TEST(XMLWriterTest, FlushClosesOpenElements){
    auto sink = std::make_shared<CStringDataSink>();
    {
        CXMLWriter writer(sink);

        SXMLEntity start;
        start.DType = SXMLEntity::EType::StartElement;
        start.DNameData = "outside";
        writer.WriteEntity(start);

        SXMLEntity cool;
        cool.DType = SXMLEntity::EType::StartElement;
        cool.DNameData = "cool";
        writer.WriteEntity(cool);

    } 

    EXPECT_EQ(sink->String(), "<outside><cool></cool></outside>");
}

TEST(XMLWriterTest, MismatchedEndElement){
    auto sink = std::make_shared<CStringDataSink>();
    CXMLWriter writer(sink);

    SXMLEntity start;
    start.DType = SXMLEntity::EType::StartElement;
    start.DNameData = "tag";
    writer.WriteEntity(start);

    SXMLEntity end;
    end.DType = SXMLEntity::EType::EndElement;
    end.DNameData = "last";
    writer.WriteEntity(end);

    writer.Flush();

    EXPECT_EQ(sink->String(), "<tag></last></tag>");
}
