#include <gtest/gtest.h>
#include "DSVReader.h"
#include "DSVWriter.h"
#include "StringDataSource.h"
#include "StringDataSink.h"
#include <vector>

TEST(DSVWriterTest, EmptyRowTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(DataSink->String().empty());
    EXPECT_EQ(DataSink->String(),"");
    EXPECT_TRUE(Writer.WriteRow({}));
    EXPECT_EQ(DataSink->String(),"");

}


TEST(DSVWriterTest, SingleRowTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(DataSink->String().empty());
    EXPECT_EQ(DataSink->String(),"");
    EXPECT_TRUE(Writer.WriteRow({"A","B","C"}));
    EXPECT_EQ(DataSink->String(),"A,B,C");

}

// --- DSVReader tests ---

TEST(DSVReaderTest, SingleRow){
    auto Src = std::make_shared<CStringDataSource>("a,b,c");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;
    EXPECT_TRUE(Reader.ReadRow(row));
    EXPECT_EQ(row.size(), 3u); // what is this 
    EXPECT_EQ(row[0], "a");
    EXPECT_EQ(row[1], "b");
    EXPECT_EQ(row[2], "c");
    EXPECT_TRUE(Reader.End());
}

TEST(DSVReaderTest, QuotedFieldWithDelimiter){
    auto Src = std::make_shared<CStringDataSource>("\"a,b\",c");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;
    EXPECT_TRUE(Reader.ReadRow(row));
    EXPECT_EQ(row.size(), 2u);
    EXPECT_EQ(row[0], "a,b");
    EXPECT_EQ(row[1], "c");
}

TEST(DSVReaderTest, ConsecutiveDelimiters)
{
    auto Src = std::make_shared<CStringDataSource>("a,,c");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;
    EXPECT_TRUE(Reader.ReadRow(row)); 
    EXPECT_EQ(row.size(), 3u);
    EXPECT_EQ(row[0],"a"); 
    EXPECT_EQ(row[1],"");
    EXPECT_EQ(row[2],"c");    
}

TEST(DSVReaderTest, LeadingandTrailingEmptyFields)
{
    auto Src = std::make_shared<CStringDataSource>(",b,c");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;
    EXPECT_TRUE(Reader.ReadRow(row)); 
    EXPECT_EQ(row.size(), 3u);
    EXPECT_EQ(row[0],""); 
    EXPECT_EQ(row[1],"b");
    EXPECT_EQ(row[2],"c");   
}

TEST(DSVReaderTest, QuotedFieldWithQuotes){
    auto Src = std::make_shared<CStringDataSource>("\"a\"\"b\",c");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;

    EXPECT_TRUE(Reader.ReadRow(row));
    EXPECT_EQ(row.size(), 2u);
    EXPECT_EQ(row[0], "a\"b");
    EXPECT_EQ(row[1], "c");
}

TEST(DSVReaderTest, NewlineEndsRow){
    auto Src = std::make_shared<CStringDataSource>("a,b\nc,d");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;

    EXPECT_TRUE(Reader.ReadRow(row));
    EXPECT_EQ(row.size(), 2u);
    EXPECT_EQ(row[0], "a");
    EXPECT_EQ(row[1], "b");

    EXPECT_TRUE(Reader.ReadRow(row));
    EXPECT_EQ(row.size(), 2u);
    EXPECT_EQ(row[0], "c");
    EXPECT_EQ(row[1], "d");
}

TEST(DSVReaderTest, TrailingNewline){
    auto Src = std::make_shared<CStringDataSource>("a,b,c\n");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;

    EXPECT_TRUE(Reader.ReadRow(row));
    EXPECT_EQ(row.size(), 3u);

    EXPECT_TRUE(Reader.End());
}

TEST(DSVReaderTest, OnlyDelimiters){
    auto Src = std::make_shared<CStringDataSource>(",,");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;

    EXPECT_TRUE(Reader.ReadRow(row));
    EXPECT_EQ(row.size(), 3u);
    EXPECT_EQ(row[0], "");
    EXPECT_EQ(row[1], "");
    EXPECT_EQ(row[2], "");
}

TEST(DSVReaderTest, QuotedEmptyField){
    auto Src = std::make_shared<CStringDataSource>("\"\",a");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;

    EXPECT_TRUE(Reader.ReadRow(row));
    EXPECT_EQ(row.size(), 2u);
    EXPECT_EQ(row[0], "");
    EXPECT_EQ(row[1], "a");
}

/* TEST(DSVReaderTest, EmptySource){
    auto Src = std::make_shared<CStringDataSource>("");
    CDSVReader Reader(Src, ',');
    std::vector<std::string> row;
    
    
    EXPECT_TRUE(Reader.End());
    EXPECT_TRUE(row.empty());
} */ 

