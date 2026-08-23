# Define the tools in use
AR=ar
CC=gcc
CXX=g++

# Define the directories
INC_DIR			= ./include
SRC_DIR			= ./src
BIN_DIR			= ./bin
OBJ_DIR			= ./obj
LIB_DIR			= ./lib
TESTSRC_DIR		= ./testsrc
TESTOBJ_DIR		= ./testobj
TESTBIN_DIR		= ./testbin
TESTCOVER_DIR 	= ./htmlcov
TESTTMP_DIR		= ./testtmp

# Define the flags
PKGS			= expat
DEFINES			= 
INCLUDE			+= -I $(INC_DIR) 
CFLAGS			+= `pkg-config --cflags $(PKGS)`
CPPFLAGS		+= -std=c++20
LDFLAGS			= `pkg-config --libs $(PKGS)`

TEST_CFLAGS		= $(CFLAGS) -O0 -g --coverage
TEST_CPPFLAGS	= $(CPPFLAGS) -fno-inline
TEST_LDFLAGS	= $(LDFLAGS) -lgtest -lgtest_main -lpthread

BIN_CFLAGS		= $(CFLAGS) -O2
BIN_CPPFLAGS	= $(CPPFLAGS)
BIN_LDFLAGS		= $(LDFLAGS)

# Define the test object files
TEST_STR_OBJ_FILES		= $(TESTOBJ_DIR)/StringUtilsTest.o $(TESTOBJ_DIR)/StringUtils.o
TEST_STRSRC_OBJ_FILES	= $(TESTOBJ_DIR)/StringDataSource.o $(TESTOBJ_DIR)/StringDataSourceTest.o
TEST_STRSINK_OBJ_FILES	= $(TESTOBJ_DIR)/StringDataSink.o $(TESTOBJ_DIR)/StringDataSinkTest.o
TEST_FILESS_OBJ_FILES	= $(TESTOBJ_DIR)/FileDataFactory.o $(TESTOBJ_DIR)/FileDataSource.o $(TESTOBJ_DIR)/FileDataSink.o $(TESTOBJ_DIR)/FileDataSSTest.o
TEST_DSV_OBJ_FILES		= $(TESTOBJ_DIR)/StringDataSource.o $(TESTOBJ_DIR)/StringDataSink.o $(TESTOBJ_DIR)/DSVReader.o $(TESTOBJ_DIR)/DSVWriter.o $(TESTOBJ_DIR)/DSVTest.o
TEST_XML_OBJ_FILES		= $(TESTOBJ_DIR)/StringDataSource.o $(TESTOBJ_DIR)/StringDataSink.o $(TESTOBJ_DIR)/XMLReader.o $(TESTOBJ_DIR)/XMLWriter.o $(TESTOBJ_DIR)/XMLTest.o
TEST_CSVBS_OBJ_FILES	= $(TESTOBJ_DIR)/StringDataSource.o $(TESTOBJ_DIR)/DSVReader.o $(TESTOBJ_DIR)/CSVBusSystem.o $(TESTOBJ_DIR)/CSVBusSystemTest.o
TEST_OSM_OBJ_FILES		= $(TESTOBJ_DIR)/StringDataSource.o $(TESTOBJ_DIR)/XMLReader.o $(TESTOBJ_DIR)/OpenStreetMap.o $(TESTOBJ_DIR)/OpenStreetMapTest.o
TEST_GEOUTILS_OBJ_FILES	= $(TESTOBJ_DIR)/GeographicUtils.o $(TESTOBJ_DIR)/GeographicUtilsTest.o
TEST_KML_OBJ_FILES		= $(TESTOBJ_DIR)/StringDataSink.o $(TESTOBJ_DIR)/StringUtils.o $(TESTOBJ_DIR)/XMLWriter.o $(TESTOBJ_DIR)/KMLWriter.o $(TESTOBJ_DIR)/KMLTest.o
TEST_DPR_OBJ_FILES		= $(TESTOBJ_DIR)/DijkstraPathRouter.o $(TESTOBJ_DIR)/DijkstraPathRouterTest.o
TEST_CSVBSI_OBJ_FILES	= $(TESTOBJ_DIR)/StringDataSource.o $(TESTOBJ_DIR)/DSVReader.o $(TESTOBJ_DIR)/CSVBusSystem.o $(TESTOBJ_DIR)/BusSystemIndexer.o $(TESTOBJ_DIR)/CSVBusSystemIndexerTest.o
TEST_TP_OBJ_FILES		= $(TESTOBJ_DIR)/StringDataSource.o $(TESTOBJ_DIR)/DSVReader.o $(TESTOBJ_DIR)/XMLReader.o \
	$(TESTOBJ_DIR)/OpenStreetMap.o $(TESTOBJ_DIR)/CSVBusSystem.o $(TESTOBJ_DIR)/BusSystemIndexer.o \
	$(TESTOBJ_DIR)/DijkstraPathRouter.o $(TESTOBJ_DIR)/GeographicUtils.o \
	$(TESTOBJ_DIR)/DijkstraTransportationPlanner.o $(TESTOBJ_DIR)/CSVOSMTransportationPlannerTest.o
TEST_TPCL_OBJ_FILES		= $(TESTOBJ_DIR)/StringDataSource.o $(TESTOBJ_DIR)/StringDataSink.o \
	$(TESTOBJ_DIR)/DSVWriter.o $(TESTOBJ_DIR)/GeographicUtils.o \
	$(TESTOBJ_DIR)/TransportationPlannerCommandLine.o $(TESTOBJ_DIR)/TPCommandLineTest.o

# Define the test targets (names match spec)
TEST_STR_TARGET		= $(TESTBIN_DIR)/teststrutils
TEST_STRSRC_TARGET	= $(TESTBIN_DIR)/teststrdatasource
TEST_STRSINK_TARGET	= $(TESTBIN_DIR)/teststrdatasink
TEST_FILESS_TARGET	= $(TESTBIN_DIR)/testfiledatass
TEST_DSV_TARGET		= $(TESTBIN_DIR)/testdsv
TEST_XML_TARGET		= $(TESTBIN_DIR)/testxml
TEST_CSVBS_TARGET	= $(TESTBIN_DIR)/testcsvbs
TEST_OSM_TARGET		= $(TESTBIN_DIR)/testosm
TEST_GEOUTILS_TARGET	= $(TESTBIN_DIR)/testgeoutils
TEST_KML_TARGET		= $(TESTBIN_DIR)/testkml
TEST_DPR_TARGET		= $(TESTBIN_DIR)/testdpr
TEST_CSVBSI_TARGET	= $(TESTBIN_DIR)/testcsvbsi
TEST_TP_TARGET		= $(TESTBIN_DIR)/testtp
TEST_TPCL_TARGET	= $(TESTBIN_DIR)/testtpcl

# Define the binary object files
TRANSPLANNER_OBJ_FILES	= $(OBJ_DIR)/transplanner.o $(OBJ_DIR)/StringUtils.o \
	$(OBJ_DIR)/StringDataSource.o $(OBJ_DIR)/StringDataSink.o \
	$(OBJ_DIR)/StandardDataSource.o $(OBJ_DIR)/StandardDataSink.o $(OBJ_DIR)/StandardErrorDataSink.o \
	$(OBJ_DIR)/FileDataFactory.o $(OBJ_DIR)/FileDataSource.o $(OBJ_DIR)/FileDataSink.o \
	$(OBJ_DIR)/DSVReader.o $(OBJ_DIR)/DSVWriter.o \
	$(OBJ_DIR)/XMLReader.o $(OBJ_DIR)/XMLWriter.o \
	$(OBJ_DIR)/CSVBusSystem.o $(OBJ_DIR)/OpenStreetMap.o \
	$(OBJ_DIR)/BusSystemIndexer.o $(OBJ_DIR)/DijkstraPathRouter.o \
	$(OBJ_DIR)/GeographicUtils.o \
	$(OBJ_DIR)/DijkstraTransportationPlanner.o \
	$(OBJ_DIR)/TransportationPlannerCommandLine.o

SPEEDTEST_OBJ_FILES		= $(OBJ_DIR)/speedtest.o $(OBJ_DIR)/StringUtils.o \
	$(OBJ_DIR)/StringDataSource.o $(OBJ_DIR)/StringDataSink.o \
	$(OBJ_DIR)/StandardDataSource.o $(OBJ_DIR)/StandardDataSink.o $(OBJ_DIR)/StandardErrorDataSink.o \
	$(OBJ_DIR)/FileDataFactory.o $(OBJ_DIR)/FileDataSource.o $(OBJ_DIR)/FileDataSink.o \
	$(OBJ_DIR)/DSVReader.o $(OBJ_DIR)/DSVWriter.o \
	$(OBJ_DIR)/XMLReader.o $(OBJ_DIR)/XMLWriter.o \
	$(OBJ_DIR)/CSVBusSystem.o $(OBJ_DIR)/OpenStreetMap.o \
	$(OBJ_DIR)/BusSystemIndexer.o $(OBJ_DIR)/DijkstraPathRouter.o \
	$(OBJ_DIR)/GeographicUtils.o \
	$(OBJ_DIR)/DijkstraTransportationPlanner.o \
	$(OBJ_DIR)/TransportationPlannerCommandLine.o

# Binary targets
TRANSPLANNER_TARGET	= $(BIN_DIR)/transplanner
SPEEDTEST_TARGET	= $(BIN_DIR)/speedtest

# Run order matches spec exactly:
# teststrutils, teststrdatasource, teststrdatasink, testfiledatass,
# testdsv, testxml, testcsvbs, testosm, testdpr, testcsvbsi, testtp, testtpcl
all: directories \
		run_strtest \
		run_strsrctest \
		run_strsinktest \
		run_filesstest \
		run_dsvtest \
		run_xmltest \
		run_kmltest \
		run_csvbstest \
		run_osmtest \
		run_dprtest \
		run_csvbsitest \
		run_tptest \
		run_tpcltest \
		$(TRANSPLANNER_TARGET) \
		$(SPEEDTEST_TARGET) \
		gencoverage

run_strtest: $(TEST_STR_TARGET)
	$(TEST_STR_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_strsrctest: $(TEST_STRSRC_TARGET)
	$(TEST_STRSRC_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_strsinktest: $(TEST_STRSINK_TARGET)
	$(TEST_STRSINK_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_filesstest: $(TEST_FILESS_TARGET)
	$(TEST_FILESS_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_dsvtest: $(TEST_DSV_TARGET)
	$(TEST_DSV_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_xmltest: $(TEST_XML_TARGET)
	$(TEST_XML_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_kmltest: $(TEST_KML_TARGET)
	$(TEST_KML_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_csvbstest: $(TEST_CSVBS_TARGET)
	$(TEST_CSVBS_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_osmtest: $(TEST_OSM_TARGET)
	$(TEST_OSM_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_dprtest: $(TEST_DPR_TARGET)
	$(TEST_DPR_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_csvbsitest: $(TEST_CSVBSI_TARGET)
	$(TEST_CSVBSI_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_tptest: $(TEST_TP_TARGET)
	$(TEST_TP_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

run_tpcltest: $(TEST_TPCL_TARGET)
	$(TEST_TPCL_TARGET) --gtest_output=xml:$(TESTTMP_DIR)/$@
	mv $(TESTTMP_DIR)/$@ $@

gencoverage:
	lcov --capture --directory . --output-file $(TESTCOVER_DIR)/coverage.info --ignore-errors inconsistent,source
	lcov --remove $(TESTCOVER_DIR)/coverage.info '/usr/*' '*/testsrc/*' --output-file $(TESTCOVER_DIR)/coverage.info
	genhtml $(TESTCOVER_DIR)/coverage.info --output-directory $(TESTCOVER_DIR)

# Test binary link rules
$(TEST_STR_TARGET): $(TEST_STR_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_STR_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_STRSRC_TARGET): $(TEST_STRSRC_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_STRSRC_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_STRSINK_TARGET): $(TEST_STRSINK_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_STRSINK_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_FILESS_TARGET): $(TEST_FILESS_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_FILESS_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_DSV_TARGET): $(TEST_DSV_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_DSV_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_XML_TARGET): $(TEST_XML_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_XML_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_CSVBS_TARGET): $(TEST_CSVBS_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_CSVBS_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_OSM_TARGET): $(TEST_OSM_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_OSM_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_GEOUTILS_TARGET): $(TEST_GEOUTILS_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_GEOUTILS_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_KML_TARGET): $(TEST_KML_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_KML_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_DPR_TARGET): $(TEST_DPR_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_DPR_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_CSVBSI_TARGET): $(TEST_CSVBSI_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_CSVBSI_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_TP_TARGET): $(TEST_TP_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_TP_OBJ_FILES) $(TEST_LDFLAGS) -o $@

$(TEST_TPCL_TARGET): $(TEST_TPCL_OBJ_FILES)
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(TEST_TPCL_OBJ_FILES) $(TEST_LDFLAGS) -lgmock -o $@

# Production binary link rules
$(TRANSPLANNER_TARGET): $(TRANSPLANNER_OBJ_FILES)
	$(CXX) $(BIN_CFLAGS) $(BIN_CPPFLAGS) $(TRANSPLANNER_OBJ_FILES) $(BIN_LDFLAGS) -o $@

$(SPEEDTEST_TARGET): $(SPEEDTEST_OBJ_FILES)
	$(CXX) $(BIN_CFLAGS) $(BIN_CPPFLAGS) $(SPEEDTEST_OBJ_FILES) $(BIN_LDFLAGS) -o $@

# Compile rules for test objects
$(TESTOBJ_DIR)/%.o: $(TESTSRC_DIR)/%.cpp
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@

$(TESTOBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@

# Compile rules for production objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(BIN_CFLAGS) $(BIN_CPPFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@

.PHONY: directories
directories:
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)
	mkdir -p $(LIB_DIR)
	mkdir -p $(TESTBIN_DIR)
	mkdir -p $(TESTOBJ_DIR)
	mkdir -p $(TESTCOVER_DIR)
	mkdir -p $(TESTTMP_DIR)

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)
	rm -rf $(LIB_DIR)
	rm -rf $(TESTBIN_DIR)
	rm -rf $(TESTOBJ_DIR)
	rm -rf $(TESTCOVER_DIR)
	rm -rf $(TESTTMP_DIR)
