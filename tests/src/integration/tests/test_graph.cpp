/*
  Copyright (c) 2014-2016 DataStax
*/
#include "dse_integration.hpp"
#include "options.hpp"

#include <limits>

#define GRAPH_NAME "dse_cpp_driver"

#define GRAPH_CREATE \
  "system.graph(name).option('graph.replication_config').set(replication).ifNotExists().create()"

#define GRAPH_ALLOW_SCANS \
  "schema.config().option('graph.allow_scan').set('true')"

#define GRAPH_ENABLE_STRICT \
  "schema.config().option('graph.schema_mode').set(com.datastax.bdp.graph.api.model.Schema.Mode.Production)"

#define GRAPH_SCHEMA \
  "schema.propertyKey('name').Text().ifNotExists().create();" \
  "schema.propertyKey('age').Int().ifNotExists().create();" \
  "schema.propertyKey('lang').Text().ifNotExists().create();" \
  "schema.propertyKey('weight').Float().ifNotExists().create();" \
  "schema.vertexLabel('person').properties('name', 'age').ifNotExists().create();" \
  "schema.vertexLabel('software').properties('name', 'lang').ifNotExists().create();" \
  "schema.edgeLabel('created').properties('weight').connection('person', 'software').ifNotExists().create();" \
  "schema.edgeLabel('created').connection('software', 'software').add();" \
  "schema.edgeLabel('knows').properties('weight').connection('person', 'person').ifNotExists().create();"

#define GRAPH_DATA \
  "Vertex marko = graph.addVertex(label, 'person', 'name', 'marko', 'age', 29);" \
  "Vertex vadas = graph.addVertex(label, 'person', 'name', 'vadas', 'age', 27);" \
  "Vertex lop = graph.addVertex(label, 'software', 'name', 'lop', 'lang', 'java');" \
  "Vertex josh = graph.addVertex(label, 'person', 'name', 'josh', 'age', 32);" \
  "Vertex ripple = graph.addVertex(label, 'software', 'name', 'ripple', 'lang', 'java');" \
  "Vertex peter = graph.addVertex(label, 'person', 'name', 'peter', 'age', 35);" \
  "marko.addEdge('knows', vadas, 'weight', 0.5f);" \
  "marko.addEdge('knows', josh, 'weight', 1.0f);" \
  "marko.addEdge('created', lop, 'weight', 0.4f);" \
  "josh.addEdge('created', ripple, 'weight', 1.0f);" \
  "josh.addEdge('created', lop, 'weight', 0.4f);" \
  "peter.addEdge('created', lop, 'weight', 0.2f);"

#define MAX_SIGNED_DOUBLE std::numeric_limits<cass_double_t>::max()
#define MAX_SIGNED_INTEGER std::numeric_limits<cass_int32_t>::max()
#define MIN_SIGNED_BIG_INTEGER std::numeric_limits<cass_int64_t>::min()
#define BIG_INTEGER_VALUE test::driver::BigInteger(MIN_SIGNED_BIG_INTEGER)
#define BIG_INTEGER_NAMED_PARAMETER "big_integer_value"
#define BOOLEAN_VALUE test::driver::Boolean(cass_true)
#define BOOLEAN_NAMED_PARAMETER "boolean_value"
#define DOUBLE_VALUE test::driver::Double(MAX_SIGNED_DOUBLE)
#define DOUBLE_NAMED_PARAMETER "double_value"
#define INTEGER_VALUE test::driver::Integer(MAX_SIGNED_INTEGER)
#define INTEGER_NAMED_PARAMETER "integer_value"
#define NULL_VALUE test::driver::Varchar()
#define NULL_DATA_TYPE test::driver::Varchar
#define NULL_NAMED_PARAMETER "null_value"
#define STRING_VALUE std::string("DataStax")
#define STRING_NAMED_PARAMETER "string_value"
#define GRAPH_ARRAY_NAMED_PARAMETER "graph_array_value"
#define GRAPH_OBJECT_NAMED_PARAMETER "graph_object_value"

/**
 * Graph integration tests
 *
 * @dse_version 5.0.0
 */
class GraphIntegrationTest : public DseIntegration {
public:
  void SetUp() {
    CHECK_VERSION(5.0.0);

    // Call the parent setup function (override the CCM startup)
    is_ccm_start_requested_ = false;
    DseIntegration::SetUp();

    // Initialize the graph data for the current cluster (only once)
    if (!is_graph_initialized_) {
      // Assign DSE graph workload, start the cluster, and establish connection
      if (!ccm_->set_dse_workload(CCM::DseWorkload::DSE_WORKLOAD_GRAPH)) {
        ccm_->start_cluster();
      }

      // Indicate the graph/data has been initialized
      is_graph_initialized_ = true;
    }

    // Establish connection to server (e.g. create session)
    connect();
  }

  static void TearDownTestCase() {
    // Reset the DSE workload on the cluster
    if (is_graph_initialized_) {
      Options::ccm()->set_dse_workload(CCM::DseWorkload::DSE_WORKLOAD_CASSANDRA);
    }
  }

  /**
   * Create the graph using the specified replication factor
   *
   * @param graph_name Name of the graph to create
   * @see replication_factor_
   */
  void create_graph(const std::string& graph_name) {
    // Create the graph statement using the pre-determined replication config
    test::driver::DseGraphObject graph_object;
    graph_object.add<std::string>("name", graph_name);
    graph_object.add<std::string>("replication", replication_strategy_);
    test::driver::DseGraphStatement graph_statement(GRAPH_CREATE);
    graph_statement.bind(graph_object);

    // Execute the graph statement and ensure it was created
    dse_session_.execute(graph_statement);
  }

  /**
   * Populate the graph with the classic graph structure examples used in the
   * TinkerPop documentation.
   *
   * @see http://tinkerpop.apache.org/docs/3.1.0-incubating/#intro
   *
   * @param graph_name Name of the graph to initialize
   */
  void populate_classic_graph(const std::string& graph_name) {
    // Initialize the graph pre-populated data
    test::driver::DseGraphOptions options;
    options.set_name(graph_name);
    dse_session_.execute(GRAPH_ALLOW_SCANS, options);
    dse_session_.execute(GRAPH_ENABLE_STRICT, options);
    dse_session_.execute(GRAPH_SCHEMA, options);
    dse_session_.execute(GRAPH_DATA, options);
  }

  /**
   * Create the DSE graph array to use for testing
   *
   * @param is_array_requested True if array should be added to the array;
   *                           false otherwise
   * @param is_object_requested True if object should be added to the array;
   *                            false otherwise
   */
  test::driver::DseGraphArray create_array(bool is_array_requested = true,
    bool is_object_requested = true) {
    // Create the data type objects for the DSE graph array
    test::driver::DseGraphArray graph_array_value;
    if (is_array_requested) {
      graph_array_value.add<test::driver::DseGraphArray>(create_array(false, false));
    }
    graph_array_value.add<test::driver::BigInteger>(BIG_INTEGER_VALUE);
    graph_array_value.add<test::driver::Boolean>(BOOLEAN_VALUE);
    graph_array_value.add<test::driver::Double>(DOUBLE_VALUE);
    graph_array_value.add<test::driver::Integer>(INTEGER_VALUE);
    graph_array_value.add<NULL_DATA_TYPE>(NULL_VALUE);
    if (is_object_requested) {
      graph_array_value.add<test::driver::DseGraphObject>(create_named_object(false, false));
    }
    graph_array_value.add<std::string>(STRING_VALUE);

    return graph_array_value;
  }

  /**
   * Create the DSE graph object to use for testing
   *
   * @param is_array_requested True if array should be added to the object;
   *                           false otherwise
   * @param is_object_requested True if object should be added to the object;
   *                            false otherwise
   */
  test::driver::DseGraphObject create_named_object(bool is_array_requested = true,
    bool is_object_requested = true) {
    test::driver::DseGraphObject graph_object_value;
    if (is_array_requested) {
      graph_object_value.add<test::driver::DseGraphArray>(GRAPH_ARRAY_NAMED_PARAMETER,
        create_array(false, false));
    }
    graph_object_value.add<test::driver::BigInteger>(BIG_INTEGER_NAMED_PARAMETER, BIG_INTEGER_VALUE);
    graph_object_value.add<test::driver::Boolean>(BOOLEAN_NAMED_PARAMETER, BOOLEAN_VALUE);
    graph_object_value.add<test::driver::Double>(DOUBLE_NAMED_PARAMETER, DOUBLE_VALUE);
    graph_object_value.add<test::driver::Integer>(INTEGER_NAMED_PARAMETER, INTEGER_VALUE);
    graph_object_value.add<NULL_DATA_TYPE>(NULL_NAMED_PARAMETER, NULL_VALUE);
    if (is_object_requested) {
      graph_object_value.add<test::driver::DseGraphObject>(GRAPH_OBJECT_NAMED_PARAMETER,
        create_named_object(false, false));
    }
    graph_object_value.add<std::string>(STRING_NAMED_PARAMETER, STRING_VALUE);

    return graph_object_value;
  }

  /**
   * Generate the expected result for the multiple named parameters test using
   * executing the graph query:
   *   [
   *      big_integer_value,
   *      boolean_value,
   *      double_value,
   *      integer_value,
   *      null_value,
   *      string_value,
   *      graph_array_value,
   *      graph_object_value
   *   ]
   *
   * @return Expected result for multiple named parameters graph test
   */
  std::string expected_result() {
    std::stringstream result;
    result << BIG_INTEGER_VALUE.str() << ","
           << BOOLEAN_VALUE.str() << ","
           << DOUBLE_VALUE.str() << ","
           << INTEGER_VALUE.str() << ","
           << NULL_VALUE.str() << ","
           << "\"" << STRING_VALUE << "\","
           << as_array_or_named_object(true) << ","
           << as_array_or_named_object(false);
    return result.str();
  }

private:
  /**
   * Flag to determine if the graph workload has been initialized
   */
  static bool is_graph_initialized_;

  /**
   * Generate the member key string from the named parameter
   *
   * @param key Member key for the named parameter
   * @param is_object True if key should be generated; false otherwise
   * @return Generated key or empty string (if not object)
   */
  std::string generate_key(const std::string& key,
    bool is_object) {
    if (is_object) {
      return "\"" + key + "\":";
    }
    return "";
  }

  /**
   * Generate the result string array or named object
   *
   * @param is_array True if array should be generated; false for object
   * @param is_array_requested True if array should be added to the array or
   *                           object; false otherwise
   * @param is_object_requested True if object should be added to the array or
   *                            object; false otherwise
   * @return Generate result string for array or named object
   */
  std::string as_array_or_named_object(bool is_array,
    bool is_array_requested = true,
    bool is_object_requested = true) {
    std::stringstream result;

    result << (is_array ? "[" : "{");
    if (is_array_requested) {
      result << generate_key(GRAPH_ARRAY_NAMED_PARAMETER, !is_array)
             << as_array_or_named_object(true, false, false) << ",";
    }
    result << generate_key(BIG_INTEGER_NAMED_PARAMETER, !is_array)
           << BIG_INTEGER_VALUE.str() << ","
           << generate_key(BOOLEAN_NAMED_PARAMETER, !is_array)
           << BOOLEAN_VALUE.str() << ","
           << generate_key(DOUBLE_NAMED_PARAMETER, !is_array)
           << DOUBLE_VALUE.str() << ","
           << generate_key(INTEGER_NAMED_PARAMETER, !is_array)
           << INTEGER_VALUE.str() << ","
           << generate_key(NULL_NAMED_PARAMETER, !is_array)
           << NULL_VALUE.str() << ",";
    if (is_object_requested) {
      result << generate_key(GRAPH_OBJECT_NAMED_PARAMETER, !is_array)
             << as_array_or_named_object(false, false, false) << ",";
    }
    result << generate_key(STRING_NAMED_PARAMETER, !is_array)
           << "\"" << STRING_VALUE << "\""
           << (is_array ? "]" : "}");

    return result.str();
  }
};

// Initialize static variables
bool GraphIntegrationTest::is_graph_initialized_ = false;

/**
 * Perform simple graph statement execution - Check for existing graph
 *
 * This test will create a graph and execute a graph statement to determine if
 * that graph exists using the graph result set to parse the information. This
 * will also test the use of single named parameters using the DSE graph object.
 *
 * @jira_ticket CPP-352
 * @test_category dse:graph
 * @since 1.0.0
 * @expected_result Graph will be created and existence will be verified
 */
TEST_F(GraphIntegrationTest, GraphExists) {
  CHECK_VERSION(5.0.0);
  CHECK_FAILURE;

  // Create the graph
  create_graph(test_name_);

  // Create the graph statement to see if the default graph exists
  test::driver::DseGraphObject graph_object;
  graph_object.add<std::string>("name", test_name_);
  CHECK_FAILURE;
  test::driver::DseGraphStatement graph_statement("system.graph(name).exists()");
  graph_statement.bind(graph_object);
  CHECK_FAILURE;

  // Execute the graph statement and ensure the graph exists
  test::driver::DseGraphResultSet result_set = dse_session_.execute(graph_statement);
  CHECK_FAILURE;
  ASSERT_EQ(1, result_set.count());
  test::driver::DseGraphResult result = result_set.next();
  ASSERT_EQ(DSE_GRAPH_RESULT_TYPE_BOOL, result.type());
  ASSERT_TRUE(result.is_type<Boolean>());
  ASSERT_EQ(cass_true, result.value<Boolean>().value());
}

/**
 * Perform simple graph statement execution - Check for existing graph
 *
 * This test will create a graph and execute a graph statement to determine if
 * that graph exists using the graph result set to parse the information. This
 * will also test the use of single named parameters using the DSE graph object.
 *
 * @jira_ticket CPP-352
 * @test_category dse:graph
 * @since 1.0.0
 * @expected_result Graph will be created and existence will be verified
 */
TEST_F(GraphIntegrationTest, ServerError) {
  CHECK_VERSION(5.0.0);
  CHECK_FAILURE;

  // Execute the graph statement where a graph is used but does not exist
  test::driver::DseGraphResultSet result_set = dse_session_.execute(
    "system.graph('graph_name_does_not_exist').drop()", NULL, false);
  CHECK_FAILURE;
  ASSERT_EQ(CASS_ERROR_SERVER_INVALID_QUERY, result_set.error_code());
  ASSERT_EQ("Graph graph_name_does_not_exist does not exist",
    result_set.error_message());
}

/**
 * Perform graph statement execution - Multiple named parameters
 *
 * This test will create a graph statement that uses multiple named parameters
 * and validates the assignment using the graph result set to parse the
 * information.
 *
 * @jira_ticket CPP-352
 * @test_category dse:graph
 * @since 1.0.0
 * @expected_result Named parameters will be assigned and validated (textual)
 */
TEST_F(GraphIntegrationTest, MutlipleNamedParameters) {
  CHECK_VERSION(5.0.0);
  CHECK_FAILURE;

  // Create the graph statement (graph does not need to exist; name not required)
  std::stringstream simple_array;
  simple_array << "[" << BIG_INTEGER_NAMED_PARAMETER << ","
    << BOOLEAN_NAMED_PARAMETER << ","
    << DOUBLE_NAMED_PARAMETER << ","
    << INTEGER_NAMED_PARAMETER << ","
    << NULL_NAMED_PARAMETER << ","
    << STRING_NAMED_PARAMETER << ","
    << GRAPH_ARRAY_NAMED_PARAMETER << ","
    << GRAPH_OBJECT_NAMED_PARAMETER << "]";
  test::driver::DseGraphStatement graph_statement(simple_array.str());

  // Create the named parameters and bind the DSE graph object to the statement
  test::driver::DseGraphArray graph_array = create_array();
  CHECK_FAILURE;
  test::driver::DseGraphObject graph_object = create_named_object();
  CHECK_FAILURE;
  test::driver::DseGraphObject graph_named_values = create_named_object(false,
    false);
  CHECK_FAILURE;
  graph_named_values.add<test::driver::DseGraphArray>(GRAPH_ARRAY_NAMED_PARAMETER,
    graph_array);
  CHECK_FAILURE;
  graph_named_values.add<test::driver::DseGraphObject>(GRAPH_OBJECT_NAMED_PARAMETER,
    graph_object);
  CHECK_FAILURE;
  graph_statement.bind(graph_named_values);
  CHECK_FAILURE;

  // Execute the graph statement and validate the results
  test::driver::DseGraphResultSet result_set = dse_session_.execute(graph_statement);
  CHECK_FAILURE;
  std::string expected = "[" + expected_result() + "]";
  ASSERT_EQ(expected, Utils::shorten(result_set.str(), false));
}

/**
 * Perform graph statement execution to retrieve graph edges
 *
 * This test will create a graph, populate that graph with the classic graph
 * structure example and execute a graph statement to retrieve and validate the
 * edges via the graph result set.
 *
 * @jira_ticket CPP-352
 * @test_category dse:graph
 * @since 1.0.0
 * @expected_result Graph edges will be validated from classic example
 */
TEST_F(GraphIntegrationTest, RetrieveEdges) {
  CHECK_VERSION(5.0.0);
  CHECK_FAILURE;

  // Create the graph
  create_graph(test_name_);
  populate_classic_graph(test_name_);

  // Create the graph statement to see who created what
  test::driver::DseGraphOptions graph_options;
  graph_options.set_name(test_name_);
  test::driver::DseGraphStatement graph_statement("g.E().hasLabel('created')",
    graph_options);

  // Execute the graph statement and ensure the edges were retrieved (validate)
  test::driver::DseGraphResultSet result_set = dse_session_.execute(graph_statement);
  CHECK_FAILURE;
  ASSERT_EQ(4, result_set.count());
  for (size_t i = 0; i < 4; ++i) {
    test::driver::DseGraphResult result = result_set.next();
    test::driver::DseGraphEdge edge = result.edge();
    CHECK_FAILURE;

    ASSERT_EQ("created", edge.label().value<std::string>());
    ASSERT_EQ("software", edge.in_vertex_label().value<std::string>());
    ASSERT_EQ("person", edge.out_vertex_label().value<std::string>());
    ASSERT_EQ("edge", edge.type().value<std::string>());
  }
}
/**
 * Perform graph statement execution to retrieve graph vertices
 *
 * This test will create a graph, populate that graph with the classic graph
 * structure example and execute a graph statement to retrieve and validate the
 * vertices via the graph result set.
 *
 * @jira_ticket CPP-352
 * @test_category dse:graph
 * @since 1.0.0
 * @expected_result Graph vertices will be validated from classic example
 */
TEST_F(GraphIntegrationTest, RetrieveVertices) {
  CHECK_VERSION(5.0.0);
  CHECK_FAILURE;

  // Create the graph
  create_graph(test_name_);
  populate_classic_graph(test_name_);

  // Create the graph statement to see who Marko knows
  test::driver::DseGraphOptions graph_options;
  graph_options.set_name(test_name_);
  test::driver::DseGraphStatement graph_statement("g.V().has('name', 'marko').out('knows')",
    graph_options);

  // Execute the graph statement and ensure the vertices were retrieved (validate)
  test::driver::DseGraphResultSet result_set = dse_session_.execute(graph_statement);
  CHECK_FAILURE;
  ASSERT_EQ(2, result_set.count());
  for (size_t i = 0; i < 2; ++i) {
    test::driver::DseGraphResult result = result_set.next();
    test::driver::DseGraphVertex vertex = result.vertex();
    CHECK_FAILURE;

    ASSERT_EQ("person", vertex.label().value<std::string>());
    ASSERT_EQ("vertex", vertex.type().value<std::string>());
  }
}

/**
 * Perform graph statement execution to retrieve graph paths
 *
 * This test will create a graph, populate that graph with the classic graph
 * structure example and execute a graph statement to retrieve and validate the
 * paths via the graph result set.
 *
 * @jira_ticket CPP-352
 * @test_category dse:graph
 * @since 1.0.0
 * @expected_result Graph paths will be validated from classic example
 */
TEST_F(GraphIntegrationTest, RetrievePaths) {
  CHECK_VERSION(5.0.0);
  CHECK_FAILURE;

  // Create the graph
  create_graph(test_name_);
  populate_classic_graph(test_name_);

  /*
   * Create the graph statement to find all path traversals for a person whom
   * Marko knows that has created software and what that software is
   *
   * marko -> knows -> josh -> created -> lop
   * marko -> knows -> josh -> created -> ripple
   */
  test::driver::DseGraphOptions graph_options;
  graph_options.set_name(test_name_);
  test::driver::DseGraphStatement graph_statement("g.V().hasLabel('person')" \
    ".has('name', 'marko').as('a').outE('knows').as('b').inV().as('c', 'd')" \
    ".outE('created').as('e', 'f', 'g').inV().as('h').path()",
    graph_options);

  // Execute the graph statement and ensure the vertices were retrieved (validate)
  test::driver::DseGraphResultSet result_set = dse_session_.execute(graph_statement);
  CHECK_FAILURE;
  ASSERT_EQ(2, result_set.count());
  for (size_t i = 0; i < 2; ++i) {
    test::driver::DseGraphResult result = result_set.next();
    test::driver::DseGraphPath path = result.path();
    CHECK_FAILURE;

    // Ensure the labels are organized as expected
    test::driver::DseGraphResult labels = path.labels();
    ASSERT_EQ(DSE_GRAPH_RESULT_TYPE_ARRAY, labels.type());
    ASSERT_EQ(5, labels.element_count());
    std::string labels_values = Utils::shorten(labels.str(), false);
    labels_values = Utils::replace_all(labels_values, "\"", "");
    ASSERT_EQ("[[a],[b],[c,d],[e,f,g],[h]]", labels_values);

    // Ensure the objects matches what is expected from the paths
    test::driver::DseGraphResult objects = path.objects();
    ASSERT_EQ(5, objects.element_count());
    test::driver::DseGraphVertex marko = objects.element(0).vertex();
    CHECK_FAILURE;
    test::driver::DseGraphEdge knows = objects.element(1).edge();
    CHECK_FAILURE;
    test::driver::DseGraphVertex josh = objects.element(2).vertex();
    CHECK_FAILURE;
    test::driver::DseGraphEdge created = objects.element(3).edge();
    CHECK_FAILURE;
    test::driver::DseGraphVertex software = objects.element(4).vertex();
    CHECK_FAILURE;

    // Validate Marko (vertex)
    ASSERT_EQ("person", marko.label().value<std::string>());
    ASSERT_EQ("vertex", marko.type().value<std::string>());
    test::driver::DseGraphResult marko_properties = marko.properties();
    ASSERT_EQ(2, marko_properties.member_count());
    for (size_t j = 0; j < 2; ++j) {
      test::driver::DseGraphResult property = marko_properties.member(j);
      ASSERT_EQ(DSE_GRAPH_RESULT_TYPE_ARRAY, property.type());
      ASSERT_EQ(1, property.element_count());
      property = property.element(0);
      ASSERT_EQ(DSE_GRAPH_RESULT_TYPE_OBJECT, property.type());
      ASSERT_EQ(2, property.member_count());

      // Ensure the name is "marko" and the age is 29
      bool marko_property_asserted = false;
      if (marko_properties.key(j).compare("name") == 0) {
        for (size_t k = 0; k < 2; ++k) {
          if (property.key(k).compare("value") == 0) {
            ASSERT_EQ("marko", property.member(k).value<std::string>());
            marko_property_asserted = true;
            break;
          }
        }
      } else {
        for (size_t k = 0; k < 2; ++k) {
          if (property.key(k).compare("value") == 0) {
            ASSERT_EQ(29, property.member(k).value<Integer>());
            marko_property_asserted = true;
            break;
          }
        }
      }
      ASSERT_TRUE(marko_property_asserted);
    }

    // Get properties for the created edge to compare with software name
    test::driver::DseGraphResult created_property = created.properties();
    ASSERT_EQ(DSE_GRAPH_RESULT_TYPE_OBJECT, created_property.type());
    ASSERT_EQ(1, created_property.member_count());
    ASSERT_EQ("weight", created_property.key(0));
    created_property = created_property.member(0);
    ASSERT_EQ(DSE_GRAPH_RESULT_TYPE_NUMBER, created_property.type());
    ASSERT_TRUE(created_property.is_type<Double>());
    Double created_weight = created_property.value<Double>();

    // Validate software (should contain different values for each result set)
    test::driver::DseGraphResult software_properties = software.properties();
    ASSERT_EQ(2, software_properties.member_count());
    for (size_t j = 0; j < 2; ++j) {
      test::driver::DseGraphResult property = software_properties.member(j);
      ASSERT_EQ(DSE_GRAPH_RESULT_TYPE_ARRAY, property.type());
      ASSERT_EQ(1, property.element_count());
      property = property.element(0);
      ASSERT_EQ(DSE_GRAPH_RESULT_TYPE_OBJECT, property.type());
      ASSERT_EQ(2, property.member_count());

      // Ensure the software name is "lop" or "ripple" (in that order)
      if (marko_properties.key(j).compare("name") == 0) {
        for (size_t k = 0; k < 2; ++k) {
          if (property.key(k).compare("value") == 0) {
            // Validate the software name and created weight
            if (i == 0) {
              ASSERT_EQ("lop", property.member(k).value<std::string>());
              ASSERT_EQ(0.4, created_weight);
              break;
            } else {
              ASSERT_EQ("ripple", property.member(k).value<std::string>());
              ASSERT_EQ(1.0, created_weight);
              break;
            }
          }
        }
      }
    }
  }
}