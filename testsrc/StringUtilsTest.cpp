#include <gtest/gtest.h>
#include "StringUtils.h"

TEST(StringUtilsTest, SliceTest){
    EXPECT_EQ(StringUtils::Slice("hello", 0, 3), std::string("hel"));
    EXPECT_EQ(StringUtils::Slice("hello", 0, 0), std::string("hello"));
    EXPECT_EQ(StringUtils::Slice("hello", -3, -1), std::string("ll"));
    EXPECT_EQ(StringUtils::Slice("hello", 0, 10), std::string("hello"));
    EXPECT_EQ(StringUtils::Slice("hello", 10, 15), std::string(""));
    EXPECT_EQ(StringUtils::Slice("", 0, 0), std::string("")); // empty string
    EXPECT_EQ(StringUtils::Slice("hello", 2, 2), std::string("")); // start equals end
}

TEST(StringUtilsTest, Capitalize){
    EXPECT_EQ(StringUtils::Capitalize(""),std::string(""));
    EXPECT_EQ(StringUtils::Capitalize("hello"),std::string("Hello"));
    EXPECT_EQ(StringUtils::Capitalize(" hello world"),std::string(" hello world"));
    EXPECT_EQ(StringUtils::Capitalize("hello world"),std::string("Hello world"));
    EXPECT_EQ(StringUtils::Capitalize("HELLO WORLD"),std::string("Hello world"));
}

TEST(StringUtilsTest, Upper){
    EXPECT_EQ(StringUtils::Upper("hello"), std::string("HELLO"));
    EXPECT_EQ(StringUtils::Upper("HELLO"), std::string("HELLO"));
    EXPECT_EQ(StringUtils::Upper("Hello"), std::string("HELLO"));
    EXPECT_EQ(StringUtils::Upper("hello world"), std::string("HELLO WORLD"));
    EXPECT_EQ(StringUtils::Upper(""), std::string("")); // empty string
    EXPECT_EQ(StringUtils::Upper("123abc"), std::string("123ABC"));
    EXPECT_EQ(StringUtils::Upper("!@#$%"), std::string("!@#$%")); // special chars unchanged
}

TEST(StringUtilsTest, Lower){
    EXPECT_EQ(StringUtils::Lower("HELLO"), std::string("hello"));
    EXPECT_EQ(StringUtils::Lower("hello"), std::string("hello"));
    EXPECT_EQ(StringUtils::Lower("Hello"), std::string("hello"));
    EXPECT_EQ(StringUtils::Lower("HELLO WORLD"), std::string("hello world"));
    EXPECT_EQ(StringUtils::Lower(""), std::string("")); // empty string
    EXPECT_EQ(StringUtils::Lower("123ABC"), std::string("123abc"));
    EXPECT_EQ(StringUtils::Lower("!@#$%"), std::string("!@#$%")); // special chars unchanged
}

TEST(StringUtilsTest, LStrip){
    EXPECT_EQ(StringUtils::LStrip("Hello"),std::string("Hello"));
    EXPECT_EQ(StringUtils::LStrip(" Hello"),std::string("Hello"));
    EXPECT_EQ(StringUtils::LStrip("Hello "),std::string("Hello "));
    EXPECT_EQ(StringUtils::LStrip("   Hello"),std::string("Hello"));
    EXPECT_EQ(StringUtils::LStrip(" \r \t \nHello"),std::string("Hello"));
}

TEST(StringUtilsTest, RStrip){
    EXPECT_EQ(StringUtils::RStrip("      "),std::string(""));
    EXPECT_EQ(StringUtils::RStrip(""),std::string(""));
    EXPECT_EQ(StringUtils::RStrip("Hello"),std::string("Hello"));
    EXPECT_EQ(StringUtils::RStrip("Hello "),std::string("Hello"));
    EXPECT_EQ(StringUtils::RStrip(" Hello"),std::string(" Hello"));
    EXPECT_EQ(StringUtils::RStrip("Hello   "),std::string("Hello"));
    EXPECT_EQ(StringUtils::RStrip("Hello \r \t \n"),std::string("Hello"));
}

TEST(StringUtilsTest, Strip){
    EXPECT_EQ(StringUtils::Strip(" Hello"),std::string("Hello"));
    EXPECT_EQ(StringUtils::Strip("Hello "),std::string("Hello"));
    EXPECT_EQ(StringUtils::Strip("   Hello"),std::string("Hello"));
    EXPECT_EQ(StringUtils::Strip(" \r \t \nHello"),std::string("Hello"));
    EXPECT_EQ(StringUtils::Strip("      "),std::string(""));
    EXPECT_EQ(StringUtils::Strip(""),std::string(""));
    EXPECT_EQ(StringUtils::Strip("Hello   "),std::string("Hello"));
    EXPECT_EQ(StringUtils::Strip("Hello \r \t \n"),std::string("Hello"));
    EXPECT_EQ(StringUtils::Strip(" \r \t \nHello \r \t \n"),std::string("Hello"));
}

TEST(StringUtilsTest, Center){
    EXPECT_EQ(StringUtils::Center("hi", 5), std::string(" hi  "));
    EXPECT_EQ(StringUtils::Center("hi", 4), std::string(" hi "));
    EXPECT_EQ(StringUtils::Center("hi", 2), std::string("hi"));
    EXPECT_EQ(StringUtils::Center("hi", 1), std::string("hi"));
    EXPECT_EQ(StringUtils::Center("hi", 5, '*'), std::string("*hi**"));
    EXPECT_EQ(StringUtils::Center("", 5), std::string("     ")); // empty string
    EXPECT_EQ(StringUtils::Center("a", 3), std::string(" a ")); // single character
}

TEST(StringUtilsTest, LJust){
    EXPECT_EQ(StringUtils::LJust("hi", 5), std::string("hi   "));
    EXPECT_EQ(StringUtils::LJust("hi", 4), std::string("hi  "));
    EXPECT_EQ(StringUtils::LJust("hi", 2), std::string("hi"));
    EXPECT_EQ(StringUtils::LJust("hello", 10), std::string("hello     "));
    EXPECT_EQ(StringUtils::LJust("hi", 5, '*'), std::string("hi***"));
    EXPECT_EQ(StringUtils::LJust("", 5), std::string("     ")); // empty string
    EXPECT_EQ(StringUtils::LJust("a", 3), std::string("a  ")); // single character
}

TEST(StringUtilsTest, RJust){
    EXPECT_EQ(StringUtils::RJust("hi", 5), std::string("   hi"));
    EXPECT_EQ(StringUtils::RJust("hi", 4), std::string("  hi"));
    EXPECT_EQ(StringUtils::RJust("hi", 2), std::string("hi"));
    EXPECT_EQ(StringUtils::RJust("hello", 10), std::string("     hello"));
    EXPECT_EQ(StringUtils::RJust("hi", 5, '*'), std::string("***hi"));
    EXPECT_EQ(StringUtils::RJust("", 5), std::string("     ")); // empty string
    EXPECT_EQ(StringUtils::RJust("a", 3), std::string("  a")); // single character
}

TEST(StringUtilsTest, Replace){
    EXPECT_EQ(StringUtils::Replace("hello world", "world", "there"), std::string("hello there"));
    EXPECT_EQ(StringUtils::Replace("hello hello", "hello", "hi"), std::string("hi hi"));
    EXPECT_EQ(StringUtils::Replace("abc", "x", "y"), std::string("abc"));
    EXPECT_EQ(StringUtils::Replace("hello", "", "x"), std::string("hello")); // empty old string
    EXPECT_EQ(StringUtils::Replace("", "a", "b"), std::string("")); // empty string
}

TEST(StringUtilsTest, Split){
    std::vector<std::string> result = StringUtils::Split("a,b,c", ",");
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], std::string("a"));
    EXPECT_EQ(result[1], std::string("b"));
    EXPECT_EQ(result[2], std::string("c"));
    
    result = StringUtils::Split("hello world", "");
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], std::string("hello"));
    EXPECT_EQ(result[1], std::string("world"));
    
    result = StringUtils::Split("", ",");
    EXPECT_EQ(result.size(), 1); // empty string
    EXPECT_EQ(result[0], std::string(""));
}

TEST(StringUtilsTest, Join){
    std::vector<std::string> vec = {"a", "b", "c"};
    EXPECT_EQ(StringUtils::Join(",", vec), std::string("a,b,c"));
    
    vec = {"a"};
    EXPECT_EQ(StringUtils::Join(",", vec), std::string("a")); // single element
    
    vec = {};
    EXPECT_EQ(StringUtils::Join(",", vec), std::string("")); // empty vector
}

TEST(StringUtilsTest, ExpandTabs){
    EXPECT_EQ(StringUtils::ExpandTabs("a\tb", 4), std::string("a   b"));
    EXPECT_EQ(StringUtils::ExpandTabs("\tabc", 4), std::string("    abc"));
    EXPECT_EQ(StringUtils::ExpandTabs("", 4), std::string("")); // empty string
    EXPECT_EQ(StringUtils::ExpandTabs("no tabs", 4), std::string("no tabs")); // no tabs
}

TEST(StringUtilsTest, EditDistance){
    EXPECT_EQ(StringUtils::EditDistance("kitten", "sitting"), 3);
    EXPECT_EQ(StringUtils::EditDistance("hello", "hello"), 0);
    EXPECT_EQ(StringUtils::EditDistance("abc", ""), 3); // empty string
    EXPECT_EQ(StringUtils::EditDistance("", ""), 0); // both empty
    EXPECT_EQ(StringUtils::EditDistance("Hello", "hello", true), 0); // case-insensitive
}
